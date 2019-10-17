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

#include "modulemd-dependencies.h"
#include "private/glib-extensions.h"
#include "private/modulemd-dependencies-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _DependenciesFixture
{
} DependenciesFixture;

gboolean signaled = FALSE;

static void
sigtrap_handler (int sig_num)
{
  signaled = TRUE;
}

static void
dependencies_test_construct (DependenciesFixture *fixture,
                             gconstpointer user_data)
{
  g_autoptr (ModulemdDependencies) d = NULL;
  g_auto (GStrv) list = NULL;

  /* Test that the new() function works */
  d = modulemd_dependencies_new ();
  g_assert_nonnull (d);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d));

  list = modulemd_dependencies_get_buildtime_modules_as_strv (d);
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 0);
  g_clear_pointer (&list, g_strfreev);
  g_clear_object (&d);

  /* Test that object instantiation works */
  d = g_object_new (MODULEMD_TYPE_DEPENDENCIES, NULL);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d));
  g_clear_object (&d);
}


static void
dependencies_test_dependencies (DependenciesFixture *fixture,
                                gconstpointer user_data)
{
  g_autoptr (ModulemdDependencies) d = NULL;
  g_auto (GStrv) list = NULL;

  d = modulemd_dependencies_new ();
  g_assert_nonnull (d);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d));

  list = modulemd_dependencies_get_buildtime_modules_as_strv (d);
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 0);
  g_clear_pointer (&list, g_strfreev);
  signal (SIGTRAP, sigtrap_handler);
  list = modulemd_dependencies_get_buildtime_streams_as_strv (d, "buildmod1");
  g_assert_null (list);
  g_clear_pointer (&list, g_strfreev);

  /* Add some deps */
  modulemd_dependencies_add_buildtime_stream (d, "buildmod1", "stream1");
  modulemd_dependencies_add_runtime_stream (d, "runmod1", "stream2");
  modulemd_dependencies_add_runtime_stream (d, "runmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d, "defbuild");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d,
                                                                   "defrun");

  list = modulemd_dependencies_get_buildtime_modules_as_strv (d);
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "buildmod1");
  g_assert_cmpstr (list[1], ==, "defbuild");
  g_clear_pointer (&list, g_strfreev);
  list = modulemd_dependencies_get_buildtime_streams_as_strv (d, "buildmod1");
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 1);
  g_assert_cmpstr (list[0], ==, "stream1");
  g_clear_pointer (&list, g_strfreev);
  list = modulemd_dependencies_get_buildtime_streams_as_strv (d, "defbuild");
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 0);
  g_clear_pointer (&list, g_strfreev);
  signal (SIGTRAP, sigtrap_handler);
  list = modulemd_dependencies_get_runtime_streams_as_strv (d, "buildmod1");
  g_assert_null (list);
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_dependencies_get_runtime_modules_as_strv (d);
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "defrun");
  g_assert_cmpstr (list[1], ==, "runmod1");
  g_clear_pointer (&list, g_strfreev);
  list = modulemd_dependencies_get_runtime_streams_as_strv (d, "defrun");
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 0);
  g_clear_pointer (&list, g_strfreev);
  signal (SIGTRAP, sigtrap_handler);
  list = modulemd_dependencies_get_runtime_streams_as_strv (d, "buildmod1");
  g_assert_null (list);
  g_clear_pointer (&list, g_strfreev);
  list = modulemd_dependencies_get_runtime_streams_as_strv (d, "runmod1");
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "stream1");
  g_assert_cmpstr (list[1], ==, "stream2");
  g_clear_pointer (&list, g_strfreev);
}


