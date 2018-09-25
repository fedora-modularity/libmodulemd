/*
 * This file is part of libmodulemd
 * Copyright (C) 2017-2018 Stephen Gallagher
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */
#define MMD_DISABLE_DEPRECATION_WARNINGS 1
#include "modulemd.h"
#include "private/modulemd-yaml.h"
#include "private/modulemd-util.h"

#include <glib.h>
#include <glib/gstdio.h>
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
modulemd_yaml_test_parse_v1_file (YamlFixture *fixture,
                                  gconstpointer user_data)
{
  gboolean result;
  GError *error = NULL;
  ModulemdModule **modules = NULL;
  GPtrArray *data = NULL;
  ModulemdSimpleSet *set = NULL;
  gchar *yaml_path = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/good-v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  result = parse_yaml_file (yaml_path, &data, NULL, &error);
  g_clear_pointer (&yaml_path, g_free);
  g_assert_true (result);

  modules = mmd_yaml_dup_modules (data);
  if (!modules)
    {
      fprintf (stderr, "Failed to parse: %s\n", error->message);
      g_clear_pointer (&error, g_error_free);
    }

  g_assert_nonnull (modules);
  g_assert_nonnull (modules[0]);
  g_assert_null (modules[1]);
  g_assert_cmpuint (modulemd_module_get_mdversion (modules[0]), ==, 1);
  g_assert_cmpstr (modulemd_module_get_name (modules[0]), ==, "foo");
  g_assert_cmpstr (modulemd_module_get_stream (modules[0]), ==, "stream-name");
  g_assert_cmpstr (
    modulemd_module_get_summary (modules[0]), ==, "An example module");
  set = modulemd_module_get_rpm_artifacts (modules[0]);
  g_assert_true (set);
  g_assert_true (
    modulemd_simpleset_contains (set, "bar-0:1.23-1.module_deadbeef.x86_64"));

  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }
  g_clear_pointer (&modules, g_free);
  g_clear_pointer (&data, g_ptr_array_unref);


  yaml_path = g_strdup_printf ("%s/test_data/bad-document.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  result = parse_yaml_file (yaml_path, &data, NULL, &error);
  g_clear_pointer (&yaml_path, g_free);
  g_assert_true (result);
  g_assert_null (error);
  g_assert_nonnull (data);
  g_assert_cmpuint (data->len, ==, 0);

  g_clear_pointer (&data, g_ptr_array_unref);

  /* Validate the official reference YAML */
  g_info ("Reference YAML v1");
  yaml_path =
    g_strdup_printf ("%s/spec.v1.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  result = parse_yaml_file (yaml_path, &data, NULL, &error);
  g_free (yaml_path);
  g_assert_true (result);

  modules = mmd_yaml_dup_modules (data);
  g_assert_nonnull (modules);

  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }
  g_clear_pointer (&modules, g_free);
  g_clear_pointer (&data, g_ptr_array_unref);
}

static void
modulemd_yaml_test_v1_load (YamlFixture *fixture, gconstpointer user_data)
{
  ModulemdModule *module = NULL;
  ModulemdModule *copy = NULL;
  ModulemdModule **modules = NULL;
  gchar *yaml_path = NULL;
  GHashTable *buildrequires = NULL;
  gchar *value = NULL;


  yaml_path = g_strdup_printf ("%s/test_data/good-v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  module = modulemd_module_new_from_file (yaml_path);
  g_assert_true (module);

  buildrequires = modulemd_module_get_buildrequires (module);
  g_assert_true (buildrequires);

  value = g_hash_table_lookup (buildrequires, "platform");
  g_assert_cmpstr (value, ==, "and-its-stream-name");

  /* Copy this module */
  copy = modulemd_module_copy (module);
  g_assert_nonnull (copy);
  g_assert_cmpuint (modulemd_module_peek_mdversion (copy), ==, 1);
  g_object_unref (copy);
  g_object_unref (module);

  modulemd_module_new_all_from_file (yaml_path, &modules);
  g_clear_pointer (&yaml_path, g_free);

  g_assert_true (modules);
  g_assert_true (modules[0]);

  buildrequires = modulemd_module_get_buildrequires (modules[0]);
  g_assert_true (buildrequires);

  value = g_hash_table_lookup (buildrequires, "platform");
  g_assert_cmpstr (value, ==, "and-its-stream-name");

  /* Copy this module */
  copy = modulemd_module_copy (modules[0]);
  g_assert_nonnull (copy);
  g_assert_cmpuint (modulemd_module_peek_mdversion (copy), ==, 1);

  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }
  g_free (modules);
  g_clear_pointer (&copy, g_object_unref);
}

