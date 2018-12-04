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

G_BEGIN_DECLS


/**
 * SECTION: modulemd-defaults
 * @title: Modulemd.Defaults
 * @stability: stable
 * @short_description: Parent class for Default documents.
 * See #ModulemdDefaultsV1 for a specific type.
 */

/**
 * ModulemdDefaultsVersionEnum:
 * @MD_DEFAULTS_VERSION_ERROR: Represents an error handling mdversion.
 * @MD_DEFAULTS_VERSION_ONE: Represents v1 of the #Modulemd.Defaults metadata
 * format.
 * @MD_DEFAULTS_VERSION_LATEST: Represents the highest-supported version of the
 * #Modulemd.Defaults metadata format.
 *
 * Since: 2.0
 */
typedef enum
{
  MD_DEFAULTS_VERSION_ERROR = -1,

  MD_DEFAULTS_VERSION_UNSET = 0,

  MD_DEFAULTS_VERSION_ONE = 1,

  MD_DEFAULTS_VERSION_LATEST = MD_DEFAULTS_VERSION_ONE,
} ModulemdDefaultsVersionEnum;


#define MODULEMD_TYPE_DEFAULTS (modulemd_defaults_get_type ())

G_DECLARE_DERIVABLE_TYPE (
  ModulemdDefaults, modulemd_defaults, MODULEMD, DEFAULTS, GObject)

struct _ModulemdDefaultsClass
{
  GObjectClass parent_class;

  ModulemdDefaults *(*copy) (ModulemdDefaults *self);

  gboolean (*validate) (ModulemdDefaults *self, GError **error);

  guint64 (*get_mdversion) (ModulemdDefaults *self);

  /* Padding to allow adding up to 10 new virtual functions without
   * breaking ABI. */
  gpointer padding[10];
};


/**
 * modulemd_defaults_new:
 * @version: The version of the defaults metadata to create
 * @module_name: The name of the module to which these defaults apply
 *
 * Create a new #ModulemdDefaults.
 *
 * Returns: (transfer full): a newly created #ModulemdDefaults subtype of the
 * requested version.
 *
 * Since: 2.0
 */
ModulemdDefaults *
modulemd_defaults_new (guint64 version, const gchar *module_name);


/**
 * modulemd_defaults_copy:
 * @self: (in): This #ModulemdDefaults object
 *
 * Returns: (transfer full): A newly-allocated copy of @self
 *
 * Since: 2.0
 */
ModulemdDefaults *
modulemd_defaults_copy (ModulemdDefaults *self);


/**
 * modulemd_defaults_validate:
 * @self: (in): This #ModulemdDefaults object
 * @error: (out):  A #GError that will return the reason for a validation error.
 *
 * Returns: TRUE if validation passed, FALSE and sets @error appropriately if
 * validation failed.
 *
 * Since: 2.0
 */
gboolean
modulemd_defaults_validate (ModulemdDefaults *self, GError **error);


/**
 * modulemd_defaults_upgrade:
 * @self: (in): This #ModulemdDefaults object
 * @mdversion: (in): The version to upgrade to.
 * @error: (out):  A #GError that will return the reason for an upgrade error.
 *
 * Returns: (transfer full): A newly-allocated copy of @self upgraded to the
 * requested defaults version. NULL if the upgrade cannot be performed and sets
 * @error appropriately. This function does not modify @self.
 *
 * Since: 2.0
 */
ModulemdDefaults *
modulemd_defaults_upgrade (ModulemdDefaults *self,
                           guint64 mdversion,
                           GError **error);


/**
 * modulemd_defaults_get_module_name:
 * @self: (in): This #ModulemdDefaults object
 *
 * Returns: (transfer none): The name of the module to which these defaults
 * apply.
 *
 * Since: 2.0
 */
const gchar *
modulemd_defaults_get_module_name (ModulemdDefaults *self);


/**
 * modulemd_defaults_get_mdversion:
 * @self: (in): This #ModulemdDefaults object
 *
 * Returns: The metadata version of this defaults object.
 *
 * Since: 2.0
 */
guint64
modulemd_defaults_get_mdversion (ModulemdDefaults *self);


/**
 * modulemd_defaults_set_modified:
 * @self: (in): This #ModulemdDefaults object
 * @modified: (in): The last modified time represented as a 64-bit integer
 * (such as 201807011200)
 *
 * Since: 2.0
 */
void
modulemd_defaults_set_modified (ModulemdDefaults *self, guint64 modified);


/**
 * modulemd_defaults_get_modified:
 * @self: (in): This #ModulemdDefaults object
 *
 * Returns: The last modified time represented as a 64-bit integer
 * (such as 201807011200)
 *
 * Since: 2.0
 */
guint64
modulemd_defaults_get_modified (ModulemdDefaults *self);

G_END_DECLS
