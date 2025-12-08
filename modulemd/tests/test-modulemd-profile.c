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

#include "modulemd-profile.h"
#include "private/glib-extensions.h"
#include "private/modulemd-profile-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _ProfileFixture
{
} ProfileFixture;

static void
profile_test_construct_regular (void)
{
  g_autoptr (ModulemdProfile) p = NULL;
  g_auto (GStrv) rpms = NULL;

  /* Test that the new() function works */
  p = modulemd_profile_new ("testprofile");
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));
  g_assert_cmpstr (modulemd_profile_get_name (p), ==, "testprofile");
  g_assert_null (modulemd_profile_get_description (p, "C"));
  rpms = modulemd_profile_get_rpms_as_strv (p);
  g_assert_nonnull (rpms);
  g_assert_cmpint (g_strv_length (rpms), ==, 0);
  g_clear_object (&p);

  /* Test that object instantiation works with a name */
  p = g_object_new (MODULEMD_TYPE_PROFILE, "name", "testprofile", NULL);
  g_assert_true (MODULEMD_IS_PROFILE (p));
  g_assert_cmpstr (modulemd_profile_get_name (p), ==, "testprofile");
  g_clear_object (&p);
}


/* Test that we abort with a NULL name to new() */
static void
profile_test_construct_new_null (void)
{
  if (g_test_subprocess ())
    {
      g_autoptr (ModulemdProfile) p = NULL;
      p = modulemd_profile_new (NULL);
      return;
    }
  g_test_trap_subprocess (NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
  g_test_trap_assert_failed ();
}


/* Test that we abort if we instantiate without a name */
static void
profile_test_construct_init_no_name (void)
{
  if (g_test_subprocess ())
    {
      g_autoptr (ModulemdProfile) p = NULL;
      p = g_object_new (MODULEMD_TYPE_PROFILE, NULL);
      return;
    }
  g_test_trap_subprocess (NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
  g_test_trap_assert_failed ();
}


/* Test that we abort if we instantiate with a NULL name */
static void
profile_test_construct_init_null_name (void)
{
  if (g_test_subprocess ())
    {
      g_autoptr (ModulemdProfile) p = NULL;

      p = g_object_new (MODULEMD_TYPE_PROFILE, "name", NULL, NULL);
      return;
    }
  g_test_trap_subprocess (NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
  g_test_trap_assert_failed ();
}


static void
profile_test_equals (void)
{
  g_autoptr (ModulemdProfile) p_1 = NULL;
  g_autoptr (ModulemdProfile) p_2 = NULL;

  /*Test 2 objects with same name*/
  p_1 = modulemd_profile_new ("testprofile");
  g_assert_nonnull (p_1);
  g_assert_true (MODULEMD_IS_PROFILE (p_1));

  p_2 = modulemd_profile_new ("testprofile");
  g_assert_nonnull (p_2);
  g_assert_true (MODULEMD_IS_PROFILE (p_2));

  g_assert_true (modulemd_profile_equals (p_1, p_2));

  g_clear_object (&p_1);
  g_clear_object (&p_2);

  /*Test 2 objects with different name*/
  p_1 = modulemd_profile_new ("testing");
  g_assert_nonnull (p_1);
  g_assert_true (MODULEMD_IS_PROFILE (p_1));

  p_2 = modulemd_profile_new ("testprofile");
  g_assert_nonnull (p_2);
  g_assert_true (MODULEMD_IS_PROFILE (p_2));

  g_assert_false (modulemd_profile_equals (p_1, p_2));

  g_clear_object (&p_1);
  g_clear_object (&p_2);

  /* Test 2 profile objects with same name and description */
  p_1 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_1, "a test");
  g_assert_nonnull (p_1);
  g_assert_true (MODULEMD_IS_PROFILE (p_1));

  p_2 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_2, "a test");
  g_assert_nonnull (p_2);
  g_assert_true (MODULEMD_IS_PROFILE (p_2));

  g_assert_true (modulemd_profile_equals (p_1, p_2));

  g_clear_object (&p_1);
  g_clear_object (&p_2);

  /* Test 2 profile objects with same name and different description */
  p_1 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_1, "a test");
  g_assert_nonnull (p_1);
  g_assert_true (MODULEMD_IS_PROFILE (p_1));

  p_2 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_2, "b test");
  g_assert_nonnull (p_2);
  g_assert_true (MODULEMD_IS_PROFILE (p_2));

  g_assert_false (modulemd_profile_equals (p_1, p_2));

  g_clear_object (&p_1);
  g_clear_object (&p_2);

  /* Test 2 profile objects with same name, description, and rpms */
  p_1 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_1, "a test");
  modulemd_profile_add_rpm (p_1, "testrpm");
  g_assert_nonnull (p_1);
  g_assert_true (MODULEMD_IS_PROFILE (p_1));

  p_2 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_2, "a test");
  modulemd_profile_add_rpm (p_2, "testrpm");
  g_assert_nonnull (p_2);
  g_assert_true (MODULEMD_IS_PROFILE (p_2));

  g_assert_true (modulemd_profile_equals (p_1, p_2));

  g_clear_object (&p_1);
  g_clear_object (&p_2);

  /* Test 2 profile objects with same name, description, and different rpms */
  p_1 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_1, "a test");
  modulemd_profile_add_rpm (p_1, "testrpm");
  g_assert_nonnull (p_1);
  g_assert_true (MODULEMD_IS_PROFILE (p_1));

  p_2 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_2, "a test");
  modulemd_profile_add_rpm (p_2, "testingrpm");
  g_assert_nonnull (p_2);
  g_assert_true (MODULEMD_IS_PROFILE (p_2));

  g_assert_false (modulemd_profile_equals (p_1, p_2));

  g_clear_object (&p_1);
  g_clear_object (&p_2);

  /*Compare two RPM sets where the first sorted value matches and the second does not.*/
  p_1 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_1, "a test");
  modulemd_profile_add_rpm (p_1, "a");
  modulemd_profile_add_rpm (p_1, "b");
  g_assert_nonnull (p_1);
  g_assert_true (MODULEMD_IS_PROFILE (p_1));

  p_2 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_2, "a test");
  modulemd_profile_add_rpm (p_1, "a");
  modulemd_profile_add_rpm (p_1, "c");
  g_assert_nonnull (p_2);
  g_assert_true (MODULEMD_IS_PROFILE (p_2));

  g_assert_false (modulemd_profile_equals (p_1, p_2));

  g_clear_object (&p_1);
  g_clear_object (&p_2);

  /*Compare two RPM sets where the first sorted value matches, but one has more entries than the other.*/
  p_1 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_1, "a test");
  modulemd_profile_add_rpm (p_1, "a");
  modulemd_profile_add_rpm (p_1, "b");
  g_assert_nonnull (p_1);
  g_assert_true (MODULEMD_IS_PROFILE (p_1));

  p_2 = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p_2, "a test");
  modulemd_profile_add_rpm (p_1, "a");
  modulemd_profile_add_rpm (p_1, "b");
  modulemd_profile_add_rpm (p_1, "c");
  g_assert_nonnull (p_2);
  g_assert_true (MODULEMD_IS_PROFILE (p_2));

  g_assert_false (modulemd_profile_equals (p_1, p_2));

  g_clear_object (&p_1);
  g_clear_object (&p_2);
}


