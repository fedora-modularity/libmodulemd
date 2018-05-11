/* test-modulemd-intent.c
 *
 * Copyright (C) 2018 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
