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

#include "modulemd-subdocument-info.h"
#include "modulemd-translation.h"
#include "modulemd-translation-entry.h"
#include "private/glib-extensions.h"
#include "private/modulemd-translation-private.h"
#include "private/modulemd-yaml.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/test-utils.h"

typedef struct _TranslationFixture
{
} TranslationFixture;

gboolean signaled = FALSE;

static void
sigtrap_handler (int sig_num)
{
  signaled = TRUE;
}


static void
translation_test_construct (TranslationFixture *fixture,
                            gconstpointer user_data)
{
  g_autoptr (ModulemdTranslation) t = NULL;
  g_auto (GStrv) locales;
  guint64 translation_version = 1;
  guint64 modified = 3;

  /* Test that the new() function works */
  t = modulemd_translation_new (1, "testmodule", "teststream", 2);
  g_assert_nonnull (t);
  g_assert_true (MODULEMD_IS_TRANSLATION (t));
  g_assert_cmpint (modulemd_translation_get_version (t), ==, 1);
  g_assert_cmpstr (modulemd_translation_get_module_name (t), ==, "testmodule");
  g_assert_cmpstr (
    modulemd_translation_get_module_stream (t), ==, "teststream");
  g_assert_cmpint (modulemd_translation_get_modified (t), ==, 2);
  locales = modulemd_translation_get_locales_as_strv (t);
  g_assert_nonnull (locales);
  g_assert_cmpint (g_strv_length (locales), ==, 0);
  g_clear_object (&t);

  /* Test that object_new works */
  // clang-format off
  t = g_object_new (MODULEMD_TYPE_TRANSLATION,
                    "version", translation_version,
                    "module_name", "testmod",
                    "module_stream", "teststr",
                    NULL);
  //clang-format on
  g_assert_nonnull (t);
  g_assert_true (MODULEMD_IS_TRANSLATION (t));
  g_assert_cmpint (modulemd_translation_get_version (t), ==, 1);
  g_assert_cmpstr (modulemd_translation_get_module_name (t), ==, "testmod");
  g_assert_cmpstr (modulemd_translation_get_module_stream (t), ==, "teststr");
  g_assert_cmpint (modulemd_translation_get_modified (t), ==, 0);
  g_clear_object (&t);

  /* Test that object_new works with modified */
  // clang-format off
  t = g_object_new (MODULEMD_TYPE_TRANSLATION,
                    "version", translation_version,
                    "module_name", "testmod",
                    "module_stream", "teststr",
                    "modified", modified,
                    NULL);
  // clang-format on
  g_assert_nonnull (t);
  g_assert_true (MODULEMD_IS_TRANSLATION (t));
  g_assert_cmpint (
    modulemd_translation_get_version (t), ==, translation_version);
  g_assert_cmpstr (modulemd_translation_get_module_name (t), ==, "testmod");
  g_assert_cmpstr (modulemd_translation_get_module_stream (t), ==, "teststr");
  g_assert_cmpint (modulemd_translation_get_modified (t), ==, modified);
  g_clear_object (&t);

  /* Test that object_new does not work without a version */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  // clang-format off
  t = g_object_new (MODULEMD_TYPE_TRANSLATION,
                    "module_name", "testmod",
                    "module_stream", "teststr",
                    NULL);
  // clang-format on
  g_assert_true (signaled);
  g_clear_object (&t);

  /* Test that object_new does not work without a name */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  // clang-format off
  t = g_object_new (MODULEMD_TYPE_TRANSLATION,
                    "version", translation_version,
                    "module_stream", "teststr", NULL);
  // clang-format on
  g_assert_true (signaled);
  g_clear_object (&t);

  /* Test that object_new does not work without a stream */
  signaled = FALSE;
  signal (SIGTRAP, sigtrap_handler);
  // clang-format off
  t = g_object_new (MODULEMD_TYPE_TRANSLATION,
                    "version", translation_version,
                    "module_name", "testmod",
                    NULL);
  // clang-format on
  g_assert_true (signaled);
  g_clear_object (&t);
}

