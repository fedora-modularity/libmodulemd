/* test-modulemd-module.c
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

#include "modulemd-module.h"
#include "modulemd-simpleset.h"
#include "modulemd-util.h"
#include "modulemd-yaml.h"

#include <glib.h>
#include <locale.h>

typedef struct _ModuleFixture
{
  ModulemdModule *md;
} ModuleFixture;

typedef struct _ModulePropString
{
  const gchar *property_name;
  const gchar *test_str;
} ModulePropString;

static void
modulemd_module_set_up (ModuleFixture *fixture, gconstpointer user_data)
{
  fixture->md = modulemd_module_new ();
}

static void
modulemd_module_tear_down (ModuleFixture *fixture, gconstpointer user_data)
{
  g_object_unref (fixture->md);
}

static void
modulemd_module_test_string_prop (ModuleFixture *fixture,
                                  gconstpointer user_data)
{
  GValue value = G_VALUE_INIT;
  GValue ref_value = G_VALUE_INIT;
  ModulemdModule *md = fixture->md;
  ModulePropString *prop_ctx = (ModulePropString *)user_data;

  g_value_init (&value, G_TYPE_STRING);
  g_value_init (&ref_value, G_TYPE_STRING);
  g_value_set_string (&ref_value, prop_ctx->test_str);

  g_object_get_property (G_OBJECT (md), prop_ctx->property_name, &value);

  /* Initial state should be NULL */
  g_assert_cmpstr (g_value_get_string (&value), ==, NULL);
  g_value_reset (&value);

  /* Assign the test value */
  g_object_set_property (G_OBJECT (md), prop_ctx->property_name, &ref_value);
  g_value_reset (&value);

  /* Verify that the value is now set */
  g_object_get_property (G_OBJECT (md), prop_ctx->property_name, &value);
  g_assert_cmpstr (
    g_value_get_string (&value), ==, g_value_get_string (&ref_value));

  g_value_unset (&ref_value);
  g_value_unset (&value);
}

static void
modulemd_module_test_get_set_buildrequires (ModuleFixture *fixture,
                                            gconstpointer user_data)
{
  GValue value = G_VALUE_INIT;
  GValue set_value = G_VALUE_INIT;
  ModulemdModule *md = fixture->md;
  GHashTable *htable = NULL;
  GHashTable *htable2 = NULL;
  GHashTable *buildrequires = NULL;

  /* Should be initialized to an empty hash table */

  htable = modulemd_module_get_buildrequires (md);
  buildrequires = _modulemd_hash_table_deep_str_copy (htable);
  htable = NULL;
  g_assert_cmpint (g_hash_table_size (buildrequires), ==, 0);

  /* Add a key and value using set_buildrequires() */
  g_hash_table_insert (
    buildrequires, g_strdup ("MyKey"), g_strdup ("MyValue"));
  modulemd_module_set_buildrequires (md, buildrequires);

  /* Verify the key and value with get_buildrequires() */
  htable = modulemd_module_get_buildrequires (md);
  buildrequires = _modulemd_hash_table_deep_str_copy (htable);
  htable = NULL;
  g_assert_cmpint (g_hash_table_size (buildrequires), ==, 1);
  g_assert_true (g_hash_table_contains (buildrequires, "MyKey"));
  g_assert_cmpstr (
    g_hash_table_lookup (buildrequires, "MyKey"), ==, "MyValue");

  /* Verify the key and value with properties */
  g_value_init (&value, G_TYPE_HASH_TABLE);
  g_object_get_property (G_OBJECT (md), "buildrequires", &value);
  htable = g_value_get_boxed (&value);
  g_value_reset (&value);

  g_assert_cmpint (g_hash_table_size (htable), ==, 1);
  g_assert_true (g_hash_table_contains (htable, "MyKey"));
  g_assert_cmpstr (g_hash_table_lookup (htable, "MyKey"), ==, "MyValue");

  /* Add a second key and value using set_buildrequires() */
  g_hash_table_insert (
    buildrequires, g_strdup ("MyKey2"), g_strdup ("MyValue2"));
  modulemd_module_set_buildrequires (md, buildrequires);

  /* Verify the second key and value with properties */
  g_object_get_property (G_OBJECT (md), "buildrequires", &value);
  htable2 = g_value_dup_boxed (&value);
  g_value_reset (&value);
  htable = _modulemd_hash_table_deep_str_copy (htable2);
  htable2 = NULL;

  g_assert_cmpint (g_hash_table_size (htable), ==, 2);
  g_assert_true (g_hash_table_contains (htable, "MyKey2"));
  g_assert_cmpstr (g_hash_table_lookup (htable, "MyKey2"), ==, "MyValue2");


  /* Add a third key using the properties interface */
  g_hash_table_insert (htable, g_strdup ("MyKey3"), g_strdup ("MyValue3"));
  g_assert_cmpint (g_hash_table_size (htable), ==, 3);

  g_value_init (&set_value, G_TYPE_HASH_TABLE);
  g_value_take_boxed (&set_value, htable);

  g_object_set_property (G_OBJECT (md), "buildrequires", &set_value);

  /* Verify the third key and value with get_buildrequires() */
  htable = modulemd_module_get_buildrequires (md);
  buildrequires = _modulemd_hash_table_deep_str_copy (htable);
  htable = NULL;
  g_assert_cmpint (g_hash_table_size (buildrequires), ==, 3);
  g_assert_true (g_hash_table_contains (buildrequires, "MyKey3"));
  g_assert_cmpstr (
    g_hash_table_lookup (buildrequires, "MyKey3"), ==, "MyValue3");
}

