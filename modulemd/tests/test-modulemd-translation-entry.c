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

#include "modulemd-translation-entry.h"
#include "private/glib-extensions.h"
#include "private/modulemd-translation-entry-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _TranslationEntryFixture
{
} TranslationEntryFixture;

gboolean signaled = FALSE;

static void
sigtrap_handler (int UNUSED (sig_num))
{
  signaled = TRUE;
}

static void
translation_entry_test_construct (void)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_auto (GStrv) profile_names;

  /* Test that the new() function works */
  te = modulemd_translation_entry_new ("en_US");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_US");
  g_assert_null (modulemd_translation_entry_get_summary (te));
  g_assert_null (modulemd_translation_entry_get_description (te));
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te, "test"));
  g_clear_object (&te);


  /* Test that standard object instantiation works with a locale */
  // clang-format off
  te = g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY,
                     "locale", "en_GB",
                     NULL);
  // clang-format on
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_clear_object (&te);


  /* Test that standard object instantiation works with a locale and summary */
  // clang-format off
  te = g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY,
                     "locale", "en_GB",
                     "summary", "foobar",
                     NULL);
  // clang-format on
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_assert_cmpstr (modulemd_translation_entry_get_summary (te), ==, "foobar");
  g_assert_null (modulemd_translation_entry_get_description (te));
  g_clear_object (&te);


  /* Test that standard object instantiation works with a locale and description */
  // clang-format off
  te = g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY,
                     "locale", "en_GB",
                     "description", "barfoo",
                     NULL);
  // clang-format on
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_assert_null (modulemd_translation_entry_get_summary (te));
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te), ==, "barfoo");
  g_clear_object (&te);


  /* Test that standard object instantiation works with a locale, summary and description */
  // clang-format off
  te = g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY,
                     "locale", "en_GB",
                     "summary", "brown fox",
                     "description", "jumped",
                     NULL);
  // clang-format on
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_assert_cmpstr (
    modulemd_translation_entry_get_summary (te), ==, "brown fox");
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te), ==, "jumped");
  g_clear_object (&te);


  /* Test that we abort if we call new() with a NULL locale */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  te = modulemd_translation_entry_new (NULL);
  g_assert_true (signaled);
  g_clear_object (&te);


  /* Test that we abort if we instatiate without a locale */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  te = g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY, NULL);
  g_assert_true (signaled);
  g_clear_object (&te);


  /* Test that we abort if we instatiate with a NULL locale */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  // clang-format off
  te = g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY,
                     "locale", NULL,
                     NULL);
  // clang-format on
  g_assert_true (signaled);
  g_clear_object (&te);
}


