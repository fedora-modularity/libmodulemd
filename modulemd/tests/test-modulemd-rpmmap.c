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
#include <glib/gstdio.h>
#include <locale.h>
#include <signal.h>

#include "modulemd-rpm-map-entry.h"
#include "private/modulemd-rpm-map-entry-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

static void
test_basic (void)
{
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *nevra = NULL;
  guint64 epoch = 0;

  /* Test that the new() function works */
  entry = modulemd_rpm_map_entry_new (
    "bar", 0, "1.23", "1.module_deadbeef", "x86_64");

  g_assert_nonnull (entry);
  g_assert_cmpstr (modulemd_rpm_map_entry_get_name (entry), ==, "bar");
  g_assert_cmpuint (modulemd_rpm_map_entry_get_epoch (entry), ==, 0);
  g_assert_cmpstr (modulemd_rpm_map_entry_get_version (entry), ==, "1.23");
  g_assert_cmpstr (
    modulemd_rpm_map_entry_get_release (entry), ==, "1.module_deadbeef");
  g_assert_cmpstr (modulemd_rpm_map_entry_get_arch (entry), ==, "x86_64");
  g_assert_true (modulemd_rpm_map_entry_validate (entry, &error));
  g_assert_null (error);
  nevra = modulemd_rpm_map_entry_get_nevra_as_string (entry);
  g_assert_nonnull (nevra);
  g_assert_cmpstr (nevra, ==, "bar-0:1.23-1.module_deadbeef.x86_64");
  g_clear_pointer (&nevra, g_free);
  g_clear_object (&entry);

  /* Test that object instantiation with no attributes works */
  entry = g_object_new (MODULEMD_TYPE_RPM_MAP_ENTRY, NULL);
  // clang-format on
  g_assert_nonnull (entry);
  g_assert_null (modulemd_rpm_map_entry_get_name (entry));
  g_assert_cmpuint (modulemd_rpm_map_entry_get_epoch (entry), ==, 0);
  g_assert_null (modulemd_rpm_map_entry_get_version (entry));
  g_assert_null (modulemd_rpm_map_entry_get_release (entry));
  g_assert_null (modulemd_rpm_map_entry_get_arch (entry));
  g_assert_false (modulemd_rpm_map_entry_validate (entry, &error));
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_clear_error (&error);
  nevra = modulemd_rpm_map_entry_get_nevra_as_string (entry);
  g_assert_null (nevra);
  g_clear_object (&entry);

  /* Test that object instantiation with attributes works */
  // clang-format off
  entry = g_object_new (MODULEMD_TYPE_RPM_MAP_ENTRY,
                       "name", "bar",
                       "epoch", epoch,
                       "version", "1.23",
                       "release", "1.module_deadbeef",
                       "arch", "x86_64",
                       NULL);
  // clang-format on
  g_assert_nonnull (entry);
  g_assert_cmpstr (modulemd_rpm_map_entry_get_name (entry), ==, "bar");
  g_assert_cmpuint (modulemd_rpm_map_entry_get_epoch (entry), ==, 0);
  g_assert_cmpstr (modulemd_rpm_map_entry_get_version (entry), ==, "1.23");
  g_assert_cmpstr (
    modulemd_rpm_map_entry_get_release (entry), ==, "1.module_deadbeef");
  g_assert_cmpstr (modulemd_rpm_map_entry_get_arch (entry), ==, "x86_64");
  g_assert_true (modulemd_rpm_map_entry_validate (entry, &error));
  g_assert_null (error);
  nevra = modulemd_rpm_map_entry_get_nevra_as_string (entry);
  g_assert_nonnull (nevra);
  g_assert_cmpstr (nevra, ==, "bar-0:1.23-1.module_deadbeef.x86_64");
  g_clear_pointer (&nevra, g_free);
  g_clear_object (&entry);
}


