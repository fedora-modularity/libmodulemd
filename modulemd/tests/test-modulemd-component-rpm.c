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

#include "modulemd-component-rpm.h"
#include "modulemd-component.h"
#include "private/glib-extensions.h"
#include "private/modulemd-component-rpm-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _ComponentRpmFixture
{
} ComponentRpmFixture;


static void
component_rpm_test_construct (void)
{
  g_autoptr (ModulemdComponentRpm) r = NULL;
  ModulemdComponent *mc = NULL;
  gint64 buildorder = 42;

  /* Test that the new() function works */
  r = modulemd_component_rpm_new ("testcomponent");
  mc = MODULEMD_COMPONENT (r);
  g_assert_nonnull (r);
  g_assert_true (MODULEMD_IS_COMPONENT_RPM (r));
  g_assert_cmpint (modulemd_component_get_buildorder (mc), ==, 0);
  g_assert_cmpstr (modulemd_component_get_name (mc), ==, "testcomponent");
  g_assert_null (modulemd_component_get_rationale (mc));
  g_assert_null (modulemd_component_rpm_get_ref (r));
  g_assert_null (modulemd_component_rpm_get_repository (r));
  g_assert_null (modulemd_component_rpm_get_cache (r));
  mc = NULL;
  g_clear_object (&r);

  /* Test that object instantiation works */
  r =
    g_object_new (MODULEMD_TYPE_COMPONENT_RPM, "name", "testcomponent2", NULL);
  g_assert_true (MODULEMD_IS_COMPONENT_RPM (r));
  g_clear_object (&r);

  /* Instantiate with some argument */
  // clang-format off
  r = g_object_new (MODULEMD_TYPE_COMPONENT_RPM,
                    "buildorder", buildorder,
                    "name", "testmodule",
                    "rationale", "Testing all the stuff",
                    "ref", "someref",
                    "repository", "somerepo",
                    "cache", "somecache",
                    NULL);
  // clang-format on
  g_assert_nonnull (r);
  g_assert_true (MODULEMD_IS_COMPONENT_RPM (r));
  mc = MODULEMD_COMPONENT (r);
  g_assert_cmpint (modulemd_component_get_buildorder (mc), ==, buildorder);
  g_assert_cmpstr (modulemd_component_get_name (mc), ==, "testmodule");
  g_assert_cmpstr (
    modulemd_component_get_rationale (mc), ==, "Testing all the stuff");
  g_assert_cmpstr (modulemd_component_rpm_get_ref (r), ==, "someref");
  g_assert_cmpstr (modulemd_component_rpm_get_repository (r), ==, "somerepo");
  g_assert_cmpstr (modulemd_component_rpm_get_cache (r), ==, "somecache");
  mc = NULL;
  g_clear_object (&r);
}


