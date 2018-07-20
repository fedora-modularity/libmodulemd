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

#ifndef _MODULEMD_SERVICELEVEL_H
#define _MODULEMD_SERVICELEVEL_H

#include "modulemd-deprecated.h"

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-servicelevel
 * @title: Modulemd.ServiceLevel
 * @short_description: Provides lifecycle information for a module stream.
 */

#define MODULEMD_TYPE_SERVICELEVEL modulemd_servicelevel_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdServiceLevel, modulemd_servicelevel, MODULEMD, SERVICELEVEL, GObject)


/**
 * modulemd_servicelevel_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdServiceLevel. This
 * object must be freed with g_object_unref().
 *
 * Since: 1.0
 */
ModulemdServiceLevel *
modulemd_servicelevel_new (void);


/**
 * modulemd_servicelevel_set_eol:
 * @date: (nullable): The date this service level ends
 *
 * Sets the end date of the service level.
 *
 * Since: 1.0
 */
void
modulemd_servicelevel_set_eol (ModulemdServiceLevel *self, const GDate *date);


/**
 * modulemd_servicelevel_get_eol:
 *
 * Retrieves the end-of-life date of this service level.
 *
 * Returns: a #GDate representing the end-of-life date of the service level.
 *
 * Deprecated: 1.1
 * Use peek_eol() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_servicelevel_peek_eol)
const GDate *
modulemd_servicelevel_get_eol (ModulemdServiceLevel *self);


/**
 * modulemd_servicelevel_peek_eol:
 *
 * Retrieves the end-of-life date of this service level.
 *
 * Returns: a #GDate representing the end-of-life date of the service level.
 *
 * Since: 1.1
 */
const GDate *
modulemd_servicelevel_peek_eol (ModulemdServiceLevel *self);


/**
 * modulemd_servicelevel_dup_eol:
 *
 * Retrieves a copy of the end-of-life date of this service level.
 *
 * Returns: a #GDate representing the end-of-life date of the service level.
 *
 * Since: 1.1
 */
GDate *
modulemd_servicelevel_dup_eol (ModulemdServiceLevel *self);


/**
 * modulemd_servicelevel_set_name:
 * @name: (nullable): The name of this servicelevel
 *
 * Set the name of this service level.
 *
 * Since: 1.0
 */
void
modulemd_servicelevel_set_name (ModulemdServiceLevel *self, const gchar *name);


/**
 * modulemd_servicelevel_get_name:
 *
 * Retrieves the name of this service level
 *
 * Returns: a string representing the name of the service level or NULL if not
 * set.
 *
 * Deprecated: 1.1
 * Use peek_name() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_servicelevel_peek_name)
const gchar *
modulemd_servicelevel_get_name (ModulemdServiceLevel *self);


/**
 * modulemd_servicelevel_peek_name:
 *
 * Retrieves the name of this service level
 *
 * Returns: a string representing the name of the service level or NULL if not
 * set.
 *
 * Since: 1.1
 */
const gchar *
modulemd_servicelevel_peek_name (ModulemdServiceLevel *self);


/**
 * modulemd_servicelevel_dup_name:
 *
 * Retrieves a copy of the name of this service level
 *
 * Returns: a copy of the string representing the name of the service level or
 * NULL if not set.
 *
 * Since: 1.1
 */
gchar *
modulemd_servicelevel_dup_name (ModulemdServiceLevel *self);


/**
 * modulemd_servicelevel_copy:
 *
 * Create a copy of this #ModulemdServiceLevel object.
 *
 * Returns: (transfer full): a copied #ModulemdServiceLevel object
 *
 * Since: 1.1
 */
ModulemdServiceLevel *
modulemd_servicelevel_copy (ModulemdServiceLevel *self);

G_END_DECLS

#endif /* _MODULEMD_SERVICELEVEL_H */
