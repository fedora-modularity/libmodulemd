/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat, Inc.
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
#include <glib/gstdio.h>
#include <locale.h>
#include <signal.h>

#include "modulemd.h"
#include "private/glib-extensions.h"
#include "private/modulemd-packager-v3-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"


static void
packager_test_construct (void)
{
  g_autoptr (ModulemdPackagerV3) packager = NULL;
  g_auto (GStrv) list = NULL;

  /* == Test that the new() function works == */
  packager = modulemd_packager_v3_new ();
  g_assert_nonnull (packager);
  g_assert_true (MODULEMD_IS_PACKAGER_V3 (packager));

  /* == Verify that it was constructed properly empty == */

  g_assert_null (modulemd_packager_v3_get_module_name (packager));
  g_assert_null (modulemd_packager_v3_get_stream_name (packager));
  g_assert_null (modulemd_packager_v3_get_summary (packager));
  g_assert_null (modulemd_packager_v3_get_description (packager));

  list = modulemd_packager_v3_get_module_licenses_as_strv (packager);
  g_assert_nonnull (list);
  g_assert_null (list[0]);
  g_clear_pointer (&list, g_strfreev);

  g_assert_null (modulemd_packager_v3_get_xmd (packager));

  list = modulemd_packager_v3_get_build_config_contexts_as_strv (packager);
  g_assert_nonnull (list);
  g_assert_null (list[0]);
  g_clear_pointer (&list, g_strfreev);

  g_assert_null (modulemd_packager_v3_get_community (packager));
  g_assert_null (modulemd_packager_v3_get_documentation (packager));
  g_assert_null (modulemd_packager_v3_get_tracker (packager));

  list = modulemd_packager_v3_get_profile_names_as_strv (packager);
  g_assert_nonnull (list);
  g_assert_null (list[0]);
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_packager_v3_get_rpm_api_as_strv (packager);
  g_assert_nonnull (list);
  g_assert_null (list[0]);
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_packager_v3_get_rpm_filters_as_strv (packager);
  g_assert_nonnull (list);
  g_assert_null (list[0]);
  g_clear_pointer (&list, g_strfreev);


  list = modulemd_packager_v3_get_rpm_component_names_as_strv (packager);
  g_assert_nonnull (list);
  g_assert_null (list[0]);
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_packager_v3_get_module_component_names_as_strv (packager);
  g_assert_nonnull (list);
  g_assert_null (list[0]);
  g_clear_pointer (&list, g_strfreev);
}


static ModulemdPackagerV3 *
read_spec (void)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdPackagerV3) packager = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;

  /* Verify a valid YAML file */
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_packager_v3.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  /* First parse to the subdocument */
  yaml_parser_set_input_file (&parser, yaml_stream);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (YAML_STREAM_START_EVENT, ==, event.type);
  yaml_event_delete (&event);

  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (YAML_DOCUMENT_START_EVENT, ==, event.type);
  yaml_event_delete (&event);

  subdoc = modulemd_yaml_parse_document_type (&parser);
  g_assert_nonnull (subdoc);
  g_assert_no_error (modulemd_subdocument_info_get_gerror (subdoc));

  g_assert_cmpint (MODULEMD_YAML_DOC_PACKAGER,
                   ==,
                   modulemd_subdocument_info_get_doctype (subdoc));
  g_assert_cmpint (MD_PACKAGER_VERSION_THREE,
                   ==,
                   modulemd_subdocument_info_get_mdversion (subdoc));
  g_assert_nonnull (modulemd_subdocument_info_get_yaml (subdoc));

  packager = modulemd_packager_v3_parse_yaml (subdoc, &error);
  g_assert_no_error (error);
  g_assert_nonnull (packager);
  g_assert_true (MODULEMD_IS_PACKAGER_V3 (packager));

  return g_steal_pointer (&packager);
}


