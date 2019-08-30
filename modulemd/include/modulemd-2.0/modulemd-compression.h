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

#pragma once

#include <glib.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-compression
 * @title: Modulemd Compression Helpers
 * @stability: stable
 * @short_description: Utility functions for working with compressed files.
 */


/**
 * ModulemdCompressionTypeEnum:
 * @MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED: Autodetection failure
 * @MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION: Unknown compression
 * @MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION: No compression
 * @MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION: gzip compression
 * @MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION: bzip2 compression
 * @MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION: LZMA compression
 * @MODULEMD_COMPRESSION_TYPE_ZCK_COMPRESSION: zchunk compression
 * @MODULEMD_COMPRESSION_TYPE_SENTINEL: Enum list terminator
 *
 * Since: 2.8
 */
typedef enum
{
  MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED = -2,
  MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION,
  MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION,
  MODULEMD_COMPRESSION_TYPE_GZ_COMPRESSION,
  MODULEMD_COMPRESSION_TYPE_BZ2_COMPRESSION,
  MODULEMD_COMPRESSION_TYPE_XZ_COMPRESSION,
  MODULEMD_COMPRESSION_TYPE_ZCK_COMPRESSION,
  MODULEMD_COMPRESSION_TYPE_SENTINEL,
} ModulemdCompressionTypeEnum;


/**
 * modulemd_compression_type:
 * @name: (in): The name of the compression type. Valid options are:
 * "gz", "gzip", "bz2", "bzip2", "xz" and "zck".
 *
 * Returns: The #ModulemdCompressionTypeEnum value corresponding to the
 * provided string if available or
 * #MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION if the string does not match
 * a known type.
 *
 * Since: 2.8
 */
ModulemdCompressionTypeEnum
modulemd_compression_type (const gchar *name);


G_END_DECLS
