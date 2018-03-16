/* test-modulemd-simpleset.c
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
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