static void
translation_test_copy (TranslationFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdTranslation) t = NULL;
  g_autoptr (ModulemdTranslation) t_copy = NULL;
  ModulemdTranslationEntry *te = NULL;
  g_auto (GStrv) locales;

  t = modulemd_translation_new (1, "testmod", "teststr", 5);
  g_assert_nonnull (t);
  g_assert_true (MODULEMD_IS_TRANSLATION (t));
  g_assert_cmpint (modulemd_translation_get_version (t), ==, 1);
  g_assert_cmpstr (modulemd_translation_get_module_name (t), ==, "testmod");
  g_assert_cmpstr (modulemd_translation_get_module_stream (t), ==, "teststr");
  g_assert_cmpint (modulemd_translation_get_modified (t), ==, 5);

  t_copy = modulemd_translation_copy (t);
  g_assert_nonnull (t_copy);
  g_assert_true (MODULEMD_IS_TRANSLATION (t_copy));
  g_assert_cmpint (modulemd_translation_get_version (t_copy), ==, 1);
  g_assert_cmpstr (
    modulemd_translation_get_module_name (t_copy), ==, "testmod");
  g_assert_cmpstr (
    modulemd_translation_get_module_stream (t_copy), ==, "teststr");
  g_assert_cmpint (modulemd_translation_get_modified (t_copy), ==, 5);
  g_clear_object (&t_copy);

  te = modulemd_translation_entry_new ("en_US");
  modulemd_translation_entry_set_summary (te, "Some summary");
  modulemd_translation_set_translation_entry (t, te);
  g_clear_pointer (&te, g_object_unref);

  t_copy = modulemd_translation_copy (t);
  g_assert_nonnull (t_copy);
  g_assert_true (MODULEMD_IS_TRANSLATION (t_copy));
  g_assert_cmpint (modulemd_translation_get_version (t_copy), ==, 1);
  g_assert_cmpstr (
    modulemd_translation_get_module_name (t_copy), ==, "testmod");
  g_assert_cmpstr (
    modulemd_translation_get_module_stream (t_copy), ==, "teststr");
  g_assert_cmpint (modulemd_translation_get_modified (t_copy), ==, 5);

  locales = modulemd_translation_get_locales_as_strv (t_copy);
  g_assert_nonnull (locales);
  g_assert_cmpint (g_strv_length (locales), ==, 1);
  g_assert_cmpstr (locales[0], ==, "en_US");

  te = modulemd_translation_get_translation_entry (t_copy, "en_US");
  g_assert_nonnull (te);
  g_assert_cmpstr (
    modulemd_translation_entry_get_summary (te), ==, "Some summary");
}

static void
translation_test_validate (TranslationFixture *fixture,
                           gconstpointer user_data)
{
  g_autoptr (ModulemdTranslation) t = NULL;

  t = modulemd_translation_new (1, "testmodule", "teststream", 5);
}

static void
translation_test_set_modified (TranslationFixture *fixture,
                               gconstpointer user_data)
{
  g_autoptr (ModulemdTranslation) t = NULL;

  t = modulemd_translation_new (1, "testmodule", "teststream", 0);
  g_assert_cmpint (modulemd_translation_get_modified (t), ==, 0);

  modulemd_translation_set_modified (t, 42);
  g_assert_cmpint (modulemd_translation_get_modified (t), ==, 42);

  modulemd_translation_set_modified (t, 0);
  g_assert_cmpint (modulemd_translation_get_modified (t), ==, 0);
}

static void
translation_test_translations (TranslationFixture *fixture,
                               gconstpointer user_data)
{
  g_autoptr (ModulemdTranslation) t = NULL;
  ModulemdTranslationEntry *te = NULL;
  g_auto (GStrv) locales;

  t = modulemd_translation_new (1, "testmodule", "teststream", 5);
  te = modulemd_translation_entry_new ("en_US");
  modulemd_translation_entry_set_summary (te, "Some summary");
  modulemd_translation_set_translation_entry (t, te);
  g_clear_pointer (&te, g_object_unref);

  locales = modulemd_translation_get_locales_as_strv (t);
  g_assert_nonnull (locales);
  g_assert_cmpint (g_strv_length (locales), ==, 1);
  g_assert_cmpstr (locales[0], ==, "en_US");

  te = modulemd_translation_get_translation_entry (t, "en_US");
  g_assert_nonnull (te);
  g_assert_cmpstr (
    modulemd_translation_entry_get_summary (te), ==, "Some summary");
}


