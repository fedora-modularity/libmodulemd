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

#pragma once

#include <glib.h>

#include "config.h"
#include "modulemd-compression.h"

#ifdef HAVE_RPMIO
#include <rpm/rpmio.h>
#endif

/**
 * SECTION: modulemd-compression-private
 * @title: Modulemd Compression Helpers (Private)
 * @stability: Private
 * @short_description: Internal utility functions for working with compressed
 * files.
 */


#ifdef HAVE_RPMIO
/**
 * mmd_Fclose:
 * @fd: (in): A FD_t from rpmio.h to be closed
 *
 * Wrapper for rpmio's Fclose() function to use with
 * #G_DEFINE_AUTO_CLEANUP_FREE_FUNC()
 *
 * Since: 2.8
 */
void
mmd_Fclose (FD_t fd);

G_DEFINE_AUTO_CLEANUP_FREE_FUNC (FD_t, mmd_Fclose, NULL);
#endif


/**
 * modulemd_detect_compression:
 * @filename: (in): The original filename that matches @fd.
 * @fd: (in): An open file descriptor pointing at a real file.
 * @error: (out): A #GError containing the reason this function failed.
 *
 * Returns: The #ModulemdCompressionTypeEnum detected from this file
 * descriptor. In the event of an error, returns
 * #MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED and sets @error
 * appropriately. Returns #MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION if
 * all detection methods complete but the type is still indeterminate.
 *
 * Since: 2.8
 */
ModulemdCompressionTypeEnum
modulemd_detect_compression (const gchar *filename, int fd, GError **error);


/**
 * modulemd_compression_suffix:
 * @comtype: (in): A #ModulemdCompressionTypeEnum.
 *
 * Returns: (transfer none): A static string representing the filename suffix
 * that a file of this compression type should have.
 *
 * Since: 2.8
 */
const gchar *
modulemd_compression_suffix (ModulemdCompressionTypeEnum comtype);


/**
 * modulemd_get_rpmio_fmode:
 * @mode: (in): A mode argument that will be passed to `fopen(3)`.
 * @comtype: (in): A #ModulemdCompressionTypeEnum.
 *
 * Returns: (transfer full): A string suitable for passing to rpmio's
 * `Fopen()` function. NULL if @mode is NULL or the @comtype is invalid.
 *
 * Since: 2.8
 */
gchar *
modulemd_get_rpmio_fmode (const gchar *mode,
                          ModulemdCompressionTypeEnum comtype);


/**
 * compressed_stream_read_fn:
 * @data: (inout): A private pointer to the data being read.
 * @buffer: (out): The buffer to write the data from the source.
 * @size: (in): The size of the buffer.
 * @size_read: (out): The actual number of bytes read from the source.
 *
 * A #ModulemdReadHandler that uses rpmio's `Fread()` function to handle
 * compressed files.
 *
 * Since: 2.8
 */
gint
compressed_stream_read_fn (void *data,
                           unsigned char *buffer,
                           size_t size,
                           size_t *size_read);
