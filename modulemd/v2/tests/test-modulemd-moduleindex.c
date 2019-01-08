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

#include "modulemd-defaults.h"
#include "modulemd-module.h"
#include "modulemd-module-index.h"
#include "modulemd-module-stream-v1.h"
#include "modulemd-module-stream-v2.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _ModuleIndexFixture
{
} ModuleIndexFixture;


static void
module_index_test_dump (ModuleIndexFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (ModulemdTranslation) translation = NULL;
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

  /* Third: some streams */
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
                   "document: modulemd-translations\n"
                   "version: 1\n"
                   "data:\n"
                   "  module: testmodule1\n"
                   "  stream: teststream1\n"
                   "  modified: 42\n"
                   "  translations:\n"
                   "  - nl_NL\n"
                   "  - summary: Een test omschrijving\n"
                   "  - ro_TA\n"
                   "  - summary: Testsummary in ro_TA\n"
                   "...\n"
                   "---\n"
                   "document: modulemd\n"
                   "version: 2\n"
                   "data:\n"
                   "  name: testmodule1\n"
                   "  stream: teststream1\n"
                   "  version: 1\n"
                   "  context: deadbeef\n"
                   "  summary: A test stream\n"
                   "  description: A test stream's description\n"
                   "  license:\n"
                   "    module:\n"
                   "    - Beerware\n"
                   "...\n"
                   "---\n"
                   "document: modulemd\n"
                   "version: 2\n"
                   "data:\n"
                   "  name: testmodule1\n"
                   "  stream: teststream2\n"
                   "  version: 2\n"
                   "  context: c0ff33\n"
                   "  summary: A second stream\n"
                   "  description: A second stream's description\n"
                   "  license:\n"
                   "    module:\n"
                   "    - Beerware\n"
                   "...\n");
}


static void
module_index_test_read (ModuleIndexFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  ModulemdSubdocumentInfo *subdoc = NULL;
  g_autofree gchar *yaml_path = NULL;

  /* Read the specification files all in */
  index = modulemd_module_index_new ();

  /* The two stream definitions */
  yaml_path =
    g_strdup_printf ("%s/spec.v1.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);
  yaml_path =
    g_strdup_printf ("%s/spec.v2.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* The translation definitions */
  yaml_path = g_strdup_printf ("%s/translations/spec.v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* The defaults definitions */
  yaml_path = g_strdup_printf ("%s/mod-defaults/spec.v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_true (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 0);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* A stream that has nonsense in "data" */
  yaml_path =
    g_strdup_printf ("%s/modulemd/v2/tests/test_data/broken_stream.yaml",
                     g_getenv ("MESON_SOURCE_ROOT"));
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
    g_strdup_printf ("%s/nothinghere.yaml", g_getenv ("MESON_SOURCE_ROOT"));
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
  yaml_path = g_strdup_printf ("%s/modulemd/v2/tests/test_data/te.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_false (modulemd_module_index_update_from_file (
    index, yaml_path, TRUE, &failures, &error));
  g_assert_no_error (error);
  g_assert_cmpint (failures->len, ==, 1);
  g_clear_pointer (&yaml_path, g_free);
  g_clear_pointer (&failures, g_ptr_array_unref);

  /* Actually verifying the contents is left to Python tests */
}


static void
module_index_test_read_mixed (ModuleIndexFixture *fixture,
                              gconstpointer user_data)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *output = NULL;

  index = modulemd_module_index_new ();

  yaml_path =
    g_strdup_printf ("%s/modulemd/v2/tests/test_data/long-valid.yaml",
                     g_getenv ("MESON_SOURCE_ROOT"));
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
module_index_test_read_unknown (ModuleIndexFixture *fixture,
                                gconstpointer user_data)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;

  index = modulemd_module_index_new ();

  yaml_path =
    g_strdup_printf ("%s/modulemd/v2/tests/test_data/good-v2-extra-keys.yaml",
                     g_getenv ("MESON_SOURCE_ROOT"));
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
module_index_test_stream_upgrade (ModuleIndexFixture *fixture,
                                  gconstpointer user_data)
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
  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef"));
  g_assert_nonnull (stream);
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
  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream2",
    2,
    "c0ff33"));
  g_assert_nonnull (stream);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);

  /* Verify that the first object was upgraded to StreamV2 */
  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef"));
  g_assert_nonnull (stream);
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
  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream3",
    3,
    "badfeed"));
  g_assert_nonnull (stream);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);
}


static void
module_index_test_index_upgrade (ModuleIndexFixture *fixture,
                                 gconstpointer user_data)
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
  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef"));
  g_assert_nonnull (stream);
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
  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream3",
    3,
    "badfeed"));
  g_assert_nonnull (stream);
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
  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef"));
  g_assert_nonnull (stream);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);

  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream3",
    3,
    "badfeed"));
  g_assert_nonnull (stream);
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
  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream1",
    1,
    "deadbeef"));
  g_assert_nonnull (stream);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_TWO);
  g_clear_object (&stream);

  stream = g_object_ref (modulemd_module_get_stream_by_NSVC (
    modulemd_module_index_get_module (index, "testmodule1"),
    "teststream3",
    3,
    "badfeed"));
  g_assert_nonnull (stream);
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


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.

  g_test_add ("/modulemd/v2/module/index/dump",
              ModuleIndexFixture,
              NULL,
              NULL,
              module_index_test_dump,
              NULL);

  g_test_add ("/modulemd/v2/module/index/read",
              ModuleIndexFixture,
              NULL,
              NULL,
              module_index_test_read,
              NULL);

  g_test_add ("/modulemd/v2/module/index/read/mixed",
              ModuleIndexFixture,
              NULL,
              NULL,
              module_index_test_read_mixed,
              NULL);

  g_test_add ("/modulemd/v2/module/index/read/unknown",
              ModuleIndexFixture,
              NULL,
              NULL,
              module_index_test_read_unknown,
              NULL);

  g_test_add ("/modulemd/v2/module/index/upgrade/stream",
              ModuleIndexFixture,
              NULL,
              NULL,
              module_index_test_stream_upgrade,
              NULL);

  g_test_add ("/modulemd/v2/module/index/upgrade/index",
              ModuleIndexFixture,
              NULL,
              NULL,
              module_index_test_index_upgrade,
              NULL);

  return g_test_run ();
}
