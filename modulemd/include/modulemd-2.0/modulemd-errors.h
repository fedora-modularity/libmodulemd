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
 * SECTION: modulemd-errors
 * @title: Modulemd Errors
 * @stability: stable
 * @short_description: Error codes for libmodulemd.
 */


/**
 * ModulemdErrorEnum:
 * @MODULEMD_ERROR_UPGRADE: Represents an error encountered while upgrading the
 * metadata version of a module stream or module defaults.
 * @MODULEMD_ERROR_VALIDATE: Represents an error encountered while validating
 * module metadata.
 * @MODULEMD_ERROR_FILE_ACCESS: Represents an error encountered when attempting
 * to access a file.
 * @MODULEMD_ERROR_NO_MATCHES: Represents an error indicating that no streams
 * matched when searching for a specific module stream. Since: 2.2
 * @MODULEMD_ERROR_TOO_MANY_MATCHES: Represents an error indicating that
 * multiple streams matched when searching for a specific module
 * stream. Since: 2.2
 * @MODULEMD_ERROR_MAGIC: Could not detect the mime type of a file for
 * automatic detection of compression format. Since: 2.8
 * @MODULEMD_ERROR_NOT_IMPLEMENTED: The requested function is not implemented
 * on this platform, likely due to needing a newer version of a dependency
 * library. Since: 2.8
 * @MODULEMD_ERROR_MISSING_REQUIRED: The object is missing some data necessary
 * for proper operation. Since: 2.9
 *
 * Since: 2.0
 */
typedef enum
{
  MODULEMD_ERROR_UPGRADE,
  MODULEMD_ERROR_VALIDATE,
  MODULEMD_ERROR_FILE_ACCESS,
  MODULEMD_ERROR_NO_MATCHES,
  MODULEMD_ERROR_TOO_MANY_MATCHES,
  MODULEMD_ERROR_MAGIC,
  MODULEMD_ERROR_NOT_IMPLEMENTED
} ModulemdErrorEnum;

/**
 * ModulemdYamlErrorEnum:
 * @MODULEMD_YAML_ERROR_OPEN: Represents an error encountered while opening a
 * YAML file.
 * @MODULEMD_YAML_ERROR_PROGRAMMING: Represents an internal programming error
 * encountered while parsing a YAML document.
 * @MODULEMD_YAML_ERROR_UNPARSEABLE: Represents an error indicating that
 * unexpected data or some other parsing error was encountered while parsing a
 * YAML document.
 * @MODULEMD_YAML_ERROR_PARSE: Represents an error indicating invalid data was
 * encountered while parsing a YAML document.
 * @MODULEMD_YAML_ERROR_EMIT: Represents an error encountered while writing a
 * YAML file.
 * @MODULEMD_YAML_ERROR_MISSING_REQUIRED: Represents an error indicating that
 * required elements are missing while parsing a YAML document.
 * @MODULEMD_YAML_ERROR_EVENT_INIT: Represents an error indicating that a YAML
 * output event could not be initialized.
 * @MODULEMD_YAML_ERROR_INCONSISTENT: Represents a data inconsistency error
 * encountered while parsing a YAML document.
 *
 * Since: 2.0
 */
typedef enum
{
  MODULEMD_YAML_ERROR_OPEN,
  MODULEMD_YAML_ERROR_PROGRAMMING,
  MODULEMD_YAML_ERROR_UNPARSEABLE,
  MODULEMD_YAML_ERROR_PARSE,
  MODULEMD_YAML_ERROR_EMIT,
  MODULEMD_YAML_ERROR_MISSING_REQUIRED,
  MODULEMD_YAML_ERROR_EVENT_INIT,
  MODULEMD_YAML_ERROR_INCONSISTENT
} ModulemdYamlErrorEnum;

G_END_DECLS
