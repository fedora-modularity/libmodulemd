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
#include <yaml.h>

#include "config.h"
#include "modulemd.h"
#include "modulemd-defaults.h"
#include "modulemd-obsoletes.h"
#include "modulemd-module-index.h"
#include "modulemd-module-stream-v1.h"
#include "modulemd-module-stream-v2.h"
#include "modulemd-module.h"
#include "modulemd-subdocument-info.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _ModuleIndexFixture
{
} ModuleIndexFixture;


static void
module_index_test_dump (void)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (ModulemdObsoletes) obsoletes = NULL;
  g_autoptr (ModulemdTranslationEntry) translation_entry = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree const gchar *string = NULL;

  /* Construct an Index with some objects */
  index = modulemd_module_index_new ();

  /* First: translations */
  translation = modulemd_translation_new (1, "testmodule1", "teststream1", 42);
  translation_entry = modulemd_translation_entry_new ("ro_TA");
  modulemd_translation_entry_set_summary (translation_entry,
                                          "Testsummary in ro_TA");
  modulemd_translation_set_translation_entry (translation, translation_entry);
  g_clear_pointer (&translation_entry, g_object_unref);
  translation_entry = modulemd_translation_entry_new ("nl_NL");
  modulemd_translation_entry_set_summary (translation_entry,
                                          "Een test omschrijving");
  modulemd_translation_set_translation_entry (translation, translation_entry);
  g_clear_pointer (&translation_entry, g_object_unref);
  g_assert_true (
    modulemd_module_index_add_translation (index, translation, &error));
  g_assert_no_error (error);
  g_clear_pointer (&translation, g_object_unref);

  /* Second: defaults */
  defaults = modulemd_defaults_new (1, "testmodule1");
  g_assert_true (modulemd_module_index_add_defaults (index, defaults, &error));
  g_assert_no_error (error);
  g_clear_pointer (&defaults, g_object_unref);

  /* Third: some obsoletes */
  obsoletes = modulemd_obsoletes_new (
    1, 202001012020, "testmodule1", "teststream2", "testmessage");
  g_assert_true (
    modulemd_module_index_add_obsoletes (index, obsoletes, &error));
  g_assert_no_error (error);
  g_clear_pointer (&obsoletes, g_object_unref);

  /* Fourth: some streams */
  stream = (ModulemdModuleStream *)modulemd_module_stream_v1_new (
    "testmodule1", "teststream1");
  modulemd_module_stream_set_version (stream, 1);
  modulemd_module_stream_set_context (stream, "deadbeef");
  modulemd_module_stream_v1_set_summary (MODULEMD_MODULE_STREAM_V1 (stream),
                                         "A test stream");
  modulemd_module_stream_v1_set_description (
    MODULEMD_MODULE_STREAM_V1 (stream), "A test stream's description");
  modulemd_module_stream_v1_add_module_license (
    MODULEMD_MODULE_STREAM_V1 (stream), "Beerware");
  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);
  g_clear_pointer (&stream, g_object_unref);
  stream = (ModulemdModuleStream *)modulemd_module_stream_v2_new (
    "testmodule1", "teststream2");
  modulemd_module_stream_set_version (stream, 2);
  modulemd_module_stream_set_context (stream, "c0ff33");
  modulemd_module_stream_v2_set_summary (MODULEMD_MODULE_STREAM_V2 (stream),
                                         "A second stream");
  modulemd_module_stream_v2_set_description (
    MODULEMD_MODULE_STREAM_V2 (stream), "A second stream's description");
  modulemd_module_stream_v2_add_module_license (
    MODULEMD_MODULE_STREAM_V2 (stream), "Beerware");
  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);
  g_clear_pointer (&stream, g_object_unref);

  /* And now... emit */
  string = modulemd_module_index_dump_to_string (index, &error);
  g_assert_no_error (error);
  g_assert_nonnull (string);

  /* Verify that all streams and defaults have been upgraded to the highest
   * version added
   */
  g_assert_cmpstr (string,
                   ==,
                   "---\n"
                   "document: modulemd-defaults\n"
                   "version: 1\n"
                   "data:\n"
                   "  module: testmodule1\n"
                   "...\n"
                   "---\n"
                   "document: modulemd-obsoletes\n"
                   "version: 1\n"
                   "data:\n"
                   "  modified: 2020-01-01T20:20Z\n"
                   "  module: testmodule1\n"
                   "  stream: \"teststream2\"\n"
                   "  message: testmessage\n"
                   "...\n"
                   "---\n"
                   "document: modulemd-translations\n"
                   "version: 1\n"
                   "data:\n"
                   "  module: testmodule1\n"
                   "  stream: \"teststream1\"\n"
                   "  modified: 42\n"
                   "  translations:\n"
                   "    nl_NL:\n"
                   "      summary: Een test omschrijving\n"
                   "    ro_TA:\n"
                   "      summary: Testsummary in ro_TA\n"
                   "...\n"
                   "---\n"
                   "document: modulemd\n"
                   "version: 2\n"
                   "data:\n"
                   "  name: testmodule1\n"
                   "  stream: \"teststream1\"\n"
                   "  version: 1\n"
                   "  context: deadbeef\n"
                   "  summary: A test stream\n"
                   "  description: >-\n"
                   "    A test stream's description\n"
                   "  license:\n"
                   "    module:\n"
                   "    - Beerware\n"
                   "...\n"
                   "---\n"
                   "document: modulemd\n"
                   "version: 2\n"
                   "data:\n"
                   "  name: testmodule1\n"
                   "  stream: \"teststream2\"\n"
                   "  version: 2\n"
                   "  context: c0ff33\n"
                   "  summary: A second stream\n"
                   "  description: >-\n"
                   "    A second stream's description\n"
                   "  license:\n"
                   "    module:\n"
                   "    - Beerware\n"
                   "...\n");
}


