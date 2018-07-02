/*
 * This file is part of libmodulemd
 * Copyright (C) 2017-2018 Stephen Gallagher
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */


#include "modulemd.h"
#include "modulemd-translation.h"

#include <glib.h>
#include <locale.h>

typedef struct _TranslationFixture
{
} TranslationFixture;


static void
modulemd_translation_test_basic (TranslationFixture *fixture,
                                 gconstpointer user_data)
{
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (ModulemdTranslation) copy = NULL;
  g_autoptr (ModulemdTranslationEntry) entry = NULL;
  g_autoptr (ModulemdTranslationEntry) retrieved_entry = NULL;
  g_autofree gchar *module_name = NULL;
  g_autofree gchar *module_stream = NULL;
  guint64 mdversion, modified;

  /* Test standard object construction succeeds */
  translation = g_object_new (MODULEMD_TYPE_TRANSLATION, NULL);
  g_assert_nonnull (translation);
  g_assert_null (modulemd_translation_peek_module_name (translation));
  g_assert_null (modulemd_translation_peek_module_stream (translation));
  g_assert_cmpint (modulemd_translation_get_modified (translation), ==, 0);
  g_clear_pointer (&translation, g_object_unref);

  /* Test construction with values set */
  translation = modulemd_translation_new_full (
    "foomodule", "barstream", 1, 201806282100llu);

  g_assert_nonnull (translation);
  g_assert_cmpstr (
    modulemd_translation_peek_module_name (translation), ==, "foomodule");
  g_assert_cmpstr (
    modulemd_translation_peek_module_stream (translation), ==, "barstream");
  g_assert_cmpint (modulemd_translation_get_mdversion (translation), ==, 1);
  g_assert_cmpint (
    modulemd_translation_get_modified (translation), ==, 201806282100llu);

  // clang-format off
  g_object_get (translation,
                "module-name", &module_name,
                "module-stream", &module_stream,
                "mdversion", &mdversion,
                "modified", &modified,
                NULL);
  // clang-format on
  //
  g_assert_cmpstr (module_name, ==, "foomodule");
  g_assert_cmpstr (module_stream, ==, "barstream");
  g_assert_cmpint (mdversion, ==, 1);
  g_assert_cmpint (modified, ==, 201806282100llu);

  entry = modulemd_translation_entry_new ("en-US");
  modulemd_translation_entry_set_summary (entry, "Summary Text");
  modulemd_translation_entry_set_description (entry, "Desc Text");

  modulemd_translation_add_entry (translation, entry);

  retrieved_entry =
    modulemd_translation_get_entry_by_locale (translation, "en-US");
  g_assert_nonnull (retrieved_entry);
  g_assert_cmpstr (modulemd_translation_entry_peek_summary (retrieved_entry),
                   ==,
                   "Summary Text");
  g_assert_cmpstr (
    modulemd_translation_entry_peek_description (retrieved_entry),
    ==,
    "Desc Text");

  g_clear_pointer (&module_name, g_free);
  g_clear_pointer (&module_stream, g_free);
  g_clear_pointer (&retrieved_entry, g_object_unref);

  copy = modulemd_translation_copy (translation);
  g_assert_nonnull (copy);

  // clang-format off
  g_object_get (copy,
                "module-name", &module_name,
                "module-stream", &module_stream,
                "mdversion", &mdversion,
                "modified", &modified,
                NULL);
  //clang-format on
  g_assert_cmpstr (module_name, ==, "foomodule");
  g_assert_cmpstr (module_stream, ==, "barstream");
  g_assert_cmpint (mdversion, ==, 1);
  g_assert_cmpint (modified, ==, 201806282100llu);

  retrieved_entry = modulemd_translation_get_entry_by_locale (copy, "en-US");
  g_assert_nonnull (retrieved_entry);
  g_assert_cmpstr (modulemd_translation_entry_peek_summary (retrieved_entry),
                   ==,
                   "Summary Text");
  g_assert_cmpstr (
    modulemd_translation_entry_peek_description (retrieved_entry),
    ==,
    "Desc Text");
}

