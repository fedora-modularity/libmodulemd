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

#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <inttypes.h>
#include <time.h>

typedef struct _DefaultsFixture
{
} DefaultsFixture;

static void
modulemd_defaults_test_good_ex1 (DefaultsFixture *fixture,
                                 gconstpointer user_data)
{
  g_autofree gchar *yaml_path = NULL;
  GPtrArray *objects = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;
  g_autoptr (ModulemdDefaults) defaults_reread = NULL;
  GHashTable *profile_defaults = NULL;
  ModulemdSimpleSet *set = NULL;
  GError *error = NULL;
  gchar *yaml_string = NULL;
  const gchar *module_name = "httpd";
  const gchar *default_stream = "2.6";

  yaml_path = g_strdup_printf ("%s/mod-defaults/ex1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  defaults = modulemd_defaults_new_from_file (yaml_path, &error);
  g_assert_nonnull (defaults);
  g_assert_null (error);

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

  /* Read the YAML back in from the string to ensure the output was valid */
  defaults_reread = modulemd_defaults_new_from_string (yaml_string, &error);
  g_assert_nonnull (defaults_reread);
  g_clear_pointer (&defaults_reread, g_object_unref);

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
  g_autoptr (ModulemdDefaults) defaults_reread = NULL;
  GHashTable *profile_defaults = NULL;
  ModulemdSimpleSet *set = NULL;
  GError *error = NULL;
  gchar *yaml_string = NULL;
  const gchar *module_name = "postgresql";
  const gchar *default_stream = "8.0";

  yaml_path = g_strdup_printf ("%s/mod-defaults/ex2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  result = parse_yaml_file (yaml_path, &objects, NULL, &error);
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

  /* Read the YAML back in from the string to ensure the output was valid */
  defaults_reread = modulemd_defaults_new_from_string (yaml_string, &error);
  g_assert_nonnull (defaults_reread);
  g_clear_pointer (&defaults_reread, g_object_unref);

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

  /* Read the YAML back in from the string to ensure the output was valid */
  defaults_reread = modulemd_defaults_new_from_string (yaml_string, &error);
  g_assert_nonnull (defaults_reread);
  g_clear_pointer (&defaults_reread, g_object_unref);

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
  g_autoptr (ModulemdDefaults) defaults_reread = NULL;
  GHashTable *profile_defaults = NULL;
  ModulemdSimpleSet *set = NULL;
  GError *error = NULL;
  gchar *yaml_string = NULL;
  const gchar *module_name = "httpd";
  const gchar *default_stream = "2.2";

  yaml_path = g_strdup_printf ("%s/mod-defaults/ex3.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  result = parse_yaml_file (yaml_path, &objects, NULL, &error);
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

  /* Read the YAML back in from the string to ensure the output was valid */
  defaults_reread = modulemd_defaults_new_from_string (yaml_string, &error);
  g_assert_nonnull (defaults_reread);
  g_clear_pointer (&defaults_reread, g_object_unref);

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

  /* Read the YAML back in from the string to ensure the output was valid */
  defaults_reread = modulemd_defaults_new_from_string (yaml_string, &error);
  g_assert_nonnull (defaults_reread);
  g_clear_pointer (&defaults_reread, g_object_unref);

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

  /* Read the YAML back in from the string to ensure the output was valid */
  defaults_reread = modulemd_defaults_new_from_string (yaml_string, &error);
  g_assert_nonnull (defaults_reread);
  g_clear_pointer (&defaults_reread, g_object_unref);

  g_clear_pointer (&yaml_string, g_free);
  g_clear_pointer (&objects, g_ptr_array_unref);
}


static void
modulemd_defaults_test_good_ex4 (DefaultsFixture *fixture,
                                 gconstpointer user_data)
{
  gboolean result = FALSE;
  gchar *yaml_path = NULL;
  GPtrArray *objects = NULL;
  GObject *object = NULL;
  ModulemdDefaults *defaults = NULL;
  g_autoptr (ModulemdDefaults) defaults_reread = NULL;
  GHashTable *profile_defaults = NULL;
  ModulemdSimpleSet *set = NULL;
  GError *error = NULL;
  gchar *yaml_string = NULL;
  const gchar *module_name = "httpd";
  const gchar *default_stream = NULL;

  yaml_path = g_strdup_printf ("%s/mod-defaults/ex4.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  result = parse_yaml_file (yaml_path, &objects, NULL, &error);
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
  g_assert_nonnull (profile_defaults);

  set = g_hash_table_lookup (profile_defaults, "2.6");
  g_assert_nonnull (set);
  g_assert_true (modulemd_simpleset_contains (set, "client"));
  g_assert_true (modulemd_simpleset_contains (set, "server"));


  /* Test emitting the YAML back out */
  modulemd_defaults_dumps (defaults, &yaml_string);
  g_assert_nonnull (yaml_string);
  g_debug ("EX1 YAML:\n%s", yaml_string);

  /* Read the YAML back in from the string to ensure the output was valid */
  defaults_reread = modulemd_defaults_new_from_string (yaml_string, &error);
  g_assert_nonnull (defaults_reread);
  g_clear_pointer (&defaults_reread, g_object_unref);

  g_clear_pointer (&yaml_string, g_free);
  g_clear_pointer (&objects, g_ptr_array_unref);
}


static void
modulemd_defaults_test_copy (DefaultsFixture *fixture, gconstpointer user_data)
{
  g_autofree gchar *yaml_path = NULL;
  GError *error = NULL;
  g_autoptr (ModulemdDefaults) orig = NULL;
  g_autoptr (ModulemdDefaults) copy = NULL;

  yaml_path = g_strdup_printf ("%s/mod-defaults/spec.v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);
  orig = modulemd_defaults_new_from_file (yaml_path, &error);
  g_assert_nonnull (orig);

  copy = modulemd_defaults_copy (orig);
  g_assert_nonnull (copy);

  g_assert_cmpstr (modulemd_defaults_peek_module_name (orig),
                   ==,
                   modulemd_defaults_peek_module_name (copy));

  g_assert_cmpstr (modulemd_defaults_peek_default_stream (orig),
                   ==,
                   modulemd_defaults_peek_default_stream (copy));

  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (orig)),
    ==,
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (copy)));

  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_intents (orig)), ==, 2);
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_intents (copy)), ==, 2);

  g_assert_true (
    g_hash_table_contains (modulemd_defaults_peek_intents (orig), "desktop"));
  g_assert_true (
    g_hash_table_contains (modulemd_defaults_peek_intents (copy), "desktop"));

  g_assert_true (
    g_hash_table_contains (modulemd_defaults_peek_intents (orig), "server"));
  g_assert_true (
    g_hash_table_contains (modulemd_defaults_peek_intents (copy), "server"));
}