static void
module_index_test_read (void)
{
  gboolean ret;
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  ModulemdSubdocumentInfo *subdoc = NULL;
  g_autofree gchar *yaml_path = NULL;

  /* Read the specification files all in */
  index = modulemd_module_index_new ();

  /* The two stream definitions */
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* The modulemd-packager definition
   * This should fail to be read into a ModuleIndex because it provides no
   * NSVCA information. It should only be importable via
   * ModuleStream.read_file()
   */
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_packager_v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  ret = modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error);
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 1);
  g_assert_false (ret);
  g_assert_error (
    modulemd_subdocument_info_get_gerror (g_ptr_array_index (failures, 0)),
    MODULEMD_ERROR,
    MMD_ERROR_MISSING_REQUIRED);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* The translation definitions */
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_translations_v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* The obsoletes definitions */
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_obsoletes_v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* The defaults definitions */
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_defaults_v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* A stream that has nonsense in "data" */
  yaml_path =
    g_strdup_printf ("%s/broken_stream.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_false (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 1);
  subdoc = g_ptr_array_index (failures, 0);
  g_assert_cmpstr (modulemd_subdocument_info_get_yaml (subdoc),
                   ==,
                   "---\n"
                   "document: modulemd\n"
                   "version: 2\n"
                   "data: foobar\n"
                   "...\n");
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);
  g_clear_pointer (&error, g_error_free);

  /* A non-existing file */
  yaml_path =
    g_strdup_printf ("%s/nothinghere.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_false (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_nonnull (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);
  g_clear_pointer (&error, g_error_free);

  /* An empty stream */
  g_assert_false (modulemd_module_index_update_from_stream (
    index, NULL, TRUE, &failures, &error));
  g_assert_nonnull (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);
  g_clear_pointer (&error, g_error_free);

  /* An empty string */
  g_assert_false (modulemd_module_index_update_from_string (
    index, NULL, TRUE, &failures, &error));
  g_assert_nonnull (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);
  g_clear_pointer (&error, g_error_free);

  /*
   * Also try to ingest a TranslationEntry.
   * This should fail, and return a failure, since it's not a top-level subdoc.
   */
  yaml_path = g_strdup_printf ("%s/te.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_false (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 1);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* Actually verifying the contents is left to Python tests */
}


static void
module_index_test_read_mixed (void)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *output = NULL;

  index = modulemd_module_index_new ();

  yaml_path =
    g_strdup_printf ("%s/long-valid.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* Verify that we can output it cleanly */
  output = modulemd_module_index_dump_to_string (index, &error);
  g_assert_nonnull (output);
  g_assert_null (error);
}


static void
module_index_test_read_unknown (void)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;

  index = modulemd_module_index_new ();

  yaml_path = g_strdup_printf ("%s/good-v2-extra-keys.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  g_assert_false (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_cmpint (failures->len, ==, 3);
  g_clear_pointer (&failures, g_ptr_array_unref);

  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, FALSE, &failures, &error));
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&failures, g_ptr_array_unref);
}


static void
module_index_test_stream_upgrade (void)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (GError) error = NULL;

  /* Construct an Index with some objects */
  index = modulemd_module_index_new ();

  /* Add some streams */

  /* First, a v1 stream */
  stream = (ModulemdModuleStream *)modulemd_module_stream_v1_new (
    "testmodule1", "teststream1");
  modulemd_module_stream_set_version (stream, 1);
  modulemd_module_stream_set_context (stream, "deadbeef");
  modulemd_module_stream_v1_set_summary (MODULEMD_MODULE_STREAM_V1 (stream),
                                         "A test stream");
  modulemd_module_stream_v1_set_description (
    MODULEMD_MODULE_STREAM_V1 (stream), "A test stream's description");
  modulemd_module_stream_v1_add_module_license (
    MODULEMD_MODULE_STREAM_V1 (stream), "Beerware");
  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);
  g_clear_pointer (&stream, g_object_unref);

  /* Verify that it was added as a StreamV1 object */
  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_ONE);
  g_clear_object (&stream);


  /* Next, add a v2 Stream */
  stream = (ModulemdModuleStream *)modulemd_module_stream_v2_new (
    "testmodule1", "teststream2");
  modulemd_module_stream_set_version (stream, 2);
  modulemd_module_stream_set_context (stream, "c0ff33");
  modulemd_module_stream_v2_set_summary (MODULEMD_MODULE_STREAM_V2 (stream),
                                         "A second stream");
  modulemd_module_stream_v2_set_description (
    MODULEMD_MODULE_STREAM_V2 (stream), "A second stream's description");
  modulemd_module_stream_v2_add_module_license (
    MODULEMD_MODULE_STREAM_V2 (stream), "Beerware");
  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);
  g_clear_pointer (&stream, g_object_unref);

  /* Verify that it was added as a StreamV2 object */
  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream2",
    2,
    "c0ff33",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);

  /* Verify that the first object was upgraded to StreamV2 */
  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);

  /* Add one more v1 Stream */
  stream = (ModulemdModuleStream *)modulemd_module_stream_v1_new (
    "testmodule1", "teststream3");
  modulemd_module_stream_set_version (stream, 3);
  modulemd_module_stream_set_context (stream, "badfeed");
  modulemd_module_stream_v1_set_summary (MODULEMD_MODULE_STREAM_V1 (stream),
                                         "A test stream");
  modulemd_module_stream_v1_set_description (
    MODULEMD_MODULE_STREAM_V1 (stream), "A test stream's description");
  modulemd_module_stream_v1_add_module_license (
    MODULEMD_MODULE_STREAM_V1 (stream), "Beerware");
  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);
  g_clear_pointer (&stream, g_object_unref);

  /* Verify that it was added as a StreamV2 object */
  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream3",
    3,
    "badfeed",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);
}


