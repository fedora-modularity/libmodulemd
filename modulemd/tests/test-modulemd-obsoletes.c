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
#include "modulemd-obsoletes.h"
#include "private/glib-extensions.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-obsoletes-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

static void
obsoletes_test_construct (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;

  /* Test new() with a valid mdversion and module name */
  e = modulemd_obsoletes_new (1, 2, "testmodule", "teststream", "testmessage");
  g_assert_nonnull (e);
  g_assert_true (MODULEMD_IS_OBSOLETES (e));
  g_assert_cmpint (modulemd_obsoletes_get_mdversion (e), ==, 1);
  g_assert_cmpstr (modulemd_obsoletes_get_module_name (e), ==, "testmodule");
  g_assert_cmpstr (modulemd_obsoletes_get_module_stream (e), ==, "teststream");
  g_assert_cmpstr (modulemd_obsoletes_get_message (e), ==, "testmessage");
  g_clear_object (&e);

  /* Test new() with a NULL module_name */
  modulemd_test_signal = 0;
  signal (SIGTRAP, modulemd_test_signal_handler);
  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_ONE, 2, NULL, "teststream", "testmessage");
  g_assert_cmpint (modulemd_test_signal, ==, SIGTRAP);
  /* If we trap the error, obsoletes actually returns a value here, so free it */
  g_clear_object (&e);

  /* Test new() with a NULL module_context */
  modulemd_test_signal = 0;
  signal (SIGTRAP, modulemd_test_signal_handler);
  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_ONE, 2, "testmodule", NULL, "testmessage");
  g_assert_cmpint (modulemd_test_signal, ==, SIGTRAP);
  /* If we trap the error, obsoletes actually returns a value here, so free it */
  g_clear_object (&e);

  /* Test new() with a NULL message */
  modulemd_test_signal = 0;
  signal (SIGTRAP, modulemd_test_signal_handler);
  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_ONE, 2, "testmodule", "teststream", NULL);
  g_assert_cmpint (modulemd_test_signal, ==, SIGTRAP);
  /* If we trap the error, obsoletes actually returns a value here, so free it */
  g_clear_object (&e);
}

static void
obsoletes_test_copy (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;
  g_autoptr (ModulemdObsoletes) e_copy = NULL;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_ONE, 2, "testmodule", "teststream", "testmessage");

  e_copy = modulemd_obsoletes_copy (e);

  g_assert_nonnull (e_copy);
  g_assert_true (MODULEMD_IS_OBSOLETES (e_copy));
  g_assert_cmpint (modulemd_obsoletes_get_mdversion (e_copy), ==, 1);
  g_assert_cmpint (modulemd_obsoletes_get_modified (e_copy), ==, 2);
  g_assert_cmpstr (
    modulemd_obsoletes_get_module_name (e_copy), ==, "testmodule");
  g_assert_cmpstr (
    modulemd_obsoletes_get_module_stream (e_copy), ==, "teststream");
  g_assert_cmpstr (modulemd_obsoletes_get_message (e_copy), ==, "testmessage");

  g_clear_object (&e_copy);

  /* Test if we also copy optional field */
  modulemd_obsoletes_set_module_context (e, "testcontext");
  modulemd_obsoletes_set_eol_date (e, 9);
  modulemd_obsoletes_set_obsoleted_by (e, "nodejs", "12");

  e_copy = modulemd_obsoletes_copy (e);
  g_assert_nonnull (e_copy);
  g_assert_true (MODULEMD_IS_OBSOLETES (e_copy));
  g_assert_cmpint (modulemd_obsoletes_get_mdversion (e_copy), ==, 1);
  g_assert_cmpint (modulemd_obsoletes_get_modified (e_copy), ==, 2);
  g_assert_cmpuint (modulemd_obsoletes_get_reset (e), ==, FALSE);
  g_assert_cmpstr (
    modulemd_obsoletes_get_module_name (e_copy), ==, "testmodule");
  g_assert_cmpstr (
    modulemd_obsoletes_get_module_stream (e_copy), ==, "teststream");
  g_assert_cmpstr (
    modulemd_obsoletes_get_module_context (e_copy), ==, "testcontext");
  g_assert_cmpuint (modulemd_obsoletes_get_eol_date (e_copy), ==, 9);
  g_assert_cmpstr (modulemd_obsoletes_get_message (e_copy), ==, "testmessage");
  g_assert_cmpstr (
    modulemd_obsoletes_get_obsoleted_by_module_name (e_copy), ==, "nodejs");
  g_assert_cmpstr (
    modulemd_obsoletes_get_obsoleted_by_module_stream (e_copy), ==, "12");
  g_clear_object (&e_copy);
  g_clear_object (&e);
}