static void
modulemd_module_test_get_set_community (ModuleFixture *fixture,
                                        gconstpointer user_data)
{
  ModulemdModule *md = fixture->md;

  /* Should be initialized to NULL */
  g_assert_cmpstr (modulemd_module_get_community (md), ==, NULL);

  /* Assign a valid string */
  modulemd_module_set_community (md, "MyCommunity");
  g_assert_cmpstr (modulemd_module_get_community (md), ==, "MyCommunity");

  /* Reassign it to NULL */
  modulemd_module_set_community (md, NULL);
  g_assert_cmpstr (modulemd_module_get_community (md), ==, NULL);
}

static void
modulemd_module_test_get_set_description (ModuleFixture *fixture,
                                          gconstpointer user_data)
{
  ModulemdModule *md = fixture->md;

  /* Should be initialized to NULL */
  g_assert_cmpstr (modulemd_module_get_description (md), ==, NULL);

  /* Assign a valid string */
  modulemd_module_set_description (md, "ModuleDesc");
  g_assert_cmpstr (modulemd_module_get_description (md), ==, "ModuleDesc");

  /* Reassign it to NULL */
  modulemd_module_set_description (md, NULL);
  g_assert_cmpstr (modulemd_module_get_description (md), ==, NULL);
}

static void
modulemd_module_test_get_set_documentation (ModuleFixture *fixture,
                                            gconstpointer user_data)
{
  ModulemdModule *md = fixture->md;

  /* Should be initialized to NULL */
  g_assert_cmpstr (modulemd_module_get_documentation (md), ==, NULL);

  /* Assign a valid string */
  modulemd_module_set_documentation (md, "ModuleDocs");
  g_assert_cmpstr (modulemd_module_get_documentation (md), ==, "ModuleDocs");

  /* Reassign it to NULL */
  modulemd_module_set_documentation (md, NULL);
  g_assert_cmpstr (modulemd_module_get_documentation (md), ==, NULL);
}

static void
modulemd_module_test_get_set_mdversion (ModuleFixture *fixture,
                                        gconstpointer user_data)
{
  ModulemdModule *md = fixture->md;

  /* Should be initialized to 0 */
  g_assert_cmpuint (modulemd_module_get_mdversion (md), ==, MD_VERSION_UNSET);

  /* Assign a valid version */
  modulemd_module_set_mdversion (md, MD_VERSION_1);
  g_assert_cmpuint (modulemd_module_get_mdversion (md), ==, MD_VERSION_1);
}