static void
translation_test_parse_yaml (TranslationFixture *fixture,
                             gconstpointer user_data)
{
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdTranslation) t = NULL;
  int yaml_ret;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;


  /* Validate that we can read the specification without issues */
  yaml_path = g_strdup_printf ("%s/translations/spec.v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rb");
  g_assert_nonnull (yaml_stream);

  yaml_parser_set_input_file (&parser, yaml_stream);

  /* The first event must be the stream start */
  yaml_ret = yaml_parser_parse (&parser, &event);
  g_assert_true (yaml_ret);
  g_assert_cmpint (event.type, ==, YAML_STREAM_START_EVENT);
  yaml_event_delete (&event);

  /* The second event must be the document start */
  yaml_ret = yaml_parser_parse (&parser, &event);
  g_assert_true (yaml_ret);
  g_assert_cmpint (event.type, ==, YAML_DOCUMENT_START_EVENT);
  yaml_event_delete (&event);

  subdoc = modulemd_yaml_parse_document_type (&parser);
  g_assert_nonnull (subdoc);
  g_assert_null (modulemd_subdocument_info_get_gerror (subdoc));

  g_assert_cmpint (modulemd_subdocument_info_get_doctype (subdoc),
                   ==,
                   MODULEMD_YAML_DOC_TRANSLATIONS);
  g_assert_cmpint (modulemd_subdocument_info_get_mdversion (subdoc), ==, 1);
  g_assert_nonnull (modulemd_subdocument_info_get_yaml (subdoc));

  t = modulemd_translation_parse_yaml (subdoc, TRUE, &error);
  g_assert_no_error (error);
  g_assert_null (error);
  g_assert_nonnull (t);

  g_assert_true (modulemd_translation_validate (t, &error));
  g_assert_null (error);

  g_assert_cmpint (modulemd_translation_get_version (t),
                   ==,
                   modulemd_subdocument_info_get_mdversion (subdoc));
}


static void
translation_test_emit_yaml (TranslationFixture *fixture,
                            gconstpointer user_data)
{
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdTranslation) t = NULL;
  g_autoptr (ModulemdTranslationEntry) te = NULL;

  t = modulemd_translation_new (1, "testmodule", "teststream", 42);
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (modulemd_translation_emit_yaml (t, &emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_nonnull (yaml_string->str);
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "document: modulemd-translations\n"
                   "version: 1\n"
                   "data:\n"
                   "  module: testmodule\n"
                   "  stream: teststream\n"
                   "  modified: 42\n"
                   "...\n");


  te = modulemd_translation_entry_new ("en_US");
  modulemd_translation_entry_set_summary (te, "Some summary");
  modulemd_translation_entry_set_description (te, "Test description");
  modulemd_translation_entry_set_profile_description (
    te, "testprofile", "Test Profile Description");
  modulemd_translation_set_translation_entry (t, te);
  g_clear_pointer (&te, g_object_unref);

  MMD_REINIT_YAML_STRING (&emitter, yaml_string);
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (modulemd_translation_emit_yaml (t, &emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_nonnull (yaml_string->str);
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "document: modulemd-translations\n"
                   "version: 1\n"
                   "data:\n"
                   "  module: testmodule\n"
                   "  stream: teststream\n"
                   "  modified: 42\n"
                   "  translations:\n"
                   "  - en_US\n"
                   "  - summary: Some summary\n"
                   "    description: Test description\n"
                   "    profiles:\n"
                   "      testprofile: Test Profile Description\n"
                   "...\n");
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/v2/translation/construct",
              TranslationFixture,
              NULL,
              NULL,
              translation_test_construct,
              NULL);

  g_test_add ("/modulemd/v2/translation/copy",
              TranslationFixture,
              NULL,
              NULL,
              translation_test_copy,
              NULL);

  g_test_add ("/modulemd/v2/translation/validate",
              TranslationFixture,
              NULL,
              NULL,
              translation_test_validate,
              NULL);

  g_test_add ("/modulemd/v2/translation/set_modified",
              TranslationFixture,
              NULL,
              NULL,
              translation_test_set_modified,
              NULL);

  g_test_add ("/modulemd/v2/translation/translations",
              TranslationFixture,
              NULL,
              NULL,
              translation_test_translations,
              NULL);

  g_test_add ("/modulemd/v2/translation/yaml/parse",
              TranslationFixture,
              NULL,
              NULL,
              translation_test_parse_yaml,
              NULL);

  g_test_add ("/modulemd/v2/translation/yaml/emit",
              TranslationFixture,
              NULL,
              NULL,
              translation_test_emit_yaml,
              NULL);

  return g_test_run ();
}