static void
obsoletes_test_validate (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;
  g_autoptr (GError) error = NULL;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_ONE, 2, "module", "stream", "message");
  g_assert_nonnull (e);

  g_assert_true (modulemd_obsoletes_validate (e, &error));
  g_assert_null (error);
  g_clear_object (&e);

  /* Invalid mdversion */
  e = modulemd_obsoletes_new (999, 2, "module", "stream", "message");
  g_assert_nonnull (e);

  g_assert_false (modulemd_obsoletes_validate (e, &error));
  g_assert_nonnull (error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_clear_object (&e);
  g_clear_error (&error);

  /* Invalid modified */
  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_ONE, 0, "module", "stream", "message");
  g_assert_nonnull (e);

  g_assert_false (modulemd_obsoletes_validate (e, &error));
  g_assert_nonnull (error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_clear_object (&e);
  g_clear_error (&error);

  /* obsoleted_by have to be both set or unset */
  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_ONE, 2, "module", "stream", "message");
  modulemd_obsoletes_set_obsoleted_by_module_name (e,
                                                   "only name without stream");
  g_assert_nonnull (e);

  g_assert_false (modulemd_obsoletes_validate (e, &error));
  g_assert_nonnull (error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_clear_object (&e);
  g_clear_error (&error);

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_ONE, 2, "module", "stream", "message");
  modulemd_obsoletes_set_obsoleted_by_module_stream (
    e, "only stream without module name");
  g_assert_nonnull (e);

  g_assert_false (modulemd_obsoletes_validate (e, &error));
  g_assert_nonnull (error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_clear_object (&e);
  g_clear_error (&error);
}

static void
obsoletes_test_get_mdversion (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;
  guint64 mdversion;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_LATEST, 2, "testmodule", "teststream", "testmessage");
  g_assert_true (MODULEMD_IS_OBSOLETES (e));

  mdversion = modulemd_obsoletes_get_mdversion (e);
  g_assert_cmpuint (mdversion, ==, MD_OBSOLETES_VERSION_LATEST);
}

static void
obsoletes_test_modified (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;
  guint64 modified;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_LATEST, 2, "testmodule", "teststream", "testmessage");
  g_assert_true (MODULEMD_IS_OBSOLETES (e));

  modified = modulemd_obsoletes_get_modified (e);
  g_assert_cmpuint (modified, ==, 2);


  modulemd_obsoletes_set_modified (e, 9);
  modified = modulemd_obsoletes_get_modified (e);
  g_assert_cmpuint (modified, ==, 9);
}

static void
obsoletes_test_reset (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_LATEST, 2, "testmodule", "teststream", "testmessage");
  g_assert_true (MODULEMD_IS_OBSOLETES (e));

  g_assert_cmpuint (modulemd_obsoletes_get_reset (e), ==, FALSE);

  modulemd_obsoletes_set_reset (e, TRUE);
  g_assert_cmpuint (modulemd_obsoletes_get_reset (e), ==, TRUE);

  g_clear_object (&e);
}

static void
obsoletes_test_eol_date (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_LATEST, 2, "testmodule", "teststream", "testmessage");
  g_assert_true (MODULEMD_IS_OBSOLETES (e));

  g_assert_cmpuint (modulemd_obsoletes_get_eol_date (e), ==, 0);

  modulemd_obsoletes_set_eol_date (e, 9);
  g_assert_cmpuint (modulemd_obsoletes_get_eol_date (e), ==, 9);
}

