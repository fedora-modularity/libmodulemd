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

#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <signal.h>

#include "modulemd-defaults.h"
#include "modulemd-defaults-v1.h"
#include "private/glib-extensions.h"
#include "private/modulemd-translation-entry-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

static void
defaults_test_construct (CommonMmdTestFixture *fixture,
                         gconstpointer user_data)
{
  g_autoptr (ModulemdDefaults) defaults = NULL;

  /* Test new() with a valid mdversion and module name */
  defaults = modulemd_defaults_new (MD_DEFAULTS_VERSION_ONE, "foo");
  g_assert_nonnull (defaults);
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults));
  g_assert_true (MODULEMD_IS_DEFAULTS_V1 (defaults));
  g_clear_object (&defaults);

  /* Test new() with a zero mdversion */
  modulemd_test_signal = 0;
  signal (SIGTRAP, modulemd_test_signal_handler);
  defaults = modulemd_defaults_new (0, "foo");
  g_assert_cmpint (modulemd_test_signal, ==, SIGTRAP);
  g_assert_null (defaults);

  /* Test new() with a too-high mdversion */
  modulemd_test_signal = 0;
  signal (SIGTRAP, modulemd_test_signal_handler);
  defaults = modulemd_defaults_new (MD_DEFAULTS_VERSION_LATEST + 1, "foo");
  g_assert_cmpint (modulemd_test_signal, ==, SIGTRAP);
  g_assert_null (defaults);

  /* Test new() with a NULL module_name */
  modulemd_test_signal = 0;
  signal (SIGTRAP, modulemd_test_signal_handler);
  defaults = modulemd_defaults_new (MD_DEFAULTS_VERSION_ONE, NULL);
  g_assert_cmpint (modulemd_test_signal, ==, SIGTRAP);
  /* If we trap the error, defaults actually returns a value here, so free
   * it
   */
  g_clear_object (&defaults);
}


static void
defaults_test_copy (CommonMmdTestFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdDefaults) defaults = NULL;
  g_autoptr (ModulemdDefaults) copied_defaults = NULL;

  defaults = modulemd_defaults_new (MD_DEFAULTS_VERSION_LATEST, "foo");
  g_assert_nonnull (defaults);
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults));
  g_assert_true (MODULEMD_IS_DEFAULTS_V1 (defaults));

  copied_defaults = modulemd_defaults_copy (defaults);
  g_assert_nonnull (copied_defaults);
  g_assert_true (MODULEMD_IS_DEFAULTS (copied_defaults));
  g_assert_true (MODULEMD_IS_DEFAULTS_V1 (copied_defaults));
  g_assert_cmpuint (modulemd_defaults_get_mdversion (defaults),
                    ==,
                    modulemd_defaults_get_mdversion (copied_defaults));

  g_assert_cmpstr (modulemd_defaults_get_module_name (defaults),
                   ==,
                   modulemd_defaults_get_module_name (copied_defaults));
}


static void
defaults_test_get_mdversion (CommonMmdTestFixture *fixture,
                             gconstpointer user_data)
{
  g_autoptr (ModulemdDefaults) defaults = NULL;
  guint64 mdversion;

  defaults = modulemd_defaults_new (MD_DEFAULTS_VERSION_LATEST, "foo");
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults));

  mdversion = modulemd_defaults_get_mdversion (defaults);
  g_assert_cmpuint (mdversion, ==, MD_DEFAULTS_VERSION_LATEST);
}


static void
defaults_test_get_module_name (CommonMmdTestFixture *fixture,
                               gconstpointer user_data)
{
  g_autoptr (ModulemdDefaults) defaults = NULL;
  const gchar *module_name;

  defaults = modulemd_defaults_new (MD_DEFAULTS_VERSION_LATEST, "foo");
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults));

  module_name = modulemd_defaults_get_module_name (defaults);
  g_assert_cmpstr (module_name, ==, "foo");
}


static void
defaults_test_validate (CommonMmdTestFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdDefaults) defaults = NULL;

  defaults = modulemd_defaults_new (MD_DEFAULTS_VERSION_ONE, "foo");
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults));

  /* Currently there is no way for validation to fail, since all of its
   * properties are forced to be valid at object instantiation.
   * This will need to be updated once the subclasses have reimplimented this.
   */
  g_assert_true (modulemd_defaults_validate (defaults, NULL));
}


