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
#define MMD_DISABLE_DEPRECATION_WARNINGS 1
#include "modulemd.h"
#include "private/modulemd-yaml.h"

#include <glib.h>
#include <locale.h>

typedef struct _RegressionFixture
{
} RegressionFixture;


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
  g_clear_pointer (&yaml_path, g_free);
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
  g_clear_pointer (&yaml_path, g_free);
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
  g_clear_pointer (&yaml_path, g_free);
}


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
  g_debug ("YAML dumps() content:\n%s\n", yaml);

  module2 = modulemd_module_new_from_string (yaml);
  g_assert_nonnull (module2);
  rpm_components = modulemd_module_get_rpm_components (module2);
  g_assert_cmpint (g_hash_table_size (rpm_components), >, 0);

  g_clear_pointer (&module, g_object_unref);
  g_clear_pointer (&module2, g_object_unref);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&yaml, g_free);
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
  g_clear_pointer (&yaml_content, g_free);
}

static void
modulemd_regressions_issue25 (RegressionFixture *fixture,
                              gconstpointer user_data)
{
  gchar *yaml_path;
  ModulemdModule *module = NULL;
  GHashTable *buildopts = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/issue25.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  module = modulemd_module_new_from_file (yaml_path);
  g_clear_pointer (&yaml_path, g_free);
  g_assert_nonnull (module);

  buildopts = modulemd_module_peek_rpm_buildopts (module);
  g_assert_nonnull (buildopts);
  g_assert_true (g_hash_table_contains (buildopts, "macros"));
  g_assert_cmpstr ((const gchar *)g_hash_table_lookup (buildopts, "macros"),
                   ==,
                   "%my_macro 1");

  g_object_unref (module);
}


static void
modulemd_regressions_issue26 (RegressionFixture *fixture,
                              gconstpointer user_data)
{
  gchar *yaml_path;
  ModulemdModule *module = NULL;

  /* This would segfault because we weren't checking for NULL
   * prior to attempting to free the simpleset in
   * _parse_modulemd_filters
   */
  yaml_path = g_strdup_printf ("%s/test_data/issue26.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  module = modulemd_module_new_from_file (yaml_path);
  g_assert_nonnull (module);

  g_clear_pointer (&module, g_object_unref);
  g_clear_pointer (&yaml_path, g_free);
}


static void
modulemd_regressions_issue53 (RegressionFixture *fixture,
                              gconstpointer user_data)
{
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) objects = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/issue53.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  objects = modulemd_objects_from_file (yaml_path, NULL);
  g_assert_nonnull (objects);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

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

  g_test_add ("/modulemd/regressions/issue16",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue16,
              NULL);

  g_test_add ("/modulemd/regressions/issue18",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue18,
              NULL);

  g_test_add ("/modulemd/regressions/issue25",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue25,
              NULL);

  g_test_add ("/modulemd/regressions/issue26",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue26,
              NULL);

  g_test_add ("/modulemd/regressions/issue53",
              RegressionFixture,
              NULL,
              NULL,
              modulemd_regressions_issue53,
              NULL);

  return g_test_run ();
}