static void
modulemd_module_test_get_set_name (ModuleFixture *fixture,
                                   gconstpointer user_data)
{
  ModulemdModule *md = fixture->md;

  /* Should be initialized to NULL */
  g_assert_cmpstr (modulemd_module_get_name (md), ==, NULL);

  /* Assign a valid string */
  modulemd_module_set_name (md, "ModuleName");
  g_assert_cmpstr (modulemd_module_get_name (md), ==, "ModuleName");

  /* Reassign it to NULL */
  modulemd_module_set_name (md, NULL);
  g_assert_cmpstr (modulemd_module_get_name (md), ==, NULL);
}

static void
modulemd_module_test_get_set_requires (ModuleFixture *fixture,
                                       gconstpointer user_data)
{
  GValue value = G_VALUE_INIT;
  GValue set_value = G_VALUE_INIT;
  ModulemdModule *md = fixture->md;
  GHashTable *htable = NULL;
  GHashTable *htable2 = NULL;
  GHashTable *requires = NULL;

  /* Should be initialized to an empty hash table */

  htable = modulemd_module_get_requires (md);
  requires = _modulemd_hash_table_deep_str_copy (htable);
  htable = NULL;
  g_assert_cmpint (g_hash_table_size (requires), ==, 0);

  /* Add a key and value using set_requires() */
  g_hash_table_insert (requires, g_strdup ("MyKey"), g_strdup ("MyValue"));
  modulemd_module_set_requires (md, requires);

  /* Verify the key and value with get_requires() */
  htable = modulemd_module_get_requires (md);
  requires = _modulemd_hash_table_deep_str_copy (htable);
  htable = NULL;
  g_assert_cmpint (g_hash_table_size (requires), ==, 1);
  g_assert_true (g_hash_table_contains (requires, "MyKey"));
  g_assert_cmpstr (g_hash_table_lookup (requires, "MyKey"), ==, "MyValue");

  /* Verify the key and value with properties */
  g_value_init (&value, G_TYPE_HASH_TABLE);
  g_object_get_property (G_OBJECT (md), "requires", &value);
  htable = g_value_get_boxed (&value);
  g_value_reset (&value);

  g_assert_cmpint (g_hash_table_size (htable), ==, 1);
  g_assert_true (g_hash_table_contains (htable, "MyKey"));
  g_assert_cmpstr (g_hash_table_lookup (htable, "MyKey"), ==, "MyValue");

  /* Add a second key and value using set_requires() */
  g_hash_table_insert (requires, g_strdup ("MyKey2"), g_strdup ("MyValue2"));
  modulemd_module_set_requires (md, requires);

  /* Verify the second key and value with properties */
  g_object_get_property (G_OBJECT (md), "requires", &value);
  htable2 = g_value_dup_boxed (&value);
  g_value_reset (&value);
  htable = _modulemd_hash_table_deep_str_copy (htable2);
  htable2 = NULL;

  g_assert_cmpint (g_hash_table_size (htable), ==, 2);
  g_assert_true (g_hash_table_contains (htable, "MyKey2"));
  g_assert_cmpstr (g_hash_table_lookup (htable, "MyKey2"), ==, "MyValue2");


  /* Add a third key using the properties interface */
  g_hash_table_insert (htable, g_strdup ("MyKey3"), g_strdup ("MyValue3"));
  g_assert_cmpint (g_hash_table_size (htable), ==, 3);

  g_value_init (&set_value, G_TYPE_HASH_TABLE);
  g_value_take_boxed (&set_value, htable);

  g_object_set_property (G_OBJECT (md), "requires", &set_value);

  /* Verify the third key and value with get_requires() */
  htable = modulemd_module_get_requires (md);
  requires = _modulemd_hash_table_deep_str_copy (htable);
  htable = NULL;
  g_assert_cmpint (g_hash_table_size (requires), ==, 3);
  g_assert_true (g_hash_table_contains (requires, "MyKey3"));
  g_assert_cmpstr (g_hash_table_lookup (requires, "MyKey3"), ==, "MyValue3");
}