static void
obsoletes_test_message (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;
  const gchar *message;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_LATEST, 2, "testmodule", "teststream", "testmessage");
  g_assert_true (MODULEMD_IS_OBSOLETES (e));

  message = modulemd_obsoletes_get_message (e);
  g_assert_cmpstr (message, ==, "testmessage");

  modulemd_obsoletes_set_message (e, "test");
  message = modulemd_obsoletes_get_message (e);
  g_assert_cmpstr (message, ==, "test");
}

static void
obsoletes_test_context (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;
  const gchar *context;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_LATEST, 2, "testmodule", "teststream", "testmessage");
  g_assert_true (MODULEMD_IS_OBSOLETES (e));

  context = modulemd_obsoletes_get_module_context (e);
  g_assert_null (context);

  modulemd_obsoletes_set_module_context (e, "testcontext");
  context = modulemd_obsoletes_get_module_context (e);
  g_assert_cmpstr (context, ==, "testcontext");
}


static void
obsoletes_test_obsoleted_by (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;
  const gchar *obsoleted_by_module_name;
  const gchar *obsoleted_by_module_stream;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_LATEST, 2, "testmodule", "teststream", "testcontext");
  g_assert_true (MODULEMD_IS_OBSOLETES (e));

  obsoleted_by_module_name =
    modulemd_obsoletes_get_obsoleted_by_module_name (e);
  obsoleted_by_module_stream =
    modulemd_obsoletes_get_obsoleted_by_module_stream (e);
  g_assert_null (obsoleted_by_module_name);
  g_assert_null (obsoleted_by_module_stream);

  modulemd_obsoletes_set_obsoleted_by (e, "nodejs", "12");
  obsoleted_by_module_name =
    modulemd_obsoletes_get_obsoleted_by_module_name (e);
  obsoleted_by_module_stream =
    modulemd_obsoletes_get_obsoleted_by_module_stream (e);
  g_assert_cmpstr (obsoleted_by_module_name, ==, "nodejs");
  g_assert_cmpstr (obsoleted_by_module_stream, ==, "12");
}

static void
obsoletes_test_is_active (void)
{
  g_autoptr (ModulemdObsoletes) e = NULL;

  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_LATEST, 2, "testmodule", "teststream", "testmessage");
  g_assert_true (MODULEMD_IS_OBSOLETES (e));

  g_assert_cmpuint (modulemd_obsoletes_get_eol_date (e), ==, 0);
  g_assert_true (modulemd_obsoletes_is_active (e));

  modulemd_obsoletes_set_eol_date (e, 290001011200);
  g_assert_false (modulemd_obsoletes_is_active (e));

  modulemd_obsoletes_set_eol_date (e, 199901011200);
  g_assert_true (modulemd_obsoletes_is_active (e));
}

static void
obsoletes_test_parse_yaml (void)
{
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdObsoletes) e = NULL;
  int yaml_ret;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;

  /* Validate that we can read the specification without issues */
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_obsoletes_v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
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
                   MODULEMD_YAML_DOC_OBSOLETES);
  g_assert_cmpint (modulemd_subdocument_info_get_mdversion (subdoc), ==, 1);
  g_assert_nonnull (modulemd_subdocument_info_get_yaml (subdoc));

  e = modulemd_obsoletes_parse_yaml (subdoc, TRUE, &error);
  g_assert_no_error (error);
  g_assert_null (error);
  g_assert_nonnull (e);

  g_assert_cmpint (modulemd_obsoletes_get_mdversion (e), ==, 1);
  g_assert_cmpstr (modulemd_obsoletes_get_module_name (e), ==, "nodejs");
  g_assert_cmpstr (modulemd_obsoletes_get_module_stream (e), ==, "11");
  g_assert_cmpint (modulemd_obsoletes_get_modified (e), ==, 201805231425);
  g_assert_cmpstr (modulemd_obsoletes_get_message (e),
                   ==,
                   "Module stream nodejs:11 is no longer supported. It is "
                   "recommended to switch to nodejs:12");

  g_assert_cmpstr (
    modulemd_obsoletes_get_obsoleted_by_module_name (e), ==, "nodejs");
  g_assert_cmpstr (
    modulemd_obsoletes_get_obsoleted_by_module_stream (e), ==, "12");
}

