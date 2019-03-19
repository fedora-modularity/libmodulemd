/*
 * This file is part of libmodulemd
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <signal.h>

#include "modulemd-buildopts.h"
#include "private/glib-extensions.h"
#include "private/modulemd-buildopts-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _BuildoptsFixture
{
} BuildoptsFixture;

static void
buildopts_test_construct (BuildoptsFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdBuildopts) b = NULL;
  g_auto (GStrv) whitelist;

  /* Test that the new() function works */
  b = modulemd_buildopts_new ();
  g_assert_nonnull (b);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b));
  g_assert_null (modulemd_buildopts_get_rpm_macros (b));
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b);
  g_assert_nonnull (whitelist);
  g_assert_cmpint (g_strv_length (whitelist), ==, 0);
  g_clear_object (&b);

  /* Test that object instantiation works */
  b = g_object_new (MODULEMD_TYPE_BUILDOPTS, NULL);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b));
  g_clear_object (&b);

  /* Test instantiation works with rpm_macros */
  b = g_object_new (MODULEMD_TYPE_BUILDOPTS, "rpm_macros", "A test", NULL);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b));
  g_assert_cmpstr (modulemd_buildopts_get_rpm_macros (b), ==, "A test");
  g_clear_object (&b);
}


static void
buildopts_test_equals (BuildoptsFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdBuildopts) b_1 = NULL;
  g_autoptr (ModulemdBuildopts) b_2 = NULL;

  /*Test 2 objects with no rpm_macros*/
  b_1 = modulemd_buildopts_new ();
  g_assert_nonnull (b_1);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_1));

  b_2 = modulemd_buildopts_new ();
  g_assert_nonnull (b_2);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_2));

  g_assert_true (modulemd_buildopts_equals (b_1, b_2));
  g_clear_object (&b_1);
  g_clear_object (&b_2);

  /*Test 2 objects with matching rpm_macros*/
  b_1 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_1, "a test");
  g_assert_nonnull (b_1);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_1));

  b_2 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_2, "a test");
  g_assert_nonnull (b_2);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_2));

  g_assert_true (modulemd_buildopts_equals (b_1, b_2));
  g_clear_object (&b_1);
  g_clear_object (&b_2);

  /*Test 2 objects with different rpm_macros*/
  b_1 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_1, "a test");
  g_assert_nonnull (b_1);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_1));

  b_2 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_2, "b test");
  g_assert_nonnull (b_2);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_2));

  g_assert_false (modulemd_buildopts_equals (b_1, b_2));
  g_clear_object (&b_1);
  g_clear_object (&b_2);

  /*Test 2 objects with matching rpm_macros and whitelist*/
  b_1 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_1, "a test");
  modulemd_buildopts_add_rpm_to_whitelist (b_1, "testrpm");
  g_assert_nonnull (b_1);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_1));

  b_2 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_2, "a test");
  modulemd_buildopts_add_rpm_to_whitelist (b_2, "testrpm");
  g_assert_nonnull (b_2);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_2));

  g_assert_true (modulemd_buildopts_equals (b_1, b_2));
  g_clear_object (&b_1);
  g_clear_object (&b_2);

  /*Test 2 objects with matching rpm_macros and different whitelist*/
  b_1 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_1, "a test");
  modulemd_buildopts_add_rpm_to_whitelist (b_1, "testrpm");
  g_assert_nonnull (b_1);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_1));

  b_2 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_2, "a test");
  modulemd_buildopts_add_rpm_to_whitelist (b_2, "testing");
  g_assert_nonnull (b_2);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_2));

  g_assert_false (modulemd_buildopts_equals (b_1, b_2));
  g_clear_object (&b_1);
  g_clear_object (&b_2);

  /*Test 2 objects with matching rpm_macros and subsets of matching whitelist*/
  b_1 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_1, "a test");
  modulemd_buildopts_add_rpm_to_whitelist (b_1, "a");
  modulemd_buildopts_add_rpm_to_whitelist (b_1, "b");
  g_assert_nonnull (b_1);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_1));

  b_2 = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b_2, "a test");
  modulemd_buildopts_add_rpm_to_whitelist (b_2, "a");
  modulemd_buildopts_add_rpm_to_whitelist (b_2, "b");
  modulemd_buildopts_add_rpm_to_whitelist (b_2, "c");
  g_assert_nonnull (b_2);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_2));

  g_assert_false (modulemd_buildopts_equals (b_1, b_2));
  g_clear_object (&b_1);
  g_clear_object (&b_2);
}


