/* test-modulemd-dependencies.c
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

#include "modulemd.h"

#include <glib.h>
#include <locale.h>

typedef struct _DependenciesFixture
{
  ModulemdDependencies *dep;
} DependenciesFixture;

static void
modulemd_dependencies_set_up (DependenciesFixture *fixture,
                              gconstpointer user_data)
{
  fixture->dep = modulemd_dependencies_new ();
}

static void
modulemd_dependencies_tear_down (DependenciesFixture *fixture,
                                 gconstpointer user_data)
{
  g_object_unref (fixture->dep);
}

typedef void (*DepSetSingleFn) (ModulemdDependencies *dep,
                                const gchar *module,
                                const gchar *stream);
typedef void (*DepSetMultiFn) (ModulemdDependencies *dep,
                               const gchar *module,
                               const gchar **streams);
typedef GHashTable *(*DepGetFn) (ModulemdDependencies *dep);

static void
_modulemd_dependencies_test_get_set (DependenciesFixture *fixture,
                                     gconstpointer user_data,
                                     DepSetSingleFn dep_set_single_fn,
                                     DepSetMultiFn dep_set_multi_fn,
                                     DepGetFn dep_get_fn)
{
  gboolean copied = *(gboolean *)user_data;
  const gchar **streams = g_new0 (const gchar *, 3);
  ModulemdSimpleSet *platform = NULL;
  ModulemdSimpleSet *empty = NULL;
  GList *keys = NULL;
  GHashTable *deptable = NULL;

  /* Set an empty stream for requires */
  dep_set_multi_fn (fixture->dep, "empty", streams);

  /* Verify that we have one key in the dictionary */
  deptable = dep_get_fn (fixture->dep);
  keys = g_hash_table_get_keys (deptable);
  g_assert_cmpuint (g_list_length (keys), ==, 1);
  g_list_free (keys);
  if (copied)
    g_hash_table_unref (deptable);

  /* Verify that this key contains no streams */
  deptable = dep_get_fn (fixture->dep);
  empty = g_hash_table_lookup (deptable, "empty");
  g_assert_nonnull (empty);
  g_assert_cmpuint (modulemd_simpleset_size (empty), ==, 0);
  if (copied)
    g_hash_table_unref (deptable);

  /* Set a single stream as a requires */
  dep_set_single_fn (fixture->dep, "platform", "f28");

  /* Verify that we have two keys in the dictionary */
  deptable = dep_get_fn (fixture->dep);
  keys = g_hash_table_get_keys (deptable);
  g_assert_cmpuint (g_list_length (keys), ==, 2);
  g_list_free (keys);
  if (copied)
    g_hash_table_unref (deptable);

  /* Set multiple streams as requires */
  streams[0] = g_strdup ("f29");
  streams[1] = g_strdup ("-f30");
  dep_set_multi_fn (fixture->dep, "platform", streams);

  /* Check that each of the expected values are present in the list */
  deptable = dep_get_fn (fixture->dep);
  platform = g_hash_table_lookup (deptable, "platform");
  g_assert_nonnull (platform);

  g_assert_true (modulemd_simpleset_contains (platform, "f28"));
  g_assert_true (modulemd_simpleset_contains (platform, "f29"));
  g_assert_true (modulemd_simpleset_contains (platform, "-f30"));

  g_assert_cmpuint (modulemd_simpleset_size (platform), ==, 3);

  /* Verify that we still have two keys in the dictionary */
  keys = g_hash_table_get_keys (deptable);
  g_assert_cmpuint (g_list_length (keys), ==, 2);
  g_list_free (keys);
  if (copied)
    g_hash_table_unref (deptable);

  /* Add duplicates */
  dep_set_multi_fn (fixture->dep, "platform", streams);

  /* Verify that the list hasn't changed */
  deptable = dep_get_fn (fixture->dep);
  platform = g_hash_table_lookup (deptable, "platform");
  g_assert_nonnull (platform);

  g_assert_true (modulemd_simpleset_contains (platform, "f28"));
  g_assert_true (modulemd_simpleset_contains (platform, "f29"));
  g_assert_true (modulemd_simpleset_contains (platform, "-f30"));

  g_assert_cmpuint (modulemd_simpleset_size (platform), ==, 3);
  if (copied)
    g_hash_table_unref (deptable);

  /* Verify that we still have two keys in the dictionary */
  deptable = dep_get_fn (fixture->dep);
  keys = g_hash_table_get_keys (deptable);
  g_assert_cmpuint (g_list_length (keys), ==, 2);
  g_list_free (keys);
  if (copied)
    g_hash_table_unref (deptable);

  g_clear_pointer (&streams[0], g_free);
  g_clear_pointer (&streams[1], g_free);
  g_clear_pointer (&streams, g_free);
}

static void
modulemd_dependencies_test_get_set_buildrequires (DependenciesFixture *fixture,
                                                  gconstpointer user_data)
{
  _modulemd_dependencies_test_get_set (
    fixture,
    user_data,
    modulemd_dependencies_add_buildrequires_single,
    modulemd_dependencies_add_buildrequires,
    modulemd_dependencies_get_buildrequires);
}

static void
modulemd_dependencies_test_get_set_requires (DependenciesFixture *fixture,
                                             gconstpointer user_data)
{
  _modulemd_dependencies_test_get_set (
    fixture,
    user_data,
    modulemd_dependencies_add_requires_single,
    modulemd_dependencies_add_requires,
    modulemd_dependencies_get_requires);
}


static void
modulemd_dependencies_test_dup_set_buildrequires (DependenciesFixture *fixture,
                                                  gconstpointer user_data)
{
  _modulemd_dependencies_test_get_set (
    fixture,
    user_data,
    modulemd_dependencies_add_buildrequires_single,
    modulemd_dependencies_add_buildrequires,
    modulemd_dependencies_dup_buildrequires);
}

static void
modulemd_dependencies_test_dup_set_requires (DependenciesFixture *fixture,
                                             gconstpointer user_data)
{
  _modulemd_dependencies_test_get_set (
    fixture,
    user_data,
    modulemd_dependencies_add_requires_single,
    modulemd_dependencies_add_requires,
    modulemd_dependencies_dup_requires);
}


int
main (int argc, char *argv[])
{
  gboolean copied = TRUE;
  gboolean uncopied = FALSE;
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.
  g_test_add ("/modulemd/dependencies/test_dependencies_buildrequires",
              DependenciesFixture,
              &uncopied,
              modulemd_dependencies_set_up,
              modulemd_dependencies_test_get_set_buildrequires,
              modulemd_dependencies_tear_down);
  g_test_add ("/modulemd/dependencies/test_dependencies_requires",
              DependenciesFixture,
              &uncopied,
              modulemd_dependencies_set_up,
              modulemd_dependencies_test_get_set_requires,
              modulemd_dependencies_tear_down);

  g_test_add ("/modulemd/dependencies/test_dependencies_buildrequires_dup",
              DependenciesFixture,
              &copied,
              modulemd_dependencies_set_up,
              modulemd_dependencies_test_dup_set_buildrequires,
              modulemd_dependencies_tear_down);
  g_test_add ("/modulemd/dependencies/test_dependencies_requires_dup",
              DependenciesFixture,
              &copied,
              modulemd_dependencies_set_up,
              modulemd_dependencies_test_dup_set_requires,
              modulemd_dependencies_tear_down);

  return g_test_run ();
}
