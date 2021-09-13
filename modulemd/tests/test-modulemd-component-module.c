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

#include "modulemd-component-module.h"
#include "modulemd-component.h"
#include "private/glib-extensions.h"
#include "private/modulemd-component-module-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _ComponentModuleFixture
{
} ComponentModuleFixture;

gboolean signaled = FALSE;

static void
sigtrap_handler (int UNUSED (sig_num))
{
  signaled = TRUE;
}

static void
component_module_test_construct (void)
{
  g_autoptr (ModulemdComponentModule) m = NULL;
  ModulemdComponent *mc = NULL;
  gint64 buildorder = 42;

  /* Test that the new() function works */
  m = modulemd_component_module_new ("testmodule");
  mc = MODULEMD_COMPONENT (m);
  g_assert_nonnull (m);
  g_assert_true (MODULEMD_IS_COMPONENT_MODULE (m));
  g_assert_cmpint (modulemd_component_get_buildorder (mc), ==, 0);
  g_assert_cmpstr (modulemd_component_get_name (mc), ==, "testmodule");
  g_assert_null (modulemd_component_get_rationale (mc));
  g_assert_null (modulemd_component_module_get_ref (m));
  g_assert_null (modulemd_component_module_get_repository (m));
  mc = NULL;
  g_clear_object (&m);

  /* Test that object instantiation works */
  m =
    g_object_new (MODULEMD_TYPE_COMPONENT_MODULE, "name", "testmodule2", NULL);
  g_assert_true (MODULEMD_IS_COMPONENT_MODULE (m));
  g_clear_object (&m);

  /* Instantiate with some argument */
  // clang-format off
  m = g_object_new (MODULEMD_TYPE_COMPONENT_MODULE,
                    "buildorder", buildorder,
                    "name", "testmodule",
                    "rationale", "Testing all the stuff",
                    "ref", "someref",
                    "repository", "somerepo",
                    NULL);
  // clang-format on
  g_assert_nonnull (m);
  g_assert_true (MODULEMD_IS_COMPONENT_MODULE (m));
  mc = MODULEMD_COMPONENT (m);
  g_assert_cmpint (modulemd_component_get_buildorder (mc), ==, buildorder);
  g_assert_cmpstr (modulemd_component_get_name (mc), ==, "testmodule");
  g_assert_cmpstr (
    modulemd_component_get_rationale (mc), ==, "Testing all the stuff");
  g_assert_cmpstr (modulemd_component_module_get_ref (m), ==, "someref");
  g_assert_cmpstr (
    modulemd_component_module_get_repository (m), ==, "somerepo");
  mc = NULL;
  g_clear_object (&m);

  /* Test that we abort with a NULL name to new() */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  m = modulemd_component_module_new (NULL);
  g_assert_true (signaled);
  g_clear_object (&m);

  /* Test that init fails without name */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  m = g_object_new (MODULEMD_TYPE_COMPONENT_MODULE, NULL);
  g_assert_true (signaled);
  g_clear_object (&m);

  /* Test that init fails with a NULL name */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  m = g_object_new (MODULEMD_TYPE_COMPONENT_MODULE, "name", NULL, NULL);
  g_assert_true (signaled);
  g_clear_object (&m);
}


static void
component_module_test_equals (void)
{
  g_autoptr (ModulemdComponentModule) m_1 = NULL;
  g_autoptr (ModulemdComponentModule) m_2 = NULL;

  /*Everything is same*/
  m_1 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_1), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_1),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_1, "someref");
  modulemd_component_module_set_repository (m_1, "somerepo");

  m_2 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_2), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_2),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_2, "someref");
  modulemd_component_module_set_repository (m_2, "somerepo");

  g_assert_true (modulemd_component_equals (MODULEMD_COMPONENT (m_1),
                                            MODULEMD_COMPONENT (m_2)));
  g_clear_object (&m_1);
  g_clear_object (&m_2);

  /*repository is different*/
  m_1 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_1), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_1),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_1, "someref");
  modulemd_component_module_set_repository (m_1, "repoA");

  m_2 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_2), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_2),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_2, "someref");
  modulemd_component_module_set_repository (m_2, "somerepo");

  g_assert_false (modulemd_component_equals (MODULEMD_COMPONENT (m_1),
                                             MODULEMD_COMPONENT (m_2)));
  g_clear_object (&m_1);
  g_clear_object (&m_2);

  /*ref is different*/
  m_1 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_1), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_1),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_1, "someref");
  modulemd_component_module_set_repository (m_1, "somerepo");

  m_2 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_2), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_2),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_2, "refA");
  modulemd_component_module_set_repository (m_2, "somerepo");

  g_assert_false (modulemd_component_equals (MODULEMD_COMPONENT (m_1),
                                             MODULEMD_COMPONENT (m_2)));
  g_clear_object (&m_1);
  g_clear_object (&m_2);

  /*No ref*/
  m_1 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_1), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_1),
                                    "Testing all the stuff");
  modulemd_component_module_set_repository (m_1, "somerepo");

  m_2 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_2), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_2),
                                    "Testing all the stuff");
  modulemd_component_module_set_repository (m_2, "somerepo");

  g_assert_true (modulemd_component_equals (MODULEMD_COMPONENT (m_1),
                                            MODULEMD_COMPONENT (m_2)));
  g_clear_object (&m_1);
  g_clear_object (&m_2);

  /*No repository*/
  m_1 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_1), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_1),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_1, "someref");

  m_2 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_2), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_2),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_2, "someref");

  g_assert_true (modulemd_component_equals (MODULEMD_COMPONENT (m_1),
                                            MODULEMD_COMPONENT (m_2)));
  g_clear_object (&m_1);
  g_clear_object (&m_2);

  /*different ref and no repo*/
  m_1 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_1), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_1),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_1, "someref");

  m_2 = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_2), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_2),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_2, "refAA");

  g_assert_false (modulemd_component_equals (MODULEMD_COMPONENT (m_1),
                                             MODULEMD_COMPONENT (m_2)));
  g_clear_object (&m_1);
  g_clear_object (&m_2);
}