static void
profile_test_copy (void)
{
  g_autoptr (ModulemdProfile) p = NULL;
  g_autoptr (ModulemdProfile) p_copy = NULL;
  g_auto (GStrv) rpms = NULL;

  p = modulemd_profile_new ("testprofile");
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));
  g_assert_cmpstr (modulemd_profile_get_name (p), ==, "testprofile");
  g_assert_null (modulemd_profile_get_description (p, "C"));
  rpms = modulemd_profile_get_rpms_as_strv (p);
  g_assert_nonnull (rpms);
  g_assert_cmpint (g_strv_length (rpms), ==, 0);
  g_clear_pointer (&rpms, g_strfreev);

  p_copy = modulemd_profile_copy (p);
  g_assert_nonnull (p_copy);
  g_assert_true (MODULEMD_IS_PROFILE (p_copy));
  g_assert_cmpstr (modulemd_profile_get_name (p_copy), ==, "testprofile");
  g_assert_null (modulemd_profile_get_description (p_copy, "C"));
  rpms = modulemd_profile_get_rpms_as_strv (p_copy);
  g_assert_nonnull (rpms);
  g_assert_cmpint (g_strv_length (rpms), ==, 0);
  g_clear_pointer (&rpms, g_strfreev);
  g_clear_object (&p);
  g_clear_object (&p_copy);

  /* Test copying profile with a description */
  p = modulemd_profile_new ("testprofile");
  modulemd_profile_set_description (p, "a test");
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));
  g_assert_cmpstr (modulemd_profile_get_name (p), ==, "testprofile");
  g_assert_cmpstr (modulemd_profile_get_description (p, "C"), ==, "a test");
  rpms = modulemd_profile_get_rpms_as_strv (p);
  g_assert_nonnull (rpms);
  g_assert_cmpint (g_strv_length (rpms), ==, 0);
  g_clear_pointer (&rpms, g_strfreev);

  p_copy = modulemd_profile_copy (p);
  g_assert_nonnull (p_copy);
  g_assert_true (MODULEMD_IS_PROFILE (p_copy));
  g_assert_cmpstr (modulemd_profile_get_name (p_copy), ==, "testprofile");
  g_assert_cmpstr (
    modulemd_profile_get_description (p_copy, "C"), ==, "a test");
  rpms = modulemd_profile_get_rpms_as_strv (p_copy);
  g_assert_nonnull (rpms);
  g_assert_cmpint (g_strv_length (rpms), ==, 0);
  g_clear_pointer (&rpms, g_strfreev);
  g_clear_object (&p);
  g_clear_object (&p_copy);

  /* Test copying profile with rpms */
  p = modulemd_profile_new ("testprofile");
  modulemd_profile_add_rpm (p, "testrpm");
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));
  g_assert_cmpstr (modulemd_profile_get_name (p), ==, "testprofile");
  g_assert_null (modulemd_profile_get_description (p, "C"));
  rpms = modulemd_profile_get_rpms_as_strv (p);
  g_assert_nonnull (rpms);
  g_assert_cmpint (g_strv_length (rpms), ==, 1);
  g_assert_cmpstr (rpms[0], ==, "testrpm");
  g_clear_pointer (&rpms, g_strfreev);

  p_copy = modulemd_profile_copy (p);
  g_assert_nonnull (p_copy);
  g_assert_true (MODULEMD_IS_PROFILE (p_copy));
  g_assert_cmpstr (modulemd_profile_get_name (p_copy), ==, "testprofile");
  g_assert_null (modulemd_profile_get_description (p_copy, "C"));
  rpms = modulemd_profile_get_rpms_as_strv (p_copy);
  g_assert_nonnull (rpms);
  g_assert_cmpint (g_strv_length (rpms), ==, 1);
  g_assert_cmpstr (rpms[0], ==, "testrpm");
  g_clear_pointer (&rpms, g_strfreev);
  g_clear_object (&p);
  g_clear_object (&p_copy);

  /* Test copying profile with default set */
  p = modulemd_profile_new ("testprofile");
  modulemd_profile_set_default (p);
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));
  g_assert_cmpstr (modulemd_profile_get_name (p), ==, "testprofile");
  g_assert_true (modulemd_profile_is_default (p));

  p_copy = modulemd_profile_copy (p);
  g_assert_nonnull (p_copy);
  g_assert_true (MODULEMD_IS_PROFILE (p_copy));
  g_assert_cmpstr (modulemd_profile_get_name (p_copy), ==, "testprofile");
  g_assert_true (modulemd_profile_is_default (p_copy));
  g_clear_object (&p);
  g_clear_object (&p_copy);

  /* Test copying profile with default unset */
  p = modulemd_profile_new ("testprofile");
  modulemd_profile_unset_default (p);
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));
  g_assert_cmpstr (modulemd_profile_get_name (p), ==, "testprofile");
  g_assert_false (modulemd_profile_is_default (p));

  p_copy = modulemd_profile_copy (p);
  g_assert_nonnull (p_copy);
  g_assert_true (MODULEMD_IS_PROFILE (p_copy));
  g_assert_cmpstr (modulemd_profile_get_name (p_copy), ==, "testprofile");
  g_assert_false (modulemd_profile_is_default (p_copy));
  g_clear_object (&p);
  g_clear_object (&p_copy);
}