static void
module_index_test_index_upgrade (void)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;
  g_autoptr (GError) error = NULL;

  /* Construct an Index with some objects */
  index = modulemd_module_index_new ();

  /* Add some streams */

  /* Add a v1 streams */
  stream = (ModulemdModuleStream *)modulemd_module_stream_v1_new (
    "testmodule1", "teststream1");
  modulemd_module_stream_set_version (stream, 1);
  modulemd_module_stream_set_context (stream, "deadbeef");
  modulemd_module_stream_v1_set_summary (MODULEMD_MODULE_STREAM_V1 (stream),
                                         "A test stream");
  modulemd_module_stream_v1_set_description (
    MODULEMD_MODULE_STREAM_V1 (stream), "A test stream's description");
  modulemd_module_stream_v1_add_module_license (
    MODULEMD_MODULE_STREAM_V1 (stream), "Beerware");
  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);
  g_clear_pointer (&stream, g_object_unref);

  /* Verify that it was added as a StreamV1 object */
  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_ONE);
  g_clear_object (&stream);


  /* Add one more v1 Stream */
  stream = (ModulemdModuleStream *)modulemd_module_stream_v1_new (
    "testmodule1", "teststream3");
  modulemd_module_stream_set_version (stream, 3);
  modulemd_module_stream_set_context (stream, "badfeed");
  modulemd_module_stream_v1_set_summary (MODULEMD_MODULE_STREAM_V1 (stream),
                                         "A test stream");
  modulemd_module_stream_v1_set_description (
    MODULEMD_MODULE_STREAM_V1 (stream), "A test stream's description");
  modulemd_module_stream_v1_add_module_license (
    MODULEMD_MODULE_STREAM_V1 (stream), "Beerware");
  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);
  g_clear_pointer (&stream, g_object_unref);

  /* Verify that it was added as a StreamV1 object */
  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream3",
    3,
    "badfeed",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_ONE);
  g_clear_object (&stream);

  /* Add some defaults */
  defaults = modulemd_defaults_new (1, "testmodule1");
  g_assert_true (modulemd_module_index_add_defaults (index, defaults, &error));
  g_assert_no_error (error);
  g_clear_pointer (&defaults, g_object_unref);

  /* Verify that the index is at stream and defaults v1 */
  g_assert_cmpint (modulemd_module_index_get_stream_mdversion (index),
                   ==,
                   MD_MODULESTREAM_VERSION_ONE);
  g_assert_cmpint (modulemd_module_index_get_defaults_mdversion (index),
                   ==,
                   MD_MODULESTREAM_VERSION_ONE);

  /* Verify that upgrades from stream v1 to v2 work */
  g_assert_true (modulemd_module_index_upgrade_streams (
    index, MD_MODULESTREAM_VERSION_TWO, NULL));
  g_assert_cmpint (modulemd_module_index_get_stream_mdversion (index),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);

  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream3",
    3,
    "badfeed",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);


  /* Verify that upgrades to the same stream version work. */
  g_assert_true (modulemd_module_index_upgrade_streams (
    index, MD_MODULESTREAM_VERSION_TWO, NULL));
  g_assert_cmpint (modulemd_module_index_get_stream_mdversion (index),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);

  stream = g_object_ref (modulemd_module_get_stream_by_NSVCA (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream3",
    3,
    "badfeed",
    NULL,
    &error));
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);


  /* Verify that upgrades to the same defaults version work */
  g_assert_true (modulemd_module_index_upgrade_defaults (
    index, MD_DEFAULTS_VERSION_ONE, NULL));
  g_assert_cmpint (modulemd_module_index_get_defaults_mdversion (index),
                   ==,
                   MD_DEFAULTS_VERSION_ONE);
  defaults = g_object_ref (modulemd_module_get_defaults (
    modulemd_module_index_get_module (index, "testmodule1")));
  g_assert_cmpint (
    modulemd_defaults_get_mdversion (defaults), ==, MD_DEFAULTS_VERSION_ONE);
  g_clear_object (&defaults);


  /* Verify that upgrades to an unknown version fail */
  g_assert_false (modulemd_module_index_upgrade_streams (
    index, MD_MODULESTREAM_VERSION_LATEST + 1, &error));
  g_assert_nonnull (error);
  g_clear_pointer (&error, g_error_free);

  g_assert_false (modulemd_module_index_upgrade_defaults (
    index, MD_DEFAULTS_VERSION_LATEST + 1, &error));
  g_assert_nonnull (error);
  g_clear_pointer (&error, g_error_free);


  /* Verify that upgrades to a lower version fail */
  g_assert_false (modulemd_module_index_upgrade_streams (
    index, MD_MODULESTREAM_VERSION_ONE, &error));
  g_assert_nonnull (error);
  g_clear_pointer (&error, g_error_free);

  g_assert_false (modulemd_module_index_upgrade_defaults (index, 0, &error));
  g_assert_nonnull (error);
  g_clear_pointer (&error, g_error_free);
}


static void
module_index_test_remove_module (void)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;

  index = modulemd_module_index_new ();

  yaml_path =
    g_strdup_printf ("%s/long-valid.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_cmpint (failures->len, ==, 0);
  g_assert_no_error (error);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* Verify that the 'reviewboard' module exists in the index */
  g_assert_nonnull (modulemd_module_index_get_module (index, "reviewboard"));

  /* Remove the 'reviewboard' module from the index */
  g_assert_true (modulemd_module_index_remove_module (index, "reviewboard"));

  /* Verify that the 'reviewboard' module no longer exists in the index */
  g_assert_null (modulemd_module_index_get_module (index, "reviewboard"));

  /* Remove a nonexistent module from the index */
  g_assert_null (modulemd_module_index_get_module (index, "nosuchmodule"));
  g_assert_false (modulemd_module_index_remove_module (index, "nosuchmodule"));
  g_assert_null (modulemd_module_index_get_module (index, "nosuchmodule"));
}