static void
buildopts_test_copy (BuildoptsFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdBuildopts) b = NULL;
  g_autoptr (ModulemdBuildopts) b_copy = NULL;
  g_auto (GStrv) whitelist;

  b = modulemd_buildopts_new ();
  g_assert_nonnull (b);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b));
  g_assert_null (modulemd_buildopts_get_rpm_macros (b));
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b);
  g_assert_nonnull (whitelist);
  g_assert_cmpint (g_strv_length (whitelist), ==, 0);
  g_clear_pointer (&whitelist, g_strfreev);

  b_copy = modulemd_buildopts_copy (b);
  g_assert_nonnull (b_copy);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_copy));
  g_assert_null (modulemd_buildopts_get_rpm_macros (b_copy));
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b_copy);
  g_assert_nonnull (whitelist);
  g_assert_cmpint (g_strv_length (whitelist), ==, 0);
  g_clear_pointer (&whitelist, g_strfreev);
  g_clear_object (&b);
  g_clear_object (&b_copy);

  /* Test copying buildopts with rpm_macros */
  b = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (b, "a test");
  g_assert_nonnull (b);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b));
  g_assert_cmpstr (modulemd_buildopts_get_rpm_macros (b), ==, "a test");
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b);
  g_assert_nonnull (whitelist);
  g_assert_cmpint (g_strv_length (whitelist), ==, 0);
  g_clear_pointer (&whitelist, g_strfreev);

  b_copy = modulemd_buildopts_copy (b);
  g_assert_nonnull (b_copy);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_copy));
  g_assert_cmpstr (modulemd_buildopts_get_rpm_macros (b_copy), ==, "a test");
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b);
  g_assert_nonnull (whitelist);
  g_assert_cmpint (g_strv_length (whitelist), ==, 0);
  g_clear_pointer (&whitelist, g_strfreev);
  g_clear_object (&b);
  g_clear_object (&b_copy);

  /* Test copying buildopts with whitelist */
  b = modulemd_buildopts_new ();
  modulemd_buildopts_add_rpm_to_whitelist (b, "testrpm");
  g_assert_nonnull (b);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b));
  g_assert_null (modulemd_buildopts_get_rpm_macros (b));
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b);
  g_assert_nonnull (whitelist);
  g_assert_cmpint (g_strv_length (whitelist), ==, 1);
  g_assert_cmpstr (whitelist[0], ==, "testrpm");
  g_clear_pointer (&whitelist, g_strfreev);

  b_copy = modulemd_buildopts_copy (b);
  g_assert_nonnull (b_copy);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b_copy));
  g_assert_null (modulemd_buildopts_get_rpm_macros (b_copy));
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b_copy);
  g_assert_nonnull (whitelist);
  g_assert_cmpint (g_strv_length (whitelist), ==, 1);
  g_assert_cmpstr (whitelist[0], ==, "testrpm");
  g_clear_pointer (&whitelist, g_strfreev);
  g_clear_object (&b);
  g_clear_object (&b_copy);
}


static void
buildopts_test_get_set_rpm_macros (BuildoptsFixture *fixture,
                                   gconstpointer user_data)
{
  g_autoptr (ModulemdBuildopts) b = NULL;
  g_autofree gchar *rpm_macros;

  b = modulemd_buildopts_new ();
  g_assert_nonnull (b);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b));

  g_assert_null (modulemd_buildopts_get_rpm_macros (b));
  g_object_get (b, "rpm_macros", &rpm_macros, NULL);
  g_assert_null (rpm_macros);
  g_clear_pointer (&rpm_macros, g_free);

  /* Set rpm macros */
  modulemd_buildopts_set_rpm_macros (b, "Some macro");
  g_assert_cmpstr (modulemd_buildopts_get_rpm_macros (b), ==, "Some macro");
  g_object_get (b, "rpm_macros", &rpm_macros, NULL);
  g_assert_cmpstr (rpm_macros, ==, "Some macro");
  g_clear_pointer (&rpm_macros, g_free);

  /* Clear rpm_macros */
  modulemd_buildopts_set_rpm_macros (b, NULL);
  g_object_get (b, "rpm_macros", &rpm_macros, NULL);
  g_assert_null (rpm_macros);
  g_clear_pointer (&rpm_macros, g_free);
}

