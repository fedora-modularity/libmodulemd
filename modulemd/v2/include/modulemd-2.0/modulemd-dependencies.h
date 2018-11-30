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

G_BEGIN_DECLS

/**
 * SECTION: modulemd-dependencies
 * @title: Modulemd.Dependencies
 * @stability: stable
 * @short_description: Object to represent build-time and runtime dependencies of a module stream.
 */

#define MODULEMD_TYPE_DEPENDENCIES (modulemd_dependencies_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdDependencies, modulemd_dependencies, MODULEMD, DEPENDENCIES, GObject)

/**
 * modulemd_dependencies_new:
 *
 * Returns: (transfer full): A newly-allocated #ModuleDependencies. This object must be freed with g_object_unref().
 *
 * Since: 2.0
 */
ModulemdDependencies *
modulemd_dependencies_new (void);


/**
 * modulemd_dependencies_copy:
 * @self: This #ModulemdDependencies
 *
 * Create a copy of this #ModulemdDependencies object.
 *
 * Returns: (transfer full): a copied #ModulemdDependencies object
 *
 * Since: 2.0
 */
ModulemdDependencies *
modulemd_dependencies_copy (ModulemdDependencies *self);


/**
 * modulemd_dependencies_add_buildtime_stream:
 * @self: This #ModulemdDependencies
 * @module_name: The name of the module to depend on.
 * @module_stream: The name of the module stream to depend on.
 *
 * Add a single stream of a module that is required to build another dependent module. The matrix of streams and module names will be calculated by the build-system. If the provided module name is already present, the streams will be added (with deduplication).
 *
 * Since: 2.0
 */
void
modulemd_dependencies_add_buildtime_stream (ModulemdDependencies *self,
                                            const gchar *module_name,
                                            const gchar *module_stream);


/**
 * modulemd_dependencies_set_empty_buildtime_dependencies_for_module:
 * @self: This #ModuleDependencies
 * @module_name: The name of the module to add dependencies on.
 *
 * Adds a module and inserts an empty list for it as buildtime dependency.
 *
 * Since: 2.0
 */
void
modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
  ModulemdDependencies *self, const gchar *module_name);


/**
 * modulemd_dependencies_get_buildtime_modules_as_strv: (rename-to modulemd_dependencies_get_buildtime_modules)
 * @self: This #ModulemdDependencies
 *
 * Returns: (transfer full): An ordered list of module names of build-time dependencies.
 *
 * Since: 2.0
 */
GStrv
modulemd_dependencies_get_buildtime_modules_as_strv (
  ModulemdDependencies *self);


/**
 * modulemd_dependencies_get_buildtime_streams_as_strv: (rename-to modulemd_dependencies_get_buildtime_streams)
 * @self: This #ModulemdDependencies
 * @module: The name of the module.
 *
 * Returns: (transfer full) (nullable): An ordered list of module streams associated with the specified module that are required at build-time.
 *
 * Since: 2.0
 */
GStrv
modulemd_dependencies_get_buildtime_streams_as_strv (
  ModulemdDependencies *self, const gchar *module);


/**
 * modulemd_dependencies_add_runtime_stream:
 * @self: This #ModulemdDependencies
 * @module_name: The name of the module to depend on.
 * @module_stream: The name of the module stream to depend on.
 *
 * Adds a module and its stream that is required at runtime by a dependent module. The matrix of streams and module names will be calculated by the build-system. If the listed provided module name is already present, the streams will be added (with deduplication).
 *
 * Since: 2.0
 */
void
modulemd_dependencies_add_runtime_stream (ModulemdDependencies *self,
                                          const gchar *module_name,
                                          const gchar *module_stream);


/**
 * modulemd_dependencies_set_empty_runtime_dependencies_for_module:
 * @self: This #ModuleDependencies
 * @module_name: The name of the module to add dependencies on.
 *
 * Adds a module and inserts an empty list for it as runtime dependency.
 *
 * Since: 2.0
 */
void
modulemd_dependencies_set_empty_runtime_dependencies_for_module (
  ModulemdDependencies *self, const gchar *module_name);


/**
 * modulemd_dependencies_get_runtime_modules_as_strv: (rename-to modulemd_dependencies_get_runtime_modules)
 * @self: This #ModulemdDependencies
 *
 * Returns: (transfer full): An ordered list of module names of run-time dependencies.
 *
 * Since: 2.0
 */
GStrv
modulemd_dependencies_get_runtime_modules_as_strv (ModulemdDependencies *self);


/**
 * modulemd_dependencies_get_runtime_streams_as_strv: (rename-to modulemd_dependencies_get_runtime_streams)
 * @self: This #ModulemdDependencies
 * @module: The name of the module.
 *
 * Returns: (transfer full) (nullable): An ordered list of module streams associated with the specified module that are required at run-time.
 *
 * Since: 2.0
 */
GStrv
modulemd_dependencies_get_runtime_streams_as_strv (ModulemdDependencies *self,
                                                   const gchar *module);
