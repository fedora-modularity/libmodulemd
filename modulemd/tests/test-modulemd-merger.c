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

#include "modulemd-module-index.h"
#include "modulemd-module-index-merger.h"
#include "private/test-utils.h"


static void
merger_test_constructors (CommonMmdTestFixture *fixture,
                          gconstpointer user_data)
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
merger_test_deduplicate (CommonMmdTestFixture *fixture,
                         gconstpointer user_data)
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


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/v2/module/index/merger/constructors",
              CommonMmdTestFixture,
              NULL,
              NULL,
              merger_test_constructors,
              NULL);

  g_test_add ("/modulemd/v2/module/index/merger/deduplicate",
              CommonMmdTestFixture,
              NULL,
              NULL,
              merger_test_deduplicate,
              NULL);

  return g_test_run ();
}
