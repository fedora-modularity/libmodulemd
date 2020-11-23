/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat, Inc.
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

#include "private/glib-extensions.h"
#include "private/modulemd-build-config.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"


static void
buildconfig_test_construct (void)
{
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_auto (GStrv) requires = NULL;


  /* == Test that the new() function works == */
  bc = modulemd_build_config_new ();
  g_assert_nonnull (bc);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc));


  /* == Verify that it is constructed properly == */

  /* context should be NULL */
  g_assert_null (modulemd_build_config_get_context (bc));

  /* platform should be NULL */
  g_assert_null (modulemd_build_config_get_platform (bc));

  /* runtime requires should be empty */
  requires = modulemd_build_config_get_runtime_modules_as_strv (bc);
  g_assert_nonnull (requires);
  g_assert_null (requires[0]);
  g_clear_pointer (&requires, g_strfreev);

  /* buildtime requires should be empty */
  requires = modulemd_build_config_get_buildtime_modules_as_strv (bc);
  g_assert_nonnull (requires);
  g_assert_null (requires[0]);
  g_clear_pointer (&requires, g_strfreev);

  /* There should be no associated build options */
  g_assert_null (modulemd_build_config_get_buildopts (bc));
}


static void
buildconfig_test_context (void)
{
  g_autoptr (ModulemdBuildConfig) bc = NULL;

  bc = modulemd_build_config_new ();
  g_assert_nonnull (bc);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc));

  /* context should be NULL */
  g_assert_null (modulemd_build_config_get_context (bc));

  /* Set a context value */
  modulemd_build_config_set_context (bc, "CTX1");

  /* Verify that we can retrieve this value */
  g_assert_nonnull (modulemd_build_config_get_context (bc));
  g_assert_cmpstr ("CTX1", ==, modulemd_build_config_get_context (bc));

  /* Unset the context */
  modulemd_build_config_set_context (bc, NULL);

  /* context should be NULL */
  g_assert_null (modulemd_build_config_get_context (bc));
}


static void
buildconfig_test_platform (void)
{
  g_autoptr (ModulemdBuildConfig) bc = NULL;

  bc = modulemd_build_config_new ();
  g_assert_nonnull (bc);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc));

  /* platform should be NULL */
  g_assert_null (modulemd_build_config_get_platform (bc));

  /* Set a platform value */
  modulemd_build_config_set_platform (bc, "f33");

  /* Verify that we can retrieve this value */
  g_assert_nonnull (modulemd_build_config_get_platform (bc));
  g_assert_cmpstr ("f33", ==, modulemd_build_config_get_platform (bc));

  /* Unset the platform */
  modulemd_build_config_set_platform (bc, NULL);

  /* platform should be NULL */
  g_assert_null (modulemd_build_config_get_platform (bc));
}


