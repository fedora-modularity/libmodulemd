/* test-modulemd-simpleset.c
 *
 * Copyright (C) 2017 Stephen Gallagher
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

#include "modulemd-simpleset.h"

#include <glib.h>
#include <locale.h>

typedef struct _SimpleSetFixture
{
  ModulemdSimpleSet *set;
} SimpleSetFixture;

static void
modulemd_simpleset_set_up (SimpleSetFixture *fixture, gconstpointer user_data)
{
  fixture->set = modulemd_simpleset_new ();
}

static void
modulemd_simpleset_tear_down (SimpleSetFixture *fixture,
                              gconstpointer user_data)
{
  g_object_unref (fixture->set);
}

static gboolean
_array_contains (GPtrArray *array, const gchar *value)
{
  for (gsize i = 0; i < array->len; i++)
    {
      if (g_strcmp0 (g_ptr_array_index (array, i), value) == 0)
        {
          return TRUE;
        }
    }
  return FALSE;
}

static void
modulemd_simpleset_test_get_set (SimpleSetFixture *fixture,
                                 gconstpointer user_data)
{
  gsize i = 0;
  GPtrArray *array = g_ptr_array_new_full (3, g_free);
  GPtrArray *array2;
  gchar *entry;

  entry = g_strdup ("a");
  g_ptr_array_add (array, entry);

  entry = g_strdup ("b");
  g_ptr_array_add (array, entry);

  /* Add duplicate value to ensure uniqueness of resulting set */
  entry = g_strdup ("a");
  g_ptr_array_add (array, entry);

  /* Create the set from a strv */
  modulemd_simpleset_set (fixture->set, array);

  for (i = 0; i < array->len; i++)
    {
      g_assert_true (modulemd_simpleset_contains (
        fixture->set, g_ptr_array_index (array, i)));
    }

  array2 = modulemd_simpleset_get (fixture->set);

  for (i = 0; i < array->len; i++)
    {
      g_assert_true (_array_contains (array2, g_ptr_array_index (array, i)));
    }


  /* The size of the resulting set should only be two entries,
     * since one of them was a duplicate.
     */
  g_assert_cmpint (array2->len, ==, 2);
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.
  g_test_add ("/modulemd/simpleset/test_get_set",
              SimpleSetFixture,
              NULL,
              modulemd_simpleset_set_up,
              modulemd_simpleset_test_get_set,
              modulemd_simpleset_tear_down);

  return g_test_run ();
}