static void
modulemd_defaults_test_merging (DefaultsFixture *fixture,
                                gconstpointer user_data)
{
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) objects = NULL;
  g_autoptr (GPtrArray) override_objects = NULL;
  g_autoptr (GPtrArray) merged_base = NULL;
  g_autoptr (GPtrArray) overridden = NULL;
  ModulemdDefaults *defaults = NULL;
  GError *error = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/defaults/merging-base.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  objects = modulemd_objects_from_file (yaml_path, &error);
  g_clear_pointer (&yaml_path, g_free);
  g_assert_nonnull (objects);
  g_assert_cmpint (objects->len, ==, 7);

  merged_base = modulemd_merge_defaults (objects, NULL, FALSE, &error);
  if (!merged_base)
    {
      fprintf (stderr, "Error merging defaults: %s\n", error->message);
    }
  g_assert_nonnull (merged_base);
  g_assert_cmpint (merged_base->len, ==, 3);

  /* They should be in alphabetical order now */

  /* HTTPD */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_base, 0));
  g_assert_cmpstr (modulemd_defaults_peek_module_name (defaults), ==, "httpd");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "2.2");
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (defaults)),
    ==,
    2);
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "2.2"));
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "2.8"));

  /* NODEJS */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_base, 1));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "nodejs");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "8.0");
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (defaults)),
    ==,
    3);
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "6.0"));
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "8.0"));
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "9.0"));

  /* POSTGRESQL */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_base, 2));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "postgresql");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "8.1");
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (defaults)),
    ==,
    2);
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "8.1"));
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "8.2"));


  /* Now test overriding with a higher-priority repo */

  yaml_path = g_strdup_printf ("%s/test_data/defaults/overriding.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  override_objects = modulemd_objects_from_file (yaml_path, &error);
  g_clear_pointer (&yaml_path, g_free);
  g_assert_nonnull (override_objects);
  g_assert_cmpint (override_objects->len, ==, 3);

  overridden =
    modulemd_merge_defaults (merged_base, override_objects, TRUE, &error);
  if (!overridden)
    {
      fprintf (stderr, "Error merging defaults: %s\n", error->message);
    }
  g_assert_nonnull (overridden);
  g_assert_cmpint (overridden->len, ==, 3);


  /* They should be in alphabetical order now */

  /* HTTPD */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (overridden, 0));
  g_assert_cmpstr (modulemd_defaults_peek_module_name (defaults), ==, "httpd");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "2.4");
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (defaults)),
    ==,
    2);
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "2.2"));
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "2.4"));
  g_assert_false (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "2.8"));

  /* NODEJS */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (overridden, 1));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "nodejs");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "9.0");
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (defaults)),
    ==,
    3);
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "6.0"));
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "8.0"));
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "9.0"));

  /* POSTGRESQL */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (overridden, 2));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "postgresql");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "8.1");
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (defaults)),
    ==,
    1);
  g_assert_true (g_hash_table_contains (
    modulemd_defaults_peek_profile_defaults (defaults), "8.1"));
}


