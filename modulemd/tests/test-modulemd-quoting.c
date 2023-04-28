/*
 * This file is part of libmodulemd
 * Copyright (C) 2023 Red Hat, Inc.
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
#include "modulemd-profile.h"
#include "private/modulemd-yaml.h"
#include "private/modulemd-profile-private.h"

/*#include <glib/gstdio.h>


#include "private/glib-extensions.h"
#include "private/test-utils.h"*/

struct item
{
  const char *input;
  int quoted; /* true for quoting expected, otherwise unquoted expected */
};

/*
 * Test that strings only consisting of a number are quoted to prevent
 * consumers from interpretting them as a number and thus mangling the
 * string value by normalizing the number.
 * Ability to quote numerical strings at each part of a YAML document is
 * tested in tests for the particular document type, i.e. not in this file.
 * This code uses an RPM package list of a stream profile for the purpose
 * of testing. It's the most brief usage of quoting.
 */
static void
test_quoting (gconstpointer data)
{
  const struct item *test_case = (const struct item *)data;
  g_autoptr (ModulemdProfile) p = NULL;
  g_autoptr (GError) error = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);
  g_autoptr (GString) expected = g_string_new (NULL);

  if (test_case->quoted)
    g_string_printf (expected,
                     "---\n"
                     "\"0\":\n"
                     "  rpms:\n"
                     "  - \"%s\"\n"
                     "...\n",
                     test_case->input);
  else
    g_string_printf (expected,
                     "---\n"
                     "\"0\":\n"
                     "  rpms:\n"
                     "  - %s\n"
                     "...\n",
                     test_case->input);

  p = modulemd_profile_new ("0");

  modulemd_profile_add_rpm (p, test_case->input);

  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  g_assert_true (mmd_emitter_start_document (&emitter, &error));
  g_assert_true (
    mmd_emitter_start_mapping (&emitter, YAML_BLOCK_MAPPING_STYLE, &error));
  g_assert_true (modulemd_profile_emit_yaml (p, &emitter, &error));
  g_assert_true (mmd_emitter_end_mapping (&emitter, &error));
  g_assert_true (mmd_emitter_end_document (&emitter, &error));
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  if (g_strcmp0 (yaml_string->str, expected->str))
    {
      g_test_message (
        "Expected=\"%s\"\nGot=\"%s\"", expected->str, yaml_string->str);
      g_test_fail ();
    }
}


int
main (int argc, char *argv[])
{
  /* clang-format off */
  struct item test_cases[] = {
    {"0", 1}, /* YAML/JSON floats */
    {"0.", 1},
    {"0.0", 1},
    {".0", 1},
    {"-1", 1},
    {"-1.", 1},
    {"-1.0", 1},
    {"-.0", 1},
    {"+1", 1}, /* Handle "+" for sure */
    {"+1.", 1},
    {"+1.0", 1},
    {"+.0", 1},
    {"1.0e1", 1},
    {"-1.0e1", 1},
    {"+1.0e1", 1},
    {"1.0e-1", 1},
    {"-1.0e-1", 1},
    {"+1.0e-1", 1},
    {"1.0e+1", 1},
    {"-1.0e+1", 1},
    {"+1.0e+1", 1},
    {".inf", 1},
    {"-.inf", 1},
    {"+.inf", 1},
    {".nan", 1},

    {"0x", 0}, /* Incomplete hexadecimal */
    {"0x0", 1}, /* YAML hexadecicmal notation */
    {"0xa", 1},
    {"0xA", 1},
    {"0xg", 0}, /* Invalid hexadecimal */

    {"0o", 0}, /* Incomplete octal */
    {"0o0", 1}, /* YAML octal notation */
    {"0o8", 0}, /* Invalid octal */

    {"0a", 0}, /* This does not need quoting. Common in refs. */
    {NULL, 0}
  };
  /* clang-format on */
  struct item *test_case;
  g_autoptr (GString) testpath = g_string_new (NULL);

  setlocale (LC_ALL, "");
  g_test_init (&argc, &argv, NULL);
  g_test_set_nonfatal_assertions ();

  for (test_case = test_cases; test_case->input != NULL; test_case++)
    {
      g_string_printf (
        testpath, "/modulemd/yaml/quoting/%s", test_case->input);
      g_test_add_data_func (
        testpath->str, (gconstpointer)test_case, test_quoting);
    }

  return g_test_run ();
}
