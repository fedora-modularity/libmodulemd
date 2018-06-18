/* test-modulemd-buildopts.c
 *
 * Copyright (C) 2018 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
  g_auto (GValue) value = G_VALUE_INIT;

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
  g_value_init (&value, G_TYPE_STRING);
  g_object_get_property (G_OBJECT (buildopts), "rpm-macros", &value);
  g_assert_cmpstr (g_value_get_string (&value), ==, demo_macros);

  /* Assign another value */
  g_value_reset (&value);
  g_value_set_static_string (&value, demo_macros2);
  g_object_set_property (G_OBJECT (buildopts), "rpm-macros", &value);
  retrieved_macros = modulemd_buildopts_get_rpm_macros (buildopts);
  g_assert_cmpstr (retrieved_macros, ==, demo_macros2);


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
  g_value_unset (&value);
  g_value_init (&value, G_TYPE_STRV);
  boxed_whitelist = g_malloc0_n (2, sizeof (gchar *));
  boxed_whitelist[0] = g_strdup ("jonsnow");

  g_object_get_property (G_OBJECT (buildopts), "rpm-whitelist", &value);
  retrieved_whitelist = g_value_get_boxed (&value);

  g_assert_cmpstr (retrieved_whitelist[0], ==, demo_whitelist[0]);
  g_assert_cmpstr (retrieved_whitelist[1], ==, demo_whitelist[1]);
  g_assert_null (retrieved_whitelist[2]);
  g_value_reset (&value);

  g_value_set_boxed (&value, boxed_whitelist);
  g_object_set_property (G_OBJECT (buildopts), "rpm-whitelist", &value);
  retrieved_whitelist = modulemd_buildopts_get_rpm_whitelist (buildopts);

  g_assert_cmpstr (retrieved_whitelist[0], ==, boxed_whitelist[0]);
  g_assert_null (retrieved_whitelist[1]);
  g_clear_pointer (&retrieved_whitelist, g_strfreev);
  g_value_reset (&value);


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
