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

#include "modulemd.h"

#include "config.h"
#include "private/test-utils.h"

static void
test_modulemd_get_version (void)
{
  g_assert_cmpstr (modulemd_get_version (), ==, LIBMODULEMD_VERSION);
}


static void
test_modulemd_load_file (void)
{
  g_autofree gchar *yaml_file = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdModuleIndex) idx = NULL;

  /* This function is a wrapper around lower-level functions, so it should be
   * okay to just test basic success and failure here.
   */

  /* Valid, large datafile */
  g_clear_error (&error);
  g_clear_object (&idx);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/f29.yaml", g_getenv ("TEST_DATA_PATH"));
  idx = modulemd_load_file (yaml_file, &error);
  g_assert_no_error (error);
  g_assert_nonnull (idx);


  /* Nonexistent file */
  g_clear_error (&error);
  g_clear_object (&idx);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file =
    g_strdup_printf ("%s/nosuchfile.yaml", g_getenv ("TEST_DATA_PATH"));
  idx = modulemd_load_file (yaml_file, &error);
  g_assert_null (idx);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_OPEN);


  /* Readable, non-YAML file */
  g_clear_error (&error);
  g_clear_object (&idx);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/nl.po", g_getenv ("TEST_DATA_PATH"));
  idx = modulemd_load_file (yaml_file, &error);
  g_assert_null (idx);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_UNPARSEABLE);


  /* Readable, but invalid YAML file */
  g_clear_error (&error);
  g_clear_object (&idx);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file =
    g_strdup_printf ("%s/good_and_bad.yaml", g_getenv ("TEST_DATA_PATH"));
  idx = modulemd_load_file (yaml_file, &error);
  g_assert_null (idx);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
}


static void
test_modulemd_load_string (void)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdModuleIndex) idx = NULL;
  g_autofree gchar *output = NULL;

  /* This function is a wrapper around lower-level functions, so it should be
   * okay to just test basic success and failure here.
   */

  /* Trivial modulemd */
  g_clear_error (&error);
  g_clear_object (&idx);
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
    "    module: DUMMY\n"
    "...\n";
  idx = modulemd_load_string (yaml_string, &error);
  g_assert_no_error (error);
  g_assert_nonnull (idx);

  /* Make sure loaded index dumps to string cleanly */
  g_clear_error (&error);
  g_clear_pointer (&output, g_free);
  output = modulemd_module_index_dump_to_string (idx, &error);
  g_assert_no_error (error);
  g_assert_nonnull (output);


  /* NULL string should raise an exception */
  g_clear_error (&error);
  g_clear_object (&idx);
  modulemd_test_signal = 0;
  signal (SIGTRAP, modulemd_test_signal_handler);
  idx = modulemd_load_string (NULL, &error);
  g_assert_cmpint (modulemd_test_signal, ==, SIGTRAP);
  g_assert_null (idx);


  /* An empty string is valid YAML, so it returns a non-NULL but empty index. */
  g_clear_error (&error);
  g_clear_object (&idx);
  yaml_string = "";
  idx = modulemd_load_string (yaml_string, &error);
  g_assert_no_error (error);
  g_assert_nonnull (idx);


  /* Invalid YAML string */
  g_clear_error (&error);
  g_clear_object (&idx);
  yaml_string = "Hello, World!\n";
  idx = modulemd_load_string (yaml_string, &error);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
  g_assert_null (idx);
}


