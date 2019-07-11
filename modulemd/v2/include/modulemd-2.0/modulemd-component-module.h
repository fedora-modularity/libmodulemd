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

#include <glib-object.h>
#include "modulemd-component.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-component-module
 * @title: Modulemd.ComponentModule
 * @stability: stable
 * @short_description: A module component that goes into a module stream.
 */
#define MODULEMD_TYPE_COMPONENT_MODULE (modulemd_component_module_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdComponentModule,
                      modulemd_component_module,
                      MODULEMD,
                      COMPONENT_MODULE,
                      ModulemdComponent)


/**
 * modulemd_component_module_new:
 * @key: (not nullable): The key of this module component. Used when looking up
 * components from a #ModulemdModuleStream.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdComponentModule object.
 *
 * Since: 2.0
 */
ModulemdComponentModule *
modulemd_component_module_new (const gchar *key);


/**
 * modulemd_component_module_set_ref:
 * @self: This #ModulemdComponentModule object.
 * @ref: (in) (nullable): The commit ID in the SCM repository.
 *
 * Since: 2.0
 */
void
modulemd_component_module_set_ref (ModulemdComponentModule *self,
                                   const gchar *ref);


/**
 * modulemd_component_module_get_ref:
 * @self: This #ModulemdComponentModule object.
 *
 * Returns: (transfer none): The commit ID in the SCM repository.
 *
 * Since: 2.0
 */
const gchar *
modulemd_component_module_get_ref (ModulemdComponentModule *self);


/**
 * modulemd_component_module_set_repository:
 * @self: This #ModulemdComponentModule object.
 * @repository: (in) (nullable): The URI of the SCM repository.
 *
 * Since: 2.0
 */
void
modulemd_component_module_set_repository (ModulemdComponentModule *self,
                                          const gchar *repository);


/**
 * modulemd_component_module_get_repository:
 * @self: This #ModulemdComponentModule object.
 *
 * Returns: (transfer none): The URI of the SCM repository.
 *
 * Since: 2.0
 */
const gchar *
modulemd_component_module_get_repository (ModulemdComponentModule *self);

G_END_DECLS