static void
dependencies_test_equals (DependenciesFixture *fixture,
                          gconstpointer user_data)
{
  g_autoptr (ModulemdDependencies) d_1 = NULL;
  g_autoptr (ModulemdDependencies) d_2 = NULL;

  /*With no hashtables*/
  d_1 = modulemd_dependencies_new ();
  g_assert_nonnull (d_1);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_1));

  d_2 = modulemd_dependencies_new ();
  g_assert_nonnull (d_2);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_2));

  g_assert_true (modulemd_dependencies_equals (d_1, d_2));
  g_clear_object (&d_1);
  g_clear_object (&d_2);

  /*With same buildtime_stream hashtables*/
  d_1 = modulemd_dependencies_new ();
  g_assert_nonnull (d_1);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_1));
  modulemd_dependencies_add_buildtime_stream (d_1, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d_1, "buildmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d_1, "builddef");

  d_2 = modulemd_dependencies_new ();
  g_assert_nonnull (d_2);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_2));
  modulemd_dependencies_add_buildtime_stream (d_2, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d_2, "buildmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d_2, "builddef");

  g_assert_true (modulemd_dependencies_equals (d_1, d_2));
  g_clear_object (&d_1);
  g_clear_object (&d_2);

  /*With different buildtime_stream hashtables*/
  d_1 = modulemd_dependencies_new ();
  g_assert_nonnull (d_1);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_1));
  modulemd_dependencies_add_buildtime_stream (d_1, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d_1, "buildmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d_1, "builddef");

  d_2 = modulemd_dependencies_new ();
  g_assert_nonnull (d_2);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_2));
  modulemd_dependencies_add_buildtime_stream (d_2, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d_2, "buildmod1", "stream1");
  modulemd_dependencies_add_buildtime_stream (d_2, "buildmod1", "stream3");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d_2, "builddef");

  g_assert_false (modulemd_dependencies_equals (d_1, d_2));
  g_clear_object (&d_1);
  g_clear_object (&d_2);

  /*With same runtime_stream hashtables*/
  d_1 = modulemd_dependencies_new ();
  g_assert_nonnull (d_1);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_1));
  modulemd_dependencies_add_runtime_stream (d_1, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d_1, "runmod1", "stream4");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d_1,
                                                                   "rundef");

  d_2 = modulemd_dependencies_new ();
  g_assert_nonnull (d_2);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_2));
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream4");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d_2,
                                                                   "rundef");

  g_assert_true (modulemd_dependencies_equals (d_1, d_2));
  g_clear_object (&d_1);
  g_clear_object (&d_2);

  /*With different runtime_stream hashtables*/
  d_1 = modulemd_dependencies_new ();
  g_assert_nonnull (d_1);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_1));
  modulemd_dependencies_add_runtime_stream (d_1, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d_1, "runmod1", "stream4");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d_1,
                                                                   "rundef");

  d_2 = modulemd_dependencies_new ();
  g_assert_nonnull (d_2);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_2));
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream4");
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream5");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d_2,
                                                                   "rundef");

  g_assert_false (modulemd_dependencies_equals (d_1, d_2));
  g_clear_object (&d_1);
  g_clear_object (&d_2);

  /*With same bulidtime_stream and runtime_stream hashtables*/
  d_1 = modulemd_dependencies_new ();
  g_assert_nonnull (d_1);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_1));
  modulemd_dependencies_add_buildtime_stream (d_1, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d_1, "buildmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d_1, "builddef");
  modulemd_dependencies_add_runtime_stream (d_1, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d_1, "runmod1", "stream4");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d_1,
                                                                   "rundef");

  d_2 = modulemd_dependencies_new ();
  g_assert_nonnull (d_2);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_2));
  modulemd_dependencies_add_buildtime_stream (d_2, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d_2, "buildmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d_2, "builddef");
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream4");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d_2,
                                                                   "rundef");

  g_assert_true (modulemd_dependencies_equals (d_1, d_2));
  g_clear_object (&d_1);
  g_clear_object (&d_2);

  /*With different bulidtime_stream and same runtime_stream hashtables*/
  d_1 = modulemd_dependencies_new ();
  g_assert_nonnull (d_1);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_1));
  modulemd_dependencies_add_buildtime_stream (d_1, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d_1, "buildmod1", "stream1");
  modulemd_dependencies_add_buildtime_stream (d_1, "buildmod1", "stream8");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d_1, "builddef");
  modulemd_dependencies_add_runtime_stream (d_1, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d_1, "runmod1", "stream4");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d_1,
                                                                   "rundef");

  d_2 = modulemd_dependencies_new ();
  g_assert_nonnull (d_2);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_2));
  modulemd_dependencies_add_buildtime_stream (d_2, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d_2, "buildmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d_2, "builddef");
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream4");
  modulemd_dependencies_add_runtime_stream (d_2, "runmod1", "stream5");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d_2,
                                                                   "rundef");

  g_assert_false (modulemd_dependencies_equals (d_1, d_2));
  g_clear_object (&d_1);
  g_clear_object (&d_2);
}