static void
modulemd_module_test_get_set_stream (ModuleFixture *fixture,
                                     gconstpointer user_data)
{
  ModulemdModule *md = fixture->md;

  /* Should be initialized to NULL */
  g_assert_cmpstr (modulemd_module_get_stream (md), ==, NULL);

  /* Assign a valid string */
  modulemd_module_set_stream (md, "ModuleStream");
  g_assert_cmpstr (modulemd_module_get_stream (md), ==, "ModuleStream");

  /* Reassign it to NULL */
  modulemd_module_set_stream (md, NULL);
  g_assert_cmpstr (modulemd_module_get_stream (md), ==, NULL);
}

static void
modulemd_module_test_get_set_summary (ModuleFixture *fixture,
                                      gconstpointer user_data)
{
  ModulemdModule *md = fixture->md;

  /* Should be initialized to NULL */
  g_assert_cmpstr (modulemd_module_get_summary (md), ==, NULL);

  /* Assign a valid string */
  modulemd_module_set_summary (md, "ModuleSummary");
  g_assert_cmpstr (modulemd_module_get_summary (md), ==, "ModuleSummary");

  /* Reassign it to NULL */
  modulemd_module_set_summary (md, NULL);
  g_assert_cmpstr (modulemd_module_get_summary (md), ==, NULL);
}

static void
modulemd_module_test_get_set_tracker (ModuleFixture *fixture,
                                      gconstpointer user_data)
{
  ModulemdModule *md = fixture->md;

  /* Should be initialized to NULL */
  g_assert_cmpstr (modulemd_module_get_tracker (md), ==, NULL);

  /* Assign a valid string */
  modulemd_module_set_tracker (md, "ModuleTracker");
  g_assert_cmpstr (modulemd_module_get_tracker (md), ==, "ModuleTracker");

  /* Reassign it to NULL */
  modulemd_module_set_tracker (md, NULL);
  g_assert_cmpstr (modulemd_module_get_tracker (md), ==, NULL);
}

static void
modulemd_module_test_get_set_version (ModuleFixture *fixture,
                                      gconstpointer user_data)
{
  ModulemdModule *md = fixture->md;

  /* Should be initialized to 0 */
  g_assert_cmpuint (modulemd_module_get_version (md), ==, 0);

  /* Assign a valid version */
  modulemd_module_set_version (md, 1);
  g_assert_cmpuint (modulemd_module_get_version (md), ==, 1);

  /* Reassign it to 0 */
  modulemd_module_set_version (md, 0);
  g_assert_cmpuint (modulemd_module_get_version (md), ==, 0);
}

static void
modulemd_module_test_get_set_dependencies (ModuleFixture *fixture,
                                           gconstpointer user_data)
{
  ModulemdDependencies *dep = NULL;
  const GPtrArray *deps = NULL;
  const gchar **platforms = g_new0 (const gchar *, 3);
  platforms[0] = "f27";
  platforms[1] = "f28";

  modulemd_module_set_mdversion (fixture->md, 2);

  dep = modulemd_dependencies_new ();
  modulemd_dependencies_add_buildrequires (dep, "platform", platforms);
  modulemd_dependencies_add_requires (dep, "platform", platforms);

  modulemd_module_add_dependencies (fixture->md, dep);
  modulemd_module_add_dependencies (fixture->md, dep);

  deps = modulemd_module_get_dependencies (fixture->md);
  g_assert_nonnull (deps);
  g_assert_cmpint (deps->len, ==, 2);

  /* We've previously had a bug where repeated get() calls were unrefing
   * values, so make sure that doesn't reappear
   */
  deps = modulemd_module_get_dependencies (fixture->md);
  g_assert_nonnull (deps);
  g_assert_cmpint (deps->len, ==, 2);

  g_free (platforms);
}