static void
test_packager_read_file (void)
{
  g_autofree gchar *yaml_file = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GObject) object = NULL;
  GType otype = G_TYPE_INVALID;

  /* This function is a wrapper around lower-level functions, so it should be
   * okay to just test basic success and failure here.
   */

  /* Valid packager v2 file */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/yaml_specs/modulemd_packager_v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  otype = modulemd_read_packager_file (yaml_file, &object, &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));


  /* Valid packager v2 file with module/stream name overrides */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/yaml_specs/modulemd_packager_v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  otype = modulemd_read_packager_file_ext (
    yaml_file, &object, "modulename-override", "streamname-override", &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));
  g_assert_cmpstr (
    modulemd_module_stream_get_module_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "modulename-override");
  g_assert_cmpstr (
    modulemd_module_stream_get_stream_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "streamname-override");


  /* Valid packager v3 file */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/yaml_specs/modulemd_packager_v3.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  otype = modulemd_read_packager_file (yaml_file, &object, &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_PACKAGER_V3);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_PACKAGER_V3 (object));


  /* Valid packager v3 file with module/stream name overrides */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/yaml_specs/modulemd_packager_v3.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  otype = modulemd_read_packager_file_ext (
    yaml_file, &object, "modulename-override", "streamname-override", &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_PACKAGER_V3);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_PACKAGER_V3 (object));
  g_assert_cmpstr (
    modulemd_packager_v3_get_module_name (MODULEMD_PACKAGER_V3 (object)),
    ==,
    "modulename-override");
  g_assert_cmpstr (
    modulemd_packager_v3_get_stream_name (MODULEMD_PACKAGER_V3 (object)),
    ==,
    "streamname-override");


  /* Valid stream v2 file */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  otype = modulemd_read_packager_file (yaml_file, &object, &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));


  /* Valid stream v2 file with module/stream name overrides */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  otype = modulemd_read_packager_file_ext (
    yaml_file, &object, "modulename-override", "streamname-override", &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));
  g_assert_cmpstr (
    modulemd_module_stream_get_module_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "modulename-override");
  g_assert_cmpstr (
    modulemd_module_stream_get_stream_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "streamname-override");


  /* Valid stream v1 file, should get upgraded to v2 */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  otype = modulemd_read_packager_file (yaml_file, &object, &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));


  /* Valid stream v1 file, should get upgraded to v2,
   * with module/stream name overrides
   */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  otype = modulemd_read_packager_file_ext (
    yaml_file, &object, "modulename-override", "streamname-override", &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));
  g_assert_cmpstr (
    modulemd_module_stream_get_module_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "modulename-override");
  g_assert_cmpstr (
    modulemd_module_stream_get_stream_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "streamname-override");


  /* Nonexistent file */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file =
    g_strdup_printf ("%s/nosuchfile.yaml", g_getenv ("TEST_DATA_PATH"));
  otype = modulemd_read_packager_file (yaml_file, &object, &error);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_OPEN);
  g_assert_true (otype == G_TYPE_INVALID);
  g_assert_null (object);


  /* Readable, non-YAML file */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/nl.po", g_getenv ("TEST_DATA_PATH"));
  otype = modulemd_read_packager_file (yaml_file, &object, &error);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
  g_assert_true (otype == G_TYPE_INVALID);
  g_assert_null (object);


  /* Readable, but invalid YAML packager file */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file =
    g_strdup_printf ("%s/broken_stream.yaml", g_getenv ("TEST_DATA_PATH"));
  otype = modulemd_read_packager_file (yaml_file, &object, &error);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
  g_assert_true (otype == G_TYPE_INVALID);
  g_assert_null (object);


  /* YAML file with multiple documents */
  g_clear_error (&error);
  g_clear_object (&object);
  g_clear_pointer (&yaml_file, g_free);
  yaml_file = g_strdup_printf ("%s/f29.yaml", g_getenv ("TEST_DATA_PATH"));
  otype = modulemd_read_packager_file (yaml_file, &object, &error);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
  g_assert_true (otype == G_TYPE_INVALID);
  g_assert_null (object);
}