static void
modulemd_translation_test_yaml (TranslationFixture *fixture,
                                gconstpointer user_data)
{
  g_autoptr (GPtrArray) objects = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml_path = NULL;
  ModulemdTranslation *translation = NULL;
  ModulemdTranslationEntry *entry = NULL;
  g_autofree gchar *module_name;
  g_autofree gchar *module_stream;
  guint64 mdversion, modified;

  yaml_path = g_strdup_printf ("%s/translations/spec.v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));

  objects = modulemd_objects_from_file_ext (yaml_path, &failures, &error);
  g_assert_nonnull (objects);
  g_assert_cmpint (objects->len, ==, 1);

  translation = g_ptr_array_index (objects, 0);
  g_assert_true (MODULEMD_IS_TRANSLATION (translation));

  // clang-format off
  g_object_get (translation,
                "module-name", &module_name,
                "module-stream", &module_stream,
                "mdversion", &mdversion,
                "modified", &modified,
                NULL);
  // clang-format on

  g_assert_cmpstr (module_name, ==, "foo");
  g_assert_cmpstr (module_stream, ==, "latest");
  g_assert_cmpint (mdversion, ==, 1);
  g_assert_cmpint (modified, ==, 201805231425llu);

  entry = modulemd_translation_get_entry_by_locale (translation, "ja");
  g_assert_nonnull (entry);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (entry));
  g_assert_cmpstr (modulemd_translation_entry_peek_locale (entry), ==, "ja");
  g_assert_cmpstr (
    modulemd_translation_entry_peek_summary (entry), ==, "モジュールの例");
  g_assert_cmpstr (modulemd_translation_entry_peek_description (entry),
                   ==,
                   "モジュールの例です。");
  g_assert_cmpstr (
    modulemd_translation_entry_peek_profile_description (entry, "profile_a"),
    ==,
    "プロファイルの例");
}