static void
component_rpm_test_equals (void)
{
  g_autoptr (ModulemdComponentRpm) r_1 = NULL;
  g_autoptr (ModulemdComponentRpm) r_2 = NULL;

  /*Everything is same*/
  r_1 = modulemd_component_rpm_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (r_1), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (r_1),
                                    "Testing all the stuff");
  modulemd_component_rpm_set_ref (r_1, "someref");
  modulemd_component_rpm_set_repository (r_1, "somerepo");
  modulemd_component_rpm_set_cache (r_1, "somecache");
  modulemd_component_rpm_set_buildroot (r_1, TRUE);
  modulemd_component_rpm_set_srpm_buildroot (r_1, TRUE);
  modulemd_component_rpm_add_restricted_arch (r_1, "x86_64");
  modulemd_component_rpm_add_restricted_arch (r_1, "i686");
  modulemd_component_rpm_add_multilib_arch (r_1, "ppc64le");
  modulemd_component_rpm_add_multilib_arch (r_1, "s390x");

  r_2 = modulemd_component_rpm_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (r_2), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (r_2),
                                    "Testing all the stuff");
  modulemd_component_rpm_set_ref (r_2, "someref");
  modulemd_component_rpm_set_repository (r_2, "somerepo");
  modulemd_component_rpm_set_cache (r_2, "somecache");
  modulemd_component_rpm_set_buildroot (r_2, TRUE);
  modulemd_component_rpm_set_srpm_buildroot (r_2, TRUE);
  modulemd_component_rpm_add_restricted_arch (r_2, "x86_64");
  modulemd_component_rpm_add_restricted_arch (r_2, "i686");
  modulemd_component_rpm_add_multilib_arch (r_2, "ppc64le");
  modulemd_component_rpm_add_multilib_arch (r_2, "s390x");

  g_assert_true (modulemd_component_equals (MODULEMD_COMPONENT (r_1),
                                            MODULEMD_COMPONENT (r_2)));
  g_clear_object (&r_1);
  g_clear_object (&r_2);

  /*Different ref and cache, everything else matching*/
  r_1 = modulemd_component_rpm_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (r_1), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (r_1),
                                    "Testing all the stuff");
  modulemd_component_rpm_set_ref (r_1, "refA");
  modulemd_component_rpm_set_repository (r_1, "somerepo");
  modulemd_component_rpm_set_cache (r_1, "cacheA");
  modulemd_component_rpm_set_buildroot (r_1, TRUE);
  modulemd_component_rpm_set_srpm_buildroot (r_1, TRUE);
  modulemd_component_rpm_add_restricted_arch (r_1, "x86_64");
  modulemd_component_rpm_add_restricted_arch (r_1, "i686");
  modulemd_component_rpm_add_multilib_arch (r_1, "ppc64le");
  modulemd_component_rpm_add_multilib_arch (r_1, "s390x");

  r_2 = modulemd_component_rpm_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (r_2), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (r_2),
                                    "Testing all the stuff");
  modulemd_component_rpm_set_ref (r_2, "someref");
  modulemd_component_rpm_set_repository (r_2, "somerepo");
  modulemd_component_rpm_set_cache (r_2, "somecache");
  modulemd_component_rpm_set_buildroot (r_2, TRUE);
  modulemd_component_rpm_set_srpm_buildroot (r_2, TRUE);
  modulemd_component_rpm_add_restricted_arch (r_2, "x86_64");
  modulemd_component_rpm_add_restricted_arch (r_2, "i686");
  modulemd_component_rpm_add_multilib_arch (r_2, "ppc64le");
  modulemd_component_rpm_add_multilib_arch (r_2, "s390x");

  g_assert_false (modulemd_component_equals (MODULEMD_COMPONENT (r_1),
                                             MODULEMD_COMPONENT (r_2)));
  g_clear_object (&r_1);
  g_clear_object (&r_2);

  /*Different hashtables,everything else matching*/
  r_1 = modulemd_component_rpm_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (r_1), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (r_1),
                                    "Testing all the stuff");
  modulemd_component_rpm_set_ref (r_1, "someref");
  modulemd_component_rpm_set_repository (r_1, "somerepo");
  modulemd_component_rpm_set_cache (r_1, "somecache");
  modulemd_component_rpm_set_buildroot (r_1, TRUE);
  modulemd_component_rpm_set_srpm_buildroot (r_1, TRUE);
  modulemd_component_rpm_add_restricted_arch (r_1, "x86_65");
  modulemd_component_rpm_add_restricted_arch (r_1, "i687");
  modulemd_component_rpm_add_multilib_arch (r_1, "ppc64le");
  modulemd_component_rpm_add_multilib_arch (r_1, "s390x");

  r_2 = modulemd_component_rpm_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (r_2), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (r_2),
                                    "Testing all the stuff");
  modulemd_component_rpm_set_ref (r_2, "someref");
  modulemd_component_rpm_set_repository (r_2, "somerepo");
  modulemd_component_rpm_set_cache (r_2, "somecache");
  modulemd_component_rpm_set_buildroot (r_2, TRUE);
  modulemd_component_rpm_set_srpm_buildroot (r_2, TRUE);
  modulemd_component_rpm_add_restricted_arch (r_2, "x86_64");
  modulemd_component_rpm_add_restricted_arch (r_2, "i686");
  modulemd_component_rpm_add_multilib_arch (r_2, "ppc64le");
  modulemd_component_rpm_add_multilib_arch (r_2, "s390x");

  g_assert_false (modulemd_component_equals (MODULEMD_COMPONENT (r_1),
                                             MODULEMD_COMPONENT (r_2)));
  g_clear_object (&r_1);
  g_clear_object (&r_2);
}