static void
dependencies_test_copy (DependenciesFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdDependencies) d = NULL;
  g_autoptr (ModulemdDependencies) d_copy = NULL;
  g_auto (GStrv) list = NULL;

  d = modulemd_dependencies_new ();
  g_assert_nonnull (d);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d));
  list = modulemd_dependencies_get_buildtime_modules_as_strv (d);
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 0);
  g_clear_pointer (&list, g_strfreev);
  signal (SIGTRAP, sigtrap_handler);
  list = modulemd_dependencies_get_buildtime_streams_as_strv (d, "module1");
  g_assert_null (list);
  g_clear_pointer (&list, g_strfreev);

  d_copy = modulemd_dependencies_copy (d);
  g_assert_nonnull (d_copy);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_copy));
  list = modulemd_dependencies_get_buildtime_modules_as_strv (d_copy);
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 0);
  g_clear_pointer (&list, g_strfreev);
  signal (SIGTRAP, sigtrap_handler);
  list = modulemd_dependencies_get_buildtime_streams_as_strv (d, "module1");
  g_assert_null (list);
  g_clear_pointer (&list, g_strfreev);
  g_clear_object (&d_copy);

  modulemd_dependencies_add_buildtime_stream (d, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d, "buildmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d, "builddef");
  modulemd_dependencies_add_runtime_stream (d, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d, "runmod1", "stream4");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d,
                                                                   "rundef");

  d_copy = modulemd_dependencies_copy (d);
  g_assert_nonnull (d_copy);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d_copy));
  list = modulemd_dependencies_get_buildtime_modules_as_strv (d_copy);
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "builddef");
  g_assert_cmpstr (list[1], ==, "buildmod1");
  g_clear_pointer (&list, g_strfreev);
  signal (SIGTRAP, sigtrap_handler);
  list = modulemd_dependencies_get_buildtime_streams_as_strv (d_copy,
                                                              "nosuchmodule");
  g_assert_null (list);
  g_clear_pointer (&list, g_strfreev);
  list =
    modulemd_dependencies_get_buildtime_streams_as_strv (d_copy, "buildmod1");
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "stream1");
  g_assert_cmpstr (list[1], ==, "stream2");
  g_clear_pointer (&list, g_strfreev);
  list =
    modulemd_dependencies_get_buildtime_streams_as_strv (d_copy, "builddef");
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 0);
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_dependencies_get_runtime_modules_as_strv (d_copy);
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "rundef");
  g_assert_cmpstr (list[1], ==, "runmod1");
  g_clear_pointer (&list, g_strfreev);
  signal (SIGTRAP, sigtrap_handler);
  list =
    modulemd_dependencies_get_runtime_streams_as_strv (d_copy, "nosuchmodule");
  g_assert_null (list);
  g_clear_pointer (&list, g_strfreev);
  list = modulemd_dependencies_get_runtime_streams_as_strv (d_copy, "runmod1");
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "stream3");
  g_assert_cmpstr (list[1], ==, "stream4");
  g_clear_pointer (&list, g_strfreev);
  list = modulemd_dependencies_get_runtime_streams_as_strv (d_copy, "rundef");
  g_assert_nonnull (list);
  g_assert_cmpint (g_strv_length (list), ==, 0);
  g_clear_pointer (&list, g_strfreev);
}