static void
profile_test_get_name (void)
{
  g_autoptr (ModulemdProfile) p = NULL;
  g_autofree gchar *name;

  p = modulemd_profile_new ("testprofile");
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));

  g_assert_cmpstr (modulemd_profile_get_name (p), ==, "testprofile");

  g_object_get (p, "name", &name, NULL);
  g_assert_cmpstr (name, ==, "testprofile");
}


/* Test that name is immutable */
static void
profile_test_name_is_immutable (void)
{
  if (g_test_subprocess ())
    {
      g_autoptr (ModulemdProfile) p = NULL;
      p = modulemd_profile_new ("testprofile");

      g_object_set (p, "name", "notatest", NULL);
      return;
    }
  g_test_trap_subprocess (NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
  g_test_trap_assert_failed ();
}


static void
profile_test_get_set_description (void)
{
  g_autoptr (ModulemdProfile) p = NULL;

  p = modulemd_profile_new ("testprofile");
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));

  g_assert_null (modulemd_profile_get_description (p, "C"));

  /* Set a description */
  modulemd_profile_set_description (p, "Some description");
  g_assert_cmpstr (
    modulemd_profile_get_description (p, "C"), ==, "Some description");

  /* Clear the description */
  modulemd_profile_set_description (p, NULL);
  g_assert_null (modulemd_profile_get_description (p, "C"));
}