static void
component_rpm_test_copy (void)
{
  g_autoptr (ModulemdComponentRpm) r_orig = NULL;
  g_autoptr (ModulemdComponentRpm) r = NULL;
  g_auto (GStrv) list = NULL;
  ModulemdComponent *mc = NULL;

  r_orig = modulemd_component_rpm_new ("testmodule");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (r_orig), 42);
  modulemd_component_set_rationale (MODULEMD_COMPONENT (r_orig),
                                    "Testing all the stuff");
  modulemd_component_rpm_set_ref (r_orig, "someref");
  modulemd_component_rpm_set_repository (r_orig, "somerepo");
  modulemd_component_rpm_set_cache (r_orig, "somecache");
  modulemd_component_rpm_set_buildroot (r_orig, TRUE);
  modulemd_component_rpm_set_srpm_buildroot (r_orig, TRUE);
  modulemd_component_rpm_add_restricted_arch (r_orig, "x86_64");
  modulemd_component_rpm_add_restricted_arch (r_orig, "i686");
  modulemd_component_rpm_add_multilib_arch (r_orig, "ppc64le");
  modulemd_component_rpm_add_multilib_arch (r_orig, "s390x");
  g_assert_nonnull (r_orig);
  g_assert_true (MODULEMD_IS_COMPONENT_RPM (r_orig));

  r = MODULEMD_COMPONENT_RPM (
    modulemd_component_copy (MODULEMD_COMPONENT (r_orig), NULL));
  g_assert_nonnull (r);
  g_assert_true (MODULEMD_IS_COMPONENT_RPM (r));
  mc = MODULEMD_COMPONENT (r);
  g_assert_cmpint (modulemd_component_get_buildorder (mc), ==, 42);
  g_assert_cmpstr (modulemd_component_get_name (mc), ==, "testmodule");
  g_assert_cmpstr (
    modulemd_component_get_rationale (mc), ==, "Testing all the stuff");
  g_assert_cmpstr (modulemd_component_rpm_get_ref (r), ==, "someref");
  g_assert_cmpstr (modulemd_component_rpm_get_repository (r), ==, "somerepo");
  g_assert_cmpstr (modulemd_component_rpm_get_cache (r), ==, "somecache");
  g_assert_true (modulemd_component_rpm_get_buildroot (r));
  g_assert_true (modulemd_component_rpm_get_srpm_buildroot (r));

  list = modulemd_component_rpm_get_arches_as_strv (r);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "i686");
  g_assert_cmpstr (list[1], ==, "x86_64");
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_component_rpm_get_multilib_arches_as_strv (r);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "ppc64le");
  g_assert_cmpstr (list[1], ==, "s390x");
  g_clear_pointer (&list, g_strfreev);

  mc = NULL;
  g_clear_object (&r);

  r = MODULEMD_COMPONENT_RPM (
    modulemd_component_copy (MODULEMD_COMPONENT (r_orig), "renamedrpm"));
  g_assert_nonnull (r);
  g_assert_true (MODULEMD_IS_COMPONENT_RPM (r));
  mc = MODULEMD_COMPONENT (r);
  g_assert_cmpint (modulemd_component_get_buildorder (mc), ==, 42);
  g_assert_cmpstr (modulemd_component_get_name (mc), ==, "renamedrpm");
  g_assert_cmpstr (
    modulemd_component_get_rationale (mc), ==, "Testing all the stuff");
  g_assert_cmpstr (modulemd_component_rpm_get_ref (r), ==, "someref");
  g_assert_cmpstr (modulemd_component_rpm_get_repository (r), ==, "somerepo");
  g_assert_cmpstr (modulemd_component_rpm_get_cache (r), ==, "somecache");
  g_assert_true (modulemd_component_rpm_get_buildroot (r));
  g_assert_true (modulemd_component_rpm_get_srpm_buildroot (r));

  list = modulemd_component_rpm_get_arches_as_strv (r);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "i686");
  g_assert_cmpstr (list[1], ==, "x86_64");
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_component_rpm_get_multilib_arches_as_strv (r);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "ppc64le");
  g_assert_cmpstr (list[1], ==, "s390x");
  g_clear_pointer (&list, g_strfreev);

  mc = NULL;
  g_clear_object (&r);
}


static void
component_rpm_test_emit_yaml (void)
{
  g_autoptr (ModulemdComponentRpm) r = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  r = modulemd_component_rpm_new ("testcomponent");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_component_rpm_emit_yaml (r, &emitter, &error));
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

  modulemd_component_set_rationale (MODULEMD_COMPONENT (r), "testrationale");
  modulemd_component_set_buildorder (MODULEMD_COMPONENT (r), 42);
  modulemd_component_rpm_set_repository (r, "testrepository");
  modulemd_component_rpm_set_ref (r, "testref");
  modulemd_component_rpm_set_cache (r, "testcache");
  modulemd_component_rpm_set_buildroot (r, TRUE);
  modulemd_component_rpm_set_srpm_buildroot (r, TRUE);
  modulemd_component_rpm_add_restricted_arch (r, "x86_64");
  modulemd_component_rpm_add_restricted_arch (r, "i686");
  modulemd_component_rpm_add_multilib_arch (r, "ppc64le");
  modulemd_component_rpm_add_multilib_arch (r, "s390x");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_component_rpm_emit_yaml (r, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "testcomponent:\n"
                   "  rationale: testrationale\n"
                   "  repository: testrepository\n"
                   "  cache: testcache\n"
                   "  ref: testref\n"
                   "  buildroot: true\n"
                   "  srpm-buildroot: true\n"
                   "  buildorder: 42\n"
                   "  arches: [i686, x86_64]\n"
                   "  multilib: [ppc64le, s390x]\n"
                   "...\n");
}


