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

#include "modulemd.h"

#include "config.h"
#include "private/test-utils.h"

static void
test_modulemd_get_version (void)
{
  g_assert_cmpstr (modulemd_get_version (), ==, LIBMODULEMD_VERSION);
}


static void
test_modulemd_load_file (void)
{
  g_autofree gchar *yaml_file = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdModuleIndex) idx = NULL;

  /* This function is a wrapper around lower-level functions, so it should be
   * okay to just test basic success and failure here.
   */

  /* Valid, large datafile */
  g_clear_error (&error);
  g_clear_object (&idx);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/f29.yaml", g_getenv ("TEST_DATA_PATH"));
  idx = modulemd_load_file (yaml_file, &error);
  g_assert_no_error (error);
  g_assert_nonnull (idx);


  /* Nonexistent file */
  g_clear_error (&error);
  g_clear_object (&idx);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file =
    g_strdup_printf ("%s/nosuchfile.yaml", g_getenv ("TEST_DATA_PATH"));
  idx = modulemd_load_file (yaml_file, &error);
  g_assert_null (idx);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_OPEN);


  /* Readable, non-YAML file */
  g_clear_error (&error);
  g_clear_object (&idx);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/nl.po", g_getenv ("TEST_DATA_PATH"));
  idx = modulemd_load_file (yaml_file, &error);
  g_assert_null (idx);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_UNPARSEABLE);


  /* Readable, but invalid YAML file */
  g_clear_error (&error);
  g_clear_object (&idx);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file =
    g_strdup_printf ("%s/good_and_bad.yaml", g_getenv ("TEST_DATA_PATH"));
  idx = modulemd_load_file (yaml_file, &error);
  g_assert_null (idx);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
}


static void
test_modulemd_load_string (void)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdModuleIndex) idx = NULL;
  g_autofree gchar *output = NULL;

  /* This function is a wrapper around lower-level functions, so it should be
   * okay to just test basic success and failure here.
   */

  /* Trivial modulemd */
  g_clear_error (&error);
  g_clear_object (&idx);
  yaml_string =
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  name: trivialname\n"
    "  stream: trivialstream\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license:\n"
    "    module: MIT\n"
    "...\n";
  idx = modulemd_load_string (yaml_string, &error);
  g_assert_no_error (error);
  g_assert_nonnull (idx);

  /* Make sure loaded index dumps to string cleanly */
  g_clear_error (&error);
  g_clear_pointer (&output, g_free);
  output = modulemd_module_index_dump_to_string (idx, &error);
  g_assert_no_error (error);
  g_assert_nonnull (output);


  /* NULL string should raise an exception */
  g_clear_error (&error);
  g_clear_object (&idx);
  modulemd_test_signal = 0;
  signal (SIGTRAP, modulemd_test_signal_handler);
  idx = modulemd_load_string (NULL, &error);
  g_assert_cmpint (modulemd_test_signal, ==, SIGTRAP);
  g_assert_null (idx);


  /* An empty string is valid YAML, so it returns a non-NULL but empty index. */
  g_clear_error (&error);
  g_clear_object (&idx);
  yaml_string = "";
  idx = modulemd_load_string (yaml_string, &error);
  g_assert_no_error (error);
  g_assert_nonnull (idx);


  /* Invalid YAML string */
  g_clear_error (&error);
  g_clear_object (&idx);
  yaml_string = "Hello, World!\n";
  idx = modulemd_load_string (yaml_string, &error);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
  g_assert_null (idx);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add_func ("/modulemd/v2/common/get_version",
                   test_modulemd_get_version);

  g_test_add_func ("/modulemd/v2/common/load_file", test_modulemd_load_file);
  g_test_add_func ("/modulemd/v2/common/load_string",
                   test_modulemd_load_string);

  return g_test_run ();
}