static void
translation_entry_test_copy (void)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_autoptr (ModulemdTranslationEntry) te_copy = NULL;
  g_auto (GStrv) profile_names;

  /* Test copying an empty translation entry */
  te = modulemd_translation_entry_new ("en_GB");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_assert_null (modulemd_translation_entry_get_summary (te));
  g_assert_null (modulemd_translation_entry_get_description (te));
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te, "test"));
  g_clear_pointer (&profile_names, g_strfreev);

  te_copy = modulemd_translation_entry_copy (te);
  g_assert_nonnull (te_copy);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te_copy));
  g_assert_cmpstr (
    modulemd_translation_entry_get_locale (te_copy), ==, "en_GB");
  g_assert_null (modulemd_translation_entry_get_summary (te_copy));
  g_assert_null (modulemd_translation_entry_get_description (te_copy));
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te_copy);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te_copy, "test"));
  g_clear_pointer (&profile_names, g_strfreev);
  g_clear_object (&te);
  g_clear_object (&te_copy);

  /* Test copying a translation entry with a summary */
  te = modulemd_translation_entry_new ("en_GB");
  modulemd_translation_entry_set_summary (te, "foobar");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_assert_cmpstr (modulemd_translation_entry_get_summary (te), ==, "foobar");
  g_assert_null (modulemd_translation_entry_get_description (te));
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te, "test"));
  g_clear_pointer (&profile_names, g_strfreev);

  te_copy = modulemd_translation_entry_copy (te);
  g_assert_nonnull (te_copy);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te_copy));
  g_assert_cmpstr (
    modulemd_translation_entry_get_locale (te_copy), ==, "en_GB");
  g_assert_cmpstr (
    modulemd_translation_entry_get_summary (te_copy), ==, "foobar");
  g_assert_null (modulemd_translation_entry_get_description (te_copy));
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te_copy);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te_copy, "test"));
  g_clear_pointer (&profile_names, g_strfreev);
  g_clear_object (&te);
  g_clear_object (&te_copy);

  /* Test copying a translation entry with a description */
  te = modulemd_translation_entry_new ("en_GB");
  modulemd_translation_entry_set_description (te, "barfoo");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_assert_null (modulemd_translation_entry_get_summary (te));
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te), ==, "barfoo");
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te, "test"));
  g_clear_pointer (&profile_names, g_strfreev);

  te_copy = modulemd_translation_entry_copy (te);
  g_assert_nonnull (te_copy);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te_copy));
  g_assert_cmpstr (
    modulemd_translation_entry_get_locale (te_copy), ==, "en_GB");
  g_assert_null (modulemd_translation_entry_get_summary (te_copy));
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te_copy), ==, "barfoo");
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te_copy);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te_copy, "test"));
  g_clear_pointer (&profile_names, g_strfreev);
  g_clear_object (&te);
  g_clear_object (&te_copy);

  /* Test copying a translation entry with a summary and description */
  te = modulemd_translation_entry_new ("en_GB");
  modulemd_translation_entry_set_summary (te, "foobar");
  modulemd_translation_entry_set_description (te, "barfoo");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_assert_cmpstr (modulemd_translation_entry_get_summary (te), ==, "foobar");
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te), ==, "barfoo");
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te, "test"));
  g_clear_pointer (&profile_names, g_strfreev);

  te_copy = modulemd_translation_entry_copy (te);
  g_assert_nonnull (te_copy);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te_copy));
  g_assert_cmpstr (
    modulemd_translation_entry_get_locale (te_copy), ==, "en_GB");
  g_assert_cmpstr (
    modulemd_translation_entry_get_summary (te_copy), ==, "foobar");
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te_copy), ==, "barfoo");
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te_copy);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te_copy, "test"));
  g_clear_pointer (&profile_names, g_strfreev);
  g_clear_object (&te);
  g_clear_object (&te_copy);

  /* Test copying a translation entry with a summary, description and a profile */
  te = modulemd_translation_entry_new ("en_GB");
  modulemd_translation_entry_set_summary (te, "foobar");
  modulemd_translation_entry_set_description (te, "barfoo");
  modulemd_translation_entry_set_profile_description (te, "test", "brown fox");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_assert_cmpstr (modulemd_translation_entry_get_summary (te), ==, "foobar");
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te), ==, "barfoo");
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 1);
  g_assert_cmpstr (
    modulemd_translation_entry_get_profile_description (te, "test"),
    ==,
    "brown fox");
  g_clear_pointer (&profile_names, g_strfreev);

  te_copy = modulemd_translation_entry_copy (te);
  g_assert_nonnull (te_copy);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te_copy));
  g_assert_cmpstr (
    modulemd_translation_entry_get_locale (te_copy), ==, "en_GB");
  g_assert_cmpstr (
    modulemd_translation_entry_get_summary (te_copy), ==, "foobar");
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te_copy), ==, "barfoo");
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te_copy);
  g_assert_nonnull (profile_names);
  g_assert_cmpint (g_strv_length (profile_names), ==, 1);
  g_assert_cmpstr (
    modulemd_translation_entry_get_profile_description (te_copy, "test"),
    ==,
    "brown fox");
  g_clear_pointer (&profile_names, g_strfreev);
  g_clear_object (&te);
  g_clear_object (&te_copy);
}


static void
translation_entry_test_get_locale (void)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_autofree gchar *locale;

  te = modulemd_translation_entry_new ("en_US");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));

  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_US");

  g_object_get (te, "locale", &locale, NULL);
  g_assert_cmpstr (locale, ==, "en_US");

  /* Test that locale is immutable */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  g_object_set (te, "locale", "en_GB", NULL);
  g_assert_true (signaled);
}


