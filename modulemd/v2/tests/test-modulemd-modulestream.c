#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <signal.h>

#include "modulemd-module-stream.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-stream-private.h"
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

  return g_test_run ();
}