static void
profile_test_default (void)
{
  g_autoptr (ModulemdProfile) p = NULL;

  p = modulemd_profile_new ("testprofile");
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));

  g_assert_false (modulemd_profile_is_default (p));

  modulemd_profile_set_default (p);
  g_assert_true (modulemd_profile_is_default (p));

  modulemd_profile_unset_default (p);
  g_assert_false (modulemd_profile_is_default (p));
}


static void
profile_test_rpms (void)
{
  g_autoptr (ModulemdProfile) p = NULL;
  g_auto (GStrv) rpms = NULL;

  p = modulemd_profile_new ("testprofile");
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));

  /* Assert we start with 0 rpms */
  rpms = modulemd_profile_get_rpms_as_strv (p);
  g_assert_cmpint (g_strv_length (rpms), ==, 0);
  g_clear_pointer (&rpms, g_strfreev);

  /* Add  some rpms */
  modulemd_profile_add_rpm (p, "test2");
  modulemd_profile_add_rpm (p, "test3");
  modulemd_profile_add_rpm (p, "test1");
  rpms = modulemd_profile_get_rpms_as_strv (p);
  g_assert_cmpint (g_strv_length (rpms), ==, 3);
  // They should be sorted
  g_assert_cmpstr (rpms[0], ==, "test1");
  g_assert_cmpstr (rpms[1], ==, "test2");
  g_assert_cmpstr (rpms[2], ==, "test3");
  g_clear_pointer (&rpms, g_strfreev);

  /* Remove some rpms */
  modulemd_profile_remove_rpm (p, "test2");
  rpms = modulemd_profile_get_rpms_as_strv (p);
  g_assert_cmpint (g_strv_length (rpms), ==, 2);
  // They should be sorted
  g_assert_cmpstr (rpms[0], ==, "test1");
  g_assert_cmpstr (rpms[1], ==, "test3");
  g_clear_pointer (&rpms, g_strfreev);
}