static void
dependencies_test_parse_yaml (DependenciesFixture *fixture,
                              gconstpointer user_data)
{
  g_autoptr (ModulemdDependencies) d = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_auto (GStrv) list = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  yaml_path = g_strdup_printf ("%s/d.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  d = modulemd_dependencies_parse_yaml (&parser, TRUE, &error);
  g_assert_nonnull (d);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d));

  list = modulemd_dependencies_get_buildtime_modules_as_strv (d);
  g_assert_cmpint (g_strv_length (list), ==, 1);
  g_assert_cmpstr (list[0], ==, "platform");
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_dependencies_get_buildtime_streams_as_strv (d, "platform");
  g_assert_cmpint (g_strv_length (list), ==, 3);
  g_assert_cmpstr (list[0], ==, "-epel7");
  g_assert_cmpstr (list[1], ==, "-f27");
  g_assert_cmpstr (list[2], ==, "-f28");
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_dependencies_get_runtime_modules_as_strv (d);
  g_assert_cmpint (g_strv_length (list), ==, 1);
  g_assert_cmpstr (list[0], ==, "platform");
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_dependencies_get_runtime_streams_as_strv (d, "platform");
  g_assert_cmpint (g_strv_length (list), ==, 3);
  g_assert_cmpstr (list[0], ==, "-epel7");
  g_assert_cmpstr (list[1], ==, "-f27");
  g_assert_cmpstr (list[2], ==, "-f28");
  g_clear_pointer (&list, g_strfreev);
}