static void
modulemd_yaml_test_v2_load (YamlFixture *fixture, gconstpointer user_data)
{
  gboolean result;
  ModulemdModule *module = NULL;
  ModulemdModule *copy = NULL;
  ModulemdModule **modules = NULL;
  GPtrArray *data = NULL;
  gchar *yaml_path = NULL;
  GError *error = NULL;


  yaml_path = g_strdup_printf ("%s/test_data/good-v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  module = modulemd_module_new_from_file (yaml_path);
  g_assert_true (module);
  g_object_unref (module);

  modulemd_module_new_all_from_file (yaml_path, &modules);

  g_assert_nonnull (modules);
  g_assert_nonnull (modules[0]);
  g_assert_nonnull (modules[1]);
  g_assert_nonnull (modules[2]);
  g_assert_null (modules[3]);

  /* Copy this module */
  copy = modulemd_module_copy (modules[0]);
  g_assert_nonnull (copy);
  g_assert_cmpuint (modulemd_module_peek_mdversion (copy), ==, 2);

  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }

  g_clear_pointer (&modules, g_free);
  g_free (yaml_path);

  yaml_path = g_strdup_printf ("%s/test_data/mixed-v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  modulemd_module_new_all_from_file (yaml_path, &modules);

  g_assert_nonnull (modules);
  g_assert_nonnull (modules[0]);
  g_assert_nonnull (modules[1]);
  g_assert_null (modules[2]);

  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }

  g_free (modules);
  g_free (yaml_path);


  /* Validate the official reference YAML */
  g_info ("Reference YAML v2");
  yaml_path =
    g_strdup_printf ("%s/spec.v2.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  result = parse_yaml_file (yaml_path, &data, NULL, &error);
  g_free (yaml_path);
  g_assert_true (result);

  modules = mmd_yaml_dup_modules (data);
  g_assert_nonnull (modules);
  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }

  g_free (modules);
  g_clear_pointer (&data, g_ptr_array_unref);
  g_clear_pointer (&copy, g_object_unref);
}

static void
modulemd_yaml_test_emit_v1_string (YamlFixture *fixture,
                                   gconstpointer user_data)
{
  gchar *yaml = NULL;
  gchar *yaml2 = NULL;
  gboolean result;
  GError *error = NULL;
  GPtrArray *modules = NULL;
  GPtrArray *reloaded_modules = NULL;
  gchar *yaml_path = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/good-v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  modulemd_module_new_all_from_file_ext (yaml_path, &modules);
  g_clear_pointer (&yaml_path, g_free);

  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_true (result);
  g_assert_true (yaml);
  g_debug ("YAML:\n%s", yaml);

  /* Load this string and emit it again. It must produce the same output. */
  modulemd_module_new_all_from_string_ext (yaml, &reloaded_modules);
  result = emit_yaml_string (modules, &yaml2, &error);
  g_assert_true (result);
  g_assert_true (yaml2);
  g_assert_cmpstr (yaml, ==, yaml2);


  g_clear_pointer (&yaml, g_free);
  g_clear_pointer (&yaml2, g_free);
  g_ptr_array_unref (modules);
  g_ptr_array_unref (reloaded_modules);
}

static void
modulemd_yaml_test_emit_v2_string (YamlFixture *fixture,
                                   gconstpointer user_data)
{
  gchar *yaml = NULL;
  gchar *yaml2 = NULL;
  gboolean result;
  GError *error = NULL;
  GPtrArray *modules = NULL;
  GPtrArray *reloaded_modules = NULL;
  gchar *yaml_path = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/good-v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  modulemd_module_new_all_from_file_ext (yaml_path, &modules);
  g_clear_pointer (&yaml_path, g_free);

  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_true (result);
  g_assert_nonnull (yaml);
  g_debug ("YAML:\n%s", yaml);

  /* Emit the same string again to confirm that we haven't screwed up any of
   * the memory management.
   */
  result = emit_yaml_string (modules, &yaml2, &error);
  g_assert_true (result);
  g_assert_nonnull (yaml2);
  g_assert_cmpstr (yaml, ==, yaml2);
  g_clear_pointer (&yaml2, g_free);

  /* Load this string and emit it again. It must produce the same output. */
  modulemd_module_new_all_from_string_ext (yaml, &reloaded_modules);
  result = emit_yaml_string (reloaded_modules, &yaml2, &error);
  g_assert_true (result);
  g_assert_nonnull (yaml2);
  g_assert_cmpstr (yaml, ==, yaml2);

  g_clear_pointer (&yaml, g_free);
  g_clear_pointer (&yaml2, g_free);
  g_ptr_array_unref (modules);
  g_ptr_array_unref (reloaded_modules);
}


