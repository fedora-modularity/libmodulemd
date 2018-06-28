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
#include "modulemd-translation.h"

#include <glib.h>
#include <locale.h>

typedef struct _TranslationFixture
{
} TranslationFixture;


static void
modulemd_translation_test_basic (TranslationFixture *fixture,
                                 gconstpointer user_data)
{
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (ModulemdTranslation) copy = NULL;
  g_autoptr (ModulemdTranslationEntry) entry = NULL;
  g_autoptr (ModulemdTranslationEntry) retrieved_entry = NULL;
  g_autofree gchar *module_name;
  g_autofree gchar *module_stream;
  guint64 mdversion, modified;

  /* Test standard object construction succeeds */
  translation = g_object_new (MODULEMD_TYPE_TRANSLATION, NULL);
  g_assert_nonnull (translation);
  g_assert_null (modulemd_translation_peek_module_name (translation));
  g_assert_null (modulemd_translation_peek_module_stream (translation));
  g_assert_cmpint (modulemd_translation_get_modified (translation), ==, 0);
  g_clear_pointer (&translation, g_object_unref);

  /* Test construction with values set */
  translation =
    modulemd_translation_new ("foomodule", "barstream", 1, 20180628210000llu);

  g_assert_nonnull (translation);
  g_assert_cmpstr (
    modulemd_translation_peek_module_name (translation), ==, "foomodule");
  g_assert_cmpstr (
    modulemd_translation_peek_module_stream (translation), ==, "barstream");
  g_assert_cmpint (modulemd_translation_get_mdversion (translation), ==, 1);
  g_assert_cmpint (
    modulemd_translation_get_modified (translation), ==, 20180628210000llu);

  // clang-format off
  g_object_get (translation,
                "module-name", &module_name,
                "module-stream", &module_stream,
                "mdversion", &mdversion,
                "modified", &modified,
                NULL);
  // clang-format on
  //
  g_assert_cmpstr (module_name, ==, "foomodule");
  g_assert_cmpstr (module_stream, ==, "barstream");
  g_assert_cmpint (mdversion, ==, 1);
  g_assert_cmpint (modified, ==, 20180628210000llu);

  entry = modulemd_translation_entry_new ("en-US");
  modulemd_translation_entry_set_summary (entry, "Summary Text");
  modulemd_translation_entry_set_description (entry, "Desc Text");

  modulemd_translation_add_entry (translation, entry);

  retrieved_entry =
    modulemd_translation_get_entry_by_locale (translation, "en-US");
  g_assert_nonnull (retrieved_entry);
  g_assert_cmpstr (modulemd_translation_entry_peek_summary (retrieved_entry),
                   ==,
                   "Summary Text");
  g_assert_cmpstr (
    modulemd_translation_entry_peek_description (retrieved_entry),
    ==,
    "Desc Text");

  g_clear_pointer (&module_name, g_free);
  g_clear_pointer (&module_stream, g_free);
  g_clear_pointer (&retrieved_entry, g_object_unref);

  copy = modulemd_translation_copy (translation);
  g_assert_nonnull (copy);

  // clang-format off
  g_object_get (copy,
                "module-name", &module_name,
                "module-stream", &module_stream,
                "mdversion", &mdversion,
                "modified", &modified,
                NULL);
  //clang-format on
  g_assert_cmpstr (module_name, ==, "foomodule");
  g_assert_cmpstr (module_stream, ==, "barstream");
  g_assert_cmpint (mdversion, ==, 1);
  g_assert_cmpint (modified, ==, 20180628210000llu);

  retrieved_entry = modulemd_translation_get_entry_by_locale (copy, "en-US");
  g_assert_nonnull (retrieved_entry);
  g_assert_cmpstr (modulemd_translation_entry_peek_summary (retrieved_entry),
                   ==,
                   "Summary Text");
  g_assert_cmpstr (
    modulemd_translation_entry_peek_description (retrieved_entry),
    ==,
    "Desc Text");
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests
  g_test_add ("/modulemd/translation/test_basic",
              TranslationFixture,
              NULL,
              NULL,
              modulemd_translation_test_basic,
              NULL);

  return g_test_run ();
}