static void
dependencies_test_parse_bad_yaml (DependenciesFixture *fixture,
                                  gconstpointer user_data)
{
  g_autoptr (ModulemdDependencies) d = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  yaml_path =
    g_strdup_printf ("%s/mismatched-deps.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  d = modulemd_dependencies_parse_yaml (&parser, TRUE, &error);
  g_assert_nonnull (d);
  g_assert_true (MODULEMD_IS_DEPENDENCIES (d));

  g_assert_false (modulemd_dependencies_validate (d, &error));
  g_assert_error (error, MODULEMD_ERROR, MODULEMD_ERROR_VALIDATE);
}


static void
dependencies_test_emit_yaml (DependenciesFixture *fixture,
                             gconstpointer user_data)
{
  g_autoptr (ModulemdDependencies) d = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  d = modulemd_dependencies_new ();
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_sequence (&emitter, YAML_BLOCK_SEQUENCE_STYLE, &error));
  g_assert_true (modulemd_dependencies_emit_yaml (d, &emitter, &error));
  g_assert_true (mmd_emitter_end_sequence (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str, ==, "---\n- {}\n...\n");

  g_clear_pointer (&yaml_string, modulemd_yaml_string_free);
  yaml_emitter_delete (&emitter);
  yaml_emitter_initialize (&emitter);
  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));
  yaml_emitter_set_output (&emitter, write_yaml_string, (void *)yaml_string);
  modulemd_dependencies_add_buildtime_stream (d, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d, "buildmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d, "builddef");
  modulemd_dependencies_add_runtime_stream (d, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d, "runmod1", "stream4");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d,
                                                                   "rundef");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_sequence (&emitter, YAML_BLOCK_SEQUENCE_STYLE, &error));
  g_assert_true (modulemd_dependencies_emit_yaml (d, &emitter, &error));
  g_assert_true (mmd_emitter_end_sequence (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "- buildrequires:\n"
                   "    builddef: []\n"
                   "    buildmod1: [stream1, stream2]\n"
                   "  requires:\n"
                   "    rundef: []\n"
                   "    runmod1: [stream3, stream4]\n"
                   "...\n");


  /* Test with only adding a buildrequires */

  g_clear_pointer (&yaml_string, modulemd_yaml_string_free);
  yaml_emitter_delete (&emitter);
  yaml_emitter_initialize (&emitter);
  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));
  yaml_emitter_set_output (&emitter, write_yaml_string, (void *)yaml_string);

  g_clear_object (&d);
  d = modulemd_dependencies_new ();
  modulemd_dependencies_add_buildtime_stream (d, "buildmod1", "stream2");
  modulemd_dependencies_add_buildtime_stream (d, "buildmod1", "stream1");
  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    d, "builddef");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_sequence (&emitter, YAML_BLOCK_SEQUENCE_STYLE, &error));
  g_assert_true (modulemd_dependencies_emit_yaml (d, &emitter, &error));
  g_assert_true (mmd_emitter_end_sequence (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "- buildrequires:\n"
                   "    builddef: []\n"
                   "    buildmod1: [stream1, stream2]\n"
                   "...\n");

  /* Test with only adding a runtime requires */

  g_clear_pointer (&yaml_string, modulemd_yaml_string_free);
  yaml_emitter_delete (&emitter);
  yaml_emitter_initialize (&emitter);
  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));
  yaml_emitter_set_output (&emitter, write_yaml_string, (void *)yaml_string);

  g_clear_object (&d);
  d = modulemd_dependencies_new ();

  modulemd_dependencies_add_runtime_stream (d, "runmod1", "stream3");
  modulemd_dependencies_add_runtime_stream (d, "runmod1", "stream4");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (d,
                                                                   "rundef");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_sequence (&emitter, YAML_BLOCK_SEQUENCE_STYLE, &error));
  g_assert_true (modulemd_dependencies_emit_yaml (d, &emitter, &error));
  g_assert_true (mmd_emitter_end_sequence (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "- requires:\n"
                   "    rundef: []\n"
                   "    runmod1: [stream3, stream4]\n"
                   "...\n");
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/v2/dependencies/construct",
              DependenciesFixture,
              NULL,
              NULL,
              dependencies_test_construct,
              NULL);

  g_test_add ("/modulemd/v2/dependencies/dependencies",
              DependenciesFixture,
              NULL,
              NULL,
              dependencies_test_dependencies,
              NULL);

  g_test_add ("/modulemd/v2/dependencies/equals",
              DependenciesFixture,
              NULL,
              NULL,
              dependencies_test_equals,
              NULL);

  g_test_add ("/modulemd/v2/dependencies/copy",
              DependenciesFixture,
              NULL,
              NULL,
              dependencies_test_copy,
              NULL);

  g_test_add ("/modulemd/v2/dependencies/yaml/parse",
              DependenciesFixture,
              NULL,
              NULL,
              dependencies_test_parse_yaml,
              NULL);

  g_test_add ("/modulemd/v2/dependencies/yaml/parse/bad",
              DependenciesFixture,
              NULL,
              NULL,
              dependencies_test_parse_bad_yaml,
              NULL);

  g_test_add ("/modulemd/v2/dependencies/yaml/emit",
              DependenciesFixture,
              NULL,
              NULL,
              dependencies_test_emit_yaml,
              NULL);

  /*
  g_test_add ("/modulemd/v2/profile/copy",
              DependenciesFixture,
              NULL,
              NULL,
              profile_test_copy,
              NULL);

  g_test_add ("/modulemd/v2/profile/get_name",
              DependenciesFixture,
              NULL,
              NULL,
              profile_test_get_name,
              NULL);

  g_test_add ("/modulemd/v2/profile/get_set_description",
              DependenciesFixture,
              NULL,
              NULL,
              profile_test_get_set_description,
              NULL);

  g_test_add ("/modulemd/v2/profile/rpms",
              DependenciesFixture,
              NULL,
              NULL,
              profile_test_rpms,
              NULL);

  g_test_add ("/modulemd/v2/profile/yaml/parse",
              DependenciesFixture,
              NULL,
              NULL,
              profile_test_parse_yaml,
              NULL);

  g_test_add ("/modulemd/v2/profile/yaml/emit",
              DependenciesFixture,
              NULL,
              NULL,
              profile_test_emit_yaml,
              NULL);
              */

  return g_test_run ();
}
