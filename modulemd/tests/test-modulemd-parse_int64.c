/*
 * This file is part of libmodulemd
 * Copyright (C) 2021 Red Hat, Inc.
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#include "config.h"
#include <glib.h>
#include <locale.h>
#include <string.h>

#include "private/modulemd-yaml.h"
#include "private/test-utils.h"
#include <yaml.h>

static void
test (const char *input, gint64 expected_value, gboolean expected_error)
{
  gint64 parsed;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);

  yaml_parser_set_input_string (
    &parser, (const unsigned char *)input, strlen (input));
  parser_skip_document_start (&parser);

  parsed = modulemd_yaml_parse_int64 (&parser, &error);
  if (expected_error)
    g_assert_nonnull (error);
  else
    g_assert_null (error);
  g_assert_cmpint (parsed, ==, expected_value);
}

static void
test_int64_valid (void)
{
  test ("42", 42, FALSE);
}

static void
test_int64_invalid_no_digit (void)
{
  test ("foo", 0, TRUE);
}

static void
test_int64_invalid_incomplete (void)
{
  test ("42foo", 0, TRUE);
}

static void
test_int64_valid_negative (void)
{
  test ("-42", -42, FALSE);
}

static void
test_int64_invalid_too_big (void)
{
  test ("9223372036854775808", 0, TRUE);
}

static void
test_int64_invalid_overflowed (void)
{
#ifdef HAVE_OVERFLOWED_BUILDORDER
  test ("18446744073709551615", -1, FALSE);
#else
  test ("18446744073709551615", 0, TRUE);
#endif
}

static void
test_int64_invalid_too_small (void)
{
  test ("-9223372036854775809", 0, TRUE);
}

static void
utest (const char *input, guint64 expected_value, gboolean expected_error)
{
  guint64 parsed;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (parser);

  yaml_parser_set_input_string (
    &parser, (const unsigned char *)input, strlen (input));
  parser_skip_document_start (&parser);

  parsed = modulemd_yaml_parse_uint64 (&parser, &error);
  if (expected_error)
    g_assert_nonnull (error);
  else
    g_assert_null (error);
  g_assert_cmpuint (parsed, ==, expected_value);
}

static void
test_uint64_valid (void)
{
  utest ("42", 42u, FALSE);
}

static void
test_uint64_invalid_no_digit (void)
{
  utest ("foo", 0u, TRUE);
}

static void
test_uint64_invalid_incomplete (void)
{
  utest ("42foo", 0u, TRUE);
}

static void
test_uint64_invalid_negative (void)
{
  utest ("-42", 0u, TRUE);
}

static void
test_uint64_invalid_too_big (void)
{
  utest ("18446744073709551616", 0u, TRUE);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/modulemd/v2/int64/yaml/parse/valid", test_int64_valid);
  g_test_add_func ("/modulemd/v2/int64/yaml/parse/invalid_no_digit",
                   test_int64_invalid_no_digit);
  g_test_add_func ("/modulemd/v2/int64/yaml/parse/invalid_incomplete",
                   test_int64_invalid_incomplete);
  g_test_add_func ("/modulemd/v2/int64/yaml/parse/valid_negative",
                   test_int64_valid_negative);
  g_test_add_func ("/modulemd/v2/int64/yaml/parse/invalid_too_big",
                   test_int64_invalid_too_big);
  g_test_add_func ("/modulemd/v2/int64/yaml/parse/invalid_too_small",
                   test_int64_invalid_too_small);
  g_test_add_func ("/modulemd/v2/int64/yaml/parse/invalid_overflowed",
                   test_int64_invalid_overflowed);

  g_test_add_func ("/modulemd/v2/uint64/yaml/parse/valid", test_uint64_valid);
  g_test_add_func ("/modulemd/v2/uint64/yaml/parse/invalid_no_digit",
                   test_uint64_invalid_no_digit);
  g_test_add_func ("/modulemd/v2/uint64/yaml/parse/invalid_incomplete",
                   test_uint64_invalid_incomplete);
  g_test_add_func ("/modulemd/v2/uint64/yaml/parse/invalid_negative",
                   test_uint64_invalid_negative);
  g_test_add_func ("/modulemd/v2/uint64/yaml/parse/invalid_too_big",
                   test_uint64_invalid_too_big);

  return g_test_run ();
}
