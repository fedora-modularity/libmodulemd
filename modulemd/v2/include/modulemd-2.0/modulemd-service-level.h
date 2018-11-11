/*
 * This file is part of libmodulemd
 * Copyright (C) 2017-2018 Stephen Gallagher
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
#include "private/glib-extensions.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-service-level
 * @title: Modulemd.ServiceLevel
 * @stability: stable
 * @short_description: Provides lifecycle information for a module stream.
 */

#define MODULEMD_TYPE_SERVICE_LEVEL (modulemd_service_level_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdServiceLevel,
                      modulemd_service_level,
                      MODULEMD,
                      SERVICE_LEVEL,
                      GObject)

/**
 * modulemd_service_level_new:
 * @name: (not nullable): The name of the service level.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdServiceLevel. This
 * object must be freed with g_object_unref().
 *
 * Since: 2.0
 */
ModulemdServiceLevel *
modulemd_service_level_new (const gchar *name);


/**
 * modulemd_service_level_copy:
 * @self: This #ModulemdServiceLevel
 *
 * Create a copy of this #ModulemdServiceLevel object.
 *
 * Returns: (transfer full): a copied #ModulemdServiceLevel object
 *
 * Since: 2.0
 */
ModulemdServiceLevel *
modulemd_service_level_copy (ModulemdServiceLevel *self);


/**
 * modulemd_service_level_get_name:
 * @self: This #ModulemdServiceLevel
 *
 * Get the name of this service level.
 *
 * Returns: (transfer none): The name of this service level. This is a pointer
 * to the internal memory location and must not be freed.
 *
 * Since: 2.0
 */
const gchar *
modulemd_service_level_get_name (ModulemdServiceLevel *self);


/**
 * modulemd_service_level_set_eol:
 * @self: This #ModulemdServiceLevel
 * @date: (nullable): The date this service level ends.
 *
 * Sets the end date of the service level. If the #GDate passed in is invalid
 * or NULL, the EOL will be unset.
 *
 * Since: 2.0
 */
void
modulemd_service_level_set_eol (ModulemdServiceLevel *self, GDate *date);


/**
 * modulemd_service_level_set_eol_ymd:
 * @self: This #ModulemdServiceLevel
 * @year: The year this service level ends.
 * @month: The month this service level ends.
 * @day: The day of the month this service level ends.
 *
 * Since: 2.0
 */
void
modulemd_service_level_set_eol_ymd (ModulemdServiceLevel *self,
                                    GDateYear year,
                                    GDateMonth month,
                                    GDateDay day);


/**
 * modulemd_service_level_remove_eol:
 * @self: This #ModulemdServiceLevel
 *
 * Remove the EOL from this Service Level
 *
 * Since: 2.0
 */
void
modulemd_service_level_remove_eol (ModulemdServiceLevel *self);

/**
 * modulemd_service_level_get_eol:
 * @self: This #ModulemdServiceLevel
 *
 * Returns: (transfer none): The end date of the service level as a #GDate.
 *
 * Since: 2.0
 */
GDate *
modulemd_service_level_get_eol (ModulemdServiceLevel *self);

/**
 * modulemd_service_level_get_eol_string:
 * @self: This #ModulemdServiceLevel
 *
 * Returns: (transfer full) (nullable): The end date of the service level as a
 * string of the form "YYYY-MM-DD" or NULL if the date is unset or invalid.
 *
 * Since: 2.0
 */
gchar *
modulemd_service_level_get_eol_string (ModulemdServiceLevel *self);

G_END_DECLS
