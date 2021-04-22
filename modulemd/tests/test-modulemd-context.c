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

#include <locale.h>
#include <glib.h>
#include "modulemd.h"
#include "config.h"

static void
test_modulemd_v3_context_valid (void)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GObject) object = NULL;
  GType type = G_TYPE_INVALID;

  yaml_string =
    "---\n"
    "document: modulemd-packager\n"
    "version: 3\n"
    "data:\n"
    "  name: trivialname\n"
    "  stream: trivialstream\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license: [MIT]\n"
    "  configurations:\n"
    "    - context: a234567890\n"
    "      platform: foo\n"
    "...\n";
  type = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_true (type == MODULEMD_TYPE_PACKAGER_V3);
  g_assert_no_error (error);
}


static void
test_modulemd_v3_context_overlong (void)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GObject) object = NULL;
  GType type = G_TYPE_INVALID;

  yaml_string =
    "---\n"
    "document: modulemd-packager\n"
    "version: 3\n"
    "data:\n"
    "  name: trivialname\n"
    "  stream: trivialstream\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license: [MIT]\n"
    "  configurations:\n"
    "    - context: a2345678901\n"
    "      platform: foo\n"
    "...\n";
  type = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_true (type == G_TYPE_INVALID);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
}


static void
test_modulemd_v3_context_bad_underscore (void)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GObject) object = NULL;
  GType type = G_TYPE_INVALID;

  yaml_string =
    "---\n"
    "document: modulemd-packager\n"
    "version: 3\n"
    "data:\n"
    "  name: trivialname\n"
    "  stream: trivialstream\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license: [MIT]\n"
    "  configurations:\n"
    "    - context: _\n"
    "      platform: foo\n"
    "...\n";
  type = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_true (type == G_TYPE_INVALID);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
}


static void
test_modulemd_v2_context_valid (void)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GObject) object = NULL;
  GType type = G_TYPE_INVALID;

  yaml_string =
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  name: trivialname\n"
    "  stream: trivialstream\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license:\n"
    "    module: [MIT]\n"
    "  static_context: true\n"
    "  context: a234567890_23\n"
    "...\n";
  type = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_true (type == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_no_error (error);
  /* Reading v2 document does not validate it; validating explictly */
  g_assert_true (
    modulemd_module_stream_validate (MODULEMD_MODULE_STREAM (object), &error));
}


static void
test_modulemd_v2_context_overlong (void)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GObject) object = NULL;
  GType type = G_TYPE_INVALID;

  yaml_string =
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  name: trivialname\n"
    "  stream: trivialstream\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license:\n"
    "    module: [MIT]\n"
    "  static_context: true\n"
    "  context: a234567890_234\n"
    "...\n";
  type = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_true (type == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_no_error (error);
  /* Reading v2 document does not validate it; validating explictly */
  g_assert_false (
    modulemd_module_stream_validate (MODULEMD_MODULE_STREAM (object), &error));
}


static void
test_modulemd_v2_context_bad_character (void)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GObject) object = NULL;
  GType type = G_TYPE_INVALID;

  yaml_string =
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  name: trivialname\n"
    "  stream: trivialstream\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license:\n"
    "    module: [MIT]\n"
    "  static_context: true\n"
    "  context: '-'\n"
    "...\n";
  type = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_true (type == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_no_error (error);
  /* Reading v2 document does not validate it; validating explictly */
  g_assert_false (
    modulemd_module_stream_validate (MODULEMD_MODULE_STREAM (object), &error));
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  g_test_init (&argc, &argv, NULL);
  g_test_set_nonfatal_assertions ();
  g_test_bug_base ("https://github.com/fedora-modularity/libmodulemd/issues/");
  g_test_bug ("549");

  g_test_add_func ("/modulemd/v3/context/valid",
                   test_modulemd_v3_context_valid);
  g_test_add_func ("/modulemd/v3/context/overlong",
                   test_modulemd_v3_context_overlong);
  g_test_add_func ("/modulemd/v3/context/bad_underscore",
                   test_modulemd_v3_context_bad_underscore);

  g_test_add_func ("/modulemd/v2/context/valid",
                   test_modulemd_v2_context_valid);
  g_test_add_func ("/modulemd/v2/context/overlong",
                   test_modulemd_v2_context_overlong);
  g_test_add_func ("/modulemd/v2/context/bad_character",
                   test_modulemd_v2_context_bad_character);

  return g_test_run ();
}