static void
modulemd_translation_test_import (TranslationFixture *fixture,
                                  gconstpointer user_data)
{
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml_path = NULL;
  ModulemdTranslationEntry *entry = NULL;
  g_autofree gchar *module_name;
  g_autofree gchar *module_stream;
  guint64 mdversion, modified;

  yaml_path = g_strdup_printf ("%s/translations/spec.v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));

  translation = modulemd_translation_new ();
  modulemd_translation_import_from_file (translation, yaml_path, &error);

  g_assert_nonnull (translation);
  g_assert_true (MODULEMD_IS_TRANSLATION (translation));

  // clang-format off
  g_object_get (translation,
                "module-name", &module_name,
                "module-stream", &module_stream,
                "mdversion", &mdversion,
                "modified", &modified,
                NULL);
  // clang-format on

  g_assert_cmpstr (module_name, ==, "foo");
  g_assert_cmpstr (module_stream, ==, "latest");
  g_assert_cmpint (mdversion, ==, 1);
  g_assert_cmpint (modified, ==, 201805231425llu);

  entry = modulemd_translation_get_entry_by_locale (translation, "ja");
  g_assert_nonnull (entry);
  g_assert_true (MODULEMD_IS_TRANSLATION_ENTRY (entry));
  g_assert_cmpstr (modulemd_translation_entry_peek_locale (entry), ==, "ja");
  g_assert_cmpstr (
    modulemd_translation_entry_peek_summary (entry), ==, "モジュールの例");
  g_assert_cmpstr (modulemd_translation_entry_peek_description (entry),
                   ==,
                   "モジュールの例です。");
  g_assert_cmpstr (
    modulemd_translation_entry_peek_profile_description (entry, "profile_a"),
    ==,
    "プロファイルの例");
}


static void
modulemd_translation_test_emitter (TranslationFixture *fixture,
                                   gconstpointer user_data)
{
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autofree gchar *output_yaml = NULL;

  yaml_path = g_strdup_printf ("%s/translations/spec.v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  translation = modulemd_translation_new ();
  modulemd_translation_import_from_file (translation, yaml_path, &error);

  output_yaml = modulemd_translation_dumps (translation, &error);
  if (output_yaml == NULL)
    {
      g_debug ("Error: %s", error->message);
    }
  g_assert_nonnull (output_yaml);
  g_assert_true (output_yaml[0]);

  g_debug ("\n%s\n", output_yaml);

  g_assert_cmpstr (
    output_yaml,
    ==,
    "---\ndocument: modulemd-translations\nversion: 1\ndata:\n  module: foo\n "
    " stream: latest\n  modified: 201805231425\n  translations:\n    en_GB:\n "
    "     summary: An example module\n      description: An example module.\n "
    "     profiles:\n        profile_a: An example profile\n    es_ES:\n      "
    "summary: Un módulo de ejemplo\n      description: Un módulo de "
    "ejemplo.\n      profiles:\n        profile_a: Un perfil de ejemplo\n    "
    "ja:\n      summary: モジュールの例\n      description: "
    "モジュールの例です。\n      profiles:\n        profile_a: "
    "プロファイルの例\n...\n");
}


static void
modulemd_translation_test_index (TranslationFixture *fixture,
                                 gconstpointer user_data)
{
  g_autoptr (GHashTable) index = NULL;
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) error = NULL;
  ModulemdImprovedModule *module = NULL;
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (GHashTable) profiles = NULL;
  ModulemdProfile *profile = NULL;
  g_autofree gchar *result_yaml = NULL;

  yaml_path = g_strdup_printf ("%s/test_data/translations.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  index = modulemd_index_from_file (yaml_path, &failures, &error);
  g_assert_nonnull (index);

  g_assert_true (g_hash_table_contains (index, "foo"));

  module = g_hash_table_lookup (index, "foo");
  g_assert_nonnull (module);

  g_assert_cmpstr (modulemd_improvedmodule_peek_name (module), ==, "foo");

  stream = modulemd_improvedmodule_get_stream_by_name (module, "stream-name");
  g_assert_nonnull (stream);

  translation = modulemd_modulestream_get_translation (stream);
  g_assert_nonnull (translation);

  g_assert_cmpstr (
    modulemd_translation_peek_module_name (translation), ==, "foo");
  g_assert_cmpstr (
    modulemd_translation_peek_module_stream (translation), ==, "stream-name");
  g_assert_cmpuint (
    modulemd_translation_get_modified (translation), ==, 201805231425llu);

  result_yaml = modulemd_dumps_index (index, &error);
  g_assert_nonnull (result_yaml);

  g_debug ("YAML:\n%s", result_yaml);

  g_assert_cmpstr (
    result_yaml,
    ==,
    "---\ndocument: modulemd\nversion: 2\ndata:\n  name: foo\n  stream: "
    "stream-name\n  version: 20160927144203\n  context: c0ffee43\n  arch: "
    "x86_64\n  summary: An example module\n  description: >-\n    A module "
    "for the demonstration of the metadata format. Also, the obligatory "
    "lorem\n    ipsum dolor sit amet goes right here.\n  servicelevels:\n    "
    "bug_fixes:\n      eol: 2077-10-23\n    rawhide:\n      eol: 2077-10-23\n "
    "   security_fixes:\n      eol: 2077-10-23\n    stable_api:\n      eol: "
    "2077-10-23\n  license:\n    module:\n    - MIT\n    content:\n    - "
    "Beerware\n    - GPLv2+\n    - zlib\n  xmd:\n    some_key: some_data\n  "
    "dependencies:\n  - buildrequires:\n      platform: [-epel7, -f27, "
    "-f28]\n    requires:\n      platform: [-epel7, -f27, -f28]\n  - "
    "buildrequires:\n      buildtools: [v1, v2]\n      compatible: [v3]\n     "
    " platform: [f27]\n    requires:\n      compatible: [v3, v4]\n      "
    "platform: [f27]\n  - buildrequires:\n      platform: [f28]\n    "
    "requires:\n      platform: [f28]\n      runtime: [a, b]\n  - "
    "buildrequires:\n      extras: []\n      moreextras: [bar, foo]\n      "
    "platform: [epel7]\n    requires:\n      extras: []\n      moreextras: "
    "[bar, foo]\n      platform: [epel7]\n  references:\n    community: "
    "http://www.example.com/\n    documentation: http://www.example.com/\n    "
    "tracker: http://www.example.com/\n  profiles:\n    buildroot:\n      "
    "rpms:\n      - bar-devel\n    container:\n      rpms:\n      - bar\n     "
    " - bar-devel\n    default:\n      description: An example profile\n      "
    "rpms:\n      - bar\n      - "
    "bar-extras\n      - baz\n    minimal:\n      description: Minimal "
    "profile installing only the bar package.\n      rpms:\n      - bar\n    "
    "srpm-buildroot:\n      rpms:\n      - bar-extras\n  api:\n    rpms:\n    "
    "- bar\n    - bar-devel\n    - bar-extras\n    - baz\n    - xxx\n  "
    "filter:\n    rpms:\n    - baz-nonfoo\n  buildopts:\n    rpms:\n      "
    "macros: |\n        %demomacro 1\n        %demomacro2 %{demomacro}23\n    "
    "  whitelist:\n      - fooscl-1-bar\n      - fooscl-1-baz\n      - xxx\n  "
    "    - xyz\n  components:\n    rpms:\n      bar:\n        rationale: We "
    "need this to demonstrate stuff.\n        repository: "
    "https://pagure.io/bar.git\n        cache: https://example.com/cache\n    "
    "    ref: 26ca0c0\n      baz:\n        rationale: This one is here to "
    "demonstrate other stuff.\n      xxx:\n        rationale: xxx "
    "demonstrates arches and multilib.\n        arches: [i686, x86_64]\n      "
    "  multilib: [x86_64]\n      xyz:\n        rationale: xyz is a bundled "
    "dependency of xxx.\n        buildorder: 10\n    modules:\n      "
    "includedmodule:\n        rationale: Included in the stack, just "
    "because.\n        repository: https://pagure.io/includedmodule.git\n     "
    "   ref: https://pagure.io/includedmodule.git\n        buildorder: 100\n  "
    "artifacts:\n    rpms:\n    - bar-0:1.23-1.module_deadbeef.x86_64\n    - "
    "bar-devel-0:1.23-1.module_deadbeef.x86_64\n    - "
    "bar-extras-0:1.23-1.module_deadbeef.x86_64\n    - "
    "baz-0:42-42.module_deadbeef.x86_64\n    - "
    "xxx-0:1-1.module_deadbeef.i686\n    - xxx-0:1-1.module_deadbeef.x86_64\n "
    "   - xyz-0:1-1.module_deadbeef.x86_64\n...\n---\ndocument: "
    "modulemd-translations\nversion: 1\ndata:\n  module: foo\n  stream: "
    "stream-name\n  modified: 201805231425\n  translations:\n    en_GB:\n     "
    " summary: An example module\n      description: An example module.\n     "
    " profiles:\n        default: An example profile\n    es_ES:\n      "
    "summary: Un módulo de ejemplo\n      description: Un módulo de "
    "ejemplo.\n      profiles:\n        default: Un perfil de ejemplo\n    "
    "ja:\n      summary: モジュールの例\n      description: "
    "モジュールの例です。\n      profiles:\n        default: "
    "プロファイルの例\n...\n---\ndocument: modulemd-defaults\nversion: "
    "1\ndata:\n  module: foo\n  stream: stream-name\n  profiles:\n    "
    "stream_name: [default]\n  intents: {}\n...\n");

  /* Test specific translations */
  g_assert_cmpstr (modulemd_modulestream_get_localized_summary (stream, "ja"),
                   ==,
                   "モジュールの例");
  g_assert_cmpstr (
    modulemd_modulestream_get_localized_description (stream, "ja"),
    ==,
    "モジュールの例です。");

  profiles = modulemd_modulestream_get_profiles (stream);
  g_assert_nonnull (profiles);

  g_assert_true (g_hash_table_contains (profiles, "default"));

  profile = g_hash_table_lookup (profiles, "default");
  g_assert_nonnull (profile);

  g_assert_cmpstr (modulemd_profile_get_localized_description (profile, "ja"),
                   ==,
                   "プロファイルの例");
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests

  g_test_add ("/modulemd/translation/test_basic",
              TranslationFixture,
              NULL,
              NULL,
              modulemd_translation_test_basic,
              NULL);

  g_test_add ("/modulemd/translation/test_yaml",
              TranslationFixture,
              NULL,
              NULL,
              modulemd_translation_test_yaml,
              NULL);

  g_test_add ("/modulemd/translation/test_import",
              TranslationFixture,
              NULL,
              NULL,
              modulemd_translation_test_import,
              NULL);

  g_test_add ("/modulemd/translation/test_emitter",
              TranslationFixture,
              NULL,
              NULL,
              modulemd_translation_test_emitter,
              NULL);

  g_test_add ("/modulemd/translation/test_index",
              TranslationFixture,
              NULL,
              NULL,
              modulemd_translation_test_index,
              NULL);

  return g_test_run ();
}
