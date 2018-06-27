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

#include "modulemd-servicelevel.h"

#include <glib.h>
#include <locale.h>

typedef struct _ServiceLevelFixture
{
} ServiceLevelFixture;

static void
modulemd_servicelevel_test_get_set_name (ServiceLevelFixture *fixture,
                                         gconstpointer user_data)
{
  const gchar *refname = "rawhide";
  const gchar *name = NULL;
  ModulemdServiceLevel *sl = modulemd_servicelevel_new ();
  g_assert_nonnull (sl);

  modulemd_servicelevel_set_name (sl, refname);

  name = modulemd_servicelevel_peek_name (sl);
  g_assert_cmpstr (name, ==, refname);
  g_clear_pointer (&sl, g_object_unref);
}

static void
modulemd_servicelevel_test_get_set_eol (ServiceLevelFixture *fixture,
                                        gconstpointer user_data)
{
  GDate *refeol = NULL;
  const GDate *eol = NULL;

  ModulemdServiceLevel *sl = modulemd_servicelevel_new ();
  g_assert_nonnull (sl);

  refeol = g_date_new_dmy (29, 2, 2020);

  modulemd_servicelevel_set_eol (sl, refeol);

  eol = modulemd_servicelevel_peek_eol (sl);

  g_assert_cmpint (g_date_compare (eol, refeol), ==, 0);

  g_clear_pointer (&refeol, g_date_free);
  g_clear_pointer (&sl, g_object_unref);
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/servicelevel/modulemd_servicelevel_test_get_set_name",
              ServiceLevelFixture,
              NULL,
              NULL,
              modulemd_servicelevel_test_get_set_name,
              NULL);

  g_test_add ("/modulemd/servicelevel/modulemd_servicelevel_test_get_set_eol",
              ServiceLevelFixture,
              NULL,
              NULL,
              modulemd_servicelevel_test_get_set_eol,
              NULL);

  return g_test_run ();
}
