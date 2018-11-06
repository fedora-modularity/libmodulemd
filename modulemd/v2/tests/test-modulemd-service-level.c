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

#include "modulemd-service-level.h"

#include <glib.h>
#include <locale.h>
#include <signal.h>

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
service_level_test_construct (ServiceLevelFixture *fixture,
                              gconstpointer user_data)
{
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_autoptr (GDate) eol = NULL;


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


  /* Test that standard object instantiation works with a name and EOL */
  eol = g_date_new_dmy (7, 11, 2018);

  // clang-format off
  sl = g_object_new (MODULEMD_TYPE_SERVICE_LEVEL,
                     "name", "bar",
                     "eol", eol,
                     NULL);
  // clang-format on
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl));
  g_assert_cmpstr (modulemd_service_level_get_name (sl), ==, "bar");
  g_assert_nonnull (modulemd_service_level_get_eol (sl));
  g_assert_cmpint (
    g_date_compare (eol, modulemd_service_level_get_eol (sl)), ==, 0);
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
service_level_test_get_name (ServiceLevelFixture *fixture,
                             gconstpointer user_data)
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
service_level_test_get_set_eol (ServiceLevelFixture *fixture,
                                gconstpointer user_data)
{
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_autoptr (GDate) eol = NULL;
  GDate *returned_eol = NULL;
  g_autoptr (GDate) copied_eol = NULL;
  g_autofree gchar *eol_string = NULL;


  /* First create a service level */
  sl = modulemd_service_level_new ("foo");
  g_assert_nonnull (sl);
  g_assert_true (MODULEMD_IS_SERVICE_LEVEL (sl));

  /* Test that get_eol() returns NULL at first */
  g_assert_null (modulemd_service_level_get_eol (sl));
  g_assert_null (modulemd_service_level_get_eol_string (sl));


  /* Test looking up the EOL by object properties returns NULL */
  // clang-format off
  g_object_get (sl,
                "eol", &eol,
                NULL);
  // clang-format on
  g_assert_null (eol);


  /* Set the EOL with the set_eol() method */
  eol = g_date_new_dmy (7, 11, 2018);
  modulemd_service_level_set_eol (sl, eol);

  returned_eol = modulemd_service_level_get_eol (sl);
  g_assert_nonnull (returned_eol);
  g_assert_true (g_date_valid (returned_eol));
  g_assert_cmpint (g_date_compare (eol, returned_eol), ==, 0);

  // clang-format off
  g_object_get(sl,
               "eol", &copied_eol,
               NULL);
  // clang-format on
  g_assert_nonnull (copied_eol);
  g_assert_true (g_date_valid (copied_eol));
  g_assert_cmpint (g_date_compare (eol, copied_eol), ==, 0);
  g_clear_pointer (&copied_eol, g_date_free);

  eol_string = modulemd_service_level_get_eol_string (sl);
  g_assert_nonnull (eol_string);
  g_assert_cmpstr (eol_string, ==, "2018-11-07");
  g_clear_pointer (&eol_string, g_free);


  /* Set the EOL with the set_eol_ymd() method */
  modulemd_service_level_set_eol_ymd (sl, 2018, 11, 7);

  returned_eol = modulemd_service_level_get_eol (sl);
  g_assert_nonnull (returned_eol);
  g_assert_true (g_date_valid (returned_eol));
  g_assert_cmpint (g_date_compare (eol, returned_eol), ==, 0);

  // clang-format off
  g_object_get(sl,
               "eol", &copied_eol,
               NULL);
  // clang-format on
  g_assert_nonnull (copied_eol);
  g_assert_true (g_date_valid (copied_eol));
  g_assert_cmpint (g_date_compare (eol, copied_eol), ==, 0);
  g_clear_pointer (&copied_eol, g_date_free);

  eol_string = modulemd_service_level_get_eol_string (sl);
  g_assert_nonnull (eol_string);
  g_assert_cmpstr (eol_string, ==, "2018-11-07");
  g_clear_pointer (&eol_string, g_free);


  /* Set an invalid date */
  g_clear_pointer (&eol, g_date_free);
  eol = g_date_new ();
  modulemd_service_level_set_eol (sl, eol);
  g_assert_null (modulemd_service_level_get_eol (sl));

  // clang-format off
  g_object_set (sl,
                "eol", eol,
                NULL);
  // clang-format on
  g_assert_null (modulemd_service_level_get_eol (sl));

  modulemd_service_level_set_eol_ymd (sl, 2018, 2, 31);
  g_assert_null (modulemd_service_level_get_eol (sl));

  modulemd_service_level_set_eol_ymd (sl, 9999, 99, 99);
  g_assert_null (modulemd_service_level_get_eol (sl));

  /* Set the EOL back to something valid */
  modulemd_service_level_set_eol_ymd (sl, 2018, 11, 7);
  returned_eol = modulemd_service_level_get_eol (sl);
  g_assert_nonnull (returned_eol);
  eol_string = modulemd_service_level_get_eol_string (sl);
  g_assert_nonnull (eol_string);
  g_assert_cmpstr (eol_string, ==, "2018-11-07");
  g_clear_pointer (&eol_string, g_free);

  /* Test the remove_eol() function */
  modulemd_service_level_remove_eol (sl);
  g_assert_null (modulemd_service_level_get_eol (sl));
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/v2/servicelevel/construct",
              ServiceLevelFixture,
              NULL,
              NULL,
              service_level_test_construct,
              NULL);

  g_test_add ("/modulemd/v2/servicelevel/get_set_name",
              ServiceLevelFixture,
              NULL,
              NULL,
              service_level_test_get_name,
              NULL);

  g_test_add ("/modulemd/v2/servicelevel/get_set_eol",
              ServiceLevelFixture,
              NULL,
              NULL,
              service_level_test_get_set_eol,
              NULL);

  return g_test_run ();
}
