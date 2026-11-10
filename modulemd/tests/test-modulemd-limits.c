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

#include "config.h"
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


/* parse_yaml_string() is the closest non-static function to
 * _read_yaml_and_type(). */
static void
module_limits_test_parse_document_type_exact (void)
{
  g_autoptr (GString) yaml_string = NULL;
  g_autoptr (GPtrArray) documents = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  gboolean retval;

  /* Build a document with a depth matching, i.e. fitting, the recursion limit. */
  yaml_string =
    build_modulemd_stream_v2_document (LIBMODULEMD_RECURSION_LIMIT - 2);

  /* Parse it and exhibit the recursion limit */
  retval = parse_yaml_string (yaml_string->str, &documents, &failures, &error);

  /* Check that the parsed documents is exactly one modulemd document */
  g_assert_true (retval);
  g_assert_no_error (error);
  g_assert_nonnull (documents);
  if (failures)
    g_assert_cmpint (failures->len, ==, 0);
  g_assert_cmpint (documents->len, ==, 1);
  g_assert_true (MODULEMD_IS_MODULESTREAM (g_ptr_array_index (documents, 0)));
}


static void
module_limits_test_parse_document_type_oversized (void)
{
  g_autoptr (GString) yaml_string = NULL;
  g_autoptr (GPtrArray) documents = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  gboolean retval;

  /* Build a document with a depth exceeding the recursion limit. */
  yaml_string =
    build_modulemd_stream_v2_document (LIBMODULEMD_RECURSION_LIMIT - 2 + 1);

  /* Parse it and exhibit the recursion limit */
  retval = parse_yaml_string (yaml_string->str, &documents, &failures, &error);

  /* Check that parsing failed */
  g_assert_false (retval);
  g_assert_error (error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE);
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

  return g_test_run ();
}