static void
translation_entry_test_get_set_summary (void)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_autofree gchar *summary;

  te = modulemd_translation_entry_new ("en_US");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));

  /* Check that summary is empty */
  g_assert_null (modulemd_translation_entry_get_summary (te));
  g_object_get (te, "summary", &summary, NULL);
  g_assert_null (summary);
  g_clear_pointer (&summary, g_free);

  /* Set a summary */
  modulemd_translation_entry_set_summary (te, "foobar");
  g_assert_cmpstr (modulemd_translation_entry_get_summary (te), ==, "foobar");
  g_object_get (te, "summary", &summary, NULL);
  g_assert_cmpstr (summary, ==, "foobar");
  g_clear_pointer (&summary, g_free);

  /* Clear the summary */
  modulemd_translation_entry_set_summary (te, NULL);
  g_assert_null (modulemd_translation_entry_get_summary (te));
  g_object_get (te, "summary", &summary, NULL);
  g_assert_null (summary);
  g_clear_pointer (&summary, g_free);

  /* Try setting summary to unicode */
  modulemd_translation_entry_set_summary (te, "��");
  g_assert_cmpstr (modulemd_translation_entry_get_summary (te), ==, "��");
  g_object_get (te, "summary", &summary, NULL);
  g_assert_cmpstr (summary, ==, "��");
  g_clear_pointer (&summary, g_free);
}


static void
translation_entry_test_get_set_description (void)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_autofree gchar *description;

  te = modulemd_translation_entry_new ("en_US");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));

  /* Check that description is empty */
  g_assert_null (modulemd_translation_entry_get_description (te));
  g_object_get (te, "description", &description, NULL);
  g_assert_null (description);
  g_clear_pointer (&description, g_free);

  /* Set a description */
  modulemd_translation_entry_set_description (te, "foobar");
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te), ==, "foobar");
  g_object_get (te, "description", &description, NULL);
  g_assert_cmpstr (description, ==, "foobar");
  g_clear_pointer (&description, g_free);

  /* Clear the description */
  modulemd_translation_entry_set_description (te, NULL);
  g_assert_null (modulemd_translation_entry_get_description (te));
  g_object_get (te, "description", &description, NULL);
  g_assert_null (description);
  g_clear_pointer (&description, g_free);

  /* Try setting description to unicode */
  modulemd_translation_entry_set_description (te, "��");
  g_assert_cmpstr (modulemd_translation_entry_get_description (te), ==, "��");
  g_object_get (te, "description", &description, NULL);
  g_assert_cmpstr (description, ==, "��");
  g_clear_pointer (&description, g_free);
}


static void
translation_entry_test_profile_descriptions (void)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_auto (GStrv) profile_names;

  te = modulemd_translation_entry_new ("en_US");
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));

  /* Assert we start with 0 profiles */
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_cmpint (g_strv_length (profile_names), ==, 0);
  g_assert_false (g_strv_contains ((const gchar **)profile_names, "test1"));
  g_assert_false (g_strv_contains ((const gchar **)profile_names, "test2"));
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te, "test1"));
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te, "test2"));
  g_clear_pointer (&profile_names, g_strfreev);

  /* Add a profile */
  modulemd_translation_entry_set_profile_description (te, "test1", "foobar");
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_cmpint (g_strv_length (profile_names), ==, 1);
  g_assert_true (g_strv_contains ((const gchar **)profile_names, "test1"));
  g_assert_false (g_strv_contains ((const gchar **)profile_names, "test2"));
  g_assert_cmpstr (
    modulemd_translation_entry_get_profile_description (te, "test1"),
    ==,
    "foobar");
  g_assert_null (
    modulemd_translation_entry_get_profile_description (te, "test2"));
  g_clear_pointer (&profile_names, g_strfreev);

  /* Add a second profile */
  modulemd_translation_entry_set_profile_description (te, "test2", "barfoo");
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_cmpint (g_strv_length (profile_names), ==, 2);
  g_assert_true (g_strv_contains ((const gchar **)profile_names, "test1"));
  g_assert_true (g_strv_contains ((const gchar **)profile_names, "test2"));
  g_assert_cmpstr (
    modulemd_translation_entry_get_profile_description (te, "test1"),
    ==,
    "foobar");
  g_assert_cmpstr (
    modulemd_translation_entry_get_profile_description (te, "test2"),
    ==,
    "barfoo");
  g_clear_pointer (&profile_names, g_strfreev);
}