static void
validate_spec (ModulemdPackagerV3 *packager)
{
  g_auto (GStrv) strv = NULL;

  g_assert_true (MODULEMD_IS_PACKAGER_V3 (packager));
  g_assert_cmpstr ("foo", ==, modulemd_packager_v3_get_module_name (packager));

  g_assert_cmpstr (
    "latest", ==, modulemd_packager_v3_get_stream_name (packager));

  g_assert_cmpstr (
    "An example module", ==, modulemd_packager_v3_get_summary (packager));

  g_assert_cmpstr (
    "A module for the demonstration of the metadata format. Also, the "
    "obligatory lorem ipsum dolor sit amet goes right here.",
    ==,
    modulemd_packager_v3_get_description (packager));

  strv = modulemd_packager_v3_get_module_licenses_as_strv (packager);
  g_assert_nonnull (strv);
  g_assert_nonnull (strv[0]);
  g_assert_cmpstr ("MIT", ==, strv[0]);
  g_assert_null (strv[1]);
  g_clear_pointer (&strv, g_strfreev);

  /* Skipping XMD because those will be much easier to validate in Python */

  strv = modulemd_packager_v3_get_build_config_contexts_as_strv (packager);
  g_assert_nonnull (strv);
  g_assert_nonnull (strv[0]);
  g_assert_cmpstr ("CTX1", ==, strv[0]);
  g_assert_nonnull (strv[1]);
  g_assert_cmpstr ("CTX2", ==, strv[1]);
  g_assert_null (strv[2]);
  g_clear_pointer (&strv, g_strfreev);

  g_assert_cmpstr ("http://www.example.com/",
                   ==,
                   modulemd_packager_v3_get_community (packager));
  g_assert_cmpstr ("http://www.example.com/",
                   ==,
                   modulemd_packager_v3_get_documentation (packager));
  g_assert_cmpstr ("http://www.example.com/",
                   ==,
                   modulemd_packager_v3_get_tracker (packager));

  strv = modulemd_packager_v3_get_profile_names_as_strv (packager);
  g_assert_nonnull (strv);
  g_assert_nonnull (strv[0]);
  g_assert_cmpstr ("buildroot", ==, strv[0]);
  g_assert_nonnull (strv[1]);
  g_assert_cmpstr ("container", ==, strv[1]);
  g_assert_nonnull (strv[2]);
  g_assert_cmpstr ("minimal", ==, strv[2]);
  g_assert_nonnull (strv[3]);
  g_assert_cmpstr ("srpm-buildroot", ==, strv[3]);
  g_assert_null (strv[4]);
  g_clear_pointer (&strv, g_strfreev);

  strv = modulemd_packager_v3_get_rpm_api_as_strv (packager);
  g_assert_nonnull (strv);
  g_assert_nonnull (strv[0]);
  g_assert_cmpstr ("bar", ==, strv[0]);
  g_assert_nonnull (strv[1]);
  g_assert_cmpstr ("bar-devel", ==, strv[1]);
  g_assert_nonnull (strv[2]);
  g_assert_cmpstr ("bar-extras", ==, strv[2]);
  g_assert_nonnull (strv[3]);
  g_assert_cmpstr ("baz", ==, strv[3]);
  g_assert_nonnull (strv[4]);
  g_assert_cmpstr ("xxx", ==, strv[4]);
  g_assert_null (strv[5]);
  g_clear_pointer (&strv, g_strfreev);

  strv = modulemd_packager_v3_get_rpm_filters_as_strv (packager);
  g_assert_nonnull (strv);
  g_assert_nonnull (strv[0]);
  g_assert_cmpstr ("baz-nonfoo", ==, strv[0]);
  g_assert_null (strv[1]);
  g_clear_pointer (&strv, g_strfreev);

  strv = modulemd_packager_v3_get_rpm_component_names_as_strv (packager);
  g_assert_nonnull (strv);
  g_assert_nonnull (strv[0]);
  g_assert_cmpstr ("bar", ==, strv[0]);
  g_assert_nonnull (strv[1]);
  g_assert_cmpstr ("baz", ==, strv[1]);
  g_assert_nonnull (strv[2]);
  g_assert_cmpstr ("xxx", ==, strv[2]);
  g_assert_null (strv[3]);
  g_clear_pointer (&strv, g_strfreev);

  strv = modulemd_packager_v3_get_module_component_names_as_strv (packager);
  g_assert_nonnull (strv);
  g_assert_nonnull (strv[0]);
  g_assert_cmpstr ("includedmodule", ==, strv[0]);
  g_assert_null (strv[1]);
  g_clear_pointer (&strv, g_strfreev);
}


