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

#ifndef MODULEMD_COMPONENT_MODULE_H
#define MODULEMD_COMPONENT_MODULE_H

#include "modulemd.h"
#include "modulemd-component.h"
#include "modulemd-simpleset.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-component-module
 * @title: Modulemd.ComponentModule
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
 *
 * Returns: (transfer full): A newly-allocated #ModulemdComponentModule. Use
 * g_object_unref() to free it.
 *
 * Since: 1.0
 */
ModulemdComponentModule *
modulemd_component_module_new (void);


/**
 * modulemd_component_module_set_ref
 * @ref: (nullable): A string: The particular repository commit hash, branch or tag name
 * used in this module.
 *
 * Since: 1.0
 */
void
modulemd_component_module_set_ref (ModulemdComponentModule *self,
                                   const gchar *ref);


/**
 * modulemd_component_module_get_ref:
 *
 * Retrieves the repository ref.
 *
 * Returns: A string containing the repository ref.
 *
 * Deprecated: 1.1
 * Use peek_ref() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_component_module_peek_ref)
const gchar *
modulemd_component_module_get_ref (ModulemdComponentModule *self);

/**
 * modulemd_component_module_peek_ref:
 *
 * Retrieves the repository ref.
 *
 * Returns: A string containing the repository ref.
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_module_peek_ref (ModulemdComponentModule *self);


/**
 * modulemd_component_module_dup_ref:
 *
 * Retrieves a copy of the repository ref.
 *
 * Returns: A copy of the string containing the repository ref.
 *
 * Since: 1.1
 */
gchar *
modulemd_component_module_dup_ref (ModulemdComponentModule *self);


/**
 * modulemd_component_module_set_repository
 * @repository: (nullable): A string: The VCS repository with the modulemd file, and other
 * module data.
 *
 * Since: 1.0
 */
void
modulemd_component_module_set_repository (ModulemdComponentModule *self,
                                          const gchar *repository);


/**
 * modulemd_component_module_get_repository:
 *
 * Retrieves the repository location.
 *
 * Returns: A string containing the repository location.
 *
 * Deprecated: 1.1
 * Use peek_repository() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_component_module_peek_repository)
const gchar *
modulemd_component_module_get_repository (ModulemdComponentModule *self);


/**
 * modulemd_component_module_peek_repository:
 *
 * Retrieves the repository location.
 *
 * Returns: A string containing the repository location.
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_module_peek_repository (ModulemdComponentModule *self);


/**
 * modulemd_component_module_dup_repository:
 *
 * Retrieves a copy of the repository location.
 *
 * Returns: A copy of the string containing the repository location.
 *
 * Since: 1.1
 */
gchar *
modulemd_component_module_dup_repository (ModulemdComponentModule *self);

G_END_DECLS

#endif /* MODULEMD_COMPONENT_MODULE_H */