static void
buildconfig_test_requires (void)
{
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_auto (GStrv) required_modules = NULL;


  bc = modulemd_build_config_new ();
  g_assert_nonnull (bc);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc));

  /* Add a runtime requirement */
  modulemd_build_config_add_runtime_requirement (bc, "framework", "1.0");

  /* Confirm that it was added */
  required_modules = modulemd_build_config_get_runtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("framework", ==, required_modules[0]);
  g_assert_cmpstr (
    "1.0",
    ==,
    modulemd_build_config_get_runtime_requirement_stream (bc, "framework"));
  g_assert_null (required_modules[1]);
  g_clear_pointer (&required_modules, g_strfreev);

  /* Add another requirement */
  modulemd_build_config_add_runtime_requirement (bc, "docbuilder", "rolling");

  /* Confirm that it was added */
  required_modules = modulemd_build_config_get_runtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("docbuilder", ==, required_modules[0]);
  g_assert_cmpstr (
    "rolling",
    ==,
    modulemd_build_config_get_runtime_requirement_stream (bc, "docbuilder"));
  g_assert_nonnull (required_modules[1]);
  g_assert_cmpstr ("framework", ==, required_modules[1]);
  g_assert_cmpstr (
    "1.0",
    ==,
    modulemd_build_config_get_runtime_requirement_stream (bc, "framework"));
  g_assert_null (required_modules[2]);
  g_clear_pointer (&required_modules, g_strfreev);

  /* Replace a dependency with a different stream */
  modulemd_build_config_add_runtime_requirement (bc, "docbuilder", "stable");

  /* Confirm that it was added */
  required_modules = modulemd_build_config_get_runtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("docbuilder", ==, required_modules[0]);
  g_assert_cmpstr (
    "stable",
    ==,
    modulemd_build_config_get_runtime_requirement_stream (bc, "docbuilder"));
  g_assert_nonnull (required_modules[1]);
  g_assert_cmpstr ("framework", ==, required_modules[1]);
  g_assert_cmpstr (
    "1.0",
    ==,
    modulemd_build_config_get_runtime_requirement_stream (bc, "framework"));
  g_assert_null (required_modules[2]);
  g_clear_pointer (&required_modules, g_strfreev);

  /* Remove a dependency */
  modulemd_build_config_remove_runtime_requirement (bc, "framework");

  /* Confirm that it was removed */
  required_modules = modulemd_build_config_get_runtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("docbuilder", ==, required_modules[0]);
  g_assert_cmpstr (
    "stable",
    ==,
    modulemd_build_config_get_runtime_requirement_stream (bc, "docbuilder"));
  g_assert_null (required_modules[1]);
  g_clear_pointer (&required_modules, g_strfreev);

  /* Remove a nonexistent dependency */
  modulemd_build_config_remove_runtime_requirement (bc, "notpresent");

  /* Confirm that nothing changed */
  required_modules = modulemd_build_config_get_runtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("docbuilder", ==, required_modules[0]);
  g_assert_cmpstr (
    "stable",
    ==,
    modulemd_build_config_get_runtime_requirement_stream (bc, "docbuilder"));
  g_assert_null (required_modules[1]);
  g_clear_pointer (&required_modules, g_strfreev);


  /* Clear all the requirements */
  modulemd_build_config_clear_runtime_requirements (bc);

  required_modules = modulemd_build_config_get_runtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_null (required_modules[0]);
  g_clear_pointer (&required_modules, g_strfreev);
}