static void
translation_entry_test_parse_yaml (void)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_PARSER (parser);
  g_autofree gchar *yaml_path = NULL;
  g_auto (GStrv) profile_names = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  yaml_path = g_strdup_printf ("%s/te.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  /* Advance the parser past STREAM_START and DOCUMENT_START */
  parser_skip_document_start (&parser);

  te = modulemd_translation_entry_parse_yaml (&parser, "en_GB", TRUE, &error);
  g_assert_nonnull (te);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (te));
  g_assert_cmpstr (modulemd_translation_entry_get_locale (te), ==, "en_GB");
  g_assert_cmpstr (
    modulemd_translation_entry_get_summary (te), ==, "An example module");
  g_assert_cmpstr (
    modulemd_translation_entry_get_description (te), ==, "An example module.");
  profile_names = modulemd_translation_entry_get_profiles_as_strv (te);
  g_assert_cmpint (g_strv_length (profile_names), ==, 1);
  g_assert_cmpstr (
    modulemd_translation_entry_get_profile_description (te, "profile_a"),
    ==,
    "An example profile");
}


static void
translation_entry_test_emit_yaml (void)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);


  te = modulemd_translation_entry_new ("en_GB");
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_translation_entry_emit_yaml (te, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "en_GB: {}\n"
                   "...\n");

  g_clear_pointer (&yaml_string, modulemd_yaml_string_free);
  yaml_emitter_delete (&emitter);
  yaml_emitter_initialize (&emitter);
  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));
  yaml_emitter_set_output (&emitter, write_yaml_string, (void *)yaml_string);
  modulemd_translation_entry_set_summary (te, "An example module");
  modulemd_translation_entry_set_description (te, "An example module.");
  modulemd_translation_entry_set_profile_description (
    te, "profile_a", "An example profile");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_translation_entry_emit_yaml (te, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "en_GB:\n"
                   "  summary: An example module\n"
                   "  description: An example module.\n"
                   "  profiles:\n"
                   "    profile_a: An example profile\n"
                   "...\n");
}


static void
translation_entry_test_quoting_yaml (void)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  te = modulemd_translation_entry_new ("0");
  modulemd_translation_entry_set_summary (te, "1");
  modulemd_translation_entry_set_description (te, "2");
  modulemd_translation_entry_set_profile_description (te, "3", "4");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_translation_entry_emit_yaml (te, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "\"0\":\n"
                   "  summary: \"1\"\n"
                   "  description: \"2\"\n"
                   "  profiles:\n"
                   "    \"3\": \"4\"\n"
                   "...\n");
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add_func ("/modulemd/v2/translationentry/construct",
                   translation_entry_test_construct);

  g_test_add_func ("/modulemd/v2/translationentry/copy",
                   translation_entry_test_copy);

  g_test_add_func ("/modulemd/v2/translationentry/get_locale",
                   translation_entry_test_get_locale);

  g_test_add_func ("/modulemd/v2/translationentry/get_set_summary",
                   translation_entry_test_get_set_summary);

  g_test_add_func ("/modulemd/v2/translationentry/get_set_description",
                   translation_entry_test_get_set_description);

  g_test_add_func ("/modulemd/v2/translationentry/profile_descriptions",
                   translation_entry_test_profile_descriptions);

  g_test_add_func ("/modulemd/v2/translationentry/yaml/parse",
                   translation_entry_test_parse_yaml);

  g_test_add_func ("/modulemd/v2/translationentry/yaml/emit",
                   translation_entry_test_emit_yaml);

  g_test_add_func ("/modulemd/v2/translationentry/yaml/quoting",
                   translation_entry_test_quoting_yaml);

  return g_test_run ();
}
