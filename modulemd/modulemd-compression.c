/*
 * This file is part of libmodulemd
 * Copyright (C) 2019 Red Hat, Inc.
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <inttypes.h>
#include <unistd.h>


#ifdef HAVE_RPMIO
#include <rpm/rpmio.h>
#endif

#include "modulemd-compression.h"
#include "modulemd-errors.h"

#include "private/modulemd-compression-private.h"
#include "private/modulemd-util.h"


ModulemdCompressionTypeEnum
modulemd_detect_compression (const gchar *filename, int fd, GError **error)
{
  g_return_val_if_fail (filename, MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED);
  g_return_val_if_fail (!error || *error == NULL,
                        MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED);

  if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR))
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_FILE_ACCESS,
                   "File %s does not exist or is not a regular file",
                   filename);
      return MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED;
    }


  /* If the filename has a known suffix, assume it is accurate */
  if (g_str_has_suffix (filename, ".gz") ||
      g_str_has_suffix (filename, ".gzip") ||
      g_str_has_suffix (filename, ".gunzip"))
    {
      return MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION;
    }
  if (g_str_has_suffix (filename, ".bz2") ||
      g_str_has_suffix (filename, ".bzip2"))
    {
      return MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION;
    }
  if (g_str_has_suffix (filename, ".xz"))
    {
      return MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION;
    }
  if (g_str_has_suffix (filename, ".zst"))
    {
      return MODULEMD_COMPRESSION_TYPE_ZSTD_COMPRESSION;
    }
  if (g_str_has_suffix (filename, ".yaml") ||
      g_str_has_suffix (filename, ".yml") ||
      g_str_has_suffix (filename, ".txt"))
    {
      return MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION;
    }

  /* No known suffix? Inspect magic bytes in the content. */
  unsigned char buffer[6]; /* gzip, bzip2, zstd have a 4-byte header.
                                    xz has a 6-byte header. */
  size_t filled = 0;
  ssize_t retval = 0;
  do
    {
      retval = read (fd, buffer + filled, sizeof (buffer) - filled);
      if (retval == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
          g_set_error (error,
                       MODULEMD_ERROR,
                       MMD_ERROR_MAGIC,
                       "Could not read from file %s: %s",
                       filename,
                       g_strerror (errno));
          lseek (fd, 0, SEEK_SET);
          return MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED;
        }
      else if (retval > 0)
        filled += retval;
    }
  while (retval != 0 && filled < sizeof (buffer));

  /* Reset the file descriptor to the start of the file */
  if ((off_t)-1 == lseek (fd, 0, SEEK_SET))
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_MAGIC,
                   "Could not reset a position in %s file: %s",
                   filename,
                   g_strerror (errno));
      return MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED;
    }

  /* Classify files shorter than the buffer as a plain text */
  if (filled < sizeof (buffer))
    {
      g_debug ("%s: File %s is too short (%zu B) to be compressed",
               __func__,
               filename,
               filled);
      return MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION;
    }

  /* Now inspect the file content */
  if (buffer[0] == 0x1f && buffer[1] == 0x8b)
    /* RFC 1952. */
    return MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION;
  if (buffer[0] == 'B' && buffer[1] == 'Z' && buffer[2] == 'h')
    /* bzip2 and libbzip2, version 1.0.8: A program and library for data compression. */
    return MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION;
  if (buffer[0] == 0xfd && buffer[1] == '7' && buffer[2] == 'z' &&
      buffer[3] == 'X' && buffer[4] == 'Z' && buffer[5] == 0x00)
    /* The .xz File Format, Version 1.1.0 (2022-12-11). */
    return MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION;
  if (/* Zstandard frame */
      (buffer[0] == 0x28 && buffer[1] == 0xb5 && buffer[2] == 0x2f &&
       buffer[3] == 0xfd) ||
      /* Skippable frame */
      ((buffer[0] & 0xf0) == 0x50 && buffer[1] == 0x2a && buffer[2] == 0x4d &&
       buffer[3] == 0x18))
    /* RFC 8878. */
    return MODULEMD_COMPRESSION_TYPE_ZSTD_COMPRESSION;

  /* Fall back to no compression. YAML parser will error later on a binary
   * garbage. YAML 1.2.2 requires to support UTF-8, UTF-16, UTF-32, with and
   * without a byte-order mark. There is no reliable detection except of
   * reading the complete file and validating the UTF encoding. */
  return MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION;
}

ModulemdCompressionTypeEnum
modulemd_compression_type (const gchar *name)
{
  if (!name)
    {
      return MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION;
    }

  int type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION;

  if (!g_strcmp0 (name, "gz") || !g_strcmp0 (name, "gzip") ||
      !g_strcmp0 (name, "gunzip"))
    {
      type = MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION;
    }
  if (!g_strcmp0 (name, "bz2") || !g_strcmp0 (name, "bzip2"))
    {
      type = MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION;
    }
  if (!g_strcmp0 (name, "xz"))
    {
      type = MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION;
    }
  if (!g_strcmp0 (name, "zck"))
    {
      type = MODULEMD_COMPRESSION_TYPE_ZCK_COMPRESSION;
    }
  if (!g_strcmp0 (name, "zstd"))
    {
      type = MODULEMD_COMPRESSION_TYPE_ZSTD_COMPRESSION;
    }

  return type;
}

const gchar *
modulemd_compression_suffix (ModulemdCompressionTypeEnum comtype)
{
  switch (comtype)
    {
    case MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION: return ".gz";
    case MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION: return ".bz2";
    case MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION: return ".xz";
    case MODULEMD_COMPRESSION_TYPE_ZSTD_COMPRESSION: return ".zst";
    default: return NULL;
    }
}


static const gchar *
get_comtype_string (ModulemdCompressionTypeEnum comtype)
{
  /* see rpmio/rpmio.c in the RPM sources for the origin of these
   * magic strings.
   */
  switch (comtype)
    {
    case MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION: return "fdio"; break;

    case MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION: return "gzdio"; break;

    case MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION: return "bzdio"; break;

    case MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION: return "xzdio"; break;

    case MODULEMD_COMPRESSION_TYPE_ZSTD_COMPRESSION: return "zstdio"; break;

    default:
      g_info ("Unknown compression type: %d", comtype);
      return NULL;
      break;
    }
}


gchar *
modulemd_get_rpmio_fmode (const gchar *mode,
                          ModulemdCompressionTypeEnum comtype)
{
  const gchar *type_string;

  if (!mode)
    {
      return NULL;
    }

  type_string = get_comtype_string (comtype);

  if (type_string == NULL)
    {
      return NULL;
    }

  return g_strdup_printf ("%s.%s", mode, type_string);
}


#ifdef HAVE_RPMIO
gint
compressed_stream_read_fn (void *data,
                           unsigned char *buffer,
                           size_t size,
                           size_t *size_read)
{
  FD_t rpmio_fd = (FD_t)data;
  ssize_t read = Fread (buffer, sizeof (*buffer), size, rpmio_fd);

  if (read < 0)
    {
      g_warning ("Got error [%d] reading the file", Ferror (rpmio_fd));
      return 0;
    }

  *size_read = read;

  return 1;
}
#else
gint
compressed_stream_read_fn (void *data,
                           unsigned char *buffer,
                           size_t size,
                           size_t *size_read)
{
  /* Not implemented without librpm available */
  return 0;
}
#endif


#ifdef HAVE_RPMIO
void
mmd_Fclose (FD_t fd)
{
  Fclose (fd);
}

#endif
