/*
 * This file is part of libmodulemd
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#include <glib.h>
#include <yaml.h>

#include "modulemd-defaults-v1.h"
#include "modulemd-defaults.h"
#include "modulemd-module-index-merger.h"
#include "modulemd-module-index.h"
#include "private/test-utils.h"


static void
merger_test_constructors (void)
{
  g_autoptr (ModulemdModuleIndexMerger) merger = NULL;

  /* Test standard object instantiation */
  merger = g_object_new (MODULEMD_TYPE_MODULE_INDEX_MERGER, NULL);
  g_assert_nonnull (merger);
  g_clear_object (&merger);

  /* Test new() */
  merger = modulemd_module_index_merger_new ();
  g_assert_nonnull (merger);
  g_clear_object (&merger);
}


static void
merger_test_deduplicate (void)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (ModulemdModuleIndex) index2 = NULL;
  g_autoptr (ModulemdModuleIndex) merged_index = NULL;
  g_autoptr (ModulemdModuleIndexMerger) merger = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *baseline = NULL;
  g_autofree gchar *deduplicated = NULL;

  yaml_path =
    g_strdup_printf ("%s/f29-updates.yaml", g_getenv ("TEST_DATA_PATH"));

  index = modulemd_module_index_new ();

  g_assert_nonnull (yaml_path);

  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* Save the baseline output for later comparison */
  baseline = modulemd_module_index_dump_to_string (index, &error);
  g_assert_nonnull (baseline);
  g_assert_null (error);


  index2 = modulemd_module_index_new ();

  g_assert_nonnull (yaml_path);

  g_assert_true (modulemd_module_index_update_from_file (
    index2, yaml_path, TRUE, &failures, &error));
  g_assert_cmpint (failures->len, ==, 0);
  g_assert_nonnull (index2);
  g_assert_null (error);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* Create a ModuleIndexMerger and add both copies to it */
  merger = modulemd_module_index_merger_new ();
  g_assert_nonnull (merger);

  modulemd_module_index_merger_associate_index (merger, index, 0);
  modulemd_module_index_merger_associate_index (merger, index2, 0);

  /* Resolve the merge, which should deduplicate all entries */
  merged_index = modulemd_module_index_merger_resolve (merger, &error);
  g_assert_no_error (error);
  g_assert_nonnull (merged_index);

  deduplicated = modulemd_module_index_dump_to_string (merged_index, &error);
  g_assert_nonnull (deduplicated);
  g_assert_null (error);

  g_assert_cmpstr (baseline, ==, deduplicated);
}