static void
modulemd_module_test_get_set_xmd (ModuleFixture *fixture,
                                  gconstpointer user_data)
{
  GValue value = G_VALUE_INIT;
  GValue *set_value;
  ModulemdModule *md = fixture->md;
  GHashTable *htable;

  /* Should be initialized to an empty hash table */
  GHashTable *xmd = modulemd_module_get_xmd (md);
  g_assert_cmpint (g_hash_table_size (xmd), ==, 0);

  /* Add a key and value using set_xmd() */
  g_hash_table_insert (xmd, g_strdup ("MyKey"), g_strdup ("MyValue"));
  modulemd_module_set_xmd (md, xmd);

  /* Verify the key and value with get_xmd() */
  xmd = modulemd_module_get_xmd (md);
  g_assert_cmpint (g_hash_table_size (xmd), ==, 1);
  g_assert_true (g_hash_table_contains (xmd, "MyKey"));
  g_assert_cmpstr (g_hash_table_lookup (xmd, "MyKey"), ==, "MyValue");

  /* Verify the key and value with properties */
  g_value_init (&value, G_TYPE_HASH_TABLE);
  g_object_get_property (G_OBJECT (md), "xmd", &value);
  htable = g_value_get_boxed (&value);
  g_value_reset (&value);

  g_assert_cmpint (g_hash_table_size (htable), ==, 1);
  g_assert_true (g_hash_table_contains (htable, "MyKey"));
  g_assert_cmpstr (g_hash_table_lookup (htable, "MyKey"), ==, "MyValue");

  /* Add a second key and value using set_xmd() */
  g_hash_table_insert (xmd, g_strdup ("MyKey2"), g_strdup ("MyValue2"));
  modulemd_module_set_xmd (md, xmd);

  /* Verify the second key and value with properties */
  g_object_get_property (G_OBJECT (md), "xmd", &value);
  htable = g_value_get_boxed (&value);
  g_value_reset (&value);

  g_assert_cmpint (g_hash_table_size (htable), ==, 2);
  g_assert_true (g_hash_table_contains (htable, "MyKey2"));
  g_assert_cmpstr (g_hash_table_lookup (htable, "MyKey2"), ==, "MyValue2");


  /* Add a third key using the properties interface */
  g_hash_table_insert (htable, g_strdup ("MyKey3"), g_strdup ("MyValue3"));

  set_value = g_new0 (GValue, 1);
  g_value_init (set_value, G_TYPE_HASH_TABLE);
  g_value_set_boxed (set_value, htable);
  modulemd_module_set_xmd (md, htable);

  g_object_set_property (G_OBJECT (md), "xmd", set_value);

  /* Verify the third key and value with get_xmd() */
  xmd = modulemd_module_get_xmd (md);
  g_assert_cmpint (g_hash_table_size (xmd), ==, 3);
  g_assert_true (g_hash_table_contains (xmd, "MyKey3"));
  g_assert_cmpstr (g_hash_table_lookup (xmd, "MyKey3"), ==, "MyValue3");
}

static void
modulemd_module_test_construct_v1 (ModuleFixture *fixture,
                                   gconstpointer user_data)
{
  ModulemdModule **modules = NULL;
  ModulemdSimpleSet *licenses = NULL;
  GError *error = NULL;
  gchar *yaml = NULL;
  gboolean result;

  /* Add mdversion (required) */
  modulemd_module_set_mdversion (fixture->md, 1);

  /* Add summary (required) */
  modulemd_module_set_summary (fixture->md, "The summary");

  /* Add description (required) */
  modulemd_module_set_description (fixture->md, "The description");

  /* Add module license (required) */
  licenses = modulemd_simpleset_new ();
  modulemd_simpleset_add (licenses, "MIT");
  modulemd_module_set_module_licenses (fixture->md, licenses);

  /* Dump it to YAML to validate it */
  modules = g_new0 (ModulemdModule *, 1);
  g_assert_nonnull (modules);
  modules[0] = fixture->md;

  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_true (result);
  g_assert_nonnull (yaml);

  g_message ("v1 YAML:\n%s", yaml);

  g_free (modules);
}