static void
modulemd_defaults_test_prioritizer (DefaultsFixture *fixture,
                                    gconstpointer user_data)
{
  g_autofree gchar *yaml_base_path = NULL;
  g_autofree gchar *yaml_override_path = NULL;
  g_autoptr (GPtrArray) base_objects = NULL;
  g_autoptr (GPtrArray) override_objects = NULL;
  g_autoptr (GPtrArray) override_nodejs_objects = NULL;
  g_autoptr (GPtrArray) merged_objects = NULL;
  g_autoptr (ModulemdPrioritizer) prioritizer = NULL;
  g_autoptr (GError) error = NULL;
  GHashTable *htable = NULL;
  gint64 prio;
  ModulemdDefaults *defaults = NULL;
  gboolean result;

  yaml_base_path = g_strdup_printf ("%s/test_data/defaults/merging-base.yaml",
                                    g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_base_path);

  base_objects = modulemd_objects_from_file (yaml_base_path, &error);
  g_assert_nonnull (base_objects);
  g_assert_cmpint (base_objects->len, ==, 7);


  yaml_override_path =
    g_strdup_printf ("%s/test_data/defaults/overriding-nodejs.yaml",
                     g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_override_path);

  override_nodejs_objects =
    modulemd_objects_from_file (yaml_override_path, &error);
  g_clear_pointer (&yaml_override_path, g_free);
  g_assert_nonnull (override_nodejs_objects);
  g_assert_cmpint (override_nodejs_objects->len, ==, 1);

  yaml_override_path = g_strdup_printf (
    "%s/test_data/defaults/overriding.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_override_path);

  override_objects = modulemd_objects_from_file (yaml_override_path, &error);
  g_assert_nonnull (override_objects);
  g_assert_cmpint (override_objects->len, ==, 3);


  /* Test that importing the base objects work. These objects include several
   * exact duplicates which will be cleaned up by this process.
   */

  /* Pick a random number from 0-99 for the higher priority.
   * This will help exercise the sort function and make sure it doesn't
   * occasionally fail.
   */
  prio = g_test_rand_int_range (0, 99);
  g_info ("Random low priority level: %" PRIi64 "\n", prio);

  prioritizer = modulemd_prioritizer_new ();
  result = modulemd_prioritizer_add (prioritizer, base_objects, prio, &error);
  if (!result)
    {
      fprintf (stderr, "Merge error: %s", error->message);
    }
  g_assert_true (result);


  /*
   * Test that importing the base objects works again. This will be a worst-
   * case scenario where all of the values being imported are duplicated.
   */
  result = modulemd_prioritizer_add (prioritizer, base_objects, prio, &error);
  if (!result)
    {
      fprintf (stderr, "Merge error: %s", error->message);
    }
  g_assert_true (result);


  /*
   * Test that importing the nodejs overrides at the same priority level fails.
   *
   * This YAML has a conflicting default stream which should be ignored and set
   * to "no default stream".
   */

  result = modulemd_prioritizer_add (
    prioritizer, override_nodejs_objects, prio, &error);
  g_assert_true (result);

  merged_objects = modulemd_prioritizer_resolve (prioritizer, &error);
  g_assert_nonnull (merged_objects);
  g_assert_cmpint (merged_objects->len, ==, 3);

  for (gint i = 0; i < merged_objects->len; i++)
    {
      if (MODULEMD_IS_DEFAULTS (g_ptr_array_index (merged_objects, i)))
        {
          defaults = g_ptr_array_index (merged_objects, i);
          if (!g_strcmp0 (modulemd_defaults_peek_module_name (defaults),
                          "nodejs"))
            {
              g_assert_null (modulemd_defaults_peek_default_stream (defaults));
            }
        }
    }

  g_clear_pointer (&merged_objects, g_ptr_array_unref);


  /* Start over and test profile conflicts */
  g_clear_pointer (&prioritizer, g_object_unref);
  prioritizer = modulemd_prioritizer_new ();
  result = modulemd_prioritizer_add (prioritizer, base_objects, prio, &error);
  if (!result)
    {
      fprintf (stderr, "Merge error: %s", error->message);
    }
  g_assert_true (result);


  /*
   * Test that importing the overrides at the same priority level fails.
   *
   * These objects have several conflicts with the base objects that cannot be
   * merged.
   */
  result =
    modulemd_prioritizer_add (prioritizer, override_objects, prio, &error);
  g_assert_false (result);
  g_assert_cmpstr (
    g_quark_to_string (error->domain), ==, "modulemd-defaults-error-quark");
  g_assert_cmpint (
    error->code, ==, MODULEMD_DEFAULTS_ERROR_CONFLICTING_PROFILES);
  g_clear_pointer (&error, g_error_free);

  /* The object's internal state is undefined after an error, so delete it */
  g_clear_pointer (&prioritizer, g_object_unref);


  /* Start over and add the base */
  prioritizer = modulemd_prioritizer_new ();
  result = modulemd_prioritizer_add (prioritizer, base_objects, prio, &error);
  if (!result)
    {
      fprintf (stderr, "Merge error: %s", error->message);
    }
  g_assert_true (result);


  /* Test that importing the overrides at a higher priority level succeeds. */

  /* Pick a random number from 100-999 for the higher priority.
   * This will help exercise the sort function and make sure it doesn't
   * occasionally fail.
   */
  prio = g_test_rand_int_range (100, 999);
  g_info ("Random high priority level: %" PRIi64 "\n", prio);

  result =
    modulemd_prioritizer_add (prioritizer, override_objects, prio, &error);
  g_assert_true (result);

  /*
   * Test that re-importing the overrides at the same priority level
   * succeeds.
   */
  result =
    modulemd_prioritizer_add (prioritizer, override_objects, prio, &error);
  g_assert_true (result);


  /* Merge all of the results together */
  merged_objects = modulemd_prioritizer_resolve (prioritizer, &error);
  g_assert_nonnull (merged_objects);
  g_assert_cmpint (merged_objects->len, ==, 3);


  /* HTTPD */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_objects, 2));
  g_assert_cmpstr (modulemd_defaults_peek_module_name (defaults), ==, "httpd");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "2.4");
  htable = modulemd_defaults_peek_profile_defaults (defaults);
  g_assert_cmpint (g_hash_table_size (htable), ==, 2);
  g_assert_true (g_hash_table_contains (htable, "2.2"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.2"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.2"), "server"));
  g_assert_true (g_hash_table_contains (htable, "2.4"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.2"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.4"), "server"));
  g_assert_false (g_hash_table_contains (htable, "2.8"));

  /* NODEJS */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_objects, 1));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "nodejs");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "9.0");
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (defaults)),
    ==,
    3);

  htable = modulemd_defaults_peek_profile_defaults (defaults);
  g_assert_cmpint (g_hash_table_size (htable), ==, 3);
  g_assert_true (g_hash_table_contains (htable, "6.0"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "6.0"), "default"));
  g_assert_true (g_hash_table_contains (htable, "8.0"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.0"), "minimal"));
  g_assert_true (g_hash_table_contains (htable, "9.0"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "9.0"), "supermegaultra"));

  /* POSTGRESQL */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_objects, 0));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "postgresql");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "8.1");
  htable = modulemd_defaults_peek_profile_defaults (defaults);
  g_assert_cmpint (g_hash_table_size (htable), ==, 1);
  g_assert_true (g_hash_table_contains (htable, "8.1"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.1"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.1"), "server"));
  g_assert_true (
    modulemd_simpleset_contains (g_hash_table_lookup (htable, "8.1"), "foo"));
}