static void
modulemd_yaml_test_v2_stream (YamlFixture *fixture, gconstpointer user_data)
{
  gboolean result;
  ModulemdModule *module = NULL;
  ModulemdModule *copy = NULL;
  ModulemdModule **modules = NULL;
  GPtrArray *data = NULL;
  gchar *yaml_path = NULL;
  GError *error = NULL;
  FILE *stream = NULL;


  yaml_path = g_strdup_printf ("%s/test_data/good-v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  stream = g_fopen (yaml_path, "rb");
  g_assert_nonnull (stream);

  module = modulemd_module_new_from_stream (stream, &error);
  g_assert_true (module);
  g_assert_null (error);
  g_object_unref (module);

  modulemd_module_new_all_from_file (yaml_path, &modules);

  g_assert_nonnull (modules);
  g_assert_nonnull (modules[0]);
  g_assert_nonnull (modules[1]);
  g_assert_nonnull (modules[2]);
  g_assert_null (modules[3]);

  /* Copy this module */
  copy = modulemd_module_copy (modules[0]);
  g_assert_nonnull (copy);
  g_assert_cmpuint (modulemd_module_peek_mdversion (copy), ==, 2);

  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }

  g_clear_pointer (&modules, g_free);
  g_free (yaml_path);

  yaml_path = g_strdup_printf ("%s/test_data/mixed-v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  modulemd_module_new_all_from_file (yaml_path, &modules);

  g_assert_nonnull (modules);
  g_assert_nonnull (modules[0]);
  g_assert_nonnull (modules[1]);
  g_assert_null (modules[2]);

  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }

  g_free (modules);
  g_free (yaml_path);


  /* Validate the official reference YAML */
  g_info ("Reference YAML v2");
  yaml_path =
    g_strdup_printf ("%s/spec.v2.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  result = parse_yaml_file (yaml_path, &data, NULL, &error);
  g_free (yaml_path);
  g_assert_true (result);

  modules = mmd_yaml_dup_modules (data);
  g_assert_nonnull (modules);
  for (gsize i = 0; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }

  g_free (modules);
  g_clear_pointer (&data, g_ptr_array_unref);
  g_clear_pointer (&copy, g_object_unref);
}


static void
modulemd_yaml_test_validate_nevra (YamlFixture *fixture,
                                   gconstpointer user_data)
{
  const gchar *good = "nodejs-devel-1:8.10.0-3.module_1572+d7ec111e.x86_64";
  const gchar *garbage = "DEADBEEF";
  const gchar *garbage2 = "DEAD.BEEF";
  const gchar *garbage3 = "MORE-DEAD.BEEF";
  const gchar *missing_epoch =
    "nodejs-devel-8.10.0-3.module_1572+d7ec111e.x86_64";
  const gchar *nonint_epoch =
    "nodejs-devel-FOO:8.10.0-3.module_1572+d7ec111e.x86_64";

  g_assert_true (modulemd_validate_nevra (good));
  g_assert_false (modulemd_validate_nevra (garbage));
  g_assert_false (modulemd_validate_nevra (garbage2));
  g_assert_false (modulemd_validate_nevra (garbage3));
  g_assert_false (modulemd_validate_nevra (missing_epoch));
  g_assert_false (modulemd_validate_nevra (nonint_epoch));
}


static void
modulemd_yaml_test_artifact_validation (YamlFixture *fixture,
                                        gconstpointer user_data)
{
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) objects = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;

  /* Attempt to read in a modulemd with a data.artifacts.rpm section
   * containing values without the Epoch included.
   */
  yaml_path = g_strdup_printf ("%s/test_data/issue46.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  objects = modulemd_objects_from_file_ext (yaml_path, &failures, &error);
  g_assert_nonnull (objects);
  g_assert_cmpint (objects->len, ==, 0);
  g_assert_null (error);
  g_assert_nonnull (failures);
  g_assert_cmpint (failures->len, ==, 1);
  g_assert_true (g_str_has_prefix (
    modulemd_subdocument_get_gerror (g_ptr_array_index (failures, 0))->message,
    "RPM artifacts not in NEVRA format ["));
}


static void
modulemd_yaml_test_index_from_file (YamlFixture *fixture,
                                    gconstpointer user_data)
{
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GHashTable) module_index = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  ModulemdImprovedModule *module = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/long-valid.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  module_index = parse_module_index_from_file (yaml_path, &failures, &error);
  g_assert_nonnull (module_index);

  g_assert_true (g_hash_table_contains (module_index, "nodejs"));

  module = g_hash_table_lookup (module_index, "nodejs");
  g_assert_nonnull (module);
  g_assert_true (MODULEMD_IS_IMPROVEDMODULE (module));

  g_assert_cmpstr (modulemd_improvedmodule_peek_name (module), ==, "nodejs");
  g_assert_nonnull (modulemd_improvedmodule_peek_defaults (module));
}


