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
#include "modulemd-deprecated.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-errors
 * @title: Modulemd Errors
 * @stability: stable
 * @short_description: Error codes for libmodulemd.
 */


/**
 * MODULEMD_ERROR:
 *
 * A convenience macro for identifying an error in the general modulemd domain.
 *
 * Since: 2.9
 */
#define MODULEMD_ERROR modulemd_error_quark ()

/**
 * modulemd_error_quark:
 *
 * Returns: A #GQuark used to identify an error in the general modulemd domain.
 *
 * Since: 2.9
 */
GQuark
modulemd_error_quark (void);


/**
 * ModulemdError:
 * @MMD_ERROR_UPGRADE: Represents an error encountered while upgrading the
 * metadata version of a module stream or module defaults.
 * @MMD_ERROR_VALIDATE: Represents an error encountered while validating
 * module metadata.
 * @MMD_ERROR_FILE_ACCESS: Represents an error encountered when attempting
 * to access a file.
 * @MMD_ERROR_NO_MATCHES: Represents an error indicating that no streams
 * matched when searching for a specific module stream.
 * @MMD_ERROR_TOO_MANY_MATCHES: Represents an error indicating that
 * multiple streams matched when searching for a specific module
 * stream.
 * @MMD_ERROR_MAGIC: Could not detect the mime type of a file for
 * automatic detection of compression format.
 * @MMD_ERROR_NOT_IMPLEMENTED: The requested function is not implemented
 * on this platform, likely due to needing a newer version of a dependency
 * library.
 * @MMD_ERROR_MISSING_REQUIRED: The object is missing some data necessary
 * for proper operation.
 *
 * Since: 2.9
 */
typedef enum
{
  MMD_ERROR_UPGRADE,
  MMD_ERROR_VALIDATE,
  MMD_ERROR_FILE_ACCESS,
  MMD_ERROR_NO_MATCHES,
  MMD_ERROR_TOO_MANY_MATCHES,
  MMD_ERROR_MAGIC,
  MMD_ERROR_NOT_IMPLEMENTED,
  MMD_ERROR_MISSING_REQUIRED
} ModulemdError;


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
 *
 * Since: 2.0
 * Deprecated: 2.9
 * Use #ModulemdError instead.
 */
MMD_DEPRECATED_TYPE_FOR (ModulemdError)
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
 * MODULEMD_YAML_ERROR:
 *
 * A convenience macro for identifying an error in the modulemd yaml domain.
 *
 * Since: 2.9
 */
#define MODULEMD_YAML_ERROR modulemd_yaml_error_quark ()

/**
 * modulemd_yaml_error_quark:
 *
 * Returns: A #GQuark used to identify an error in the modulemd yaml domain.
 *
 * Since: 2.9
 */
GQuark
modulemd_yaml_error_quark (void);


/**
 * ModulemdYamlError:
 * @MMD_YAML_ERROR_OPEN: Represents an error encountered while opening a
 * YAML file.
 * @MMD_YAML_ERROR_PROGRAMMING: Represents an internal programming error
 * encountered while parsing a YAML document.
 * @MMD_YAML_ERROR_UNPARSEABLE: Represents an error indicating that
 * unexpected data or some other parsing error that violates the YAML spec
 * was encountered while parsing a YAML document.
 * @MMD_YAML_ERROR_PARSE: Represents an error indicating invalid data
 * violating the modulemd YAML spec was encountered while parsing a YAML
 * document.
 * @MMD_YAML_ERROR_EMIT: Represents an error encountered while writing a
 * YAML file.
 * @MMD_YAML_ERROR_MISSING_REQUIRED: Represents an error indicating that
 * required elements are missing while parsing a YAML document.
 * @MMD_YAML_ERROR_EVENT_INIT: Represents an error indicating that a YAML
 * output event could not be initialized.
 * @MMD_YAML_ERROR_INCONSISTENT: Represents a data inconsistency error
 * encountered while parsing a YAML document.
 * @MMD_YAML_ERROR_UNKNOWN_ATTRS: While parsing a document in strict mode, an
 * attribute was encountered that does not belong in this document.
 *
 * Since: 2.9
 */
typedef enum
{
  MMD_YAML_ERROR_OPEN,
  MMD_YAML_ERROR_PROGRAMMING,
  MMD_YAML_ERROR_UNPARSEABLE,
  MMD_YAML_ERROR_PARSE,
  MMD_YAML_ERROR_EMIT,
  MMD_YAML_ERROR_MISSING_REQUIRED,
  MMD_YAML_ERROR_EVENT_INIT,
  MMD_YAML_ERROR_INCONSISTENT,
  MMD_YAML_ERROR_UNKNOWN_ATTR
} ModulemdYamlError;


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
 * Deprecated: 2.9
 * Use #ModulemdYamlError instead.
 */
MMD_DEPRECATED_TYPE_FOR (ModulemdYamlError)
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