static void
buildconfig_test_buildrequires (void)
{
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_auto (GStrv) required_modules = NULL;


  bc = modulemd_build_config_new ();
  g_assert_nonnull (bc);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc));

  /* Add a buildtime requirement */
  modulemd_build_config_add_buildtime_requirement (bc, "framework", "1.0");

  /* Confirm that it was added */
  required_modules = modulemd_build_config_get_buildtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("framework", ==, required_modules[0]);
  g_assert_cmpstr (
    "1.0",
    ==,
    modulemd_build_config_get_buildtime_requirement_stream (bc, "framework"));
  g_assert_null (required_modules[1]);
  g_clear_pointer (&required_modules, g_strfreev);

  /* Add another requirement */
  modulemd_build_config_add_buildtime_requirement (
    bc, "docbuilder", "rolling");

  /* Confirm that it was added */
  required_modules = modulemd_build_config_get_buildtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("docbuilder", ==, required_modules[0]);
  g_assert_cmpstr (
    "rolling",
    ==,
    modulemd_build_config_get_buildtime_requirement_stream (bc, "docbuilder"));
  g_assert_nonnull (required_modules[1]);
  g_assert_cmpstr ("framework", ==, required_modules[1]);
  g_assert_cmpstr (
    "1.0",
    ==,
    modulemd_build_config_get_buildtime_requirement_stream (bc, "framework"));
  g_assert_null (required_modules[2]);
  g_clear_pointer (&required_modules, g_strfreev);

  /* Replace a dependency with a different stream */
  modulemd_build_config_add_buildtime_requirement (bc, "docbuilder", "stable");

  /* Confirm that it was added */
  required_modules = modulemd_build_config_get_buildtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("docbuilder", ==, required_modules[0]);
  g_assert_cmpstr (
    "stable",
    ==,
    modulemd_build_config_get_buildtime_requirement_stream (bc, "docbuilder"));
  g_assert_nonnull (required_modules[1]);
  g_assert_cmpstr ("framework", ==, required_modules[1]);
  g_assert_cmpstr (
    "1.0",
    ==,
    modulemd_build_config_get_buildtime_requirement_stream (bc, "framework"));
  g_assert_null (required_modules[2]);
  g_clear_pointer (&required_modules, g_strfreev);

  /* Remove a dependency */
  modulemd_build_config_remove_buildtime_requirement (bc, "framework");

  /* Confirm that it was removed */
  required_modules = modulemd_build_config_get_buildtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("docbuilder", ==, required_modules[0]);
  g_assert_cmpstr (
    "stable",
    ==,
    modulemd_build_config_get_buildtime_requirement_stream (bc, "docbuilder"));
  g_assert_null (required_modules[1]);
  g_clear_pointer (&required_modules, g_strfreev);

  /* Remove a nonexistent dependency */
  modulemd_build_config_remove_buildtime_requirement (bc, "notpresent");

  /* Confirm that nothing changed */
  required_modules = modulemd_build_config_get_buildtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_nonnull (required_modules[0]);
  g_assert_cmpstr ("docbuilder", ==, required_modules[0]);
  g_assert_cmpstr (
    "stable",
    ==,
    modulemd_build_config_get_buildtime_requirement_stream (bc, "docbuilder"));
  g_assert_null (required_modules[1]);
  g_clear_pointer (&required_modules, g_strfreev);


  /* Clear all the requirements */
  modulemd_build_config_clear_buildtime_requirements (bc);

  required_modules = modulemd_build_config_get_buildtime_modules_as_strv (bc);
  g_assert_nonnull (required_modules);
  g_assert_null (required_modules[0]);
  g_clear_pointer (&required_modules, g_strfreev);
}


static void
buildconfig_test_buildopts (void)
{
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_autoptr (ModulemdBuildopts) opts = NULL;
  ModulemdBuildopts *retrieved = NULL;

  bc = modulemd_build_config_new ();
  g_assert_nonnull (bc);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc));

  g_assert_null (modulemd_build_config_get_buildopts (bc));

  /* Create a buildopts object to store */
  opts = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (opts, "%global test 1");
  modulemd_build_config_set_buildopts (bc, opts);

  retrieved = modulemd_build_config_get_buildopts (bc);
  g_assert_nonnull (retrieved);
  g_assert_true (modulemd_buildopts_equals (opts, retrieved));

  /* Confirm we're getting back a copy and not the same pointer */
  g_assert_false (opts == retrieved);

  /* Unset the buildopts */
  modulemd_build_config_set_buildopts (bc, NULL);
  g_assert_null (modulemd_build_config_get_buildopts (bc));
}