static void
obsoletes_test_emit_yaml (void)
{
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdObsoletes) e = NULL;
  e = modulemd_obsoletes_new (MD_OBSOLETES_VERSION_LATEST,
                              202001012020,
                              "testmodule",
                              "teststream",
                              "testmessage");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (modulemd_obsoletes_emit_yaml (e, &emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_nonnull (yaml_string->str);
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "document: modulemd-obsoletes\n"
                   "version: 1\n"
                   "data:\n"
                   "  modified: 2020-01-01T20:20Z\n"
                   "  module: testmodule\n"
                   "  stream: \"teststream\"\n"
                   "  message: testmessage\n"
                   "...\n");

  modulemd_obsoletes_set_module_context (e, "testcontext");
  modulemd_obsoletes_set_eol_date (e, 202001010000);
  modulemd_obsoletes_set_obsoleted_by (e, "nodejs", "12");

  MMD_REINIT_YAML_STRING (&emitter, yaml_string);
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (modulemd_obsoletes_emit_yaml (e, &emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_nonnull (yaml_string->str);
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "document: modulemd-obsoletes\n"
                   "version: 1\n"
                   "data:\n"
                   "  modified: 2020-01-01T20:20Z\n"
                   "  module: testmodule\n"
                   "  stream: \"teststream\"\n"
                   "  context: testcontext\n"
                   "  eol_date: 2020-01-01T00:00Z\n"
                   "  message: testmessage\n"
                   "  obsoleted_by:\n"
                   "    module: nodejs\n"
                   "    stream: \"12\"\n"
                   "...\n");
}

static void
obsoletes_test_quoting (void)
{
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdObsoletes) e = NULL;
  e = modulemd_obsoletes_new (
    MD_OBSOLETES_VERSION_LATEST, 202001012020, "0", "1", "2");
  modulemd_obsoletes_set_module_context (e, "3");
  modulemd_obsoletes_set_eol_date (e, 202001010000);
  modulemd_obsoletes_set_obsoleted_by (e, "4", "5");

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (modulemd_obsoletes_emit_yaml (e, &emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_assert_nonnull (yaml_string->str);
  g_assert_cmpstr (yaml_string->str,
                   ==,
                   "---\n"
                   "document: modulemd-obsoletes\n"
                   "version: 1\n"
                   "data:\n"
                   "  modified: 2020-01-01T20:20Z\n"
                   "  module: \"0\"\n"
                   "  stream: \"1\"\n"
                   "  context: \"3\"\n"
                   "  eol_date: 2020-01-01T00:00Z\n"
                   "  message: \"2\"\n"
                   "  obsoleted_by:\n"
                   "    module: \"4\"\n"
                   "    stream: \"5\"\n"
                   "...\n");
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add_func ("/modulemd/v2/obsoletes/construct",
                   obsoletes_test_construct);

  g_test_add_func ("/modulemd/v2/obsoletes/copy", obsoletes_test_copy);

  g_test_add_func ("/modulemd/v2/obsoletes/validate", obsoletes_test_validate);

  g_test_add_func ("/modulemd/v2/obsoletes/version",
                   obsoletes_test_get_mdversion);
  g_test_add_func ("/modulemd/v2/obsoletes/modified", obsoletes_test_modified);
  g_test_add_func ("/modulemd/v2/obsoletes/reset", obsoletes_test_reset);
  g_test_add_func ("/modulemd/v2/obsoletes/eol_date", obsoletes_test_eol_date);
  g_test_add_func ("/modulemd/v2/obsoletes/message", obsoletes_test_message);
  g_test_add_func ("/modulemd/v2/obsoletes/context", obsoletes_test_context);
  g_test_add_func ("/modulemd/v2/obsoletes/obsoletes_by",
                   obsoletes_test_obsoleted_by);
  g_test_add_func ("/modulemd/v2/obsoletes/active", obsoletes_test_is_active);

  g_test_add_func ("/modulemd/v2/obsoletes/yaml/parse",
                   obsoletes_test_parse_yaml);
  g_test_add_func ("/modulemd/v2/obsoletes/yaml/emit",
                   obsoletes_test_emit_yaml);
  g_test_add_func ("/modulemd/v2/obsoletes/yaml/quoting",
                   obsoletes_test_quoting);

  return g_test_run ();
}