static void
test_packager_read_string (void)
{
  const gchar *yaml_string = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GObject) object = NULL;
  GType otype = G_TYPE_INVALID;

  /* This function is a wrapper around lower-level functions, so it should be
   * okay to just test basic success and failure here.
   */

  /* Trivial modulemd packager v2 */
  g_clear_error (&error);
  g_clear_object (&object);
  yaml_string =
    "---\n"
    "document: modulemd-packager\n"
    "version: 2\n"
    "data:\n"
    "  name: trivialname\n"
    "  stream: trivialstream\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license:\n"
    "    module: DUMMY\n"
    "...\n";
  otype = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));
  /* with module/stream name overrides */
  g_clear_error (&error);
  g_clear_object (&object);
  otype = modulemd_read_packager_string_ext (yaml_string,
                                             &object,
                                             "modulename-override",
                                             "streamname-override",
                                             &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));
  g_assert_cmpstr (
    modulemd_module_stream_get_module_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "modulename-override");
  g_assert_cmpstr (
    modulemd_module_stream_get_stream_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "streamname-override");

  /* Trivial modulemd packager v3 */
  g_clear_error (&error);
  g_clear_object (&object);
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
    "  license: DUMMY\n"
    "...\n";
  otype = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_PACKAGER_V3);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_PACKAGER_V3 (object));
  /* with module/stream name overrides */
  g_clear_error (&error);
  g_clear_object (&object);
  otype = modulemd_read_packager_string_ext (yaml_string,
                                             &object,
                                             "modulename-override",
                                             "streamname-override",
                                             &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_PACKAGER_V3);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_PACKAGER_V3 (object));
  g_assert_cmpstr (
    modulemd_packager_v3_get_module_name (MODULEMD_PACKAGER_V3 (object)),
    ==,
    "modulename-override");
  g_assert_cmpstr (
    modulemd_packager_v3_get_stream_name (MODULEMD_PACKAGER_V3 (object)),
    ==,
    "streamname-override");

  /* Trivial modulemd stream v2 */
  g_clear_error (&error);
  g_clear_object (&object);
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
    "    module: DUMMY\n"
    "...\n";
  otype = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));
  /* with module/stream name overrides */
  g_clear_error (&error);
  g_clear_object (&object);
  otype = modulemd_read_packager_string_ext (yaml_string,
                                             &object,
                                             "modulename-override",
                                             "streamname-override",
                                             &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));
  g_assert_cmpstr (
    modulemd_module_stream_get_module_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "modulename-override");
  g_assert_cmpstr (
    modulemd_module_stream_get_stream_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "streamname-override");

  /* Trivial modulemd stream v1, should get upgraded to v2 */
  g_clear_error (&error);
  g_clear_object (&object);
  yaml_string =
    "---\n"
    "document: modulemd\n"
    "version: 1\n"
    "data:\n"
    "  name: trivialname\n"
    "  stream: trivialstream\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license:\n"
    "    module: DUMMY\n"
    "...\n";
  otype = modulemd_read_packager_string (yaml_string, &object, &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));
  /* with module/stream name overrides */
  g_clear_error (&error);
  g_clear_object (&object);
  otype = modulemd_read_packager_string_ext (yaml_string,
                                             &object,
                                             "modulename-override",
                                             "streamname-override",
                                             &error);
  g_assert_no_error (error);
  g_assert_true (otype == MODULEMD_TYPE_MODULE_STREAM_V2);
  g_assert_nonnull (object);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (object));
  g_assert_cmpstr (
    modulemd_module_stream_get_module_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "modulename-override");
  g_assert_cmpstr (
    modulemd_module_stream_get_stream_name (MODULEMD_MODULE_STREAM (object)),
    ==,
    "streamname-override");

  /* NULL string should raise an exception */
  g_clear_error (&error);
  g_clear_object (&object);
  modulemd_test_signal = 0;
  signal (SIGTRAP, modulemd_test_signal_handler);
  otype = modulemd_read_packager_string (NULL, &object, &error);
  g_assert_cmpint (modulemd_test_signal, ==, SIGTRAP);
  g_assert_cmpint (otype, ==, G_TYPE_INVALID);
  g_assert_null (object);

  /* An empty string is not a valid packager format */
  g_clear_error (&error);
  g_clear_object (&object);
  otype = modulemd_read_packager_string ("", &object, &error);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
  g_assert_true (otype == G_TYPE_INVALID);
  g_assert_null (object);

  /* Invalid YAML string */
  g_clear_error (&error);
  g_clear_object (&object);
  otype = modulemd_read_packager_string ("Hello, World!\n", &object, &error);
  g_assert_error (error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_PARSE);
  g_assert_true (otype == G_TYPE_INVALID);
  g_assert_null (object);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add_func ("/modulemd/v2/common/get_version",
                   test_modulemd_get_version);

  g_test_add_func ("/modulemd/v2/common/load_file", test_modulemd_load_file);
  g_test_add_func ("/modulemd/v2/common/load_string",
                   test_modulemd_load_string);

  g_test_add_func ("/modulemd/v2/common/packager/read_file",
                   test_packager_read_file);
  g_test_add_func ("/modulemd/v2/common/packager/read_string",
                   test_packager_read_string);

  return g_test_run ();
}