static void
packager_test_parse_spec (void)
{
  g_autoptr (ModulemdPackagerV3) packager = NULL;

  packager = read_spec ();
  validate_spec (packager);
}


static void
packager_test_parse_spec_copy (void)
{
  g_autoptr (ModulemdPackagerV3) orig = NULL;
  g_autoptr (ModulemdPackagerV3) packager = NULL;

  orig = read_spec ();
  packager = modulemd_packager_v3_copy (orig);

  validate_spec (packager);
}


static void
packager_test_map_to_stream_v2 (void)
{
  g_autoptr (ModulemdPackagerV3) packager = NULL;
  g_autoptr (ModulemdModuleStreamV2) v2_stream = NULL;
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml_str = NULL;
  g_autofree gchar *expected_path = NULL;
  g_autofree gchar *expected_str = NULL;

  packager = read_spec ();

  v2_stream = modulemd_packager_v3_to_stream_v2 (packager, &error);
  g_assert_no_error (error);
  g_assert_nonnull (v2_stream);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (v2_stream));
  g_clear_object (&v2_stream);

  index = modulemd_packager_v3_to_stream_v2_ext (packager, &error);
  g_assert_no_error (error);
  g_assert_nonnull (index);
  g_assert_true (MODULEMD_IS_MODULE_INDEX (index));

  yaml_str = modulemd_module_index_dump_to_string (index, &error);
  g_assert_no_error (error);
  g_assert_nonnull (yaml_str);

  g_debug ("YAML dump of index from PackageV3 to StreamV2 mapping:\n%s",
           yaml_str);

  expected_path = g_strdup_printf ("%s/upgrades/packager_v3_to_stream_v2.yaml",
                                   g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (expected_path);
  g_assert_true (
    g_file_get_contents (expected_path, &expected_str, NULL, &error));
  g_assert_no_error (error);
  g_assert_nonnull (expected_str);

  g_assert_cmpstr (expected_str, ==, yaml_str);

  g_clear_object (&index);
  g_clear_pointer (&yaml_str, g_free);
  g_clear_pointer (&expected_path, g_free);
  g_clear_pointer (&expected_str, g_free);
}