static void
modulemd_module_test_construct_v2 (ModuleFixture *fixture,
                                   gconstpointer user_data)
{
  ModulemdModule **modules = NULL;
  ModulemdSimpleSet *licenses = NULL;
  GError *error = NULL;
  gchar *yaml = NULL;
  gboolean result;

  modules = g_new0 (ModulemdModule *, 1);
  g_assert_nonnull (modules);
  modules[0] = fixture->md;

  /* Verify that it fails when mdversion is unset */
  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_false (result);
  g_clear_pointer (&yaml, g_free);
  g_clear_error (&error);

  /* Add mdversion (required) */
  modulemd_module_set_mdversion (fixture->md, 2);

  /* Verify that it fails when summary is unset */
  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_false (result);
  g_clear_pointer (&yaml, g_free);
  g_clear_error (&error);

  /* Add summary (required) */
  modulemd_module_set_summary (fixture->md, "The summary");

  /* Verify that it fails when description is unset */
  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_false (result);
  g_clear_pointer (&yaml, g_free);
  g_clear_error (&error);

  /* Add description (required) */
  modulemd_module_set_description (fixture->md, "The description");

  /* Verify that it fails when module license is unset */
  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_false (result);
  g_clear_pointer (&yaml, g_free);
  g_clear_error (&error);

  /* Add module license (required) */
  licenses = modulemd_simpleset_new ();
  modulemd_simpleset_add (licenses, "MIT");
  modulemd_module_set_module_licenses (fixture->md, licenses);

  /* Dump it to YAML to validate it */
  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_true (result);
  g_assert_nonnull (yaml);

  g_message ("v2 YAML:\n%s", yaml);

  g_free (modules);
  g_clear_pointer (&yaml, g_free);
}