static void
buildopts_test_whitelist (BuildoptsFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdBuildopts) b = NULL;
  g_auto (GStrv) whitelist;

  b = modulemd_buildopts_new ();
  g_assert_nonnull (b);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b));

  /* Assert we start with 0 rpms */
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b);
  g_assert_cmpint (g_strv_length (whitelist), ==, 0);
  g_clear_pointer (&whitelist, g_strfreev);

  /* Whitelist some rpms */
  modulemd_buildopts_add_rpm_to_whitelist (b, "test2");
  modulemd_buildopts_add_rpm_to_whitelist (b, "test3");
  modulemd_buildopts_add_rpm_to_whitelist (b, "test1");
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b);
  g_assert_cmpint (g_strv_length (whitelist), ==, 3);
  // They should be sorted
  g_assert_cmpstr (whitelist[0], ==, "test1");
  g_assert_cmpstr (whitelist[1], ==, "test2");
  g_assert_cmpstr (whitelist[2], ==, "test3");
  g_clear_pointer (&whitelist, g_strfreev);

  /* Remove some rpms */
  modulemd_buildopts_remove_rpm_from_whitelist (b, "test2");
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b);
  g_assert_cmpint (g_strv_length (whitelist), ==, 2);
  // They should be sorted
  g_assert_cmpstr (whitelist[0], ==, "test1");
  g_assert_cmpstr (whitelist[1], ==, "test3");
  g_clear_pointer (&whitelist, g_strfreev);
}


static void
buildopts_test_parse_yaml (BuildoptsFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdBuildopts) b = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_auto (GStrv) whitelist = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  yaml_path = g_strdup_printf ("%s/modulemd/v2/tests/test_data/b.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rb");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_document_start (&parser);

  b = modulemd_buildopts_parse_yaml (&parser, TRUE, &error);
  g_assert_nonnull (b);
  g_assert_true (MODULEMD_IS_BUILDOPTS (b));
  g_assert_cmpstr (modulemd_buildopts_get_rpm_macros (b),
                   ==,
                   "%demomacro 1\n"
                   "%demomacro2 %{demomacro}23\n");
  whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (b);
  g_assert_cmpint (g_strv_length (whitelist), ==, 4);
  g_assert_cmpstr (whitelist[0], ==, "fooscl-1-bar");
  g_assert_cmpstr (whitelist[1], ==, "fooscl-1-baz");
  g_assert_cmpstr (whitelist[2], ==, "xxx");
  g_assert_cmpstr (whitelist[3], ==, "xyz");
}


static void
buildopts_test_emit_yaml (BuildoptsFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdBuildopts) b = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  b = modulemd_buildopts_new ();
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_buildopts_emit_yaml (b, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "rpms: {}\n"
                   "...\n");

  g_clear_pointer (&yaml_string, modulemd_yaml_string_free);
  yaml_emitter_delete (&emitter);
  yaml_emitter_initialize (&emitter);
  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));
  yaml_emitter_set_output (&emitter, write_yaml_string, (void *)yaml_string);
  modulemd_buildopts_set_rpm_macros (b, "%testmacro 1\n%anothermacro 2");
  modulemd_buildopts_add_rpm_to_whitelist (b, "test2");
  modulemd_buildopts_add_rpm_to_whitelist (b, "test3");
  modulemd_buildopts_add_rpm_to_whitelist (b, "test1");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_buildopts_emit_yaml (b, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "rpms:\n"
                   "  macros: >-\n"
                   "    %testmacro 1\n"
                   "\n"
                   "    %anothermacro 2\n"
                   "  whitelist:\n"
                   "  - test1\n"
                   "  - test2\n"
                   "  - test3\n"
                   "...\n");
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/v2/buildopts/construct",
              BuildoptsFixture,
              NULL,
              NULL,
              buildopts_test_construct,
              NULL);

  g_test_add ("/modulemd/v2/buildopts/equals",
              BuildoptsFixture,
              NULL,
              NULL,
              buildopts_test_equals,
              NULL);

  g_test_add ("/modulemd/v2/buildopts/copy",
              BuildoptsFixture,
              NULL,
              NULL,
              buildopts_test_copy,
              NULL);

  g_test_add ("/modulemd/v2/buildopts/get_set_rpm_macros",
              BuildoptsFixture,
              NULL,
              NULL,
              buildopts_test_get_set_rpm_macros,
              NULL);

  g_test_add ("/modulemd/v2/buildopts/whitelist",
              BuildoptsFixture,
              NULL,
              NULL,
              buildopts_test_whitelist,
              NULL);

  g_test_add ("/modulemd/v2/buildopts/yaml/parse",
              BuildoptsFixture,
              NULL,
              NULL,
              buildopts_test_parse_yaml,
              NULL);

  g_test_add ("/modulemd/v2/buildopts/yaml/emit",
              BuildoptsFixture,
              NULL,
              NULL,
              buildopts_test_emit_yaml,
              NULL);

  return g_test_run ();
}