static void
modulemd_defaults_test_prioritizer_modified (DefaultsFixture *fixture,
                                             gconstpointer user_data)
{
  g_autofree gchar *yaml_base_path = NULL;
  g_autofree gchar *yaml_override_path = NULL;
  g_autoptr (GPtrArray) base_objects = NULL;
  g_autoptr (GPtrArray) override_objects = NULL;
  g_autoptr (GPtrArray) merged_objects = NULL;
  g_autoptr (ModulemdPrioritizer) prioritizer = NULL;
  g_autoptr (GError) error = NULL;
  GHashTable *htable = NULL;
  ModulemdDefaults *defaults = NULL;
  gboolean result;

  yaml_base_path = g_strdup_printf ("%s/test_data/defaults/merging-base.yaml",
                                    g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_base_path);

  base_objects = modulemd_objects_from_file (yaml_base_path, &error);
  g_assert_nonnull (base_objects);
  g_assert_cmpint (base_objects->len, ==, 7);

  yaml_override_path =
    g_strdup_printf ("%s/test_data/defaults/overriding-modified.yaml",
                     g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_override_path);

  override_objects = modulemd_objects_from_file (yaml_override_path, &error);
  g_assert_nonnull (override_objects);
  g_assert_cmpint (override_objects->len, ==, 3);


  /* Test that importing the base objects work. These objects include several
   * exact duplicates which will be cleaned up by this process.
   */

  prioritizer = modulemd_prioritizer_new ();
  result = modulemd_prioritizer_add (prioritizer, base_objects, 0, &error);
  if (!result)
    {
      fprintf (stderr, "Merge error: %s", error->message);
    }
  g_assert_true (result);


  /*
   * Test that importing the base objects works again. This will be a worst-
   * case scenario where all of the values being imported are duplicated.
   */
  result = modulemd_prioritizer_add (prioritizer, base_objects, 0, &error);
  if (!result)
    {
      fprintf (stderr, "Merge error: %s", error->message);
    }
  g_assert_true (result);


  /*
   * Test that importing the overrides at the same priority level succeeds.
   *
   * These objects have several conflicts with the base objects, but the
   * modified field overrides it.
   */
  result = modulemd_prioritizer_add (prioritizer, override_objects, 0, &error);
  g_assert_true (result);


  /* Merge all of the results together */
  merged_objects = modulemd_prioritizer_resolve (prioritizer, &error);
  g_assert_nonnull (merged_objects);
  g_assert_cmpint (merged_objects->len, ==, 3);


  /* HTTPD */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_objects, 0));
  g_assert_cmpstr (modulemd_defaults_peek_module_name (defaults), ==, "httpd");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "2.4");
  htable = modulemd_defaults_peek_profile_defaults (defaults);
  g_assert_cmpint (g_hash_table_size (htable), ==, 3);
  g_assert_true (g_hash_table_contains (htable, "2.2"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.2"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.2"), "server"));
  g_assert_true (g_hash_table_contains (htable, "2.4"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.2"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.4"), "server"));
  g_assert_true (g_hash_table_contains (htable, "2.8"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.8"), "notreal"));


  /* NODEJS */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_objects, 1));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "nodejs");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "9.0");
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (defaults)),
    ==,
    3);

  htable = modulemd_defaults_peek_profile_defaults (defaults);
  g_assert_cmpint (g_hash_table_size (htable), ==, 3);
  g_assert_true (g_hash_table_contains (htable, "6.0"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "6.0"), "default"));
  g_assert_true (g_hash_table_contains (htable, "8.0"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.0"), "minimal"));
  g_assert_true (g_hash_table_contains (htable, "9.0"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "9.0"), "supermegaultra"));

  /* POSTGRESQL */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_objects, 2));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "postgresql");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "8.1");
  htable = modulemd_defaults_peek_profile_defaults (defaults);
  g_assert_cmpint (g_hash_table_size (htable), ==, 2);
  g_assert_true (g_hash_table_contains (htable, "8.1"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.1"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.1"), "server"));
  g_assert_false (
    modulemd_simpleset_contains (g_hash_table_lookup (htable, "8.1"), "foo"));
  g_assert_true (g_hash_table_contains (htable, "8.2"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.2"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.2"), "server"));
  g_assert_true (
    modulemd_simpleset_contains (g_hash_table_lookup (htable, "8.2"), "foo"));
}


