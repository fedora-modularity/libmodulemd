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


#include "modulemd.h"
#include "modulemd-translation-entry.h"

#include <glib.h>
#include <locale.h>

typedef struct _EntryFixture
{
} EntryFixture;


static void
modulemd_translation_entry_test_basic (EntryFixture *fixture,
                                       gconstpointer user_data)
{
  g_autoptr (ModulemdTranslationEntry) entry = NULL;
  g_autofree gchar *locale;
  g_autofree gchar *summary;
  g_autofree gchar *description;
  g_autoptr (GHashTable) profile_descriptions = NULL;

  /* Test standard object construction succeeds */
  entry = g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY, NULL);
  g_assert_nonnull (entry);
  /* The default locale should be C.UTF-8 */
  g_assert_cmpstr (
    modulemd_translation_entry_peek_locale (entry), ==, "C.UTF-8");
  g_assert_null (modulemd_translation_entry_peek_summary (entry));
  g_assert_null (modulemd_translation_entry_peek_description (entry));
  g_clear_pointer (&entry, g_object_unref);

  /* Test new() constructor */
  entry = modulemd_translation_entry_new ("en-US");
  g_assert_nonnull (entry);
  g_assert_cmpstr (
    modulemd_translation_entry_peek_locale (entry), ==, "en-US");
  g_assert_null (modulemd_translation_entry_peek_summary (entry));
  g_assert_null (modulemd_translation_entry_peek_description (entry));
  g_clear_pointer (&entry, g_object_unref);

  /* Test construction with values set */
  // clang-format off
  entry = g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY,
                        "locale", "en-US",
                        "summary", "Sample module",
                        "description", "A sample module",
                        NULL);
  // clang-format on

  g_assert_nonnull (entry);
  g_assert_cmpstr (
    modulemd_translation_entry_peek_locale (entry), ==, "en-US");
  g_assert_cmpstr (
    modulemd_translation_entry_peek_summary (entry), ==, "Sample module");
  g_assert_cmpstr (modulemd_translation_entry_peek_description (entry),
                   ==,
                   "A sample module");

  // clang-format off
  g_object_get (entry,
                "locale", &locale,
                "summary", &summary,
                "description", &description,
                NULL);
  // clang-format on
  g_assert_cmpstr (locale, ==, "en-US");
  g_assert_cmpstr (summary, ==, "Sample module");
  g_assert_cmpstr (description, ==, "A sample module");

  modulemd_translation_entry_set_profile_description (
    entry, "a_profile", "Words");

  g_assert_nonnull (
    modulemd_translation_entry_peek_profile_description (entry, "a_profile"));
  g_assert_cmpstr (
    modulemd_translation_entry_peek_profile_description (entry, "a_profile"),
    ==,
    "Words");

  profile_descriptions =
    modulemd_translation_entry_get_all_profile_descriptions (entry);
  g_assert_true (g_hash_table_contains (profile_descriptions, "a_profile"));
  g_assert_cmpstr (
    g_hash_table_lookup (profile_descriptions, "a_profile"), ==, "Words");
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests
  g_test_add ("/modulemd/translation/entry/test_basic",
              EntryFixture,
              NULL,
              NULL,
              modulemd_translation_entry_test_basic,
              NULL);

  return g_test_run ();
}