struct custom_string
{
  gchar *string;
  gchar *current;
};

static int
custom_string_read_handler (void *data,
                            unsigned char *buffer,
                            size_t size,
                            size_t *size_read)
{
  struct custom_string *custom = (struct custom_string *)data;
  const gchar *end = custom->string + strlen (custom->string);

  if (custom->current == end)
    {
      *size_read = 0;
      return 1;
    }

  if (size > (size_t) (end - custom->current))
    {
      size = end - custom->current;
    }

  memcpy (buffer, custom->current, size);
  custom->current += size;
  *size_read = size;
  return 1;
}


static void
module_index_test_custom_read (void)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;


  // clang-format off
  g_autofree gchar *str = g_strdup(
"---\n"
"document: modulemd\n"
"version: 2\n"
"data:\n"
"  name: testmodule\n"
"  stream: master\n"
"  version: 20180405123256\n"
"  context: c2c572ec\n"
"  arch: x86_64\n"
"  summary: A test module in all its beautiful beauty\n"
"  description: >-\n"
"    This module demonstrates how to write simple modulemd files And can be used for\n"
"    testing the build and release pipeline.\n"
"  license:\n"
"    module:\n"
"    - MIT\n"
"    content:\n"
"    - GPL+ or Artistic\n"
"    - MIT\n"
"  xmd:\n"
"    mbs:\n"
"      scmurl: https://src.fedoraproject.org/modules/testmodule.git?#0d33e028e4561f82ea43f670ee6366675cd6a6fe\n"
"      commit: 0d33e028e4561f82ea43f670ee6366675cd6a6fe\n"
"      buildrequires:\n"
"        platform:\n"
"          ref: virtual\n"
"          stream: f29\n"
"          filtered_rpms: []\n"
"          version: 4\n"
"      rpms:\n"
"        perl-List-Compare:\n"
"          ref: c6a689a6ce2683b15b32f83e6cb5d43ffd3816f5\n"
"        tangerine:\n"
"          ref: 239ada495d941ceefd8f359e1d8a47877fbba4a9\n"
"        perl-Tangerine:\n"
"          ref: 7e96446223f1ad84a26c7cf23d6591cd9f6326c6\n"
"      requires:\n"
"        platform:\n"
"          ref: virtual\n"
"          stream: f29\n"
"          filtered_rpms: []\n"
"          version: 4\n"
"  dependencies:\n"
"  - buildrequires:\n"
"      platform: [f29]\n"
"    requires:\n"
"      platform: [f29]\n"
"  references:\n"
"    community: https://docs.pagure.org/modularity/\n"
"    documentation: https://fedoraproject.org/wiki/Fedora_Packaging_Guidelines_for_Modules\n"
"  profiles:\n"
"    default:\n"
"      rpms:\n"
"      - tangerine\n"
"  api:\n"
"    rpms:\n"
"    - perl-Tangerine\n"
"    - tangerine\n"
"  components:\n"
"    rpms:\n"
"      perl-List-Compare:\n"
"        rationale: A dependency of tangerine.\n"
"        repository: git://pkgs.fedoraproject.org/rpms/perl-List-Compare\n"
"        cache: http://pkgs.fedoraproject.org/repo/pkgs/perl-List-Compare\n"
"        ref: master\n"
"      perl-Tangerine:\n"
"        rationale: Provides API for this module and is a dependency of tangerine.\n"
"        repository: git://pkgs.fedoraproject.org/rpms/perl-Tangerine\n"
"        cache: http://pkgs.fedoraproject.org/repo/pkgs/perl-Tangerine\n"
"        ref: 7e96446\n"
"      tangerine:\n"
"        rationale: Provides API for this module.\n"
"        repository: git://pkgs.fedoraproject.org/rpms/tangerine\n"
"        cache: http://pkgs.fedoraproject.org/repo/pkgs/tangerine\n"
"        ref: master\n"
"        buildorder: 10\n"
"  artifacts:\n"
"    rpms:\n"
"    - perl-List-Compare-0:0.53-9.module_1588+5eed94c6.noarch\n"
"    - perl-Tangerine-0:0.22-2.module_1588+5eed94c6.noarch\n"
"    - tangerine-0:0.22-7.module_1588+5eed94c6.noarch\n"
"...\n");
  // clang-format on

  struct custom_string custom;
  custom.string = str;
  custom.current = str;

  index = modulemd_module_index_new ();

  g_assert_true (modulemd_module_index_update_from_custom (
    index, custom_string_read_handler, &custom, TRUE, &failures, &error));
  g_assert_cmpint (failures->len, ==, 0);
  g_assert_no_error (error);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* Verify we did indeed get the module we expected */
  g_assert_nonnull (modulemd_module_index_get_module (index, "testmodule"));
}