static void
buildconfig_test_parse_yaml (void)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  ModulemdBuildopts *opts = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;
  g_auto (GStrv) dep_modules = NULL;

  /* Verify a valid YAML file */
  yaml_path = g_strdup_printf ("%s/buildconfig/good_bc.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  bc = modulemd_build_config_parse_yaml (&parser, TRUE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (bc);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc));


  g_assert_nonnull (modulemd_build_config_get_context (bc));
  g_assert_cmpstr ("CTX1", ==, modulemd_build_config_get_context (bc));


  g_assert_nonnull (modulemd_build_config_get_platform (bc));
  g_assert_cmpstr ("f32", ==, modulemd_build_config_get_platform (bc));


  dep_modules = modulemd_build_config_get_runtime_modules_as_strv (bc);

  g_assert_nonnull (dep_modules);
  g_assert_nonnull (dep_modules[0]);
  g_assert_cmpstr ("appframework", ==, dep_modules[0]);
  g_assert_null (dep_modules[1]);
  g_assert_cmpstr (
    "v2",
    ==,
    modulemd_build_config_get_runtime_requirement_stream (bc, "appframework"));
  g_clear_pointer (&dep_modules, &g_strfreev);


  dep_modules = modulemd_build_config_get_buildtime_modules_as_strv (bc);

  g_assert_nonnull (dep_modules);
  g_assert_nonnull (dep_modules[0]);
  g_assert_cmpstr ("appframework", ==, dep_modules[0]);
  g_assert_nonnull (dep_modules[1]);
  g_assert_cmpstr ("doctool", ==, dep_modules[1]);
  g_assert_null (dep_modules[2]);
  g_assert_cmpstr ("v1",
                   ==,
                   modulemd_build_config_get_buildtime_requirement_stream (
                     bc, "appframework"));
  g_assert_cmpstr (
    "rolling",
    ==,
    modulemd_build_config_get_buildtime_requirement_stream (bc, "doctool"));
  g_clear_pointer (&dep_modules, &g_strfreev);


  /* Test that we constructed a ModulemdBuildopts object.
   * We don't need to check all of its values because that's already done in
   * the buildopts tests
   */
  opts = modulemd_build_config_get_buildopts (bc);
  g_assert_nonnull (opts);
  g_assert_true (MODULEMD_IS_BUILDOPTS (opts));
}


static void
buildconfig_test_parse_yaml_unknown_key (void)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  ModulemdBuildopts *opts = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;
  g_auto (GStrv) dep_modules = NULL;

  /* Read a good YAML file with an unknown key */
  yaml_path = g_strdup_printf ("%s/buildconfig/unknown_key.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);


  /* First try it in non-strict mode */
  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  bc = modulemd_build_config_parse_yaml (&parser, FALSE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (bc);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc));


  g_assert_nonnull (modulemd_build_config_get_context (bc));
  g_assert_cmpstr ("CTX1", ==, modulemd_build_config_get_context (bc));


  g_assert_nonnull (modulemd_build_config_get_platform (bc));
  g_assert_cmpstr ("f32", ==, modulemd_build_config_get_platform (bc));


  dep_modules = modulemd_build_config_get_runtime_modules_as_strv (bc);

  g_assert_nonnull (dep_modules);
  g_assert_nonnull (dep_modules[0]);
  g_assert_cmpstr ("appframework", ==, dep_modules[0]);
  g_assert_null (dep_modules[1]);
  g_assert_cmpstr (
    "v2",
    ==,
    modulemd_build_config_get_runtime_requirement_stream (bc, "appframework"));
  g_clear_pointer (&dep_modules, &g_strfreev);


  dep_modules = modulemd_build_config_get_buildtime_modules_as_strv (bc);

  g_assert_nonnull (dep_modules);
  g_assert_nonnull (dep_modules[0]);
  g_assert_cmpstr ("appframework", ==, dep_modules[0]);
  g_assert_nonnull (dep_modules[1]);
  g_assert_cmpstr ("doctool", ==, dep_modules[1]);
  g_assert_null (dep_modules[2]);
  g_assert_cmpstr ("v1",
                   ==,
                   modulemd_build_config_get_buildtime_requirement_stream (
                     bc, "appframework"));
  g_assert_cmpstr (
    "rolling",
    ==,
    modulemd_build_config_get_buildtime_requirement_stream (bc, "doctool"));
  g_clear_pointer (&dep_modules, &g_strfreev);


  /* Test that we constructed a ModulemdBuildopts object.
   * We don't need to check all of its values because that's already done in
   * the buildopts tests
   */
  opts = modulemd_build_config_get_buildopts (bc);
  g_assert_nonnull (opts);
  g_assert_true (MODULEMD_IS_BUILDOPTS (opts));
  g_clear_object (&bc);


  /* This should fail in strict mode */
  MMD_INIT_YAML_PARSER (strictparser);
  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&strictparser, yaml_stream);

  parser_skip_headers (&strictparser);

  bc = modulemd_build_config_parse_yaml (&strictparser, TRUE, &error);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_UNKNOWN_ATTR);
  g_assert_null (bc);
}