static void
component_module_test_copy (void)
{
  g_autoptr (ModulemdComponentModule) m_orig = NULL;
  g_autoptr (ModulemdComponentModule) m = NULL;
  ModulemdComponent *mc = NULL;

  m_orig = modulemd_component_module_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m_orig), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (m_orig),
                                    "Testing all the stuff");
  modulemd_component_module_set_ref (m_orig, "someref");
  modulemd_component_module_set_repository (m_orig, "somerepo");
  g_assert_nonnull (m_orig);
  g_assert_true (MODULEMD_IS_COMPONENT_MODULE (m_orig));

  m = MODULEMD_COMPONENT_MODULE (
    modulemd_component_copy (MODULEMD_COMPONENT (m_orig), NULL));
  g_assert_nonnull (m);
  g_assert_true (MODULEMD_IS_COMPONENT_MODULE (m));
  mc = MODULEMD_COMPONENT (m);
  g_assert_cmpint (modulemd_component_get_buildorder (mc), ==, 42);
  g_assert_cmpstr (modulemd_component_get_name (mc), ==, "testmodule");
  g_assert_cmpstr (
    modulemd_component_get_rationale (mc), ==, "Testing all the stuff");
  g_assert_cmpstr (modulemd_component_module_get_ref (m), ==, "someref");
  g_assert_cmpstr (
    modulemd_component_module_get_repository (m), ==, "somerepo");
  mc = NULL;
  g_clear_object (&m);


  m = MODULEMD_COMPONENT_MODULE (
    modulemd_component_copy (MODULEMD_COMPONENT (m_orig), "renamedmodule"));
  g_assert_nonnull (m);
  g_assert_true (MODULEMD_IS_COMPONENT_MODULE (m));
  mc = MODULEMD_COMPONENT (m);
  g_assert_cmpint (modulemd_component_get_buildorder (mc), ==, 42);
  g_assert_cmpstr (modulemd_component_get_name (mc), ==, "renamedmodule");
  g_assert_cmpstr (
    modulemd_component_get_rationale (mc), ==, "Testing all the stuff");
  g_assert_cmpstr (modulemd_component_module_get_ref (m), ==, "someref");
  g_assert_cmpstr (
    modulemd_component_module_get_repository (m), ==, "somerepo");
  mc = NULL;
  g_clear_object (&m);
}


static void
component_module_test_emit_yaml (void)
{
  g_autoptr (ModulemdComponentModule) m = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  m = modulemd_component_module_new ("testcomponent");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_component_module_emit_yaml (m, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "testcomponent: {}\n"
                   "...\n");

  g_clear_pointer (&yaml_string, modulemd_yaml_string_free);
  yaml_emitter_delete (&emitter);
  yaml_emitter_initialize (&emitter);
  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));
  yaml_emitter_set_output (&emitter, write_yaml_string, (void *)yaml_string);

  modulemd_component_set_rationale (MODULEMD_COMPONENT (m), "testrationale");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (m), 42);
  modulemd_component_module_set_repository (m, "testrepository");
  modulemd_component_module_set_ref (m, "testref");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_component_module_emit_yaml (m, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "testcomponent:\n"
                   "  rationale: testrationale\n"
                   "  repository: testrepository\n"
                   "  ref: testref\n"
                   "  buildorder: 42\n"
                   "...\n");
}


static void
component_module_test_parse_yaml (void)
{
  g_autoptr (ModulemdComponentModule) m = NULL;
  g_autoptr (GError) error = NULL;
  int result;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  yaml_path = g_strdup_printf ("%s/cm.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);
  result = yaml_parser_parse (&parser, &event);
  g_assert_cmpint (result, ==, 1);
  g_assert_cmpint (event.type, ==, YAML_SCALAR_EVENT);
  yaml_event_delete (&event);

  m = modulemd_component_module_parse_yaml (
    &parser, "includedmodule", TRUE, &error);
  g_assert_nonnull (m);
  g_assert_true (MODULEMD_IS_COMPONENT_MODULE (m));
  g_assert_cmpstr (modulemd_component_get_name (MODULEMD_COMPONENT (m)),
                   ==,
                   "includedmodule");
  g_assert_cmpstr (modulemd_component_get_rationale (MODULEMD_COMPONENT (m)),
                   ==,
                   "Included in the stack, just because.");
  g_assert_cmpint (
    modulemd_component_get_buildorder (MODULEMD_COMPONENT (m)), ==, 100);
  g_assert_cmpstr (modulemd_component_module_get_repository (m),
                   ==,
                   "https://pagure.io/includedmodule.git");
  g_assert_cmpstr (
    modulemd_component_module_get_ref (m), ==, "somecoolbranchname");
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.
  g_test_add_func ("/modulemd/v2/component/module/construct",
                   component_module_test_construct);

  g_test_add_func ("/modulemd/v2/component/module/equals",
                   component_module_test_equals);

  g_test_add_func ("/modulemd/v2/component/module/copy",
                   component_module_test_copy);

  g_test_add_func ("/modulemd/v2/component/module/yaml/emit",
                   component_module_test_emit_yaml);

  g_test_add_func ("/modulemd/v2/component/module/yaml/parse",
                   component_module_test_parse_yaml);

  return g_test_run ();
}
