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
#include <glib.h>
#include <inttypes.h>
#include <unistd.h>


#ifdef HAVE_RPMIO
#include <rpm/rpmio.h>
#endif

#include "modulemd-compression.h"
#include "modulemd-errors.h"

#include "private/modulemd-util.h"
#include "private/modulemd-compression-private.h"

#ifdef HAVE_LIBMAGIC
#include <magic.h>
G_DEFINE_AUTO_CLEANUP_FREE_FUNC (magic_t, magic_close, NULL)
#endif


ModulemdCompressionTypeEnum
modulemd_detect_compression (const gchar *filename, int fd, GError **error)
{
  ModulemdCompressionTypeEnum type =
    MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION;

  g_return_val_if_fail (filename, MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED);
  g_return_val_if_fail (!error || *error == NULL,
                        MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED);

  if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR))
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_FILE_ACCESS,
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
  else if (g_str_has_suffix (filename, ".bz2") ||
           g_str_has_suffix (filename, ".bzip2"))
    {
      return MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION;
    }
  else if (g_str_has_suffix (filename, ".xz"))
    {
      return MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION;
    }
  else if (g_str_has_suffix (filename, ".yaml") ||
           g_str_has_suffix (filename, ".yml") ||
           g_str_has_suffix (filename, ".txt"))
    {
      return MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION;
    }

#ifdef HAVE_LIBMAGIC
  /* No known suffix? Try using libmagic from file-utils */
  const char *mime_type;
  g_auto (magic_t) magic = NULL;
  int magic_fd = dup (fd);
  if (magic_fd < 0)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_MAGIC,
                   "Could not dup() the file descriptor");
      return MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED;
    }

  magic = magic_open (MAGIC_MIME);
  if (magic == NULL)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_MAGIC,
                   "magic_open() failed: Cannot allocate the magic cookie");
      return MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED;
    }

  if (magic_load (magic, NULL) == -1)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_MAGIC,
                   "magic_load() failed: %s",
                   magic_error (magic));
      return MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED;
    }

  mime_type = magic_descriptor (magic, magic_fd);
  close (magic_fd);
  /* Reset the file descriptor to the start of the file, if it has moved */
  lseek (fd, 0, SEEK_SET);

  if (mime_type)
    {
      g_debug (
        "%s: Detected mime type: %s (%s)", __func__, mime_type, filename);

      if (g_str_has_prefix (mime_type, "application/x-gzip") ||
          g_str_has_prefix (mime_type, "application/gzip") ||
          g_str_has_prefix (mime_type, "application/gzip-compressed") ||
          g_str_has_prefix (mime_type, "application/gzipped") ||
          g_str_has_prefix (mime_type, "application/x-gzip-compressed") ||
          g_str_has_prefix (mime_type, "application/x-compress") ||
          g_str_has_prefix (mime_type, "application/x-gzip") ||
          g_str_has_prefix (mime_type, "application/x-gunzip") ||
          g_str_has_prefix (mime_type, "multipart/x-gzip"))
        {
          type = MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION;
        }

      else if (g_str_has_prefix (mime_type, "application/x-bzip2") ||
               g_str_has_prefix (mime_type, "application/x-bz2") ||
               g_str_has_prefix (mime_type, "application/bzip2") ||
               g_str_has_prefix (mime_type, "application/bz2"))
        {
          type = MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION;
        }

      else if (g_str_has_prefix (mime_type, "application/x-xz"))
        {
          type = MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION;
        }

      else if (g_str_has_prefix (mime_type, "text/plain") ||
               g_str_has_prefix (mime_type, "text/x-yaml") ||
               g_str_has_prefix (mime_type, "application/x-yaml"))
        {
          type = MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION;
        }
    }
  else
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_MAGIC,
                   "mime_type() detection failed: %s",
                   magic_error (magic));
      return MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED;
    }
  /* Reset the file descriptor to the start of the file, if it has moved */
  lseek (fd, 0, SEEK_SET);
#endif /* HAVE_LIBMAGIC */

  return type;
}

ModulemdCompressionTypeEnum
modulemd_compression_type (const gchar *name)
{
  if (!name)
    return MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION;

  int type = MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION;

  if (!g_strcmp0 (name, "gz") || !g_strcmp0 (name, "gzip") ||
      !g_strcmp0 (name, "gunzip"))
    type = MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION;
  if (!g_strcmp0 (name, "bz2") || !g_strcmp0 (name, "bzip2"))
    type = MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION;
  if (!g_strcmp0 (name, "xz"))
    type = MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION;
  if (!g_strcmp0 (name, "zck"))
    type = MODULEMD_COMPRESSION_TYPE_ZCK_COMPRESSION;

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
    return NULL;

  type_string = get_comtype_string (comtype);

  if (type_string == NULL)
    return NULL;

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

  *size_read = Fread (buffer, sizeof (*buffer), size, rpmio_fd);

  if (size_read < 0)
    {
      g_warning ("Got error [%d] reading the file", Ferror (rpmio_fd));
      return 0;
    }

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