static void
modulemd_defaults_test_index_prioritizer (DefaultsFixture *fixture,
                                          gconstpointer user_data)
{
  g_autofree gchar *yaml_base_path = NULL;
  g_autofree gchar *yaml_override_path = NULL;
  g_autoptr (GHashTable) index = NULL;
  g_autoptr (GHashTable) override_index = NULL;
  g_autoptr (GPtrArray) merged_objects = NULL;
  g_autoptr (ModulemdPrioritizer) prioritizer = NULL;
  g_autoptr (GError) error = NULL;
  GHashTable *htable = NULL;
  gint64 prio;
  ModulemdDefaults *defaults = NULL;
  gboolean result;

  yaml_base_path = g_strdup_printf ("%s/test_data/defaults/merging-base.yaml",
                                    g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_base_path);

  index = modulemd_index_from_file (yaml_base_path, NULL, &error);
  g_assert_nonnull (index);
  g_assert_cmpint (g_hash_table_size (index), ==, 3);

  yaml_override_path = g_strdup_printf (
    "%s/test_data/defaults/overriding.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_override_path);

  override_index = modulemd_index_from_file (yaml_override_path, NULL, &error);
  g_assert_nonnull (override_index);
  g_assert_cmpint (g_hash_table_size (index), ==, 3);


  /* Test that importing the base objects work. These objects include several
   * exact duplicates which will be cleaned up by this process.
   */

  /* Pick a random number from 0-99 for the higher priority.
   * This will help exercise the sort function and make sure it doesn't
   * occasionally fail.
   */
  prio = g_test_rand_int_range (0, 99);
  g_info ("Random low priority level: %" PRIi64 "\n", prio);

  prioritizer = modulemd_prioritizer_new ();
  result = modulemd_prioritizer_add_index (prioritizer, index, prio, &error);
  if (!result)
    {
      fprintf (stderr, "Merge error: %s", error->message);
    }
  g_assert_true (result);


  /*
   * Test that importing the base objects works again. This will be a worst-
   * case scenario where all of the values being imported are duplicated.
   */
  result = modulemd_prioritizer_add_index (prioritizer, index, prio, &error);
  if (!result)
    {
      fprintf (stderr, "Merge error: %s", error->message);
    }
  g_assert_true (result);


  /*
   * Test that importing the overrides at the same priority level fails.
   *
   * These objects have several conflicts with the base objects that cannot be
   * merged.
   */
  result =
    modulemd_prioritizer_add_index (prioritizer, override_index, prio, &error);
  g_assert_false (result);
  g_assert_cmpstr (
    g_quark_to_string (error->domain), ==, "modulemd-defaults-error-quark");
  g_assert_cmpint (
    error->code, ==, MODULEMD_DEFAULTS_ERROR_CONFLICTING_PROFILES);
  g_clear_pointer (&error, g_error_free);

  /* The object's internal state is undefined after an error, so delete it */
  g_clear_pointer (&prioritizer, g_object_unref);


  /* Start over and add the base */
  prioritizer = modulemd_prioritizer_new ();
  result = modulemd_prioritizer_add_index (prioritizer, index, prio, &error);
  if (!result)
    {
      fprintf (stderr, "Merge error: %s", error->message);
    }
  g_assert_true (result);


  /* Test that importing the overrides at a higher priority level succeeds. */

  /* Pick a random number from 100-999 for the higher priority.
   * This will help exercise the sort function and make sure it doesn't
   * occasionally fail.
   */
  prio = g_test_rand_int_range (100, 999);
  g_info ("Random high priority level: %" PRIi64 "\n", prio);

  result =
    modulemd_prioritizer_add_index (prioritizer, override_index, prio, &error);
  g_assert_true (result);

  /*
   * Test that re-importing the overrides at the same priority level
   * succeeds.
   */
  result =
    modulemd_prioritizer_add_index (prioritizer, override_index, prio, &error);
  g_assert_true (result);


  /* Merge all of the results together */
  merged_objects = modulemd_prioritizer_resolve (prioritizer, &error);
  g_assert_nonnull (merged_objects);
  g_assert_cmpint (merged_objects->len, ==, 3);


  /* HTTPD */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_objects, 2));
  g_assert_cmpstr (modulemd_defaults_peek_module_name (defaults), ==, "httpd");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "2.4");
  htable = modulemd_defaults_peek_profile_defaults (defaults);
  g_assert_cmpint (g_hash_table_size (htable), ==, 2);
  g_assert_true (g_hash_table_contains (htable, "2.2"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.2"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.2"), "server"));
  g_assert_true (g_hash_table_contains (htable, "2.4"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.2"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "2.4"), "server"));
  g_assert_false (g_hash_table_contains (htable, "2.8"));

  /* NODEJS */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_objects, 1));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "nodejs");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "9.0");
  g_assert_cmpint (
    g_hash_table_size (modulemd_defaults_peek_profile_defaults (defaults)),
    ==,
    3);

  htable = modulemd_defaults_peek_profile_defaults (defaults);
  g_assert_cmpint (g_hash_table_size (htable), ==, 3);
  g_assert_true (g_hash_table_contains (htable, "6.0"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "6.0"), "default"));
  g_assert_true (g_hash_table_contains (htable, "8.0"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.0"), "minimal"));
  g_assert_true (g_hash_table_contains (htable, "9.0"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "9.0"), "supermegaultra"));

  /* POSTGRESQL */
  defaults = MODULEMD_DEFAULTS (g_ptr_array_index (merged_objects, 0));
  g_assert_cmpstr (
    modulemd_defaults_peek_module_name (defaults), ==, "postgresql");
  g_assert_cmpstr (
    modulemd_defaults_peek_default_stream (defaults), ==, "8.1");
  htable = modulemd_defaults_peek_profile_defaults (defaults);
  g_assert_cmpint (g_hash_table_size (htable), ==, 1);
  g_assert_true (g_hash_table_contains (htable, "8.1"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.1"), "client"));
  g_assert_true (modulemd_simpleset_contains (
    g_hash_table_lookup (htable, "8.1"), "server"));
  g_assert_true (
    modulemd_simpleset_contains (g_hash_table_lookup (htable, "8.1"), "foo"));
}


static void
modulemd_regressions_issue42 (DefaultsFixture *fixture,
                              gconstpointer user_data)
{
  g_autoptr (ModulemdPrioritizer) prioritizer = NULL;
  g_autoptr (GPtrArray) objects = NULL;
  g_autoptr (GError) error = NULL;
  prioritizer = modulemd_prioritizer_new ();

  /* Test that the prioritizer doesn't crash if it resolves zero documents */
  objects = modulemd_prioritizer_resolve (prioritizer, &error);
  g_assert_null (objects);
  g_assert_nonnull (error);
  g_assert_cmpint (
    error->code, ==, MODULEMD_PRIORITIZER_NOTHING_TO_PRIORITIZE);
}


static void
modulemd_regressions_issue44 (DefaultsFixture *fixture,
                              gconstpointer user_data)
{
  gboolean result;
  g_autoptr (ModulemdPrioritizer) prioritizer = NULL;
  g_autoptr (GPtrArray) base_objects = NULL;
  g_autoptr (GPtrArray) conflicting_objects = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml_base_path = NULL;
  g_autofree gchar *yaml_conflicting_path = NULL;

  prioritizer = modulemd_prioritizer_new ();

  /* Get a simple document */
  yaml_base_path = g_strdup_printf ("%s/test_data/defaults/issue44-1.yaml",
                                    g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_base_path);

  base_objects = modulemd_objects_from_file (yaml_base_path, &error);
  g_assert_nonnull (base_objects);

  result = modulemd_prioritizer_add (prioritizer, base_objects, 0, &error);
  g_assert_true (result);


  /* Add another almost-identical document, except with a conflicting default
   * stream set.
   *
   * NOTE: when this was written (for issue 44 on GitHub), this was meant to be
   * a hard error. As of 1.8.1 we expect this to just result in having no
   * default stream for this module. This test has been slightly modified so
   * that the expected result is now a pass.
   */
  yaml_conflicting_path = g_strdup_printf (
    "%s/test_data/defaults/issue44-2.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_conflicting_path);

  conflicting_objects =
    modulemd_objects_from_file (yaml_conflicting_path, &error);
  g_assert_nonnull (conflicting_objects);

  result =
    modulemd_prioritizer_add (prioritizer, conflicting_objects, 0, &error);
  g_assert_true (result);
}


static void
modulemd_regressions_issue45 (DefaultsFixture *fixture,
                              gconstpointer user_data)
{
  gboolean ret;
  g_autoptr (ModulemdPrioritizer) prioritizer = NULL;
  g_autoptr (GPtrArray) objects = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml_base_path = NULL;
  prioritizer = modulemd_prioritizer_new ();

  yaml_base_path = g_strdup_printf ("%s/test_data/defaults/merging-base.yaml",
                                    g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_base_path);
  objects = modulemd_objects_from_file (yaml_base_path, &error);

  ret = modulemd_prioritizer_add (
    prioritizer, objects, MODULEMD_PRIORITIZER_PRIORITY_MAX + 1, &error);
  g_assert_false (ret);
  g_assert_nonnull (error);
  g_assert_cmpint (
    error->code, ==, MODULEMD_PRIORITIZER_PRIORITY_OUT_OF_RANGE);
  g_clear_pointer (&error, g_error_free);

  /* Test that the prioritizer throws an error on a negative priority */
  ret = modulemd_prioritizer_add (prioritizer, objects, -1, &error);
  g_assert_false (ret);
  g_assert_nonnull (error);
  g_assert_cmpint (
    error->code, ==, MODULEMD_PRIORITIZER_PRIORITY_OUT_OF_RANGE);
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

  g_test_add ("/modulemd/defaults/modulemd_defaults_test_good_examples/ex4",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_defaults_test_good_ex4,
              NULL);

  g_test_add ("/modulemd/defaults/modulemd_defaults_test_copy",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_defaults_test_copy,
              NULL);


  g_test_add ("/modulemd/defaults/modulemd_defaults_test_merging",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_defaults_test_merging,
              NULL);

  g_test_add ("/modulemd/defaults/modulemd_defaults_test_prioritizer",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_defaults_test_prioritizer,
              NULL);

  g_test_add ("/modulemd/defaults/modulemd_defaults_test_prioritizer_modified",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_defaults_test_prioritizer_modified,
              NULL);

  g_test_add ("/modulemd/defaults/modulemd_defaults_test_index_prioritizer",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_defaults_test_index_prioritizer,
              NULL);

  g_test_add ("/modulemd/defaults/modulemd_regressions_issue42",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_regressions_issue42,
              NULL);

  g_test_add ("/modulemd/defaults/modulemd_regressions_issue44",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_regressions_issue44,
              NULL);

  g_test_add ("/modulemd/defaults/modulemd_regressions_issue45",
              DefaultsFixture,
              NULL,
              NULL,
              modulemd_regressions_issue45,
              NULL);

  return g_test_run ();
};
