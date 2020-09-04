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


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add_func ("/modulemd/v2/common/get_version",
                   test_modulemd_get_version);
  g_test_add_func ("/modulemd/v2/common/load_file", test_modulemd_load_file);

  return g_test_run ();
}