static void
packager_test_map_to_stream_v2_autoname (void)
{
  g_autoptr (ModulemdPackagerV3) packager = NULL;
  g_autoptr (ModulemdModuleStreamV2) v2_stream = NULL;
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml_str = NULL;
  g_auto (GStrv) list = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  /* Construct a minimal PackagerV3 with no module/stream name */
  packager = modulemd_packager_v3_new ();
  g_assert_nonnull (packager);
  g_assert_true (MODULEMD_IS_PACKAGER_V3 (packager));

  modulemd_packager_v3_set_summary (packager, "Summary");
  modulemd_packager_v3_set_description (packager, "Description");

  /* PackagerV3 to StreamV2 conversion should succeed and validate, even without
   * a module/stream name
   */
  v2_stream = modulemd_packager_v3_to_stream_v2 (packager, &error);
  g_assert_no_error (error);
  g_assert_nonnull (v2_stream);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (v2_stream));

  /* confirm everything comes back that we expect */
  g_assert_null (modulemd_module_stream_get_module_name (
    MODULEMD_MODULE_STREAM (v2_stream)));
  g_assert_null (modulemd_module_stream_get_stream_name (
    MODULEMD_MODULE_STREAM (v2_stream)));

  g_assert_cmpstr (
    "Summary", ==, modulemd_module_stream_v2_get_summary (v2_stream, "C"));
  g_assert_cmpstr ("Description",
                   ==,
                   modulemd_module_stream_v2_get_description (v2_stream, "C"));

  /* the default module license is required for StreamV2 and should have been filled in */
  list = modulemd_module_stream_v2_get_module_licenses_as_strv (v2_stream);
  g_assert_nonnull (list);
  g_assert_nonnull (list[0]);
  g_assert_cmpstr ("MIT", ==, list[0]);
  g_assert_null (list[1]);
  g_clear_pointer (&list, g_strfreev);

  /* PackagerV3 to Index conversion should automatically generate a module and
   * stream name so it can be added to an index
   */
  index = modulemd_packager_v3_to_stream_v2_ext (packager, &error);
  g_assert_no_error (error);
  g_assert_nonnull (index);
  g_assert_true (MODULEMD_IS_MODULE_INDEX (index));

  /* however, the automatic module/stream names should not appear in an index dump */
  yaml_str = modulemd_module_index_dump_to_string (index, &error);
  g_assert_no_error (error);
  g_assert_nonnull (yaml_str);
  g_assert_cmpstr (yaml_str,
                   ==,
                   "---\n"
                   "document: modulemd\n"
                   "version: 2\n"
                   "data:\n"
                   "  summary: Summary\n"
                   "  description: >-\n"
                   "    Description\n"
                   "  license:\n"
                   "    module:\n"
                   "    - MIT\n"
                   "...\n");

  g_clear_pointer (&yaml_str, g_free);
  g_clear_object (&v2_stream);
  g_clear_object (&packager);
  g_clear_object (&index);
}


static void
packager_test_read_to_index (void)
{
  gboolean ret;
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autofree gchar *yaml_str = NULL;
  g_autofree gchar *expected_path = NULL;
  g_autofree gchar *expected_str = NULL;

  /* create an index */
  index = modulemd_module_index_new ();

  /* The modulemd-packager v3 definition */
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_packager_v3.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  ret = modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error);
  g_assert_no_error (error);
  modulemd_subdocument_info_debug_dump_failures (failures);
  g_assert_true (ret);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  g_assert_cmpint (MD_MODULESTREAM_VERSION_TWO,
                   ==,
                   modulemd_module_index_get_stream_mdversion (index));

  yaml_str = modulemd_module_index_dump_to_string (index, &error);
  g_assert_no_error (error);
  g_assert_nonnull (yaml_str);

  g_debug ("packager_test_read_to_index() dump of v2 index:\n%s", yaml_str);

  /* the index should contain the packager v2 document converted to stream v2 */
  expected_path = g_strdup_printf ("%s/upgrades/packager_v3_to_stream_v2.yaml",
                                   g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (expected_path);
  g_assert_true (
    g_file_get_contents (expected_path, &expected_str, NULL, &error));
  g_assert_no_error (error);
  g_assert_nonnull (expected_str);
  g_clear_pointer (&expected_path, g_free);

  g_assert_cmpstr (expected_str, ==, yaml_str);

  g_clear_object (&index);
  g_clear_pointer (&yaml_str, g_free);
  g_clear_pointer (&expected_str, g_free);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");


  g_test_add_func ("/modulemd/v2/packager/construct", packager_test_construct);

  g_test_add_func ("/modulemd/v2/packager/yaml/spec",
                   packager_test_parse_spec);

  g_test_add_func ("/modulemd/v2/packager/yaml/spec/copy",
                   packager_test_parse_spec_copy);

  g_test_add_func ("/modulemd/v2/packager/to_stream_v2",
                   packager_test_map_to_stream_v2);

  g_test_add_func ("/modulemd/v2/packager/to_stream_v2/autoname",
                   packager_test_map_to_stream_v2_autoname);

  g_test_add_func ("/modulemd/v2/packager/index/read",
                   packager_test_read_to_index);

  return g_test_run ();
}