static void
profile_test_parse_yaml (void)
{
  g_autoptr (ModulemdProfile) p = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_auto (GStrv) rpms = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autofree gchar *name = NULL;
  yaml_path = g_strdup_printf ("%s/p.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  parser_skip_headers (&parser);

  /* Parse the name */
  name = modulemd_yaml_parse_string (&parser, &error);
  g_assert_nonnull (name);
  g_assert_cmpstr (name, ==, "default");

  p = modulemd_profile_parse_yaml (&parser, name, TRUE, &error);
  g_assert_nonnull (p);
  g_assert_true (MODULEMD_IS_PROFILE (p));
  g_assert_cmpstr (modulemd_profile_get_name (p), ==, "default");
  g_assert_cmpstr (modulemd_profile_get_description (p, NULL),
                   ==,
                   "An example profile for tests");
  rpms = modulemd_profile_get_rpms_as_strv (p);
  g_assert_cmpint (g_strv_length (rpms), ==, 3);
  g_assert_cmpstr (rpms[0], ==, "bar");
  g_assert_cmpstr (rpms[1], ==, "bar-extras");
  g_assert_cmpstr (rpms[2], ==, "baz");
}


static void
profile_test_emit_yaml (void)
{
  g_autoptr (ModulemdProfile) p = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  p = modulemd_profile_new ("testprofile");
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_profile_emit_yaml (p, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str, ==, "---\ntestprofile: {}\n...\n");

  g_clear_pointer (&yaml_string, modulemd_yaml_string_free);
  yaml_emitter_delete (&emitter);
  yaml_emitter_initialize (&emitter);
  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));
  yaml_emitter_set_output (&emitter, write_yaml_string, (void *)yaml_string);
  modulemd_profile_set_description (p, "A test profile");
  modulemd_profile_add_rpm (p, "test2");
  modulemd_profile_add_rpm (p, "test3");
  modulemd_profile_add_rpm (p, "test1");
  modulemd_profile_set_default (p);

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_profile_emit_yaml (p, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "testprofile:\n"
                   "  description: A test profile\n"
                   "  rpms:\n"
                   "  - test1\n"
                   "  - test2\n"
                   "  - test3\n"
                   "  default: true\n"
                   "...\n");
}


static void
profile_test_quoting_yaml (void)
{
  g_autoptr (ModulemdProfile) p = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  p = modulemd_profile_new ("0");
  modulemd_profile_set_description (p, "1");
  modulemd_profile_add_rpm (p, "2");
  modulemd_profile_set_default (p);

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_profile_emit_yaml (p, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "\"0\":\n"
                   "  description: \"1\"\n"
                   "  rpms:\n"
                   "  - \"2\"\n"
                   "  default: true\n"
                   "...\n");
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add_func ("/modulemd/v2/profile/construct/regular",
                   profile_test_construct_regular);
  g_test_add_func ("/modulemd/v2/profile/construct/new_null",
                   profile_test_construct_new_null);
  g_test_add_func ("/modulemd/v2/profile/construct/init_no_name",
                   profile_test_construct_init_no_name);
  g_test_add_func ("/modulemd/v2/profile/construct/init_null_name",
                   profile_test_construct_init_null_name);

  g_test_add_func ("/modulemd/v2/profile/equals", profile_test_equals);

  g_test_add_func ("/modulemd/v2/profile/copy", profile_test_copy);

  g_test_add_func ("/modulemd/v2/profile/get_name", profile_test_get_name);

  g_test_add_func ("/modulemd/v2/profile/name_is_immutable",
                   profile_test_name_is_immutable);

  g_test_add_func ("/modulemd/v2/profile/get_set_description",
                   profile_test_get_set_description);

  g_test_add_func ("/modulemd/v2/profile/default", profile_test_default);

  g_test_add_func ("/modulemd/v2/profile/rpms", profile_test_rpms);

  g_test_add_func ("/modulemd/v2/profile/yaml/parse", profile_test_parse_yaml);

  g_test_add_func ("/modulemd/v2/profile/yaml/emit", profile_test_emit_yaml);

  g_test_add_func ("/modulemd/v2/profile/yaml/quoting",
                   profile_test_quoting_yaml);

  return g_test_run ();
}
