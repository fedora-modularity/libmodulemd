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

#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>

typedef struct _IntentFixture
{
} IntentFixture;

static void
modulemd_intent_test_init (IntentFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdIntent) intent = NULL;
  g_autoptr (ModulemdIntent) intent_copy = NULL;

  intent = modulemd_intent_new ("intent_a");
  modulemd_intent_set_default_stream (intent, "a_default_stream");
  g_assert_nonnull (intent);

  g_assert_cmpstr (modulemd_intent_peek_intent_name (intent), ==, "intent_a");
  g_assert_cmpstr (
    modulemd_intent_peek_default_stream (intent), ==, "a_default_stream");

  intent_copy = modulemd_intent_copy (intent);
  g_assert_cmpstr (
    modulemd_intent_peek_intent_name (intent_copy), ==, "intent_a");
  g_assert_cmpstr (
    modulemd_intent_peek_default_stream (intent_copy), ==, "a_default_stream");
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");


  g_test_add ("/modulemd/intent/init",
              IntentFixture,
              NULL,
              NULL,
              modulemd_intent_test_init,
              NULL);

  return g_test_run ();
};
