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

#include "modulemd-upgrade-helper.h"
#include "private/test-utils.h"


static void
upgrade_helper_construct (void)
{
  g_autoptr (ModulemdUpgradeHelper) helper = modulemd_upgrade_helper_new ();
  g_assert_nonnull (helper);
  g_assert_true (MODULEMD_IS_UPGRADE_HELPER (helper));

  g_auto (GStrv) modules =
    modulemd_upgrade_helper_get_known_modules_as_strv (helper);
  g_assert_nonnull (modules);
  g_assert_null (modules[0]);
}


static void
upgrade_helper_known_streams (void)
{
  g_autoptr (ModulemdUpgradeHelper) helper = NULL;
  g_auto (GStrv) modules = NULL;
  g_auto (GStrv) streams = NULL;

  helper = modulemd_upgrade_helper_new ();
  g_assert_nonnull (helper);

  modulemd_upgrade_helper_add_known_stream (helper, "platform", "f33");
  modulemd_upgrade_helper_add_known_stream (helper, "platform", "f34");
  modulemd_upgrade_helper_add_known_stream (helper, "platform", "eln");
  modulemd_upgrade_helper_add_known_stream (helper, "django", "3.0");

  modules = modulemd_upgrade_helper_get_known_modules_as_strv (helper);
  g_assert_nonnull (modules);
  g_assert_cmpstr ("django", ==, modules[0]);
  g_assert_cmpstr ("platform", ==, modules[1]);
  g_assert_null (modules[2]);
  g_clear_pointer (&modules, g_strfreev);

  streams =
    modulemd_upgrade_helper_get_known_streams_as_strv (helper, "platform");
  g_assert_nonnull (streams);
  g_assert_cmpstr ("eln", ==, streams[0]);
  g_assert_cmpstr ("f33", ==, streams[1]);
  g_assert_cmpstr ("f34", ==, streams[2]);
  g_assert_null (streams[3]);
  g_clear_pointer (&streams, g_strfreev);

  streams =
    modulemd_upgrade_helper_get_known_streams_as_strv (helper, "django");
  g_assert_nonnull (streams);
  g_assert_cmpstr ("3.0", ==, streams[0]);
  g_assert_null (streams[1]);
  g_clear_pointer (&streams, g_strfreev);


  /* Add a duplicate to verify that we are deduplicating */
  modulemd_upgrade_helper_add_known_stream (helper, "platform", "f33");

  modules = modulemd_upgrade_helper_get_known_modules_as_strv (helper);
  g_assert_nonnull (modules);
  g_assert_cmpstr ("django", ==, modules[0]);
  g_assert_cmpstr ("platform", ==, modules[1]);
  g_assert_null (modules[2]);
  g_clear_pointer (&modules, g_strfreev);

  streams =
    modulemd_upgrade_helper_get_known_streams_as_strv (helper, "platform");
  g_assert_nonnull (streams);
  g_assert_cmpstr ("eln", ==, streams[0]);
  g_assert_cmpstr ("f33", ==, streams[1]);
  g_assert_cmpstr ("f34", ==, streams[2]);
  g_assert_null (streams[3]);
  g_clear_pointer (&streams, g_strfreev);

  streams =
    modulemd_upgrade_helper_get_known_streams_as_strv (helper, "django");
  g_assert_nonnull (streams);
  g_assert_cmpstr ("3.0", ==, streams[0]);
  g_assert_null (streams[1]);
  g_clear_pointer (&streams, g_strfreev);

  /* Verify that we get an empty list back if we look up streams for an unknown
   * module
   */
  streams =
    modulemd_upgrade_helper_get_known_streams_as_strv (helper, "unknown");
  g_assert_nonnull (streams);
  g_assert_null (streams[0]);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add_func ("/modulemd/v2/upgradehelper/construct",
                   upgrade_helper_construct);
  g_test_add_func ("/modulemd/v2/upgradehelper/known",
                   upgrade_helper_known_streams);

  return g_test_run ();
}
