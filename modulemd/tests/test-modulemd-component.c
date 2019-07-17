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
#include "modulemd-component.h"

#include <glib.h>
#include <locale.h>

typedef struct _ComponentFixture
{
  ModulemdComponent *component;
} ComponentFixture;

static void
modulemd_component_set_up (ComponentFixture *fixture, gconstpointer user_data)
{
  fixture->component = modulemd_component_new ();
}

static void
modulemd_component_tear_down (ComponentFixture *fixture,
                              gconstpointer user_data)
{
  g_object_unref (fixture->component);
}

static void
modulemd_component_test_create (ComponentFixture *fixture,
                                gconstpointer user_data)
{
  g_assert_true (MODULEMD_IS_COMPONENT (fixture->component));
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.
  g_test_add ("/modulemd/component/test_create",
              ComponentFixture,
              NULL,
              modulemd_component_set_up,
              modulemd_component_test_create,
              modulemd_component_tear_down);

  return g_test_run ();
}
