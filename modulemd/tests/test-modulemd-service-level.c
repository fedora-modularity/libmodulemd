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

#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <signal.h>

#include "modulemd-service-level.h"
#include "private/glib-extensions.h"
#include "private/modulemd-service-level-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _ServiceLevelFixture
{
} ServiceLevelFixture;

gboolean signaled = FALSE;

static void
sigtrap_handler (int sig_num)
{
  signaled = TRUE;
}

static void
service_level_test_construct (void)
{
  g_autoptr (ModulemdServiceLevel) sl = NULL;


  /* Test that the new() function works */
  sl = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl));
  g_assert_cmpstr (modulemd_service_level_get_name (sl), ==, "foo");
  g_assert_null (modulemd_service_level_get_eol (sl));
  g_clear_object (&sl);


  /* Test that standard object instantiation works with a name */
  // clang-format off
  sl = g_object_new (MODULEMD_TYPE_SERVICE_LEVEL,
                     "name", "bar",
                     NULL);
  // clang-format on
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl));
  g_assert_cmpstr (modulemd_service_level_get_name (sl), ==, "bar");
  g_assert_null (modulemd_service_level_get_eol (sl));
  g_clear_object (&sl);


  /* Test that we abort if we call new() with a NULL name */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  sl = modulemd_service_level_new (NULL);
  g_assert_true (signaled);
  g_clear_object (&sl);


  /* Test that we abort if we instatiate without a name */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  sl = g_object_new (MODULEMD_TYPE_SERVICE_LEVEL, NULL);
  g_assert_true (signaled);
  g_clear_object (&sl);


  /* Test that we abort if we instatiate with a NULL name */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  // clang-format off
  sl = g_object_new (MODULEMD_TYPE_SERVICE_LEVEL,
                     "name", NULL,
                     NULL);
  // clang-format on
  g_assert_true (signaled);
  g_clear_object (&sl);
}


static void
service_level_test_equals (void)
{
  g_autoptr (ModulemdServiceLevel) sl_1 = NULL;
  g_autoptr (ModulemdServiceLevel) sl_2 = NULL;
  g_autofree gchar *eol_string = NULL;

  /* Test 2 service levels with same name*/
  sl_1 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_1);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_1));

  sl_2 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_2);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_2));

  g_assert_true (modulemd_service_level_equals (sl_1, sl_2));
  g_clear_object (&sl_1);
  g_clear_object (&sl_2);

  /* Test 2 service levels with different names*/
  sl_1 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_1);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_1));

  sl_2 = modulemd_service_level_new ("bar");
  g_assert_nonnull (sl_2);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_2));

  g_assert_false (modulemd_service_level_equals (sl_1, sl_2));
  g_clear_object (&sl_1);
  g_clear_object (&sl_2);

  /* Test 2 service levels with same name and same eol*/
  sl_1 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_1);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_1));
  modulemd_service_level_set_eol_ymd (sl_1, 2018, 11, 13);
  g_assert_nonnull (modulemd_service_level_get_eol (sl_1));

  sl_2 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_2);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_2));
  modulemd_service_level_set_eol_ymd (sl_2, 2018, 11, 13);
  g_assert_nonnull (modulemd_service_level_get_eol (sl_2));

  g_assert_true (modulemd_service_level_equals (sl_1, sl_2));
  g_clear_object (&sl_1);
  g_clear_object (&sl_2);

  /* Test 2 service levels with same name and different eol*/
  sl_1 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_1);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_1));
  modulemd_service_level_set_eol_ymd (sl_1, 2018, 11, 13);
  g_assert_nonnull (modulemd_service_level_get_eol (sl_1));

  sl_2 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_2);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_2));
  modulemd_service_level_set_eol_ymd (sl_2, 1998, 05, 25);
  g_assert_nonnull (modulemd_service_level_get_eol (sl_2));

  g_assert_false (modulemd_service_level_equals (sl_1, sl_2));
  g_clear_object (&sl_1);
  g_clear_object (&sl_2);

  /* Test 2 service levels with same name and 1 invalid eol*/
  sl_1 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_1);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_1));
  modulemd_service_level_set_eol_ymd (sl_1, 9999, 99, 99);
  g_assert_null (modulemd_service_level_get_eol (sl_1));

  sl_2 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_2);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_2));
  modulemd_service_level_set_eol_ymd (sl_2, 1998, 05, 25);
  g_assert_nonnull (modulemd_service_level_get_eol (sl_2));

  g_assert_false (modulemd_service_level_equals (sl_1, sl_2));
  g_clear_object (&sl_1);
  g_clear_object (&sl_2);

  /* Test 2 service levels with same name and both invalid eol*/
  sl_1 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_1);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_1));
  modulemd_service_level_set_eol_ymd (sl_1, 9999, 99, 99);
  g_assert_null (modulemd_service_level_get_eol (sl_1));

  sl_2 = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl_2);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_2));
  modulemd_service_level_set_eol_ymd (sl_2, 9999, 99, 98);
  g_assert_null (modulemd_service_level_get_eol (sl_2));

  g_assert_true (modulemd_service_level_equals (sl_1, sl_2));
  g_clear_object (&sl_1);
  g_clear_object (&sl_2);
}


