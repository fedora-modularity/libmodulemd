/*
* This file is part of libmodulemd
* Copyright (C) 2020 Red Hat, Inc.
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

#include <glib-object.h>

G_BEGIN_DECLS

/**
* SECTION: modulemd-obsoletes
* @title: Modulemd.Obsoletes
* @stability: stable
* @short_description: Obsoletes information for a module stream.
*/

#define MODULEMD_TYPE_OBSOLETES (modulemd_obsoletes_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdObsoletes, modulemd_obsoletes, MODULEMD, OBSOLETES, GObject)

/**
 * ModulemdObsoletesVersionEnum:
 * @MD_OBSOLETES_VERSION_ERROR: Represents an error handling mdversion.
 * @MD_OBSOLETES_VERSION_UNSET: Represents an unset mdversion.
 * @MD_OBSOLETES_VERSION_ONE: Represents v1 of the #ModulemdObsoletes metadata
 * format.
 * @MD_OBSOLETES_VERSION_LATEST: Represents the highest-supported version of the
 * #ModulemdObsoletes metadata format.
 *
 * Since: 2.10
 */
typedef enum
{
  MD_OBSOLETES_VERSION_ERROR = -1,

  MD_OBSOLETES_VERSION_UNSET = 0,

  MD_OBSOLETES_VERSION_ONE = 1,

  MD_OBSOLETES_VERSION_LATEST = MD_OBSOLETES_VERSION_ONE,
} ModulemdObsoletesVersionEnum;

/**
 * modulemd_obsoletes_new:
 * @mdversion: (in): The metadata version of this #ModulemdObsoletes.
 * @modified: (in): The last modified time represented as a 64-bit integer (such as
 * 201807011200).
 * @module_name: (in): The name of the module to which this obsoletes applies.
 * @module_stream: (in): The name of the module stream to which this obsoletes applies.
 * @message: (in): A string describing the change, reason, etc.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdObsoletes object.
 * This object must be freed with g_object_unref().
 *
 * Since: 2.10
 */
ModulemdObsoletes *
modulemd_obsoletes_new (guint64 mdversion,
                        guint64 modified,
                        const gchar *module_name,
                        const gchar *module_stream,
                        const gchar *message);

/**
 * modulemd_obsoletes_copy:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Create a copy of this #ModulemdObsoletes object.
 *
 * Returns: (transfer full): The copied #ModulemdObsoletes object.
 *
 * Since: 2.10
 */
ModulemdObsoletes *
modulemd_obsoletes_copy (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_validate:
 * @self: (in): This #ModulemdObsoletes object.
 * @error: (out): If the object is not valid, it will return the reason.
 *
 * This method ensures that the obsoletes is internally consistent for usage
 * or dumping to YAML. It will be run implicitly prior to emitting YAML. This
 * is not a complete linter, merely a sanity check that the values are not
 * impossible.
 *
 * Since: 2.10
 */
gboolean
modulemd_obsoletes_validate (ModulemdObsoletes *self, GError **error);

/**
 * modulemd_obsoletes_get_module_context:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: (transfer none): The context name to which this #ModulemdObsoletes object applies.
 *
 * Since: 2.10
 */
const gchar *
modulemd_obsoletes_get_module_context (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_set_module_context:
 * @self: This #ModulemdObsoletes object.
 * @module_context: (in) (nullable): The name of the module context to which this obsoletes
 * applies.
 *
 * Since: 2.10
 */
void
modulemd_obsoletes_set_module_context (ModulemdObsoletes *self,
                                       const gchar *module_context);

/**
 * modulemd_obsoletes_set_modified:
 * @self: This #ModulemdObsoletes object.
 * @modified: (in): The last modified time represented as a 64-bit integer (such as
 * 201807011200).
 *
 * Since: 2.10
 */
void
modulemd_obsoletes_set_modified (ModulemdObsoletes *self, guint64 modified);

/**
 * modulemd_obsoletes_set_reset:
 * @self: This #ModulemdObsoletes object.
 * @reset: (in): Whether to reset/cancel all previously specified obsoletes.
 *
 * Sets the reset attribute on #ModulemdObsoletes object. With this boolean attribute set the
 * obsoletes resets (cancels out) all previously specified obsoletes.
 *
 * Since: 2.10
 */
void
modulemd_obsoletes_set_reset (ModulemdObsoletes *self, gboolean reset);

/**
 * modulemd_obsoletes_get_reset:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: Whether this #ModulemdObsoletes object cancels/resets all previously specified obsoletes.
 *
 * Since: 2.10
 */
gboolean
modulemd_obsoletes_get_reset (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_set_eol_date:
 * @self: This #ModulemdObsoletes object.
 * @eol_date: (in): The end-of-life date for this stream. If set to zero, the stream is
 * EOLed immediately.
 *
 * Since: 2.10
 */
void
modulemd_obsoletes_set_eol_date (ModulemdObsoletes *self, guint64 eol_date);

/**
 * modulemd_obsoletes_get_eol_date:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: A date represented as a 64-bit integer (such as 201807011200).
 *
 * Since: 2.10
 */
guint64
modulemd_obsoletes_get_eol_date (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_get_message:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: (transfer none): The message associated with this #ModulemdObsoletes object
 *
 * Since: 2.10
 */
const gchar *
modulemd_obsoletes_get_message (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_set_obsoleted_by:
 * @self: This #ModulemdObsoletes object.
 * @obsoleted_by_module_name: (in): The module name of obsoleting stream.
 * @obsoleted_by_module_stream: (in): The module stream of obsoleting stream.
 *
 * Sets both obsoleted by module name and stream because having one without
 * the other is invalid.
 *
 * Since: 2.10
 */
void
modulemd_obsoletes_set_obsoleted_by (ModulemdObsoletes *self,
                                     const gchar *obsoleted_by_module_name,
                                     const gchar *obsoleted_by_module_stream);

/**
 * modulemd_obsoletes_get_obsoleted_by_module_name:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: (transfer none): The module name of obsoleting stream.
 *
 * Since: 2.10
 */
const gchar *
modulemd_obsoletes_get_obsoleted_by_module_name (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_get_obsoleted_by_module_stream:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: (transfer none): The module stream of obsoleting stream.
 *
 * Since: 2.10
 */
const gchar *
modulemd_obsoletes_get_obsoleted_by_module_stream (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_get_modified:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: The last modified time of this #ModulemdObsoletes object
 * represented as a 64-bit integer (such as 201807011200).
 *
 * Since: 2.10
 */
guint64
modulemd_obsoletes_get_modified (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_is_active:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: If this #ModulemdObsoletes object has eol_date set and the
 * date has not occured yet it returns false otherwise it returns true.
 * (When eol_date is not set or it already occured the obsoletes is active.)
 *
 * Since: 2.10
 */
gboolean
modulemd_obsoletes_is_active (ModulemdObsoletes *self);

G_END_DECLS