static void
merger_test_merger (void)
{
  g_autoptr (ModulemdModuleIndex) base_index = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autofree gchar *module_name = NULL;
  ModulemdModule *httpd = NULL;
  ModulemdDefaultsV1 *httpd_defaults = NULL;
  GStrv httpd_profile_streams = NULL;
  g_autoptr (ModulemdModuleIndex) override_nodejs_index = NULL;
  g_autoptr (ModulemdModuleIndexMerger) merger = NULL;
  g_autoptr (ModulemdModuleIndex) merged_index = NULL;
  ModulemdModule *nodejs = NULL;
  ModulemdDefaultsV1 *nodejs_defaults = NULL;
  ModulemdDefaultsV1 *merged_httpd_defaults = NULL;
  g_autoptr (ModulemdModuleIndex) override_index = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  gint32 random_low;
  gint32 random_high;

  /* Get a set of o = NULLbjects in a ModuleIndex */
  yaml_path =
    g_strdup_printf ("%s/merging-base.yaml", g_getenv ("TEST_DATA_PATH"));
  base_index = modulemd_module_index_new ();
  modulemd_module_index_update_from_file (
    base_index, yaml_path, TRUE, &failures, &error);
  g_clear_pointer (&yaml_path, g_free);

  /* Baseline */
  httpd = modulemd_module_index_get_module (base_index, "httpd");
  httpd_defaults = (ModulemdDefaultsV1 *)modulemd_module_get_defaults (httpd);
  g_assert_nonnull (httpd_defaults);

  g_assert_cmpstr (
    modulemd_defaults_v1_get_default_stream (httpd_defaults, NULL), ==, "2.2");

  httpd_profile_streams =
    modulemd_defaults_v1_get_streams_with_default_profiles_as_strv (
      httpd_defaults, NULL);
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 2);
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "2.2"));
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "2.8"));
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
  httpd_profile_streams =
    modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
      httpd_defaults, "2.2", NULL);
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 2);
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "client"));
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "server"));
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
  httpd_profile_streams =
    modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
      httpd_defaults, "2.8", NULL);
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "notreal"));
  g_clear_pointer (&httpd_profile_streams, g_strfreev);

  g_assert_cmpstr (
    modulemd_defaults_v1_get_default_stream (httpd_defaults, "workstation"),
    ==,
    "2.4");

  httpd_profile_streams =
    modulemd_defaults_v1_get_streams_with_default_profiles_as_strv (
      httpd_defaults, "workstation");
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 2);
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "2.4"));
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "2.6"));
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
  httpd_profile_streams =
    modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
      httpd_defaults, "2.4", "workstation");
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 1);
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
  httpd_profile_streams =
    modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
      httpd_defaults, "2.6", "workstation");
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 3);
  g_clear_pointer (&httpd_profile_streams, g_strfreev);

  /*
   * Get another set of objects that will override the default stream for
   * nodejs 
   */

  yaml_path =
    g_strdup_printf ("%s/overriding-nodejs.yaml", g_getenv ("TEST_DATA_PATH"));
  override_nodejs_index = modulemd_module_index_new ();
  modulemd_module_index_update_from_file (
    override_nodejs_index, yaml_path, TRUE, &failures, &error);
  g_clear_pointer (&yaml_path, g_free);

  /*
   * Test that adding both of these at the same priority level results in
   * the no default stream
   */
  merger = modulemd_module_index_merger_new ();
  modulemd_module_index_merger_associate_index (merger, base_index, 0);
  modulemd_module_index_merger_associate_index (
    merger, override_nodejs_index, 0);

  merged_index = modulemd_module_index_merger_resolve (merger, NULL);
  g_clear_object (&merger);

  nodejs = modulemd_module_index_get_module (merged_index, "nodejs");
  g_assert_nonnull (nodejs);

  nodejs_defaults =
    (ModulemdDefaultsV1 *)modulemd_module_get_defaults (nodejs);
  g_assert_nonnull (nodejs_defaults);
  g_assert_null (
    modulemd_defaults_v1_get_default_stream (nodejs_defaults, NULL));
  g_clear_object (&merged_index);

  /* Get another set of objects that will override  g_assert_null(modulemd_defaults_v1_get_default_stream the above */
  yaml_path =
    g_strdup_printf ("%s/overriding.yaml", g_getenv ("TEST_DATA_PATH"));
  override_index = modulemd_module_index_new ();
  modulemd_module_index_update_from_file (
    override_index, yaml_path, TRUE, &failures, &error);
  g_clear_pointer (&yaml_path, g_free);

  /*
   * Test that override_index at a higher priority level succeeds
   * Test that adding both of these at the same priority level fails
   * with a merge conflict.
   * Use randomly-selected high and low values to make sure we don't have
   * sorting issues.
   */
  merger = modulemd_module_index_merger_new ();
  random_low = g_random_int_range (1, 100);
  random_high = g_random_int_range (101, 999);
  printf ("Low priority: %d, High priority: %d", random_low, random_high);
  modulemd_module_index_merger_associate_index (
    merger, base_index, random_low);
  modulemd_module_index_merger_associate_index (
    merger, override_index, random_high);

  merged_index = modulemd_module_index_merger_resolve (merger, NULL);
  g_assert_nonnull (merged_index);
  g_clear_object (&merger);

  /*Validate merged results*/

  /*HTTPD*/
  merged_httpd_defaults = (ModulemdDefaultsV1 *)modulemd_module_get_defaults (
    modulemd_module_index_get_module (merged_index, "httpd"));
  g_assert_nonnull (merged_httpd_defaults);

  g_assert_cmpstr (
    modulemd_defaults_v1_get_default_stream (merged_httpd_defaults, NULL),
    ==,
    "2.4");

  httpd_profile_streams =
    modulemd_defaults_v1_get_streams_with_default_profiles_as_strv (
      merged_httpd_defaults, NULL);
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "2.2"));
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "2.4"));
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
  httpd_profile_streams =
    modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
      merged_httpd_defaults, "2.2", NULL);
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 2);
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "client"));
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "server"));
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
  httpd_profile_streams =
    modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
      merged_httpd_defaults, "2.4", NULL);
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "client"));
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "server"));
  g_clear_pointer (&httpd_profile_streams, g_strfreev);

  g_assert_cmpstr (modulemd_defaults_v1_get_default_stream (
                     merged_httpd_defaults, "workstation"),
                   ==,
                   "2.8");

  httpd_profile_streams =
    modulemd_defaults_v1_get_streams_with_default_profiles_as_strv (
      merged_httpd_defaults, "workstation");
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 3);
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "2.4"));
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "2.6"));
  g_assert_true (
    g_strv_contains ((const char **)httpd_profile_streams, "2.8"));
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
  httpd_profile_streams =
    modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
      merged_httpd_defaults, "2.4", "workstation");
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 1);
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
  httpd_profile_streams =
    modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
      merged_httpd_defaults, "2.6", "workstation");
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 3);
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
  httpd_profile_streams =
    modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
      merged_httpd_defaults, "2.8", "workstation");
  g_assert_cmpint (g_strv_length (httpd_profile_streams), ==, 4);
  g_clear_pointer (&httpd_profile_streams, g_strfreev);
}