static void
service_level_test_copy (void)
{
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_autoptr (ModulemdServiceLevel) sl_copy = NULL;
  g_autofree gchar *eol_string = NULL;

  /* Test copying a service level with no EOL */
  sl = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl));
  g_assert_cmpstr (modulemd_service_level_get_name (sl), ==, "foo");
  g_assert_null (modulemd_service_level_get_eol (sl));

  sl_copy = modulemd_service_level_copy (sl);
  g_assert_nonnull (sl_copy);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_copy));
  g_assert_cmpstr (modulemd_service_level_get_name (sl_copy), ==, "foo");
  g_assert_null (modulemd_service_level_get_eol (sl_copy));
  g_clear_object (&sl_copy);

  /* Test copying a service level with an EOL */
  modulemd_service_level_set_eol_ymd (sl, 2018, 11, 13);

  sl_copy = modulemd_service_level_copy (sl);
  g_assert_nonnull (sl_copy);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl_copy));
  g_assert_cmpstr (modulemd_service_level_get_name (sl_copy), ==, "foo");
  g_assert_nonnull (modulemd_service_level_get_eol (sl_copy));
  eol_string = modulemd_service_level_get_eol_as_string (sl_copy);
  g_assert_cmpstr (eol_string, ==, "2018-11-13");
}


static void
service_level_test_get_name (void)
{
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_autofree gchar *name = NULL;


  /* First create a service level */
  sl = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl));

  /* Test that get_name() returns the correct value */
  g_assert_cmpstr (modulemd_service_level_get_name (sl), ==, "foo");


  /* Test looking up the name by object properties */
  // clang-format off
  g_object_get (sl,
                "name", &name,
                NULL);
  // clang-format on
  g_assert_cmpstr (name, ==, "foo");


  /* Test that trying to set the name by object properties fails.
   * The name must be immutable for the life of the object.
   */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  // clang-format off
  g_object_set (sl,
                "name", "bar",
                NULL);
  // clang-format on
  g_assert_true (signaled);
}


