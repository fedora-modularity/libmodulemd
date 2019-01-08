#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <signal.h>

#include "modulemd-module-stream.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _ModuleStreamFixture
{
} ModuleStreamFixture;


static void
module_stream_test_construct (ModuleStreamFixture *fixture,
                              gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      /* Test that the new() function works */
      stream = modulemd_module_stream_new (version, "foo", "latest");
      g_assert_nonnull (stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (stream));

      g_assert_cmpint (
        modulemd_module_stream_get_mdversion (stream), ==, version);
      g_assert_cmpstr (
        modulemd_module_stream_get_module_name (stream), ==, "foo");
      g_assert_cmpstr (
        modulemd_module_stream_get_stream_name (stream), ==, "latest");

      g_clear_object (&stream);

      /* Test that the new() function works without a stream name */
      stream = modulemd_module_stream_new (version, "foo", NULL);
      g_assert_nonnull (stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (stream));

      g_assert_cmpint (
        modulemd_module_stream_get_mdversion (stream), ==, version);
      g_assert_cmpstr (
        modulemd_module_stream_get_module_name (stream), ==, "foo");
      g_assert_null (modulemd_module_stream_get_stream_name (stream));

      g_clear_object (&stream);

      /* Test with no module name */
      stream = modulemd_module_stream_new (version, NULL, NULL);
      g_assert_nonnull (stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (stream));

      g_assert_cmpint (
        modulemd_module_stream_get_mdversion (stream), ==, version);
      g_assert_null (modulemd_module_stream_get_module_name (stream));
      g_assert_null (modulemd_module_stream_get_stream_name (stream));

      g_clear_object (&stream);
    }

  /* Test with a zero mdversion */
  stream = modulemd_module_stream_new (0, "foo", "latest");
  g_assert_null (stream);
  g_clear_object (&stream);


  /* Test with an unknown mdversion */
  stream = modulemd_module_stream_new (
    MD_MODULESTREAM_VERSION_LATEST + 1, "foo", "latest");
  g_assert_null (stream);
  g_clear_object (&stream);
}

static void
module_stream_test_copy (ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdModuleStream) copied_stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      /* Test copying with a stream name */
      stream = modulemd_module_stream_new (version, "foo", "latest");
      copied_stream = modulemd_module_stream_copy (stream, NULL, NULL);
      g_assert_nonnull (copied_stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_module_name (stream),
                       ==,
                       modulemd_module_stream_get_module_name (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_stream_name (stream),
                       ==,
                       modulemd_module_stream_get_stream_name (copied_stream));
      g_clear_object (&stream);
      g_clear_object (&copied_stream);


      /* Test copying without a stream name */
      stream = modulemd_module_stream_new (version, "foo", NULL);
      copied_stream = modulemd_module_stream_copy (stream, NULL, NULL);
      g_assert_nonnull (copied_stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_module_name (stream),
                       ==,
                       modulemd_module_stream_get_module_name (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_stream_name (stream),
                       ==,
                       modulemd_module_stream_get_stream_name (copied_stream));
      g_clear_object (&stream);
      g_clear_object (&copied_stream);

      /* Test copying with and renaming the stream name */
      stream = modulemd_module_stream_new (version, "foo", "latest");
      copied_stream = modulemd_module_stream_copy (stream, NULL, "earliest");
      g_assert_nonnull (copied_stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_module_name (stream),
                       ==,
                       modulemd_module_stream_get_module_name (copied_stream));
      g_assert_cmpstr (
        modulemd_module_stream_get_stream_name (stream), ==, "latest");
      g_assert_cmpstr (modulemd_module_stream_get_stream_name (copied_stream),
                       ==,
                       "earliest");
      g_clear_object (&stream);
      g_clear_object (&copied_stream);
    }
}

