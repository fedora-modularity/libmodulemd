/* test-modulemd-module.c
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

#include <glib.h>
#include <locale.h>

#include "modulemd.h"
#include "modulemd-yaml.h"

typedef struct _RegressionFixture
{
} RegressionFixture;

static void
modulemd_regressions_issue16 (RegressionFixture *fixture,
                              gconstpointer user_data)
{
  gchar *yaml_path = NULL;
  ModulemdModule *module = NULL;
  ModulemdModule *module2 = NULL;
  GHashTable *rpm_components = NULL;
  gchar *yaml = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/issue16.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  module = modulemd_module_new_from_file (yaml_path);
  g_assert_nonnull (module);
  rpm_components = modulemd_module_get_rpm_components (module);
  g_assert_cmpint (g_hash_table_size (rpm_components), >, 0);

  yaml = modulemd_module_dumps (module);
  g_message ("YAML dumps() content:\n%s\n", yaml);

  module2 = modulemd_module_new_from_string (yaml);
  g_assert_nonnull (module2);
  rpm_components = modulemd_module_get_rpm_components (module2);
  g_assert_cmpint (g_hash_table_size (rpm_components), >, 0);

  g_clear_pointer (&module, g_object_unref);
}

static void
modulemd_regressions_issue14_v1 (RegressionFixture *fixture,
                                 gconstpointer user_data)
{
  gchar *yaml_path = NULL;
  ModulemdModule *module = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/issue14-v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  module = modulemd_module_new_from_file (yaml_path);
  g_assert_nonnull (module);
  g_clear_pointer (&module, g_object_unref);
}

static void
modulemd_regressions_issue14_v2 (RegressionFixture *fixture,
                                 gconstpointer user_data)
{
  gchar *yaml_path = NULL;
  ModulemdModule *module = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/issue14-v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  module = modulemd_module_new_from_file (yaml_path);
  g_assert_nonnull (module);
  g_clear_pointer (&module, g_object_unref);
}

static void
modulemd_regressions_issue14_mismatch (RegressionFixture *fixture,
                                       gconstpointer user_data)
{
  gchar *yaml_path = NULL;
  ModulemdModule *module = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/issue14-mismatch.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  module = modulemd_module_new_from_file (yaml_path);
  g_assert_null (module);
  g_clear_pointer (&module, g_object_unref);
}

static void
modulemd_regressions_issue18 (RegressionFixture *fixture,
                              gconstpointer user_data)
{
  gchar *yaml_content = NULL;
  ModulemdModule *module = NULL;

  yaml_content = g_strdup ("document: modulemd\nBad YAML");
  module = modulemd_module_new_from_string (yaml_content);
  g_assert_null (module);
  g_clear_pointer (&module, g_object_unref);
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.
  g_test_add ("/modulemd/regressions/issue16",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue16,
              NULL);

  g_test_add ("/modulemd/regressions/issue14_v1",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue14_v1,
              NULL);

  g_test_add ("/modulemd/regressions/issue14_v2",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue14_v2,
              NULL);

  g_test_add ("/modulemd/regressions/issue14_mismatch",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue14_mismatch,
              NULL);

  g_test_add ("/modulemd/regressions/issue18",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue18,
              NULL);

  return g_test_run ();
}