static void
service_level_test_get_set_eol (void)
{
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_autoptr (GDate) eol = NULL;
  GDate *returned_eol = NULL;
  g_autofree gchar *eol_string = NULL;


  /* First create a service level */
  sl = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl));

  /* Test that get_eol() returns NULL at first */
  g_assert_null (modulemd_service_level_get_eol (sl));
  g_assert_null (modulemd_service_level_get_eol_as_string (sl));


  /* Set the EOL with the set_eol() method */
  eol = g_date_new_dmy (7, 11, 2018);
  modulemd_service_level_set_eol (sl, eol);

  returned_eol = modulemd_service_level_get_eol (sl);
  g_assert_nonnull (returned_eol);
  g_assert_true (g_date_valid (returned_eol));
  g_assert_cmpint (g_date_compare (eol, returned_eol), ==, 0);

  eol_string = modulemd_service_level_get_eol_as_string (sl);
  g_assert_nonnull (eol_string);
  g_assert_cmpstr (eol_string, ==, "2018-11-07");
  g_clear_pointer (&eol_string, g_free);


  /* Set the EOL with the set_eol_ymd() method */
  modulemd_service_level_set_eol_ymd (sl, 2018, 11, 7);

  returned_eol = modulemd_service_level_get_eol (sl);
  g_assert_nonnull (returned_eol);
  g_assert_true (g_date_valid (returned_eol));
  g_assert_cmpint (g_date_compare (eol, returned_eol), ==, 0);

  eol_string = modulemd_service_level_get_eol_as_string (sl);
  g_assert_nonnull (eol_string);
  g_assert_cmpstr (eol_string, ==, "2018-11-07");
  g_clear_pointer (&eol_string, g_free);


  /* Set an invalid date */
  g_clear_pointer (&eol, g_date_free);
  eol = g_date_new ();
  modulemd_service_level_set_eol (sl, eol);
  g_assert_null (modulemd_service_level_get_eol (sl));

  modulemd_service_level_set_eol_ymd (sl, 2018, 2, 31);
  g_assert_null (modulemd_service_level_get_eol (sl));

  modulemd_service_level_set_eol_ymd (sl, 9999, 99, 99);
  g_assert_null (modulemd_service_level_get_eol (sl));

  /* Set the EOL back to something valid */
  modulemd_service_level_set_eol_ymd (sl, 2018, 11, 7);
  returned_eol = modulemd_service_level_get_eol (sl);
  g_assert_nonnull (returned_eol);
  eol_string = modulemd_service_level_get_eol_as_string (sl);
  g_assert_nonnull (eol_string);
  g_assert_cmpstr (eol_string, ==, "2018-11-07");
  g_clear_pointer (&eol_string, g_free);

  /* Test the remove_eol() function */
  modulemd_service_level_remove_eol (sl);
  g_assert_null (modulemd_service_level_get_eol (sl));
}

static void
service_level_test_parse_yaml (void)
{
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autofree gchar *name = NULL;
  GDate *eol = NULL;
  yaml_path =
    g_strdup_printf ("%s/sl_with_eol.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  /* Advance the parser past STREAM_START, DOCUMENT_START and MAPPING_START */
  parser_skip_headers (&parser);

  /* Read the name */
  name = modulemd_yaml_parse_string (&parser, &error);
  g_assert_nonnull (name);
  g_assert_cmpstr (name, ==, "sl_name");

  sl = modulemd_service_level_parse_yaml (&parser, name, TRUE, &error);
  g_assert_nonnull (sl);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl));
  g_assert_cmpstr (modulemd_service_level_get_name (sl), ==, "sl_name");
  g_assert_nonnull (modulemd_service_level_get_eol (sl));

  eol = modulemd_service_level_get_eol (sl);
  g_assert_cmpint (g_date_get_year (eol), ==, 2018);
  g_assert_cmpint (g_date_get_month (eol), ==, 7);
  g_assert_cmpint (g_date_get_day (eol), ==, 11);
}


static void
service_level_test_emit_yaml (void)
{
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  /* Service Level without EOL */
  sl = modulemd_service_level_new ("foo");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));

  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));

  g_assert_true (modulemd_service_level_emit_yaml (sl, &emitter, &error));

  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));

  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));

  g_assert_cmpstr (yaml_string->str, ==, "---\nfoo: {}\n...\n");

  /* Service Level with EOL */
  g_clear_pointer (&yaml_string, modulemd_yaml_string_free);
  yaml_emitter_delete (&emitter);
  yaml_emitter_initialize (&emitter);
  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));
  yaml_emitter_set_output (&emitter, write_yaml_string, (void *)yaml_string);

  modulemd_service_level_set_eol_ymd (sl, 2018, 11, 13);

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));

  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));

  g_assert_true (modulemd_service_level_emit_yaml (sl, &emitter, &error));

  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));

  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));

  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "foo:\n"
                   "  eol: 2018-11-13\n"
                   "...\n");
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add_func ("/modulemd/v2/servicelevel/construct",
                   service_level_test_construct);

  g_test_add_func ("/modulemd/v2/servicelevel/get_set_name",
                   service_level_test_get_name);

  g_test_add_func ("/modulemd/v2/servicelevel/equals",
                   service_level_test_equals);

  g_test_add_func ("/modulemd/v2/servicelevel/copy", service_level_test_copy);

  g_test_add_func ("/modulemd/v2/servicelevel/get_set_eol",
                   service_level_test_get_set_eol);

  g_test_add_func ("/modulemd/v2/servicelevel/yaml/parse",
                   service_level_test_parse_yaml);

  g_test_add_func ("/modulemd/v2/servicelevel/yaml/emit",
                   service_level_test_emit_yaml);

  return g_test_run ();
}