static void
modulemd_yaml_test_index_from_string (YamlFixture *fixture,
                                      gconstpointer user_data)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GHashTable) module_index = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  ModulemdImprovedModule *module = NULL;

  yaml_string =
    "document: modulemd\nversion: 2\ndata:\n    name: Foo\n    summary: Foo\n "
    "   "
    "description: >\n        Bar\n    license:\n        module:\n        - "
    "MIT";

  module_index =
    parse_module_index_from_string (yaml_string, &failures, &error);
  g_assert_nonnull (module_index);

  g_assert_true (g_hash_table_contains (module_index, "Foo"));

  module = g_hash_table_lookup (module_index, "Foo");
  g_assert_nonnull (module);
  g_assert_true (MODULEMD_IS_IMPROVEDMODULE (module));

  g_assert_cmpstr (modulemd_improvedmodule_peek_name (module), ==, "Foo");
  g_assert_null (modulemd_improvedmodule_peek_defaults (module));
}


static void
modulemd_yaml_test_index_from_stream (YamlFixture *fixture,
                                      gconstpointer user_data)
{
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GHashTable) module_index = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  ModulemdImprovedModule *module = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/long-valid.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rb");
  g_assert_nonnull (yaml_stream);

  module_index =
    parse_module_index_from_stream (yaml_stream, &failures, &error);
  g_assert_nonnull (module_index);

  g_assert_true (g_hash_table_contains (module_index, "nodejs"));

  module = g_hash_table_lookup (module_index, "nodejs");
  g_assert_nonnull (module);
  g_assert_true (MODULEMD_IS_IMPROVEDMODULE (module));

  g_assert_cmpstr (modulemd_improvedmodule_peek_name (module), ==, "nodejs");
  g_assert_nonnull (modulemd_improvedmodule_peek_defaults (module));
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/yaml/test_parse_v1_file",
              YamlFixture,
              NULL,
              modulemd_yaml_set_up,
              modulemd_yaml_test_parse_v1_file,
              modulemd_yaml_tear_down);

  g_test_add ("/modulemd/yaml/test_emit_v1_string",
              YamlFixture,
              NULL,
              modulemd_yaml_set_up,
              modulemd_yaml_test_emit_v1_string,
              modulemd_yaml_tear_down);

  g_test_add ("/modulemd/yaml/test_v1_load",
              YamlFixture,
              NULL,
              modulemd_yaml_set_up,
              modulemd_yaml_test_v1_load,
              modulemd_yaml_tear_down);

  g_test_add ("/modulemd/yaml/test_v2_load",
              YamlFixture,
              NULL,
              modulemd_yaml_set_up,
              modulemd_yaml_test_v2_load,
              modulemd_yaml_tear_down);

  g_test_add ("/modulemd/yaml/test_emit_v2_string",
              YamlFixture,
              NULL,
              modulemd_yaml_set_up,
              modulemd_yaml_test_emit_v2_string,
              modulemd_yaml_tear_down);

  g_test_add ("/modulemd/yaml/test_v2_stream",
              YamlFixture,
              NULL,
              modulemd_yaml_set_up,
              modulemd_yaml_test_v2_stream,
              modulemd_yaml_tear_down);

  g_test_add ("/modulemd/yaml/test_validate_nevra",
              YamlFixture,
              NULL,
              NULL,
              modulemd_yaml_test_validate_nevra,
              NULL);

  g_test_add ("/modulemd/yaml/test_artifact_validation",
              YamlFixture,
              NULL,
              NULL,
              modulemd_yaml_test_artifact_validation,
              NULL);

  g_test_add ("/modulemd/yaml/test_index_from_file",
              YamlFixture,
              NULL,
              NULL,
              modulemd_yaml_test_index_from_file,
              NULL);

  g_test_add ("/modulemd/yaml/test_index_from_string",
              YamlFixture,
              NULL,
              NULL,
              modulemd_yaml_test_index_from_string,
              NULL);

  g_test_add ("/modulemd/yaml/test_index_from_stream",
              YamlFixture,
              NULL,
              NULL,
              modulemd_yaml_test_index_from_stream,
              NULL);

  return g_test_run ();
}
