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

#include "modulemd-compression.h"
#include "private/modulemd-compression-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"


/* == Public Functions == */
static void
test_modulemd_compression_type (void)
{
  g_assert_cmpint (modulemd_compression_type ("gz"),
                   ==,
                   MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION);


  g_assert_cmpint (modulemd_compression_type ("gzip"),
                   ==,
                   MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION);

  g_assert_cmpint (modulemd_compression_type ("bz2"),
                   ==,
                   MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION);

  g_assert_cmpint (modulemd_compression_type ("bzip2"),
                   ==,
                   MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION);

  g_assert_cmpint (modulemd_compression_type ("xz"),
                   ==,
                   MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION);

  g_assert_cmpint (modulemd_compression_type ("garbage"),
                   ==,
                   MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION);

  g_assert_cmpint (modulemd_compression_type (NULL),
                   ==,
                   MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION);
}


struct expected_compression_t
{
  const char *filename;
  ModulemdCompressionTypeEnum type;
};

/* == Private Functions == */
static void
test_modulemd_detect_compression (void)
{
  int fd;
  g_autofree gchar *filename = NULL;
  g_autoptr (FILE) filestream = NULL;
  g_autoptr (GError) error = NULL;
  ModulemdCompressionTypeEnum result;

  struct expected_compression_t expected[] = {
    { .filename = "bzipped.yaml.bz2",
      .type = MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION },
    { .filename = "gzipped.yaml.gz",
      .type = MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION },
    { .filename = "xzipped.yaml.xz",
      .type = MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION },
    { .filename = "uncompressed.yaml",
      .type = MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION },
    { .filename = "empty",
      .type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION },
    { .filename = NULL }
  };

  /* == Detect by file extension == */
  for (size_t i = 0; expected[i].filename; i++)
    {
      filename = g_strdup_printf ("%s/compression/%s",
                                  g_getenv ("TEST_DATA_PATH"),
                                  expected[i].filename);
      g_debug ("Getting compression type for %s", filename);
      filestream = g_fopen (filename, "rbe");
      g_assert_nonnull (filestream);
      fd = fileno (filestream);
      result = modulemd_detect_compression (filename, fd, &error);
      g_assert_no_error (error);
      g_assert_cmpint (result, ==, expected[i].type);
      g_clear_error (&error);
      g_clear_pointer (&filestream, fclose);
      g_clear_pointer (&filename, g_free);
    }


    /* == Test detection by libmagic == */
#ifdef HAVE_LIBMAGIC
  struct expected_compression_t expected_magic[] = {
    { .filename = "bzipped",
      .type = MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION },
    { .filename = "gzipped",
      .type = MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION },
    { .filename = "xzipped",
      .type = MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION },
    { .filename = "uncompressed",
      .type = MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION },
    { .filename = "empty",
      .type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION },
    { .filename = NULL }
  };
#else
  struct expected_compression_t expected_magic[] = {
    { .filename = "bzipped",
      .type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION },
    { .filename = "gzipped",
      .type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION },
    { .filename = "xzipped",
      .type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION },
    { .filename = "uncompressed",
      .type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION },
    { .filename = "empty",
      .type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION },
    { .filename = NULL }
  };
#endif

  for (size_t j = 0; expected_magic[j].filename; j++)
    {
      filename = g_strdup_printf ("%s/compression/%s",
                                  g_getenv ("TEST_DATA_PATH"),
                                  expected_magic[j].filename);
      filestream = g_fopen (filename, "rbe");
      g_assert_nonnull (filestream);
      fd = fileno (filestream);
      g_assert_cmpint (modulemd_detect_compression (filename, fd, &error),
                       ==,
                       expected_magic[j].type);
      g_assert_no_error (error);
      g_clear_error (&error);
      g_clear_pointer (&filestream, fclose);
      g_clear_pointer (&filename, g_free);
    }
}


struct expected_suffix_t
{
  ModulemdCompressionTypeEnum type;
  const char *suffix;
};

static void
test_modulemd_compression_suffix (void)
{
  struct expected_suffix_t expected[] = {
    { .type = MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED, .suffix = NULL },
    { .type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION, .suffix = NULL },
    { .type = MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION, .suffix = NULL },
    { .type = MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION, .suffix = ".gz" },
    { .type = MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION, .suffix = ".bz2" },
    { .type = MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION, .suffix = ".xz" },
    { .type = MODULEMD_COMPRESSION_TYPE_SENTINEL, .suffix = NULL }
  };

  for (size_t i = 0; expected[i].type != MODULEMD_COMPRESSION_TYPE_SENTINEL;
       i++)
    {
      g_assert_cmpstr (modulemd_compression_suffix (expected[i].type),
                       ==,
                       expected[i].suffix);
    }
}


struct expected_fmode_t
{
  ModulemdCompressionTypeEnum type;
  const gchar *suffix;
};

static void
test_modulemd_get_rpmio_fmode (void)
{
  struct expected_fmode_t expected[] = {
    { .type = MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED, .suffix = NULL },
    { .type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION, .suffix = NULL },
    { .type = MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION, .suffix = "fdio" },
    { .type = MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION, .suffix = "gzdio" },
    { .type = MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION, .suffix = "bzdio" },
    { .type = MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION, .suffix = "xzdio" },
    { .type = MODULEMD_COMPRESSION_TYPE_SENTINEL, .suffix = NULL }
  };

  const char *mode[] = { "r", "w", "a", "r+", "w+", "a+", NULL };
  g_autofree gchar *fmode = NULL;
  g_autofree gchar *expected_fmode = NULL;

  for (size_t i = 0; expected[i].type != MODULEMD_COMPRESSION_TYPE_SENTINEL;
       i++)
    {
      for (size_t j = 0; mode[j]; j++)
        {
          fmode = modulemd_get_rpmio_fmode (mode[j], expected[i].type);
          if (expected[i].suffix == NULL)
            {
              g_assert_null (fmode);
              g_clear_pointer (&fmode, g_free);
              continue;
            }

          expected_fmode =
            g_strdup_printf ("%s.%s", mode[j], expected[i].suffix);
          g_assert_cmpstr (fmode, ==, expected_fmode);

          g_clear_pointer (&expected_fmode, g_free);
          g_clear_pointer (&fmode, g_free);
        }
    }
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add_func ("/modulemd/compression/type",
                   test_modulemd_compression_type);

  g_test_add_func ("/modulemd/compression/detect",
                   test_modulemd_detect_compression);

  g_test_add_func ("/modulemd/compression/suffix",
                   test_modulemd_compression_suffix);

  g_test_add_func ("/modulemd/compression/rmpio/fmode",
                   test_modulemd_get_rpmio_fmode);

  return g_test_run ();
}
