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

#include "modulemd.h"
#include "modulemd-buildopts.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <inttypes.h>
#include <time.h>

typedef struct _BuildoptsFixture
{
} BuildoptsFixture;

static void
modulemd_buildopts_test_basic (BuildoptsFixture *fixture,
                               gconstpointer user_data)
{
  g_autoptr (ModulemdBuildopts) buildopts = NULL;
  g_autoptr (ModulemdBuildopts) copy = NULL;
  const gchar *demo_macros = "%demomacro 1\n%demomacro2 %{demomacro}23";
  const gchar *demo_macros2 = "foo";
  g_auto (GStrv) demo_whitelist;
  g_auto (GStrv) retrieved_whitelist;
  g_auto (GStrv) boxed_whitelist;
  g_autofree gchar *retrieved_macros = NULL;
  g_autofree gchar *rpm_macros = NULL;

  /* Test standard object construction succeeds */
  buildopts = g_object_new (MODULEMD_TYPE_BUILDOPTS, NULL);
  g_assert_nonnull (buildopts);
  g_assert_true (MODULEMD_IS_BUILDOPTS (buildopts));
  g_clear_pointer (&buildopts, g_object_unref);

  buildopts = modulemd_buildopts_new ();
  g_assert_nonnull (buildopts);
  g_assert_true (MODULEMD_IS_BUILDOPTS (buildopts));

  /* Test set/get methods for RPM macros */
  modulemd_buildopts_set_rpm_macros (buildopts, demo_macros);
  retrieved_macros = modulemd_buildopts_get_rpm_macros (buildopts);
  g_assert_cmpstr (retrieved_macros, ==, demo_macros);
  g_clear_pointer (&retrieved_macros, g_free);

  /* Test GObject properties for RPM macros*/
  g_object_get (G_OBJECT (buildopts), "rpm-macros", &rpm_macros, NULL);
  g_assert_cmpstr (rpm_macros, ==, demo_macros);

  /* Assign another value */
  g_object_set (G_OBJECT (buildopts), "rpm-macros", demo_macros2, NULL);
  retrieved_macros = modulemd_buildopts_get_rpm_macros (buildopts);
  g_assert_cmpstr (retrieved_macros, ==, demo_macros2);
  g_clear_pointer (&retrieved_macros, g_free);


  /* Test set/get methods for RPM whitelist */
  demo_whitelist = g_malloc0_n (3, sizeof (gchar *));
  demo_whitelist[0] = g_strdup ("bar");
  demo_whitelist[1] = g_strdup ("baz");

  modulemd_buildopts_set_rpm_whitelist (buildopts, demo_whitelist);
  retrieved_whitelist = modulemd_buildopts_get_rpm_whitelist (buildopts);

  g_assert_cmpstr (retrieved_whitelist[0], ==, demo_whitelist[0]);
  g_assert_cmpstr (retrieved_whitelist[1], ==, demo_whitelist[1]);
  g_assert_cmpstr (retrieved_whitelist[2], ==, demo_whitelist[2]);
  g_clear_pointer (&retrieved_whitelist, g_strfreev);

  /* Test GObject properties for RPM whitelist */
  boxed_whitelist = g_malloc0_n (2, sizeof (gchar *));
  boxed_whitelist[0] = g_strdup ("jonsnow");

  g_object_get (
    G_OBJECT (buildopts), "rpm-whitelist", &retrieved_whitelist, NULL);

  g_assert_cmpstr (retrieved_whitelist[0], ==, demo_whitelist[0]);
  g_assert_cmpstr (retrieved_whitelist[1], ==, demo_whitelist[1]);
  g_assert_null (retrieved_whitelist[2]);
  g_clear_pointer (&retrieved_whitelist, g_strfreev);

  g_object_set (G_OBJECT (buildopts), "rpm-whitelist", boxed_whitelist, NULL);
  retrieved_whitelist = modulemd_buildopts_get_rpm_whitelist (buildopts);

  g_assert_cmpstr (retrieved_whitelist[0], ==, boxed_whitelist[0]);
  g_assert_null (retrieved_whitelist[1]);
  g_clear_pointer (&retrieved_whitelist, g_strfreev);


  /* Test copying */
  copy = modulemd_buildopts_copy (buildopts);

  retrieved_macros = modulemd_buildopts_get_rpm_macros (copy);
  g_assert_cmpstr (retrieved_macros, ==, demo_macros2);

  retrieved_whitelist = modulemd_buildopts_get_rpm_whitelist (copy);
  g_assert_cmpstr (retrieved_whitelist[0], ==, boxed_whitelist[0]);
  g_assert_null (retrieved_whitelist[1]);
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add ("/modulemd/buildopts/modulemd_buildopts_test_basic",
              BuildoptsFixture,
              NULL,
              NULL,
              modulemd_buildopts_test_basic,
              NULL);

  return g_test_run ();
}
