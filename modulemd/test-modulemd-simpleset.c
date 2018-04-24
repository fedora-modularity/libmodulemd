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
_array_contains (gchar **array, const gchar *value)
{
  for (gsize i = 0; array[i]; i++)
    {
      if (g_strcmp0 (array[i], value) == 0)
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
  gchar **array = g_malloc0_n (5, sizeof (gchar *));
  gchar **array2;

  array[0] = g_strdup ("alpha");
  array[1] = g_strdup ("bravo");
  /* Add duplicate value to ensure uniqueness of resulting set */
  array[2] = g_strdup ("alpha");
  array[3] = NULL;

  /* Create the set from a strv */
  modulemd_simpleset_set (fixture->set, array);

  for (i = 0; array[i]; i++)
    {
      g_assert_true (modulemd_simpleset_contains (fixture->set, array[i]));
    }

  array2 = modulemd_simpleset_dup (fixture->set);

  for (i = 0; array[i]; i++)
    {
      g_assert_true (_array_contains (array2, array[i]));
    }

  /* Verify order */
  g_assert_cmpstr (array2[0], ==, "alpha");
  g_assert_cmpstr (array2[1], ==, "bravo");

  /* The size of the resulting set should only be two entries,
     * since one of them was a duplicate.
     */
  g_assert_cmpint (g_strv_length (array2), ==, 2);

  for (i = 0; array[i]; i++)
    {
      g_free (array[i]);
    }
  g_free (array);

  for (i = 0; array2[i]; i++)
    {
      g_free (array2[i]);
    }
  g_free (array2);
}

static void
modulemd_simpleset_test_copy (SimpleSetFixture *fixture,
                              gconstpointer user_data)
{
  gsize i = 0;
  ModulemdSimpleSet *copy = NULL;
  gchar **set = NULL;

  modulemd_simpleset_add (fixture->set, "alpha");
  modulemd_simpleset_add (fixture->set, "bravo");

  g_assert_cmpint (modulemd_simpleset_size (fixture->set), ==, 2);
  g_assert_true (modulemd_simpleset_contains (fixture->set, "alpha"));
  g_assert_true (modulemd_simpleset_contains (fixture->set, "bravo"));

  /* Test that we can copy and allocate at the same time */
  modulemd_simpleset_copy (fixture->set, &copy);

  g_assert_nonnull (copy);
  set = modulemd_simpleset_dup (copy);
  g_assert_nonnull (set);

  for (i = 0; set[i]; i++)
    {
      g_free (set[i]);
    }
  g_clear_pointer (&set, g_free);

  g_assert_cmpint (modulemd_simpleset_size (copy), ==, 2);
  g_assert_true (modulemd_simpleset_contains (copy, "alpha"));
  g_assert_true (modulemd_simpleset_contains (copy, "bravo"));

  /* Remove "alpha" and "bravo" and add "Mickey" and "Minnie" */
  modulemd_simpleset_add (fixture->set, "Mickey");
  modulemd_simpleset_add (fixture->set, "Minnie");
  modulemd_simpleset_remove (fixture->set, "alpha");
  modulemd_simpleset_remove (fixture->set, "bravo");

  g_assert_cmpint (modulemd_simpleset_size (fixture->set), ==, 2);
  g_assert_true (modulemd_simpleset_contains (fixture->set, "Mickey"));
  g_assert_true (modulemd_simpleset_contains (fixture->set, "Minnie"));
  g_assert_false (modulemd_simpleset_contains (fixture->set, "alpha"));
  g_assert_false (modulemd_simpleset_contains (fixture->set, "bravo"));

  /* Overwrite the contents of "copy" */
  modulemd_simpleset_copy (fixture->set, &copy);

  g_assert_cmpint (modulemd_simpleset_size (copy), ==, 2);
  g_assert_cmpint (modulemd_simpleset_size (copy), ==, 2);
  g_assert_true (modulemd_simpleset_contains (copy, "Mickey"));
  g_assert_true (modulemd_simpleset_contains (copy, "Minnie"));
  g_assert_false (modulemd_simpleset_contains (copy, "alpha"));
  g_assert_false (modulemd_simpleset_contains (copy, "bravo"));


  /* Copy a NULL SimpleSet */
  modulemd_simpleset_copy (NULL, &copy);

  g_assert_cmpint (modulemd_simpleset_size (copy), ==, 0);
  g_assert_false (modulemd_simpleset_contains (copy, "Mickey"));
  g_assert_false (modulemd_simpleset_contains (copy, "Minnie"));
  g_assert_false (modulemd_simpleset_contains (copy, "alpha"));
  g_assert_false (modulemd_simpleset_contains (copy, "bravo"));

  g_clear_pointer (&copy, g_object_unref);
}

static gboolean
test_validate_true (const gchar *str)
{
  return TRUE;
}

static gboolean
test_validate_false (const gchar *str)
{
  return FALSE;
}

static gboolean
test_validate_nofoo (const gchar *str)
{
  if (g_strcmp0 (str, "foo") == 0)
    {
      return FALSE;
    }
  return TRUE;
}

static void
modulemd_simpleset_test_validate (SimpleSetFixture *fixture,
                                  gconstpointer user_data)
{
  gboolean result = FALSE;
  g_autoptr (ModulemdSimpleSet) set = modulemd_simpleset_new ();
  g_autoptr (GPtrArray) failures = NULL;

  /* Add three strings to the set */
  modulemd_simpleset_add (set, "foo");
  modulemd_simpleset_add (set, "bar");
  modulemd_simpleset_add (set, "baz");

  result =
    modulemd_simpleset_validate_contents (set, test_validate_true, &failures);
  g_assert_true (result);
  g_assert_null (failures);

  result =
    modulemd_simpleset_validate_contents (set, test_validate_false, &failures);
  g_assert_false (result);
  g_assert_nonnull (failures);
  g_assert_cmpint (failures->len, ==, 3);
  g_clear_pointer (&failures, g_ptr_array_unref);

  result =
    modulemd_simpleset_validate_contents (set, test_validate_nofoo, &failures);
  g_assert_false (result);
  g_assert_nonnull (failures);
  g_assert_cmpint (failures->len, ==, 1);
  g_assert_cmpstr ((const gchar *)g_ptr_array_index (failures, 0), ==, "foo");
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

  g_test_add ("/modulemd/simpleset/test_copy",
              SimpleSetFixture,
              NULL,
              modulemd_simpleset_set_up,
              modulemd_simpleset_test_copy,
              modulemd_simpleset_tear_down);

  g_test_add ("/modulemd/simpleset/test_validate",
              SimpleSetFixture,
              NULL,
              NULL,
              modulemd_simpleset_test_validate,
              NULL);

  return g_test_run ();
}
