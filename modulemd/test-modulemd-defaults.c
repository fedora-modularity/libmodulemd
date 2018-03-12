/* test-modulemd-defaults.c
 *
 * Copyright (C) 2018 Stephen Gallagher
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

#include "modulemd.h"
#include "modulemd-yaml.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>

typedef struct _DefaultsFixture
{
} DefaultsFixture;

static void
modulemd_defaults_test_good_ex1 (DefaultsFixture *fixture,
                                 gconstpointer user_data)
{
  gboolean result = FALSE;
  gchar *yaml_path = NULL;
  GPtrArray *objects = NULL;
  GObject *object = NULL;
  ModulemdDefaults *defaults = NULL;
  GHashTable *profile_defaults = NULL;
  ModulemdSimpleSet *set = NULL;
  GError *error = NULL;
  gchar *yaml_string = NULL;
  const gchar *module_name = "httpd";
  const gchar *default_stream = "2.6";

  yaml_path = g_strdup_printf ("%s/mod-defaults/ex1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  result = parse_yaml_file (yaml_path, &objects, &error);
  g_free (yaml_path);
  g_assert_true (result);

  g_assert_cmpint (objects->len, ==, 1);

  object = g_ptr_array_index (objects, 0);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_DEFAULTS (object));

  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (objects, 0));

  g_assert_cmpint (
    modulemd_defaults_peek_version (defaults), ==, MD_DEFAULTS_VERSION_1);

  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, module_name);

  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, default_stream);

  profile_defaults = modulemd_defaults_peek_profile_defaults (defaults);

  g_assert_true (g_hash_table_contains (profile_defaults, default_stream));

  set = g_hash_table_lookup (profile_defaults, default_stream);
  g_assert_true (modulemd_simpleset_contains (set, "client"));
  g_assert_true (modulemd_simpleset_contains (set, "server"));


  /* Test emitting the YAML back out */
  modulemd_defaults_dumps (defaults, &yaml_string);
  g_assert_nonnull (yaml_string);
  g_debug ("EX1 YAML:\n%s", yaml_string);

  g_clear_pointer (&yaml_string, g_free);
  g_clear_pointer (&objects, g_ptr_array_unref);
}


static void
modulemd_defaults_test_good_ex2 (DefaultsFixture *fixture,
                                 gconstpointer user_data)
{
  gboolean result = FALSE;
  gchar *yaml_path = NULL;
  GPtrArray *objects = NULL;
  GObject *object = NULL;
  ModulemdDefaults *defaults = NULL;
  GHashTable *profile_defaults = NULL;
  ModulemdSimpleSet *set = NULL;
  GError *error = NULL;
  gchar *yaml_string = NULL;
  const gchar *module_name = "postgresql";
  const gchar *default_stream = "8.0";

  yaml_path = g_strdup_printf ("%s/mod-defaults/ex2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  result = parse_yaml_file (yaml_path, &objects, &error);
  g_free (yaml_path);
  g_assert_true (result);

  g_assert_cmpint (objects->len, ==, 2);


  /* First of the two subdocuments */
  object = g_ptr_array_index (objects, 0);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_DEFAULTS (object));

  defaults = MODULEMD_DEFAULTS (object);

  g_assert_cmpint (
    modulemd_defaults_peek_version (defaults), ==, MD_DEFAULTS_VERSION_1);

  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, module_name);

  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, default_stream);

  profile_defaults = modulemd_defaults_peek_profile_defaults (defaults);

  g_assert_true (g_hash_table_contains (profile_defaults, default_stream));

  set = g_hash_table_lookup (profile_defaults, default_stream);
  g_assert_true (modulemd_simpleset_contains (set, "server"));

  /* Test emitting the YAML back out */
  modulemd_defaults_dumps (defaults, &yaml_string);
  g_assert_nonnull (yaml_string);
  g_debug ("EX1 YAML:\n%s", yaml_string);

  g_clear_pointer (&yaml_string, g_free);


  /* Second of the two subdocuments */
  module_name = "nodejs";
  default_stream = "6.0";

  object = g_ptr_array_index (objects, 1);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_DEFAULTS (object));

  defaults = MODULEMD_DEFAULTS (object);

  g_assert_cmpint (
    modulemd_defaults_peek_version (defaults), ==, MD_DEFAULTS_VERSION_1);

  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, module_name);

  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, default_stream);

  profile_defaults = modulemd_defaults_peek_profile_defaults (defaults);

  g_assert_true (g_hash_table_contains (profile_defaults, default_stream));

  set = g_hash_table_lookup (profile_defaults, default_stream);
  g_assert_true (modulemd_simpleset_contains (set, "default"));

  /* Test emitting the YAML back out */
  modulemd_defaults_dumps (defaults, &yaml_string);
  g_assert_nonnull (yaml_string);
  g_debug ("EX1 YAML:\n%s", yaml_string);

  g_clear_pointer (&yaml_string, g_free);
  g_clear_pointer (&objects, g_ptr_array_unref);
}