static void
module_index_test_custom_write (void)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *output_string = NULL;
  g_autoptr (modulemd_yaml_string) yaml_string =
    g_malloc0_n (1, sizeof (modulemd_yaml_string));


  // clang-format off
  g_autofree gchar *str = g_strdup(
"---\n"
"document: modulemd\n"
"version: 2\n"
"data:\n"
"  name: testmodule\n"
"  stream: master\n"
"  version: 20180405123256\n"
"  context: c2c572ec\n"
"  arch: x86_64\n"
"  summary: A test module in all its beautiful beauty\n"
"  description: >-\n"
"    This module demonstrates how to write simple modulemd files And can be used for\n"
"    testing the build and release pipeline.\n"
"  license:\n"
"    module:\n"
"    - MIT\n"
"    content:\n"
"    - GPL+ or Artistic\n"
"    - MIT\n"
"  xmd:\n"
"    mbs:\n"
"      scmurl: https://src.fedoraproject.org/modules/testmodule.git?#0d33e028e4561f82ea43f670ee6366675cd6a6fe\n"
"      commit: 0d33e028e4561f82ea43f670ee6366675cd6a6fe\n"
"      buildrequires:\n"
"        platform:\n"
"          ref: virtual\n"
"          stream: f29\n"
"          filtered_rpms: []\n"
"          version: 4\n"
"      rpms:\n"
"        perl-List-Compare:\n"
"          ref: c6a689a6ce2683b15b32f83e6cb5d43ffd3816f5\n"
"        tangerine:\n"
"          ref: 239ada495d941ceefd8f359e1d8a47877fbba4a9\n"
"        perl-Tangerine:\n"
"          ref: 7e96446223f1ad84a26c7cf23d6591cd9f6326c6\n"
"      requires:\n"
"        platform:\n"
"          ref: virtual\n"
"          stream: f29\n"
"          filtered_rpms: []\n"
"          version: 4\n"
"  dependencies:\n"
"  - buildrequires:\n"
"      platform: [f29]\n"
"    requires:\n"
"      platform: [f29]\n"
"  references:\n"
"    community: https://docs.pagure.org/modularity/\n"
"    documentation: https://fedoraproject.org/wiki/Fedora_Packaging_Guidelines_for_Modules\n"
"  profiles:\n"
"    default:\n"
"      rpms:\n"
"      - tangerine\n"
"  api:\n"
"    rpms:\n"
"    - perl-Tangerine\n"
"    - tangerine\n"
"  components:\n"
"    rpms:\n"
"      perl-List-Compare:\n"
"        rationale: A dependency of tangerine.\n"
"        repository: git://pkgs.fedoraproject.org/rpms/perl-List-Compare\n"
"        cache: http://pkgs.fedoraproject.org/repo/pkgs/perl-List-Compare\n"
"        ref: master\n"
"      perl-Tangerine:\n"
"        rationale: Provides API for this module and is a dependency of tangerine.\n"
"        repository: git://pkgs.fedoraproject.org/rpms/perl-Tangerine\n"
"        cache: http://pkgs.fedoraproject.org/repo/pkgs/perl-Tangerine\n"
"        ref: 7e96446\n"
"      tangerine:\n"
"        rationale: Provides API for this module.\n"
"        repository: git://pkgs.fedoraproject.org/rpms/tangerine\n"
"        cache: http://pkgs.fedoraproject.org/repo/pkgs/tangerine\n"
"        ref: master\n"
"        buildorder: 10\n"
"  artifacts:\n"
"    rpms:\n"
"    - perl-List-Compare-0:0.53-9.module_1588+5eed94c6.noarch\n"
"    - perl-Tangerine-0:0.22-2.module_1588+5eed94c6.noarch\n"
"    - tangerine-0:0.22-7.module_1588+5eed94c6.noarch\n"
"...\n");
  // clang-format on

  index = modulemd_module_index_new ();

  g_assert_true (modulemd_module_index_update_from_string (
    index, str, TRUE, &failures, &error));
  g_assert_cmpint (failures->len, ==, 0);
  g_assert_no_error (error);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* Verify we did indeed get the module we expected */
  g_assert_nonnull (modulemd_module_index_get_module (index, "testmodule"));


  /* Write it out to another string
   * This is mainly to have a baseline to compare against
   */
  output_string = modulemd_module_index_dump_to_string (index, &error);
  g_assert_nonnull (output_string);
  g_assert_no_error (error);


  /* Write it out to a string using a custom emitter */
  g_assert_true (modulemd_module_index_dump_to_custom (
    index, write_yaml_string, yaml_string, &error));
  g_assert_nonnull (yaml_string->str);
  g_assert_no_error (error);
  g_assert_cmpstr (yaml_string->str, ==, output_string);
}


static void
module_index_test_get_default_streams (void)
{
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GHashTable) default_streams = NULL;

  yaml_path =
    g_strdup_printf ("%s/f29-updates.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (yaml_path);

  index = modulemd_module_index_new ();
  g_assert_nonnull (index);

  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);

  default_streams =
    modulemd_module_index_get_default_streams_as_hash_table (index, NULL);
  g_assert_nonnull (default_streams);

  g_assert_cmpint (g_hash_table_size (default_streams), ==, 3);

  g_assert_true (g_hash_table_contains (default_streams, "bat"));
  g_assert_cmpstr (g_hash_table_lookup (default_streams, "bat"), ==, "latest");

  g_assert_true (g_hash_table_contains (default_streams, "dwm"));
  g_assert_cmpstr (g_hash_table_lookup (default_streams, "dwm"), ==, "6.1");

  g_assert_true (g_hash_table_contains (default_streams, "stratis"));
  g_assert_cmpstr (g_hash_table_lookup (default_streams, "stratis"), ==, "1");

  g_assert_false (g_hash_table_contains (default_streams, "nodejs"));
}


static void
module_index_test_dump_empty_index (void)
{
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml = NULL;
  g_autoptr (ModulemdModuleIndex) index = modulemd_module_index_new ();

  g_assert_nonnull (index);
  g_assert_true (MODULEMD_IS_MODULE_INDEX (index));

  yaml = modulemd_module_index_dump_to_string (index, &error);
  g_assert_null (yaml);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
}


struct expected_compressed_read_t
{
  const gchar *filename;

  /* Whether this should succeed at reading */
  gboolean succeeds;

  /* If it fails, the expected error */
  GQuark error_domain;
  int error_code;
};