static void
buildconfig_test_parse_yaml_no_context (void)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;

  /* Read a YAML file that is missing context */
  yaml_path = g_strdup_printf ("%s/buildconfig/no_context.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  bc = modulemd_build_config_parse_yaml (&parser, TRUE, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_assert_null (bc);
}


static void
buildconfig_test_parse_yaml_short_context (void)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;

  /* Read a YAML file with an empty context */
  yaml_path = g_strdup_printf ("%s/buildconfig/short_context.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  bc = modulemd_build_config_parse_yaml (&parser, TRUE, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_assert_null (bc);
}


static void
buildconfig_test_parse_yaml_long_context (void)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;

  /* Read a YAML file that has a context that is too long */
  yaml_path = g_strdup_printf ("%s/buildconfig/long_context.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  bc = modulemd_build_config_parse_yaml (&parser, TRUE, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_assert_null (bc);
}


static void
buildconfig_test_parse_yaml_nonalphanum (void)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;

  /* Read a YAML file that has a context with a disallowed character */
  yaml_path = g_strdup_printf ("%s/buildconfig/nonalphanum_context.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  bc = modulemd_build_config_parse_yaml (&parser, TRUE, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_assert_null (bc);
}


static void
buildconfig_test_parse_yaml_no_platform (void)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;

  /* Read a YAML file that is missing platform */
  yaml_path = g_strdup_printf ("%s/buildconfig/no_platform.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  bc = modulemd_build_config_parse_yaml (&parser, TRUE, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_assert_null (bc);
}

static void
buildconfig_test_emit_yaml (void)
{
  g_autoptr (ModulemdBuildConfig) bc = NULL;
  g_autoptr (ModulemdBuildopts) opts = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  bc = modulemd_build_config_new ();
  modulemd_build_config_set_context (bc, "CTX1");
  modulemd_build_config_set_platform (bc, "f32");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (modulemd_build_config_emit_yaml (bc, &emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));

  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "context: CTX1\n"
                   "platform: f32\n"
                   "...\n");

  MMD_REINIT_YAML_STRING (&emitter, yaml_string);

  opts = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (opts, "%global test 1");
  modulemd_build_config_set_buildopts (bc, opts);

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (modulemd_build_config_emit_yaml (bc, &emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));

  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "context: CTX1\n"
                   "platform: f32\n"
                   "buildopts:\n"
                   "  rpms:\n"
                   "    macros: >-\n"
                   "      %global test 1\n"
                   "...\n");

  MMD_REINIT_YAML_STRING (&emitter, yaml_string);

  modulemd_build_config_add_buildtime_requirement (bc, "appframework", "v1");
  modulemd_build_config_add_buildtime_requirement (bc, "doctool", "rolling");
  modulemd_build_config_add_runtime_requirement (bc, "appframework", "v2");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (modulemd_build_config_emit_yaml (bc, &emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));

  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "context: CTX1\n"
                   "platform: f32\n"
                   "buildrequires:\n"
                   "  appframework: [v1]\n"
                   "  doctool: [rolling]\n"
                   "requires:\n"
                   "  appframework: [v2]\n"
                   "buildopts:\n"
                   "  rpms:\n"
                   "    macros: >-\n"
                   "      %global test 1\n"
                   "...\n");
}