static void
modulemd_defaults_test_good_ex3 (DefaultsFixture *fixture,
                                 gconstpointer user_data)
{
  gboolean result = FALSE;
  gchar *yaml_path = NULL;
  GPtrArray *objects = NULL;
  GObject *object = NULL;
  ModulemdDefaults *defaults = NULL;
  GHashTable *profile_defaults = NULL;
  ModulemdSimpleSet *set = NULL;
  GError *error = NULL;
  gchar *yaml_string = NULL;
  const gchar *module_name = "httpd";
  const gchar *default_stream = "2.2";

  yaml_path = g_strdup_printf ("%s/mod-defaults/ex3.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  result = parse_yaml_file (yaml_path, &objects, &error);
  g_free (yaml_path);
  g_assert_true (result);

  g_assert_cmpint (objects->len, ==, 3);

  /* First of the three subdocuments */
  object = g_ptr_array_index (objects, 0);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_DEFAULTS (object));

  defaults = MODULEMD_DEFAULTS (object);

  g_assert_cmpint (
    modulemd_defaults_peek_version (defaults), ==, MD_DEFAULTS_VERSION_1);

  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, module_name);

  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, default_stream);

  profile_defaults = modulemd_defaults_peek_profile_defaults (defaults);

  g_assert_true (g_hash_table_contains (profile_defaults, default_stream));

  set = g_hash_table_lookup (profile_defaults, default_stream);
  g_assert_true (modulemd_simpleset_contains (set, "client"));
  g_assert_true (modulemd_simpleset_contains (set, "server"));

  /* Test emitting the YAML back out */
  modulemd_defaults_dumps (defaults, &yaml_string);
  g_assert_nonnull (yaml_string);
  g_debug ("EX1 YAML:\n%s", yaml_string);

  g_clear_pointer (&yaml_string, g_free);


  /* Second of the three subdocuments */
  module_name = "postgresql";
  default_stream = "8.1";

  object = g_ptr_array_index (objects, 1);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_DEFAULTS (object));

  defaults = MODULEMD_DEFAULTS (object);

  g_assert_cmpint (
    modulemd_defaults_peek_version (defaults), ==, MD_DEFAULTS_VERSION_1);

  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, module_name);

  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, default_stream);

  profile_defaults = modulemd_defaults_peek_profile_defaults (defaults);

  g_assert_true (g_hash_table_contains (profile_defaults, default_stream));

  set = g_hash_table_lookup (profile_defaults, default_stream);
  g_assert_true (modulemd_simpleset_contains (set, "client"));
  g_assert_true (modulemd_simpleset_contains (set, "server"));
  g_assert_true (modulemd_simpleset_contains (set, "foo"));

  /* Test emitting the YAML back out */
  modulemd_defaults_dumps (defaults, &yaml_string);
  g_assert_nonnull (yaml_string);
  g_debug ("EX1 YAML:\n%s", yaml_string);

  g_clear_pointer (&yaml_string, g_free);


  /* Third of the three subdocuments */
  module_name = "nodejs";
  default_stream = "8.0";

  object = g_ptr_array_index (objects, 2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_DEFAULTS (object));

  defaults = MODULEMD_DEFAULTS (object);

  g_assert_cmpint (
    modulemd_defaults_peek_version (defaults), ==, MD_DEFAULTS_VERSION_1);

  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, module_name);

  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, default_stream);

  profile_defaults = modulemd_defaults_peek_profile_defaults (defaults);

  g_assert_true (g_hash_table_contains (profile_defaults, default_stream));

  set = g_hash_table_lookup (profile_defaults, default_stream);
  g_assert_true (modulemd_simpleset_contains (set, "super"));

  set = g_hash_table_lookup (profile_defaults, "6.0");
  g_assert_true (modulemd_simpleset_contains (set, "default"));

  /* Test emitting the YAML back out */
  modulemd_defaults_dumps (defaults, &yaml_string);
  g_assert_nonnull (yaml_string);
  g_debug ("EX1 YAML:\n%s", yaml_string);

  g_clear_pointer (&yaml_string, g_free);
  g_clear_pointer (&objects, g_ptr_array_unref);
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");


  g_test_add ("/modulemd/defaults/modulemd_defaults_test_good_examples/ex1",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_defaults_test_good_ex1,
              NULL);

  g_test_add ("/modulemd/defaults/modulemd_defaults_test_good_examples/ex2",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_defaults_test_good_ex2,
              NULL);

  g_test_add ("/modulemd/defaults/modulemd_defaults_test_good_examples/ex3",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_defaults_test_good_ex3,
              NULL);


  return g_test_run ();
};