static void
test_module_index_read_compressed (void)
{
  gboolean bret;
  g_autoptr (ModulemdModuleIndex) baseline_idx = NULL;
  g_autoptr (ModulemdModuleIndex) compressed_idx = NULL;
  g_autofree gchar *file_path = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autofree gchar *baseline_text = NULL;
  g_autofree gchar *compressed_text = NULL;

#ifdef HAVE_RPMIO
#ifdef HAVE_LIBMAGIC
  struct expected_compressed_read_t expected[] = {
    {
      .filename = "bzipped",
      .succeeds = TRUE,
    },
    {
      .filename = "bzipped.yaml.bz2",
      .succeeds = TRUE,
    },
    { .filename = "gzipped", .succeeds = TRUE },
    { .filename = "gzipped.yaml.gz", .succeeds = TRUE },
    { .filename = "xzipped", .succeeds = TRUE },
    { .filename = "xzipped.yaml.xz", .succeeds = TRUE },
    { .filename = NULL }
  };
#else /* HAVE_LIBMAGIC */
  struct expected_compressed_read_t expected[] = {
    { .filename = "bzipped",
      .succeeds = FALSE,
      .error_domain = MODULEMD_YAML_ERROR,
      .error_code = MMD_YAML_ERROR_UNPARSEABLE },
    {
      .filename = "bzipped.yaml.bz2",
      .succeeds = TRUE,
    },
    { .filename = "gzipped",
      .succeeds = FALSE,
      .error_domain = MODULEMD_YAML_ERROR,
      .error_code = MMD_YAML_ERROR_UNPARSEABLE },
    { .filename = "gzipped.yaml.gz", .succeeds = TRUE },
    { .filename = "xzipped",
      .succeeds = FALSE,
      .error_domain = MODULEMD_YAML_ERROR,
      .error_code = MMD_YAML_ERROR_UNPARSEABLE },
    { .filename = "xzipped.yaml.xz", .succeeds = TRUE },
    { .filename = NULL }
  };
#endif /* HAVE_LIBMAGIC */

#else /* HAVE_RPMIO */
  struct expected_compressed_read_t expected[] = {
    { .filename = "bzipped",
      .succeeds = FALSE,
      .error_domain = MODULEMD_ERROR,
      .error_code = MMD_ERROR_NOT_IMPLEMENTED },
    { .filename = "bzipped.yaml.bz2",
      .succeeds = FALSE,
      .error_domain = MODULEMD_ERROR,
      .error_code = MMD_ERROR_NOT_IMPLEMENTED },
    { .filename = "gzipped",
      .succeeds = FALSE,
      .error_domain = MODULEMD_ERROR,
      .error_code = MMD_ERROR_NOT_IMPLEMENTED },
    { .filename = "gzipped.yaml.gz",
      .succeeds = FALSE,
      .error_domain = MODULEMD_ERROR,
      .error_code = MMD_ERROR_NOT_IMPLEMENTED },
    { .filename = "xzipped",
      .succeeds = FALSE,
      .error_domain = MODULEMD_ERROR,
      .error_code = MMD_ERROR_NOT_IMPLEMENTED },
    { .filename = "xzipped.yaml.xz",
      .succeeds = FALSE,
      .error_domain = MODULEMD_ERROR,
      .error_code = MMD_ERROR_NOT_IMPLEMENTED },
    { .filename = NULL }
  };
#endif /* HAVE_RPMIO */

  baseline_idx = modulemd_module_index_new ();
  g_assert_nonnull (baseline_idx);

  file_path = g_strdup_printf ("%s/compression/uncompressed.yaml",
                               g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (file_path);

  bret = modulemd_module_index_update_from_file (
    baseline_idx, file_path, TRUE, &failures, &error);
  g_assert_true (bret);
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);

  baseline_text = modulemd_module_index_dump_to_string (baseline_idx, &error);
  g_assert_nonnull (baseline_text);
  g_assert_no_error (error);

  g_clear_pointer (&file_path, g_free);

  for (size_t i = 0; expected[i].filename; i++)
    {
      compressed_idx = modulemd_module_index_new ();
      g_assert_nonnull (compressed_idx);

      file_path = g_strdup_printf ("%s/compression/%s",
                                   g_getenv ("TEST_DATA_PATH"),
                                   expected[i].filename);
      g_assert_nonnull (file_path);

      g_debug ("Processing %s, expecting %s",
               file_path,
               expected[i].succeeds ? "success" : "failure");
      bret = modulemd_module_index_update_from_file (
        compressed_idx, file_path, TRUE, &failures, &error);

      if (error)
        {
          g_debug ("Error: %s", error->message);
        }

      if (expected[i].succeeds)
        {
          g_assert_true (bret);
          g_assert_no_error (error);
        }
      else
        {
          g_assert_false (bret);
          g_assert_error (
            error, expected[i].error_domain, expected[i].error_code);

          g_clear_error (&error);
          g_clear_pointer (&file_path, g_free);
          g_clear_object (&compressed_idx);
          continue;
        }
      g_assert_cmpint (failures->len, ==, 0);

      compressed_text =
        modulemd_module_index_dump_to_string (compressed_idx, &error);
      g_assert_nonnull (compressed_text);
      g_assert_no_error (error);

      g_assert_cmpstr (baseline_text, ==, compressed_text);

      g_clear_pointer (&compressed_text, g_free);
      g_clear_pointer (&file_path, g_free);
      g_clear_object (&compressed_idx);
    }
}