static void
component_rpm_test_parse_yaml (void)
{
  g_autoptr (ModulemdComponentRpm) r = NULL;
  g_autoptr (GError) error = NULL;
  int result;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_auto (GStrv) list = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  yaml_path = g_strdup_printf ("%s/cr.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);
  result = yaml_parser_parse (&parser, &event);
  g_assert_cmpint (result, ==, 1);
  g_assert_cmpint (event.type, ==, YAML_SCALAR_EVENT);
  yaml_event_delete (&event);

  r = modulemd_component_rpm_parse_yaml (&parser, "bar", TRUE, &error);
  g_assert_nonnull (r);
  g_assert_true (MODULEMD_IS_COMPONENT_RPM (r));
  g_assert_cmpstr (
    modulemd_component_get_name (MODULEMD_COMPONENT (r)), ==, "bar");
  g_assert_cmpstr (modulemd_component_get_rationale (MODULEMD_COMPONENT (r)),
                   ==,
                   "We need this to demonstrate stuff.");
  g_assert_cmpint (
    modulemd_component_get_buildorder (MODULEMD_COMPONENT (r)), ==, 100);
  g_assert_cmpstr (modulemd_component_rpm_get_repository (r),
                   ==,
                   "https://pagure.io/bar.git");
  g_assert_cmpstr (modulemd_component_rpm_get_ref (r), ==, "26ca0c0");
  g_assert_cmpstr (
    modulemd_component_rpm_get_cache (r), ==, "https://example.com/cache");
  g_assert_true (modulemd_component_rpm_get_buildroot (r));
  g_assert_true (modulemd_component_rpm_get_srpm_buildroot (r));

  list = modulemd_component_rpm_get_arches_as_strv (r);
  g_assert_cmpint (g_strv_length (list), ==, 2);
  g_assert_cmpstr (list[0], ==, "i686");
  g_assert_cmpstr (list[1], ==, "x86_64");
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_component_rpm_get_multilib_arches_as_strv (r);
  g_assert_cmpint (g_strv_length (list), ==, 1);
  g_assert_cmpstr (list[0], ==, "x86_64");
  g_clear_pointer (&list, g_strfreev);
}

static void
component_rpm_test_override_name (void)
{
  g_autoptr (ModulemdComponentRpm) r = NULL;

  r = modulemd_component_rpm_new ("a_key");


  /* Right after construction, the key and name must have the same value */
  g_assert_cmpstr (
    modulemd_component_get_key (MODULEMD_COMPONENT (r)), ==, "a_key");

  g_assert_cmpstr (modulemd_component_get_key (MODULEMD_COMPONENT (r)),
                   ==,
                   modulemd_component_get_name (MODULEMD_COMPONENT (r)));

  modulemd_component_set_name (MODULEMD_COMPONENT (r), "a_name");

  /* The key must remain the same */
  g_assert_cmpstr (
    modulemd_component_get_key (MODULEMD_COMPONENT (r)), ==, "a_key");

  /* The name will now be "a_name" */
  g_assert_cmpstr (
    modulemd_component_get_name (MODULEMD_COMPONENT (r)), ==, "a_name");

  /* Unset the name and make sure it's back to returning the original value */
  modulemd_component_set_name (MODULEMD_COMPONENT (r), NULL);

  g_assert_cmpstr (
    modulemd_component_get_key (MODULEMD_COMPONENT (r)), ==, "a_key");

  g_assert_cmpstr (modulemd_component_get_key (MODULEMD_COMPONENT (r)),
                   ==,
                   modulemd_component_get_name (MODULEMD_COMPONENT (r)));
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.
  g_test_add_func ("/modulemd/v2/component/rpm/construct",component_rpm_test_construct);

  g_test_add_func ("/modulemd/v2/component/rpm/equals",component_rpm_test_equals);

  g_test_add_func ("/modulemd/v2/component/rpm/copy",component_rpm_test_copy);

  g_test_add_func ("/modulemd/v2/component/rpm/yaml/emit",component_rpm_test_emit_yaml);

  g_test_add_func ("/modulemd/v2/component/rpm/yaml/parse",component_rpm_test_parse_yaml);

  g_test_add_func ("/modulemd/v2/component/rpm/override_name",component_rpm_test_override_name);

  return g_test_run ();
}