static void
merger_test_add_only (void)
{
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdModuleIndex) base_idx = modulemd_module_index_new ();
  g_autoptr (ModulemdModuleIndex) add_only_idx = modulemd_module_index_new ();
  g_autoptr (ModulemdModuleIndex) merged_idx = NULL;
  g_autoptr (ModulemdModuleIndexMerger) merger =
    modulemd_module_index_merger_new ();
  ModulemdModule *httpd = NULL;
  ModulemdDefaults *httpd_defs = NULL;
  g_autofree gchar *base_yaml =
    g_strdup_printf ("%s/merger/base.yaml", g_getenv ("TEST_DATA_PATH"));
  g_autofree gchar *add_only_yaml =
    g_strdup_printf ("%s/merger/add_only.yaml", g_getenv ("TEST_DATA_PATH"));

  g_assert_true (modulemd_module_index_update_from_file (
    base_idx, base_yaml, TRUE, &failures, &error));
  g_assert_true (modulemd_module_index_update_from_file (
    add_only_idx, add_only_yaml, TRUE, &failures, &error));

  modulemd_module_index_merger_associate_index (merger, base_idx, 0);
  modulemd_module_index_merger_associate_index (merger, add_only_idx, 0);

  merged_idx = modulemd_module_index_merger_resolve_ext (merger, TRUE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (merged_idx);

  httpd = modulemd_module_index_get_module (merged_idx, "httpd");
  g_assert_nonnull (httpd);

  httpd_defs = modulemd_module_get_defaults (httpd);
  g_assert_nonnull (httpd_defs);

  g_assert_cmpstr (modulemd_defaults_v1_get_default_stream (
                     MODULEMD_DEFAULTS_V1 (httpd_defs), NULL),
                   ==,
                   "2.8");

  g_assert_cmpstr (modulemd_defaults_v1_get_default_stream (
                     MODULEMD_DEFAULTS_V1 (httpd_defs), "workstation"),
                   ==,
                   "2.4");
}