static void
test_compare (void)
{
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  ModulemdRpmMapEntry *entry_pointer;
  g_autoptr (ModulemdRpmMapEntry) entry2 = NULL;
  g_autoptr (ModulemdRpmMapEntry) entry3 = NULL;

  entry = modulemd_rpm_map_entry_new (
    "bar", 0, "1.23", "1.module_deadbeef", "x86_64");
  g_assert_nonnull (entry);
  entry_pointer = entry;

  entry2 = modulemd_rpm_map_entry_new (
    "bar", 0, "1.23", "1.module_deadbeef", "x86_64");
  g_assert_nonnull (entry2);

  entry3 = modulemd_rpm_map_entry_new (
    "foo", 0, "1.23", "1.module_deadbeef", "x86_64");
  g_assert_nonnull (entry3);

  /* Test that passing the same pointer returns TRUE */
  g_assert_true (modulemd_rpm_map_entry_equals (entry, entry_pointer));

  /* Test that passing two equivalent entries returns TRUE */
  g_assert_true (modulemd_rpm_map_entry_equals (entry, entry2));

  /* Test that passing an entry with a different name returns FALSE */
  g_assert_false (modulemd_rpm_map_entry_equals (entry, entry3));
}


static void
test_parse_yaml_valid (void)
{
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);

  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;

  /* Parse a valid rpm-map entry */
  yaml_path =
    g_strdup_printf ("%s/rpm-map/valid.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);
  parser_skip_document_start (&parser);

  entry = modulemd_rpm_map_entry_parse_yaml (&parser, TRUE, &error);
  g_assert_nonnull (entry);
  g_assert_null (error);

  g_assert_true (modulemd_rpm_map_entry_validate (entry, &error));
  g_assert_null (error);
}

static void
test_parse_yaml_missing (void)
{
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);

  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  /* Try an entry that's missing a field */
  yaml_path = g_strdup_printf ("%s/rpm-map/missing-version.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);
  parser_skip_document_start (&parser);

  entry = modulemd_rpm_map_entry_parse_yaml (&parser, TRUE, &error);
  g_assert_null (entry);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
}

static void
test_parse_yaml_mismatch (void)
{
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);

  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  /* Try an entry with mismatched nevra */
  yaml_path = g_strdup_printf ("%s/rpm-map/wrong-nevra.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);
  parser_skip_document_start (&parser);

  entry = modulemd_rpm_map_entry_parse_yaml (&parser, TRUE, &error);
  g_assert_null (entry);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_INCONSISTENT);
}

static void
test_emit_yaml_valid (void)
{
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  // clang-format off
  const gchar *baseline =
    "---\n"
    "name: baz\n"
    "epoch: 2\n"
    "version: 2.18\n"
    "release: 3.module_baddad\n"
    "arch: s390x\n"
    "nevra: baz-2:2.18-3.module_baddad.s390x\n"
    "...\n";
  // clang-format on

  entry =
    modulemd_rpm_map_entry_new ("baz", 2, "2.18", "3.module_baddad", "s390x");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));

  g_assert_true (modulemd_rpm_map_entry_emit_yaml (entry, &emitter, &error));
  g_assert_no_error (error);

  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));

  g_assert_cmpstr (yaml_string->str, ==, baseline);

  g_clear_pointer (&yaml_string, modulemd_yaml_string_free);
}


static void
test_emit_yaml_invalid (void)
{
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  entry =
    modulemd_rpm_map_entry_new (NULL, 2, "2.18", "3.module_baddad", "s390x");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));

  g_assert_false (modulemd_rpm_map_entry_emit_yaml (entry, &emitter, &error));
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add_func ("/modulemd/v2/rpm_map/basic", test_basic);

  g_test_add_func ("/modulemd/v2/rpm_map/compare", test_compare);

  g_test_add_func ("/modulemd/v2/rpm_map/yaml/parse/valid",
                   test_parse_yaml_valid);

  g_test_add_func ("/modulemd/v2/rpm_map/yaml/parse/missing",
                   test_parse_yaml_missing);

  g_test_add_func ("/modulemd/v2/rpm_map/yaml/parse/mismatch",
                   test_parse_yaml_mismatch);

  g_test_add_func ("/modulemd/v2/rpm_map/yaml/emit/valid",
                   test_emit_yaml_valid);

  g_test_add_func ("/modulemd/v2/rpm_map/yaml/emit/invalid",
                   test_emit_yaml_invalid);

  return g_test_run ();
}
