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

#include "modulemd-defaults.h"
#include "modulemd-module.h"
#include "modulemd-module-index.h"
#include "modulemd-module-stream-v1.h"
#include "modulemd-module-stream-v2.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _ModuleIndexFixture
{
} ModuleIndexFixture;


static void
module_index_test_dump (ModuleIndexFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (ModulemdTranslationEntry) translation_entry = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree const gchar *string = NULL;

  /* Construct an Index with some objects */
  index = modulemd_module_index_new ();

  /* First: translations */
  translation = modulemd_translation_new (1, "testmodule1", "teststream1", 42);
  translation_entry = modulemd_translation_entry_new ("ro_TA");
  modulemd_translation_entry_set_summary (translation_entry,
                                          "Testsummary in ro_TA");
  modulemd_translation_set_translation_entry (translation, translation_entry);
  g_clear_pointer (&translation_entry, g_object_unref);
  translation_entry = modulemd_translation_entry_new ("nl_NL");
  modulemd_translation_entry_set_summary (translation_entry,
                                          "Een test omschrijving");
  modulemd_translation_set_translation_entry (translation, translation_entry);
  g_clear_pointer (&translation_entry, g_object_unref);
  g_assert_true (
    modulemd_module_index_add_translation (index, translation, &error));
  g_assert_no_error (error);
  g_clear_pointer (&translation, g_object_unref);

  /* Second: defaults */
  defaults = modulemd_defaults_new (1, "testmodule1");
  g_assert_true (modulemd_module_index_add_defaults (index, defaults, &error));
  g_assert_no_error (error);
  g_clear_pointer (&defaults, g_object_unref);

  /* Third: some streams */
  stream = (ModulemdModuleStream *)modulemd_module_stream_v1_new (
    "testmodule1", "teststream1");
  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);
  g_clear_pointer (&stream, g_object_unref);
  stream = (ModulemdModuleStream *)modulemd_module_stream_v2_new (
    "testmodule1", "teststream2");
  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);
  g_clear_pointer (&stream, g_object_unref);

  /* And now... emit */
  string = modulemd_module_index_dump_to_string (index, &error);
  g_assert_no_error (error);
  g_assert_nonnull (string);
  /* TODO: Needs to get updated with the streams once those get emited */
  g_assert_cmpstr (
    string,
    ==,
    "---\ndocument: modulemd-defaults\nversion: 1\ndata:\n  module: "
    "testmodule1\n...\n---\ndocument: modulemd-translations\nversion: "
    "1\ndata:\n  module: testmodule1\n  stream: teststream1\n  modified: 42\n "
    " translations:\n  - nl_NL\n  - summary: Een test omschrijving\n  - "
    "ro_TA\n  - summary: Testsummary in ro_TA\n...\n");
}


static void
module_index_test_read (ModuleIndexFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autofree gchar *yaml_path = NULL;

  /* Read the specification files all in */
  index = modulemd_module_index_new ();

  /* The two stream definitions */
  yaml_path =
    g_strdup_printf ("%s/spec.v1.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);
  yaml_path =
    g_strdup_printf ("%s/spec.v2.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* The translation definitions */
  yaml_path = g_strdup_printf ("%s/translations/spec.v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* The defaults definitions */
  yaml_path = g_strdup_printf ("%s/mod-defaults/spec.v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /*
   * Also try to ingest a TranslationEntry.
   * This should fail, and return a failure, since it's not a top-level subdoc.
   */
  yaml_path = g_strdup_printf ("%s/modulemd/v2/tests/test_data/te.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 1);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* Actually verifying the contents is left to Python tests */
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/v2/module/index/dump",
              ModuleIndexFixture,
              NULL,
              NULL,
              module_index_test_dump,
              NULL);

  g_test_add ("/modulemd/v2/module/index/read",
              ModuleIndexFixture,
              NULL,
              NULL,
              module_index_test_read,
              NULL);

  return g_test_run ();
}