static void
module_stream_v1_test_parse_dump (ModuleStreamFixture *fixture,
                                  gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_autoptr (GError) error = NULL;
  gboolean ret;
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  yaml_path =
    g_strdup_printf ("%s/spec.v1.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rb");
  g_assert_nonnull (yaml_stream);

  /* First parse it */
  yaml_parser_set_input_file (&parser, yaml_stream);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_STREAM_START_EVENT);
  yaml_event_delete (&event);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_DOCUMENT_START_EVENT);
  yaml_event_delete (&event);

  subdoc = modulemd_yaml_parse_document_type (&parser);
  g_assert_nonnull (subdoc);
  g_assert_null (modulemd_subdocument_info_get_gerror (subdoc));

  g_assert_cmpint (modulemd_subdocument_info_get_doctype (subdoc),
                   ==,
                   MODULEMD_YAML_DOC_MODULESTREAM);
  g_assert_cmpint (modulemd_subdocument_info_get_mdversion (subdoc), ==, 1);
  g_assert_nonnull (modulemd_subdocument_info_get_yaml (subdoc));

  stream = modulemd_module_stream_v1_parse_yaml (subdoc, TRUE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (stream);

  /* Then dump it */
  g_debug ("Starting dumping");
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  ret = modulemd_module_stream_v1_emit_yaml (stream, &emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  ret = mmd_emitter_end_stream (&emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  g_assert_nonnull (yaml_string->str);

  g_assert_cmpstr (
    yaml_string->str,
    ==,
    "---\n"
    "document: modulemd\n"
    "version: 1\n"
    "data:\n"
    "  name: foo\n"
    "  stream: stream-name\n"
    "  version: 20160927144203\n"
    "  context: c0ffee43\n"
    "  arch: x86_64\n"
    "  summary: An example module\n"
    "  description: >-\n"
    "    A module for the demonstration of the metadata format. Also, the "
    "obligatory lorem\n"
    "    ipsum dolor sit amet goes right here.\n"
    "  servicelevels:\n"
    "    bug_fixes:\n"
    "      eol: 2077-10-23\n"
    "    rawhide:\n"
    "      eol: 2077-10-23\n"
    "    security_fixes:\n"
    "      eol: 2077-10-23\n"
    "    stable_api:\n"
    "      eol: 2077-10-23\n"
    "  license:\n"
    "    module:\n"
    "    - MIT\n"
    "    content:\n"
    "    - Beerware\n"
    "    - GPLv2+\n"
    "    - zlib\n"
    "  xmd:\n"
    "    some_key: some_data\n"
    "  dependencies:\n"
    "    buildrequires:\n"
    "      extra-build-env: and-its-stream-name-too\n"
    "      platform: and-its-stream-name\n"
    "    requires:\n"
    "      platform: and-its-stream-name\n"
    "  references:\n"
    "    community: http://www.example.com/\n"
    "    documentation: http://www.example.com/\n"
    "    tracker: http://www.example.com/\n"
    "  profiles:\n"
    "    buildroot:\n"
    "      rpms:\n"
    "      - bar-devel\n"
    "    container:\n"
    "      rpms:\n"
    "      - bar\n"
    "      - bar-devel\n"
    "    default:\n"
    "      rpms:\n"
    "      - bar\n"
    "      - bar-extras\n"
    "      - baz\n"
    "    minimal:\n"
    "      description: Minimal profile installing only the bar package.\n"
    "      rpms:\n"
    "      - bar\n"
    "    srpm-buildroot:\n"
    "      rpms:\n"
    "      - bar-extras\n"
    "  api:\n"
    "    rpms:\n"
    "    - bar\n"
    "    - bar-devel\n"
    "    - bar-extras\n"
    "    - baz\n"
    "    - xxx\n"
    "  filter:\n"
    "    rpms:\n"
    "    - baz-nonfoo\n"
    "  buildopts:\n"
    "    rpms:\n"
    "      macros: >\n"
    "        %demomacro 1\n"
    "\n"
    "        %demomacro2 %{demomacro}23\n"
    "  components:\n"
    "    rpms:\n"
    "      bar:\n"
    "        rationale: We need this to demonstrate stuff.\n"
    "        repository: https://pagure.io/bar.git\n"
    "        cache: https://example.com/cache\n"
    "        ref: 26ca0c0\n"
    "      baz:\n"
    "        rationale: This one is here to demonstrate other stuff.\n"
    "      xxx:\n"
    "        rationale: xxx demonstrates arches and multilib.\n"
    "        arches: [i686, x86_64]\n"
    "        multilib: [x86_64]\n"
    "      xyz:\n"
    "        rationale: xyz is a bundled dependency of xxx.\n"
    "        buildorder: 10\n"
    "    modules:\n"
    "      includedmodule:\n"
    "        rationale: Included in the stack, just because.\n"
    "        repository: https://pagure.io/includedmodule.git\n"
    "        ref: somecoolbranchname\n"
    "        buildorder: 100\n"
    "  artifacts:\n"
    "    rpms:\n"
    "    - bar-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-devel-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-extras-0:1.23-1.module_deadbeef.x86_64\n"
    "    - baz-0:42-42.module_deadbeef.x86_64\n"
    "    - xxx-0:1-1.module_deadbeef.i686\n"
    "    - xxx-0:1-1.module_deadbeef.x86_64\n"
    "    - xyz-0:1-1.module_deadbeef.x86_64\n"
    "...\n");
}

static void
module_stream_v2_test_parse_dump (ModuleStreamFixture *fixture,
                                  gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (GError) error = NULL;
  gboolean ret;
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  yaml_path =
    g_strdup_printf ("%s/spec.v2.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rb");
  g_assert_nonnull (yaml_stream);

  /* First parse it */
  yaml_parser_set_input_file (&parser, yaml_stream);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_STREAM_START_EVENT);
  yaml_event_delete (&event);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_DOCUMENT_START_EVENT);
  yaml_event_delete (&event);

  subdoc = modulemd_yaml_parse_document_type (&parser);
  g_assert_nonnull (subdoc);
  g_assert_null (modulemd_subdocument_info_get_gerror (subdoc));

  g_assert_cmpint (modulemd_subdocument_info_get_doctype (subdoc),
                   ==,
                   MODULEMD_YAML_DOC_MODULESTREAM);
  g_assert_cmpint (modulemd_subdocument_info_get_mdversion (subdoc), ==, 2);
  g_assert_nonnull (modulemd_subdocument_info_get_yaml (subdoc));

  stream = modulemd_module_stream_v2_parse_yaml (subdoc, TRUE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (stream);

  /* Then dump it */
  g_debug ("Starting dumping");
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  ret = modulemd_module_stream_v2_emit_yaml (stream, &emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  ret = mmd_emitter_end_stream (&emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  g_assert_nonnull (yaml_string->str);

  g_assert_cmpstr (
    yaml_string->str,
    ==,
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  name: foo\n"
    "  stream: latest\n"
    "  version: 20160927144203\n"
    "  context: c0ffee43\n"
    "  arch: x86_64\n"
    "  summary: An example module\n"
    "  description: >-\n"
    "    A module for the demonstration of the metadata format. Also, the "
    "obligatory lorem\n"
    "    ipsum dolor sit amet goes right here.\n"
    "  servicelevels:\n"
    "    bug_fixes:\n"
    "      eol: 2077-10-23\n"
    "    rawhide:\n"
    "      eol: 2077-10-23\n"
    "    security_fixes:\n"
    "      eol: 2077-10-23\n"
    "    stable_api:\n"
    "      eol: 2077-10-23\n"
    "  license:\n"
    "    module:\n"
    "    - MIT\n"
    "    content:\n"
    "    - Beerware\n"
    "    - GPLv2+\n"
    "    - zlib\n"
    "  xmd:\n"
    "    some_key: some_data\n"
    "  dependencies:\n"
    "  - buildrequires:\n"
    "      platform: [-epel7, -f27, -f28]\n"
    "    requires:\n"
    "      platform: [-epel7, -f27, -f28]\n"
    "  - buildrequires:\n"
    "      buildtools: [v1, v2]\n"
    "      compatible: [v3]\n"
    "      platform: [f27]\n"
    "    requires:\n"
    "      compatible: [v3, v4]\n"
    "      platform: [f27]\n"
    "  - buildrequires:\n"
    "      platform: [f28]\n"
    "    requires:\n"
    "      platform: [f28]\n"
    "      runtime: [a, b]\n"
    "  - buildrequires:\n"
    "      extras: []\n"
    "      moreextras: [bar, foo]\n"
    "      platform: [epel7]\n"
    "    requires:\n"
    "      extras: []\n"
    "      moreextras: [bar, foo]\n"
    "      platform: [epel7]\n"
    "  references:\n"
    "    community: http://www.example.com/\n"
    "    documentation: http://www.example.com/\n"
    "    tracker: http://www.example.com/\n"
    "  profiles:\n"
    "    buildroot:\n"
    "      rpms:\n"
    "      - bar-devel\n"
    "    container:\n"
    "      rpms:\n"
    "      - bar\n"
    "      - bar-devel\n"
    "    default:\n"
    "      rpms:\n"
    "      - bar\n"
    "      - bar-extras\n"
    "      - baz\n"
    "    minimal:\n"
    "      description: Minimal profile installing only the bar package.\n"
    "      rpms:\n"
    "      - bar\n"
    "    srpm-buildroot:\n"
    "      rpms:\n"
    "      - bar-extras\n"
    "  api:\n"
    "    rpms:\n"
    "    - bar\n"
    "    - bar-devel\n"
    "    - bar-extras\n"
    "    - baz\n"
    "    - xxx\n"
    "  filter:\n"
    "    rpms:\n"
    "    - baz-nonfoo\n"
    "  buildopts:\n"
    "    rpms:\n"
    "      macros: >\n"
    "        %demomacro 1\n"
    "\n"
    "        %demomacro2 %{demomacro}23\n"
    "      whitelist:\n"
    "      - fooscl-1-bar\n"
    "      - fooscl-1-baz\n"
    "      - xxx\n"
    "      - xyz\n"
    "  components:\n"
    "    rpms:\n"
    "      bar:\n"
    "        rationale: We need this to demonstrate stuff.\n"
    "        repository: https://pagure.io/bar.git\n"
    "        cache: https://example.com/cache\n"
    "        ref: 26ca0c0\n"
    "      baz:\n"
    "        rationale: This one is here to demonstrate other stuff.\n"
    "      xxx:\n"
    "        rationale: xxx demonstrates arches and multilib.\n"
    "        arches: [i686, x86_64]\n"
    "        multilib: [x86_64]\n"
    "      xyz:\n"
    "        rationale: xyz is a bundled dependency of xxx.\n"
    "        buildorder: 10\n"
    "    modules:\n"
    "      includedmodule:\n"
    "        rationale: Included in the stack, just because.\n"
    "        repository: https://pagure.io/includedmodule.git\n"
    "        ref: somecoolbranchname\n"
    "        buildorder: 100\n"
    "  artifacts:\n"
    "    rpms:\n"
    "    - bar-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-devel-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-extras-0:1.23-1.module_deadbeef.x86_64\n"
    "    - baz-0:42-42.module_deadbeef.x86_64\n"
    "    - xxx-0:1-1.module_deadbeef.i686\n"
    "    - xxx-0:1-1.module_deadbeef.x86_64\n"
    "    - xyz-0:1-1.module_deadbeef.x86_64\n"
    "...\n");
}

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.o

  g_test_add ("/modulemd/v2/modulestream/construct",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_test_construct,
              NULL);

  g_test_add ("/modulemd/v2/modulestream/copy",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_test_copy,
              NULL);

  g_test_add ("/modulemd/v2/modulestream/v1/parse_dump",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_v1_test_parse_dump,
              NULL);

  g_test_add ("/modulemd/v2/modulestream/v2/parse_dump",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_v2_test_parse_dump,
              NULL);

  return g_test_run ();
}
