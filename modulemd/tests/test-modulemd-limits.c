/*
 * This file is part of libmodulemd
 * Copyright (C) 2018-2020 Red Hat, Inc.
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
#include <locale.h>
#include <yaml.h>

#include "config.h"
#include "modulemd-subdocument-info.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-yaml.h"

/* Return a modulemd-stream-v2 document with an xmd tree of @depth depth.
 * I.e. the total depth of the YAML document will be @depth + 2. */
static GString *
build_modulemd_stream_v2_document (int depth)
{
  g_autoptr (GString) string = g_string_new (NULL);
  g_string_append (string,
                   "document: modulemd\n"
                   "version: 2\n"
                   "data:\n"
                   "  name: demo\n"
                   "  stream: s\n"
                   "  version: 1\n"
                   "  context: c0\n"
                   "  arch: x86_64\n"
                   "  summary: s\n"
                   "  description: d\n"
                   "  license: { module: [MIT] }\n"
                   "  xmd: {\n");
  g_string_append (string, "    ");
  for (int i = 1; i < depth; i++)
    g_string_append (string, "a: {");
  for (int i = 1; i < depth; i++)
    g_string_append (string, "}");
  g_string_append (string, "\n  }\n");

  return g_steal_pointer (&string);
}


static void
module_limits_test_parse_document_type_exact (void)
{
  g_autoptr (GString) yaml_string = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  int yaml_ret;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;

  /* Build a document with a depth matching, i.e. fitting, the recursion limit. */
  yaml_string =
    build_modulemd_stream_v2_document (LIBMODULEMD_RECURSION_LIMIT - 2);

  /* Advance the parser to the first document element */
  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string->str, yaml_string->len);
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

  /* Parse it and exhibit the recursion limit */
  subdoc = modulemd_yaml_parse_document_type (&parser);
  g_assert_nonnull (subdoc);
  g_assert_null (modulemd_subdocument_info_get_gerror (subdoc));
  g_assert_cmpint (modulemd_subdocument_info_get_doctype (subdoc),
                   ==,
                   MODULEMD_YAML_DOC_MODULESTREAM);
  g_assert_cmpint (modulemd_subdocument_info_get_mdversion (subdoc), ==, 2);
  g_assert_nonnull (modulemd_subdocument_info_get_yaml (subdoc));
}


static void
module_limits_test_parse_document_type_oversized (void)
{
  g_autoptr (GString) yaml_string = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  int yaml_ret;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  const GError *error = NULL;

  /* Build a document with a depth exceeding the recursion limit. */
  yaml_string =
    build_modulemd_stream_v2_document (LIBMODULEMD_RECURSION_LIMIT - 2 + 1);

  /* Advance the parser to the first document element */
  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string->str, yaml_string->len);
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

  /* Parse it and exhibit the recursion limit */
  subdoc = modulemd_yaml_parse_document_type (&parser);
  g_assert_nonnull (subdoc);
  error = modulemd_subdocument_info_get_gerror (subdoc);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
}


/* Return an XMD YAML document with a mapping of @depth depth. */
static GString *
build_xmd_mapping_document (int depth)
{
  g_autoptr (GString) string = g_string_new (NULL);
  for (int i = 1; i < depth; i++)
    g_string_append (string, "a: {");
  for (int i = 1; i < depth; i++)
    g_string_append (string, "}");
  g_string_append (string, "\n");

  return g_steal_pointer (&string);
}


static void
module_limits_test_mmd_parse_xmd_mapping_exact (void)
{
  g_autoptr (GString) yaml_string = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  int yaml_ret;
  g_autoptr (GVariant) xmd = NULL;
  g_autoptr (GError) error = NULL;

  /* Build a document with a depth matching, i.e. fitting, the recursion limit. */
  yaml_string = build_xmd_mapping_document (LIBMODULEMD_RECURSION_LIMIT);

  /* Advance the parser to the first document element */
  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string->str, yaml_string->len);
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

  /* Parse it and exhibit the recursion limit */
  xmd = mmd_parse_xmd (&parser, &error);
  g_assert_nonnull (xmd);
  g_assert_null (error);
}


static void
module_limits_test_mmd_parse_xmd_mapping_oversized (void)
{
  g_autoptr (GString) yaml_string = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  int yaml_ret;
  g_autoptr (GVariant) xmd = NULL;
  g_autoptr (GError) error = NULL;

  /* Build a document with a depth exceeding the recursion limit. */
  yaml_string = build_xmd_mapping_document (LIBMODULEMD_RECURSION_LIMIT + 1);

  /* Advance the parser to the first document element */
  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string->str, yaml_string->len);
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

  /* Parse it and exhibit the recursion limit */
  xmd = mmd_parse_xmd (&parser, &error);
  g_assert_null (xmd);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
}


/* Return an XMD YAML document with a recursive sequence of @depth depth. */
static GString *
build_xmd_sequence_document (int depth)
{
  g_autoptr (GString) string = g_string_new (NULL);
  /* mmd_parse_xmd() requires the top-level element to be a mapping. */
  g_string_append (string, "a: ");
  for (int i = 1; i < depth; i++)
    g_string_append (string, "[");
  for (int i = 1; i < depth; i++)
    g_string_append (string, "]");
  g_string_append (string, "\n");

  return g_steal_pointer (&string);
}


static void
module_limits_test_mmd_parse_xmd_sequence_exact (void)
{
  g_autoptr (GString) yaml_string = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  int yaml_ret;
  g_autoptr (GVariant) xmd = NULL;
  g_autoptr (GError) error = NULL;

  /* Build a document with a depth matching, i.e. fitting, the recursion limit. */
  yaml_string = build_xmd_sequence_document (LIBMODULEMD_RECURSION_LIMIT);

  /* Advance the parser to the first document element */
  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string->str, yaml_string->len);
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

  /* Parse it and exhibit the recursion limit */
  xmd = mmd_parse_xmd (&parser, &error);
  g_assert_nonnull (xmd);
  g_assert_null (error);
}


static void
module_limits_test_mmd_parse_xmd_sequence_oversized (void)
{
  g_autoptr (GString) yaml_string = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);
  int yaml_ret;
  g_autoptr (GVariant) xmd = NULL;
  g_autoptr (GError) error = NULL;

  /* Build a document with a depth matching, i.e. fitting, the recursion limit. */
  yaml_string = build_xmd_sequence_document (LIBMODULEMD_RECURSION_LIMIT + 1);

  /* Advance the parser to the first document element */
  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string->str, yaml_string->len);
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

  /* Parse it and exhibit the recursion limit */
  xmd = mmd_parse_xmd (&parser, &error);
  g_assert_null (xmd);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add_func ("/modulemd/v2/limits/parse_document_type/exact",
                   module_limits_test_parse_document_type_exact);
  g_test_add_func ("/modulemd/v2/limits/parse_document_type/oversized",
                   module_limits_test_parse_document_type_oversized);

  g_test_add_func ("/modulemd/v2/limits/mmd_parse_xmd/mapping/exact",
                   module_limits_test_mmd_parse_xmd_mapping_exact);
  g_test_add_func ("/modulemd/v2/limits/mmd_parse_xmd/mapping/oversized",
                   module_limits_test_mmd_parse_xmd_mapping_oversized);

  g_test_add_func ("/modulemd/v2/limits/mmd_parse_xmd/sequence/exact",
                   module_limits_test_mmd_parse_xmd_sequence_exact);
  g_test_add_func ("/modulemd/v2/limits/mmd_parse_xmd/sequence/oversized",
                   module_limits_test_mmd_parse_xmd_sequence_oversized);

  return g_test_run ();
}