static void
modulemd_module_test_upgrade_v2 (ModuleFixture *fixture,
                                 gconstpointer user_data)
{
  ModulemdModule **modules = NULL;
  ModulemdSimpleSet *licenses = NULL;
  GError *error = NULL;
  gchar *yaml = NULL;
  gboolean result = FALSE;
  GDate *eol = NULL;
  GHashTable *servicelevels = NULL;
  GHashTable *v1_deps = NULL;
  GPtrArray *v2_deps = NULL;


  /* Add mdversion (required) */
  modulemd_module_set_mdversion (fixture->md, 1);

  /* Add summary (required) */
  modulemd_module_set_summary (fixture->md, "The summary");

  /* Add description (required) */
  modulemd_module_set_description (fixture->md, "The description");

  /* Add module license (required) */
  licenses = modulemd_simpleset_new ();
  modulemd_simpleset_add (licenses, "MIT");
  modulemd_module_set_module_licenses (fixture->md, licenses);

  /* Add EOL value */
  eol = g_date_new_dmy (3, 10, 2077);
  modulemd_module_set_eol (fixture->md, eol);

  /* There should be no "rawhide" service level yet */
  servicelevels = modulemd_module_get_servicelevels (fixture->md);
  g_assert_false (g_hash_table_contains (servicelevels, "rawhide"));
  servicelevels = NULL;

  v1_deps = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  g_hash_table_insert (v1_deps, g_strdup ("platform"), g_strdup ("f28"));

  /* Add a BuildRequires */
  modulemd_module_set_buildrequires (fixture->md, v1_deps);

  /* Add a runtime Requires */
  modulemd_module_set_requires (fixture->md, v1_deps);

  /* Upgrade to v2 */
  result = modulemd_module_upgrade (fixture->md);
  g_assert_true (result);

  g_assert_cmpuint (modulemd_module_get_mdversion (fixture->md), ==, 2);

  /* The module should now contain an entry for rawhide */
  servicelevels = modulemd_module_get_servicelevels (fixture->md);
  g_assert_true (g_hash_table_contains (servicelevels, "rawhide"));
  servicelevels = NULL;


  /* The module should now contain a single entry in the dependencies array */
  v2_deps = modulemd_module_get_dependencies (fixture->md);
  g_assert_nonnull (v2_deps);
  g_assert_cmpuint (v2_deps->len, ==, 1);

  /* Dump it to YAML to validate it */
  modules = g_new0 (ModulemdModule *, 1);
  g_assert_nonnull (modules);
  modules[0] = fixture->md;

  result = emit_yaml_string (modules, &yaml, &error);
  g_assert_true (result);
  g_assert_nonnull (yaml);

  g_message ("Upgraded YAML:\n%s", yaml);

  g_free (modules);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/module/test_get_set_buildrequires",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_buildrequires,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_community",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_community,
              modulemd_module_tear_down);

  ModulePropString community;
  community.property_name = "community";
  community.test_str = "MyCommunity";
  g_test_add ("/modulemd/module/test_prop_community",
              ModuleFixture,
              &community,
              modulemd_module_set_up,
              modulemd_module_test_string_prop,
              modulemd_module_tear_down);


  g_test_add ("/modulemd/module/test_get_set_description",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_description,
              modulemd_module_tear_down);

  ModulePropString desc;
  desc.property_name = "description";
  desc.test_str = "MyDescription";
  g_test_add ("/modulemd/module/test_prop_description",
              ModuleFixture,
              &desc,
              modulemd_module_set_up,
              modulemd_module_test_string_prop,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_documentation",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_documentation,
              modulemd_module_tear_down);

  ModulePropString doc;
  doc.property_name = "documentation";
  doc.test_str = "MyDocumentation";
  g_test_add ("/modulemd/module/test_prop_documentation",
              ModuleFixture,
              &doc,
              modulemd_module_set_up,
              modulemd_module_test_string_prop,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_mdversion",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_mdversion,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_name",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_name,
              modulemd_module_tear_down);

  ModulePropString name;
  name.property_name = "name";
  name.test_str = "MyName";
  g_test_add ("/modulemd/module/test_prop_name",
              ModuleFixture,
              &name,
              modulemd_module_set_up,
              modulemd_module_test_string_prop,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_requires",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_requires,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_stream",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_stream,
              modulemd_module_tear_down);

  ModulePropString stream;
  stream.property_name = "stream";
  stream.test_str = "MyStream";
  g_test_add ("/modulemd/module/test_prop_stream",
              ModuleFixture,
              &stream,
              modulemd_module_set_up,
              modulemd_module_test_string_prop,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_summary",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_summary,
              modulemd_module_tear_down);

  ModulePropString summary;
  summary.property_name = "summary";
  summary.test_str = "MySummary";
  g_test_add ("/modulemd/module/test_prop_summary",
              ModuleFixture,
              &summary,
              modulemd_module_set_up,
              modulemd_module_test_string_prop,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_tracker",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_tracker,
              modulemd_module_tear_down);

  ModulePropString tracker;
  tracker.property_name = "tracker";
  tracker.test_str = "MyTracker";
  g_test_add ("/modulemd/module/test_prop_tracker",
              ModuleFixture,
              &tracker,
              modulemd_module_set_up,
              modulemd_module_test_string_prop,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_version",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_version,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_xmd",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_xmd,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_get_set_dependencies",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_get_set_dependencies,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_construct_v1",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_construct_v1,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/test_construct_v2",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_construct_v2,
              modulemd_module_tear_down);

  g_test_add ("/modulemd/module/modulemd_module_test_upgrade_v2",
              ModuleFixture,
              NULL,
              modulemd_module_set_up,
              modulemd_module_test_upgrade_v2,
              modulemd_module_tear_down);


  return g_test_run ();
}