static void
buildconfig_test_comparison (void)
{
  g_autoptr (ModulemdBuildConfig) bc_1 = NULL;
  g_autoptr (ModulemdBuildConfig) bc_2 = NULL;
  g_autoptr (ModulemdBuildopts) opts = NULL;

  /* with no properties */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));

  g_assert_true (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), ==, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), ==, 0);

  /* checks when NULL is involved */
  g_assert_true (modulemd_build_config_equals (NULL, NULL));
  g_assert_false (modulemd_build_config_equals (NULL, bc_2));
  g_assert_false (modulemd_build_config_equals (bc_1, NULL));
  g_assert_cmpint (modulemd_build_config_compare (NULL, NULL), ==, 0);
  g_assert_cmpint (modulemd_build_config_compare (NULL, bc_2), <, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_1, NULL), >, 0);

  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with same contexts */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_set_context (bc_1, "CTX1");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_set_context (bc_2, "CTX1");

  g_assert_true (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), ==, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), ==, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with different contexts */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_set_context (bc_1, "CTX1");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_set_context (bc_2, "CTX2");

  g_assert_false (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), <, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), >, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with same platforms */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_set_platform (bc_1, "f33");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_set_platform (bc_2, "f33");

  g_assert_true (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), ==, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), ==, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with different platforms */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_set_platform (bc_1, "f33");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_set_platform (bc_2, "f32");

  g_assert_false (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), >, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), <, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with same buildtime requirements */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_add_buildtime_requirement (
    bc_1, "buildmod1", "stream1");
  modulemd_build_config_add_buildtime_requirement (
    bc_1, "buildmod2", "stream2");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_add_buildtime_requirement (
    bc_2, "buildmod1", "stream1");
  modulemd_build_config_add_buildtime_requirement (
    bc_2, "buildmod2", "stream2");

  g_assert_true (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), ==, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), ==, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with different buildtime requirements */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_add_buildtime_requirement (
    bc_1, "buildmod1", "stream1");
  modulemd_build_config_add_buildtime_requirement (
    bc_1, "buildmod2", "stream2");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_add_buildtime_requirement (
    bc_2, "buildmod1", "stream1");
  modulemd_build_config_add_buildtime_requirement (
    bc_2, "buildmod2", "stream2");
  modulemd_build_config_add_buildtime_requirement (
    bc_2, "buildmod3", "stream3");

  g_assert_false (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), <, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), >, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with same runtime requirements */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_add_runtime_requirement (bc_1, "runmod1", "stream3");
  modulemd_build_config_add_runtime_requirement (bc_1, "runmod2", "stream4");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_add_runtime_requirement (bc_2, "runmod1", "stream3");
  modulemd_build_config_add_runtime_requirement (bc_2, "runmod2", "stream4");

  g_assert_true (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), ==, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), ==, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with different runtime requirements */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_add_runtime_requirement (bc_1, "runmod1", "stream3");
  modulemd_build_config_add_runtime_requirement (bc_1, "runmod2", "stream4");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_add_runtime_requirement (bc_2, "runmod1", "stream3");
  modulemd_build_config_add_runtime_requirement (bc_2, "runmod2", "stream4");
  modulemd_build_config_add_runtime_requirement (bc_2, "runmod3", "stream5");

  g_assert_false (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), <, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), >, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with same buildtime and runtime requirements */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_add_buildtime_requirement (
    bc_1, "buildmod1", "stream1");
  modulemd_build_config_add_buildtime_requirement (
    bc_1, "buildmod2", "stream2");
  modulemd_build_config_add_runtime_requirement (bc_1, "runmod1", "stream3");
  modulemd_build_config_add_runtime_requirement (bc_1, "runmod2", "stream4");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_add_buildtime_requirement (
    bc_2, "buildmod1", "stream1");
  modulemd_build_config_add_buildtime_requirement (
    bc_2, "buildmod2", "stream2");
  modulemd_build_config_add_runtime_requirement (bc_2, "runmod1", "stream3");
  modulemd_build_config_add_runtime_requirement (bc_2, "runmod2", "stream4");

  g_assert_true (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), ==, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), ==, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with different buildtime and same runtime requirements */
  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_add_buildtime_requirement (
    bc_1, "buildmod1", "stream1");
  modulemd_build_config_add_buildtime_requirement (
    bc_1, "buildmod2", "stream2");
  modulemd_build_config_add_buildtime_requirement (
    bc_1, "buildmod3", "stream8");
  modulemd_build_config_add_runtime_requirement (bc_1, "runmod1", "stream3");
  modulemd_build_config_add_runtime_requirement (bc_1, "runmod2", "stream4");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_add_buildtime_requirement (
    bc_2, "buildmod1", "stream1");
  modulemd_build_config_add_buildtime_requirement (
    bc_2, "buildmod2", "stream2");
  modulemd_build_config_add_runtime_requirement (bc_2, "runmod1", "stream3");
  modulemd_build_config_add_runtime_requirement (bc_2, "runmod2", "stream4");

  g_assert_false (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), >, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), <, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);

  /* with same buildopts */
  opts = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (opts, "%global test 1");

  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_set_buildopts (bc_1, opts);

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_set_buildopts (bc_2, opts);

  g_assert_true (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), ==, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), ==, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);
  g_clear_object (&opts);

  /* with different buildopts */
  opts = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (opts, "%global test 1");

  bc_1 = modulemd_build_config_new ();
  g_assert_nonnull (bc_1);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_1));
  modulemd_build_config_set_buildopts (bc_1, opts);

  g_clear_object (&opts);
  opts = modulemd_buildopts_new ();
  modulemd_buildopts_set_rpm_macros (opts, "%global test 2");

  bc_2 = modulemd_build_config_new ();
  g_assert_nonnull (bc_2);
  g_assert_true (MODULEMD_IS_BUILD_CONFIG (bc_2));
  modulemd_build_config_set_buildopts (bc_2, opts);

  g_assert_false (modulemd_build_config_equals (bc_1, bc_2));
  g_assert_cmpint (modulemd_build_config_compare (bc_1, bc_2), <, 0);
  g_assert_cmpint (modulemd_build_config_compare (bc_2, bc_1), >, 0);
  g_clear_object (&bc_1);
  g_clear_object (&bc_2);
  g_clear_object (&opts);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add_func ("/modulemd/v2/buildconfig/construct",
                   buildconfig_test_construct);

  g_test_add_func ("/modulemd/v2/buildconfig/context",
                   buildconfig_test_context);

  g_test_add_func ("/modulemd/v2/buildconfig/platform",
                   buildconfig_test_platform);

  g_test_add_func ("/modulemd/v2/buildconfig/requires",
                   buildconfig_test_requires);

  g_test_add_func ("/modulemd/v2/buildconfig/buildrequires",
                   buildconfig_test_buildrequires);

  g_test_add_func ("/modulemd/v2/buildconfig/buildopts",
                   buildconfig_test_buildopts);

  g_test_add_func ("/modulemd/v2/buildconfig/yaml/parse",
                   buildconfig_test_parse_yaml);

  g_test_add_func ("/modulemd/v2/buildconfig/yaml/parse/bad/unknown_key",
                   buildconfig_test_parse_yaml_unknown_key);

  g_test_add_func ("/modulemd/v2/buildconfig/yaml/parse/bad/context/none",
                   buildconfig_test_parse_yaml_no_context);

  g_test_add_func ("/modulemd/v2/buildconfig/yaml/parse/bad/context/short",
                   buildconfig_test_parse_yaml_short_context);

  g_test_add_func ("/modulemd/v2/buildconfig/yaml/parse/bad/context/long",
                   buildconfig_test_parse_yaml_long_context);

  g_test_add_func (
    "/modulemd/v2/buildconfig/yaml/parse/bad/context/nonalphanum",
    buildconfig_test_parse_yaml_nonalphanum);

  g_test_add_func ("/modulemd/v2/buildconfig/yaml/parse/bad/platform/none",
                   buildconfig_test_parse_yaml_no_platform);

  g_test_add_func ("/modulemd/v2/buildconfig/yaml/emit",
                   buildconfig_test_emit_yaml);

  g_test_add_func ("/modulemd/v2/buildconfig/comparison",
                   buildconfig_test_comparison);

  return g_test_run ();
}