static void
defaults_test_equals (CommonMmdTestFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdDefaults) defaults_1 = NULL;
  g_autoptr (ModulemdDefaults) defaults_2 = NULL;

  /*Check equality for 2 default objects with same module name and mdversion*/
  defaults_1 = modulemd_defaults_new (MD_DEFAULTS_VERSION_ONE, "foo");
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults_1));
  defaults_2 = modulemd_defaults_new (MD_DEFAULTS_VERSION_ONE, "foo");
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults_2));

  g_assert_true (modulemd_defaults_equals (defaults_1, defaults_2));

  g_clear_object (&defaults_1);
  g_clear_object (&defaults_2);

  /*Check equality for 2 default objects with different module name and same mdversion*/
  defaults_1 = modulemd_defaults_new (MD_DEFAULTS_VERSION_ONE, "foo");
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults_1));
  defaults_2 = modulemd_defaults_new (MD_DEFAULTS_VERSION_ONE, "bar");
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults_2));

  g_assert_false (modulemd_defaults_equals (defaults_1, defaults_2));

  g_clear_object (&defaults_1);
  g_clear_object (&defaults_2);
}


static void
defaults_test_upgrade (CommonMmdTestFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdDefaults) defaults = NULL;
  g_autoptr (ModulemdDefaults) upgraded_defaults = NULL;
  g_autoptr (GError) error = NULL;

  defaults = modulemd_defaults_new (MD_DEFAULTS_VERSION_ONE, "foo");
  g_assert_true (MODULEMD_IS_DEFAULTS (defaults));

  /* Currently, we have only a single version, so the "upgrade" just makes a
   * copy
   */
  upgraded_defaults =
    modulemd_defaults_upgrade (defaults, MD_DEFAULTS_VERSION_ONE, &error);
  g_assert_nonnull (upgraded_defaults);
  g_assert_true (MODULEMD_IS_DEFAULTS (upgraded_defaults));
  g_assert_true (MODULEMD_IS_DEFAULTS_V1 (upgraded_defaults));

  g_assert_cmpint (modulemd_defaults_get_mdversion (upgraded_defaults),
                   ==,
                   MD_DEFAULTS_VERSION_ONE);
  g_assert_cmpstr (
    modulemd_defaults_get_module_name (upgraded_defaults), ==, "foo");
  g_clear_object (&upgraded_defaults);

  /* Test attempting to upgrade to an unknown mdversion */
  upgraded_defaults = modulemd_defaults_upgrade (
    defaults, MD_DEFAULTS_VERSION_LATEST + 1, &error);
  g_assert_null (upgraded_defaults);
  g_assert_nonnull (error);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.
  g_test_add ("/modulemd/v2/defaults/construct",
              CommonMmdTestFixture,
              NULL,
              NULL,
              defaults_test_construct,
              NULL);

  g_test_add ("/modulemd/v2/defaults/copy",
              CommonMmdTestFixture,
              NULL,
              NULL,
              defaults_test_copy,
              NULL);

  g_test_add ("/modulemd/v2/defaults/mdversion",
              CommonMmdTestFixture,
              NULL,
              NULL,
              defaults_test_get_mdversion,
              NULL);

  g_test_add ("/modulemd/v2/defaults/module_name",
              CommonMmdTestFixture,
              NULL,
              NULL,
              defaults_test_get_module_name,
              NULL);

  g_test_add ("/modulemd/v2/defaults/validate",
              CommonMmdTestFixture,
              NULL,
              NULL,
              defaults_test_validate,
              NULL);

  g_test_add ("/modulemd/v2/defaults/equals",
              CommonMmdTestFixture,
              NULL,
              NULL,
              defaults_test_equals,
              NULL);

  g_test_add ("/modulemd/v2/defaults/upgrade",
              CommonMmdTestFixture,
              NULL,
              NULL,
              defaults_test_upgrade,
              NULL);

  return g_test_run ();
}