static void
merger_test_add_conflicting_stream (void)
{
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdModuleIndex) base_idx = modulemd_module_index_new ();
  g_autoptr (ModulemdModuleIndex) add_conflicting_idx =
    modulemd_module_index_new ();
  g_autoptr (ModulemdModuleIndex) merged_idx = NULL;
  g_autoptr (ModulemdModuleIndexMerger) merger =
    modulemd_module_index_merger_new ();
  ModulemdModule *psql = NULL;
  ModulemdDefaults *psql_defs = NULL;
  g_autofree gchar *base_yaml =
    g_strdup_printf ("%s/merger/base.yaml", g_getenv ("TEST_DATA_PATH"));
  g_autofree gchar *add_conflicting_yaml = g_strdup_printf (
    "%s/merger/add_conflicting_stream.yaml", g_getenv ("TEST_DATA_PATH"));

  g_assert_true (modulemd_module_index_update_from_file (
    base_idx, base_yaml, TRUE, &failures, &error));
  g_assert_true (modulemd_module_index_update_from_file (
    add_conflicting_idx, add_conflicting_yaml, TRUE, &failures, &error));

  modulemd_module_index_merger_associate_index (merger, base_idx, 0);
  modulemd_module_index_merger_associate_index (
    merger, add_conflicting_idx, 0);

  merged_idx =
    modulemd_module_index_merger_resolve_ext (merger, FALSE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (merged_idx);

  psql = modulemd_module_index_get_module (merged_idx, "postgresql");
  g_assert_nonnull (psql);

  psql_defs = modulemd_module_get_defaults (psql);
  g_assert_nonnull (psql_defs);

  g_assert_null (modulemd_defaults_v1_get_default_stream (
    MODULEMD_DEFAULTS_V1 (psql_defs), NULL));
}


static void
merger_test_add_conflicting_stream_and_profile_modified (void)
{
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdModuleIndex) base_idx = modulemd_module_index_new ();
  g_autoptr (ModulemdModuleIndex) add_conflicting_idx =
    modulemd_module_index_new ();
  g_autoptr (ModulemdModuleIndex) merged_idx = NULL;
  g_autoptr (ModulemdModuleIndexMerger) merger =
    modulemd_module_index_merger_new ();
  ModulemdModule *psql = NULL;
  ModulemdDefaults *psql_defs = NULL;
  g_autofree gchar *base_yaml =
    g_strdup_printf ("%s/merger/base.yaml", g_getenv ("TEST_DATA_PATH"));
  g_autofree gchar *add_conflicting_yaml = g_strdup_printf (
    "%s/merger/add_conflicting_stream_and_profile_modified.yaml",
    g_getenv ("TEST_DATA_PATH"));

  g_assert_true (modulemd_module_index_update_from_file (
    base_idx, base_yaml, TRUE, &failures, &error));
  g_assert_true (modulemd_module_index_update_from_file (
    add_conflicting_idx, add_conflicting_yaml, TRUE, &failures, &error));

  modulemd_module_index_merger_associate_index (merger, base_idx, 0);
  modulemd_module_index_merger_associate_index (
    merger, add_conflicting_idx, 0);

  merged_idx =
    modulemd_module_index_merger_resolve_ext (merger, FALSE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (merged_idx);

  psql = modulemd_module_index_get_module (merged_idx, "postgresql");
  g_assert_nonnull (psql);

  psql_defs = modulemd_module_get_defaults (psql);
  g_assert_nonnull (psql_defs);

  g_assert_cmpstr (modulemd_defaults_v1_get_default_stream (
                     MODULEMD_DEFAULTS_V1 (psql_defs), NULL),
                   ==,
                   "8.2");
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add_func ("/modulemd/v2/module/index/merger/constructors",
                   merger_test_constructors);

  g_test_add_func ("/modulemd/v2/module/index/merger/deduplicate",
                   merger_test_deduplicate);

  g_test_add_func ("/modulemd/v2/module/index/merger/merger",
                   merger_test_merger);

  g_test_add_func ("/modulemd/module/index/merger/add_only",
                   merger_test_add_only);

  g_test_add_func ("/modulemd/module/index/merger/add_conflicting_stream",
                   merger_test_add_conflicting_stream);

  g_test_add_func ("/modulemd/module/index/merger/add_conflicting_both",
                   merger_test_add_conflicting_stream_and_profile_modified);

  return g_test_run ();
}