static void
test_module_index_read_def_dir (void)
{
  g_autoptr (ModulemdModuleIndex) idx = modulemd_module_index_new ();
  g_autoptr (GError) error = NULL;
  g_autofree gchar *path =
    g_build_path ("/", g_getenv ("TEST_DATA_PATH"), "defaults", NULL);
  g_autofree gchar *bad_path =
    g_build_path ("/", g_getenv ("TEST_DATA_PATH"), "bad_defaults", NULL);
  g_autofree gchar *overrides_path =
    g_build_path ("/", path, "overrides", NULL);
  g_auto (GStrv) module_names = NULL;
  g_autoptr (GHashTable) defaultdict = NULL;

  g_assert_nonnull (idx);

  /* First verify that it works without overrides */
  g_assert_true (modulemd_module_index_update_from_defaults_directory (
    idx, path, TRUE, NULL, &error));
  g_assert_no_error (error);

  /* There should be three modules in the index now:
   * - meson
   * - ninja
   * - nodejs
   */
  module_names = modulemd_module_index_get_module_names_as_strv (idx);
  g_assert_nonnull (module_names);

  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "meson"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "ninja"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "nodejs"));

  g_assert_cmpint (g_strv_length (module_names), ==, 3);

  defaultdict =
    modulemd_module_index_get_default_streams_as_hash_table (idx, NULL);
  g_assert_nonnull (defaultdict);

  g_assert_cmpstr (g_hash_table_lookup (defaultdict, "meson"), ==, "latest");
  g_assert_cmpstr (g_hash_table_lookup (defaultdict, "ninja"), ==, "latest");
  g_assert_cmpstr (g_hash_table_lookup (defaultdict, "nodejs"), ==, NULL);

  g_clear_pointer (&module_names, g_strfreev);
  g_clear_pointer (&defaultdict, g_hash_table_unref);


  /* Verify with overrides */
  g_assert_true (modulemd_module_index_update_from_defaults_directory (
    idx, path, TRUE, overrides_path, &error));
  g_assert_no_error (error);

  /* There should be four modules in the index now:
   * - meson
   * - ninja
   * - nodejs
   * - testmodule
   */
  module_names = modulemd_module_index_get_module_names_as_strv (idx);
  g_assert_nonnull (module_names);

  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "meson"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "ninja"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "nodejs"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "testmodule"));

  g_assert_cmpint (g_strv_length (module_names), ==, 4);

  defaultdict =
    modulemd_module_index_get_default_streams_as_hash_table (idx, NULL);
  g_assert_nonnull (defaultdict);

  g_assert_cmpstr (g_hash_table_lookup (defaultdict, "meson"), ==, "latest");
  g_assert_cmpstr (g_hash_table_lookup (defaultdict, "ninja"), ==, "latest");
  g_assert_cmpstr (g_hash_table_lookup (defaultdict, "nodejs"), ==, "12");
  g_assert_cmpstr (
    g_hash_table_lookup (defaultdict, "testmodule"), ==, "teststream");

  g_clear_pointer (&module_names, g_strfreev);
  g_clear_pointer (&defaultdict, g_hash_table_unref);


  /* Nonexistent defaults dir */
  g_assert_false (modulemd_module_index_update_from_defaults_directory (
    idx, "nonexistent", TRUE, NULL, &error));
  g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_NOENT);
  g_clear_error (&error);

  /* Nonexistent overrides dir */
  g_assert_false (modulemd_module_index_update_from_defaults_directory (
    idx, path, TRUE, "nonexistent", &error));
  g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_NOENT);
  g_clear_error (&error);


  g_clear_object (&idx);
  idx = modulemd_module_index_new ();
  g_assert_nonnull (idx);

  /* Base directory contains two defaults with conflicting streams for the
   * same module in separate files. Strict mode.
   */
  g_assert_false (modulemd_module_index_update_from_defaults_directory (
    idx, bad_path, TRUE, NULL, &error));
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
  g_clear_error (&error);

  /* Verify that the index has not been modified as a side-effect */
  module_names = modulemd_module_index_get_module_names_as_strv (idx);
  g_assert_cmpint (g_strv_length (module_names), ==, 0);

  g_clear_pointer (&module_names, g_strfreev);
  g_clear_object (&idx);
  idx = modulemd_module_index_new ();
  g_assert_nonnull (idx);

  /* Base directory contains two defaults with conflicting streams for the
   * same module in separate files. Non-strict mode.
   */
  g_assert_true (modulemd_module_index_update_from_defaults_directory (
    idx, bad_path, FALSE, NULL, &error));
  g_assert_no_error (error);


  /* There should be three modules in the index now:
   * - meson
   * - ninja
   * - nodejs
   */
  module_names = modulemd_module_index_get_module_names_as_strv (idx);
  g_assert_nonnull (module_names);

  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "meson"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "ninja"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)module_names, "nodejs"));

  g_assert_cmpint (g_strv_length (module_names), ==, 3);

  defaultdict =
    modulemd_module_index_get_default_streams_as_hash_table (idx, NULL);
  g_assert_nonnull (defaultdict);

  /* Make sure that in non-strict mode, meson's default is reset to NULL */
  g_assert_cmpstr (g_hash_table_lookup (defaultdict, "meson"), ==, NULL);
  g_assert_cmpstr (g_hash_table_lookup (defaultdict, "ninja"), ==, "latest");
  g_assert_cmpstr (g_hash_table_lookup (defaultdict, "nodejs"), ==, NULL);

  g_clear_pointer (&module_names, g_strfreev);
  g_clear_pointer (&defaultdict, g_hash_table_unref);
}


static void
test_modulemd_index_search_streams (void)
{
  g_autoptr (ModulemdModuleIndex) index = modulemd_module_index_new ();
  g_autoptr (GError) error = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GPtrArray) streams = NULL;
  g_autofree gchar *yaml_path = NULL;

  yaml_path = g_strdup_printf ("%s/search_streams/search_streams.yaml",
                               g_getenv ("TEST_DATA_PATH"));

  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);

  streams = modulemd_module_index_search_streams (
    index, "nodejs", NULL, NULL, NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 3);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, "nonexistent", NULL, NULL, NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 0);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams =
    modulemd_module_index_search_streams (index, NULL, NULL, NULL, NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 5);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams =
    modulemd_module_index_search_streams (index, NULL, "8", NULL, NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 1);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, NULL, "nosuchstream", NULL, NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 0);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams =
    modulemd_module_index_search_streams (index, NULL, NULL, "1", NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 3);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams =
    modulemd_module_index_search_streams (index, NULL, NULL, "10", NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 0);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, NULL, NULL, NULL, "e0c83381", NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 1);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, NULL, NULL, NULL, "c2c572ec", NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 4);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, NULL, NULL, NULL, "deadbeef", NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 0);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, NULL, NULL, NULL, NULL, "i686");
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 0);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, NULL, NULL, NULL, NULL, "x86_64");
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 2);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, NULL, NULL, NULL, NULL, "ppc64le");
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 1);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams =
    modulemd_module_index_search_streams (index, NULL, "2*", NULL, NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 1);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, NULL, "[68]", NULL, NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 2);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams (
    index, NULL, "1.?", NULL, NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 1);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams =
    modulemd_module_index_search_streams (index, NULL, "*", NULL, NULL, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 5);
  g_clear_pointer (&streams, g_ptr_array_unref);
}


