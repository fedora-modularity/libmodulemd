/* test-modulemd-simpleset.c
 *
 * Copyright (C) 2017 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "modulemd-yaml.h"

#include <glib.h>
#include <locale.h>

typedef struct _YamlFixture
{
} YamlFixture;

static void
modulemd_yaml_set_up (YamlFixture *fixture, gconstpointer user_data)
{
}

static void
modulemd_yaml_tear_down (YamlFixture *fixture, gconstpointer user_data)
{
}

static void
modulemd_yaml_test_parse_file (YamlFixture *fixture, gconstpointer user_data)
{
  GError *error = NULL;
  ModulemdModule **modules;
  ModulemdSimpleSet *set = NULL;

  modules = parse_yaml_file ("../test_data/good.yaml", &error);
  g_assert_true (modules);
  g_assert_true (modules[0]);
  g_assert_false (modules[1]);
  g_assert_true (error == NULL);
  g_assert_cmpuint (modulemd_module_get_mdversion (modules[0]), ==, 1);
  g_assert_cmpstr (modulemd_module_get_name (modules[0]), ==, "foo");
  g_assert_cmpstr (modulemd_module_get_stream (modules[0]), ==, "stream-name");
  g_assert_cmpstr (
    modulemd_module_get_summary (modules[0]), ==, "An example module");
  set = modulemd_module_get_rpm_artifacts (modules[0]);
  g_assert_true (set);
  g_assert_true (
    modulemd_simpleset_contains (set, "bar-0:1.23-1.module_deadbeef.x86_64"));

  modules = parse_yaml_file ("../test_data/bad-document.yaml", &error);
  g_assert_false (modules);
  g_assert_true (error);
  g_assert_cmpstr (error->message, ==, "Unknown document type");
}

static void
modulemd_yaml_test_load (YamlFixture *fixture, gconstpointer user_data)
{
  ModulemdModule *module = NULL;
  ModulemdModule **modules = NULL;
  GHashTable *buildrequires = NULL;
  gchar *value = NULL;

  module = modulemd_module_new_from_file ("../test_data/good.yaml");

  g_assert_true (module);

  buildrequires = modulemd_module_get_buildrequires (module);
  g_assert_true (buildrequires);

  value = g_hash_table_lookup (buildrequires, "platform");
  g_assert_cmpstr (value, ==, "and-its-stream-name");

  g_hash_table_unref (buildrequires);
  g_object_unref (module);

  modulemd_module_new_all_from_file ("../test_data/good.yaml", &modules);

  g_assert_true (modules);
  g_assert_true (modules[0]);

  buildrequires = modulemd_module_get_buildrequires (modules[0]);
  g_assert_true (buildrequires);

  value = g_hash_table_lookup (buildrequires, "platform");
  g_assert_cmpstr (value, ==, "and-its-stream-name");
  g_hash_table_unref (buildrequires);
  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }
  g_free (modules);
}

static void
modulemd_yaml_test_emit_string (YamlFixture *fixture, gconstpointer user_data)
{
  gchar *yaml;
  gboolean result;
  GError *error = NULL;
  ModulemdModule **modules;

  modulemd_module_new_all_from_file ("../test_data/good.yaml", &modules);

  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_true (result);
  g_assert_true (yaml);
  g_message ("YAML:\n%s", yaml);
  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }
  g_free (modules);
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/yaml/test_parse_file",
              YamlFixture,
              NULL,
              modulemd_yaml_set_up,
              modulemd_yaml_test_parse_file,
              modulemd_yaml_tear_down);

  g_test_add ("/modulemd/yaml/test_emit_string",
              YamlFixture,
              NULL,
              modulemd_yaml_set_up,
              modulemd_yaml_test_emit_string,
              modulemd_yaml_tear_down);

  g_test_add ("/modulemd/yaml/test_load",
              YamlFixture,
              NULL,
              modulemd_yaml_set_up,
              modulemd_yaml_test_load,
              modulemd_yaml_tear_down);
  return g_test_run ();
}