static void
test_module_index_search_streams_by_nsvca_glob (void)
{
  g_autoptr (ModulemdModuleIndex) index = modulemd_module_index_new ();
  g_autoptr (GError) error = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GPtrArray) streams = NULL;
  g_autofree gchar *yaml_path = NULL;

  yaml_path = g_strdup_printf ("%s/search_streams/search_streams.yaml",
                               g_getenv ("TEST_DATA_PATH"));

  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);

  streams = modulemd_module_index_search_streams_by_nsvca_glob (index, "*");
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 5);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams_by_nsvca_glob (index, NULL);
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 5);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams =
    modulemd_module_index_search_streams_by_nsvca_glob (index, "nodejs*");
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 3);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams =
    modulemd_module_index_search_streams_by_nsvca_glob (index, "nodejs:?*");
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 3);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams = modulemd_module_index_search_streams_by_nsvca_glob (index, "*8*");
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 4);
  g_clear_pointer (&streams, g_ptr_array_unref);

  streams =
    modulemd_module_index_search_streams_by_nsvca_glob (index, "nodejs:[68]*");
  g_assert_nonnull (streams);
  g_assert_cmpint (streams->len, ==, 2);
  g_clear_pointer (&streams, g_ptr_array_unref);
}


static void
test_module_index_search_rpms (void)
{
  g_autoptr (ModulemdModuleIndex) index = modulemd_module_index_new ();
  g_autoptr (GError) error = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GPtrArray) streams = NULL;
  g_autofree gchar *yaml_path = NULL;

  yaml_path = g_strdup_printf ("%s/search_streams/search_streams.yaml",
                               g_getenv ("TEST_DATA_PATH"));

  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);


  /* Searching for "python*" should give us ReviewBoard and Django */
  streams = modulemd_module_index_search_rpms (index, "python*");
  g_assert_cmpint (streams->len, ==, 2);
  g_clear_pointer (&streams, g_ptr_array_unref);

  /* Searching for "[nR]*" should give us three Node.js streams, plus
   * ReviewBoard
   */
  streams = modulemd_module_index_search_rpms (index, "[nR]*");
  g_assert_cmpint (streams->len, ==, 4);
  g_clear_pointer (&streams, g_ptr_array_unref);

  /* Searching for "*1.6*" should give us Django and Node.js 6 (because of
   * npm-1:3.10.10-1.6.13.1.1.module_1575+55808bea.x86_64)
   */
  streams = modulemd_module_index_search_rpms (index, "*1.6*");
  g_assert_cmpint (streams->len, ==, 2);
  g_clear_pointer (&streams, g_ptr_array_unref);

  /* Searching for "*" should give us all five streams */
  streams = modulemd_module_index_search_rpms (index, "*");
  g_assert_cmpint (streams->len, ==, 5);
  g_clear_pointer (&streams, g_ptr_array_unref);

  /* Searching for NULL should give us all five streams */
  streams = modulemd_module_index_search_rpms (index, NULL);
  g_assert_cmpint (streams->len, ==, 5);
  g_clear_pointer (&streams, g_ptr_array_unref);

  /* Searching for "perl-*" should give us nothing */
  streams = modulemd_module_index_search_rpms (index, "perl-*");
  g_assert_cmpint (streams->len, ==, 0);
  g_clear_pointer (&streams, g_ptr_array_unref);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add_func ("/modulemd/v2/module/index/dump", module_index_test_dump);

  g_test_add_func ("/modulemd/v2/module/index/read", module_index_test_read);

  g_test_add_func ("/modulemd/v2/module/index/read/mixed",
                   module_index_test_read_mixed);

  g_test_add_func ("/modulemd/v2/module/index/read/unknown",
                   module_index_test_read_unknown);

  g_test_add_func ("/modulemd/v2/module/index/upgrade/stream",
                   module_index_test_stream_upgrade);

  g_test_add_func ("/modulemd/v2/module/index/upgrade/index",
                   module_index_test_index_upgrade);

  g_test_add_func ("/modulemd/v2/module/index/remove_module",
                   module_index_test_remove_module);

  g_test_add_func ("/modulemd/v2/module/index/custom_read",
                   module_index_test_custom_read);

  g_test_add_func ("/modulemd/v2/module/index/custom_write",
                   module_index_test_custom_write);

  g_test_add_func ("/modulemd/v2/module/index/get_default_streams",
                   module_index_test_get_default_streams);

  g_test_add_func ("/modulemd/v2/module/index/empty",
                   module_index_test_dump_empty_index);

  g_test_add_func ("/modulemd/v2/module/index/compressed",
                   test_module_index_read_compressed);

  g_test_add_func ("/modulemd/v2/module/index/defaultdir",
                   test_module_index_read_def_dir);

  g_test_add_func ("/modulemd/v2/module/index/search",
                   test_modulemd_index_search_streams);

  g_test_add_func ("/modulemd/v2/module/index/search_nsvca",
                   test_module_index_search_streams_by_nsvca_glob);

  g_test_add_func ("/modulemd/v2/module/index/search_rpms",
                   test_module_index_search_rpms);

  return g_test_run ();
}
