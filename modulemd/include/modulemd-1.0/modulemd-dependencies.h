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

#ifndef MODULEMD_DEPENDENCIES_H
#define MODULEMD_DEPENDENCIES_H

#include <glib-object.h>

#include "modulemd-deprecated.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-dependencies
 * @title: Modulemd.Dependencies
 * @short_description: Object to represent build-time and runtime dependencies
 * of a module stream.
 */

#define MODULEMD_TYPE_DEPENDENCIES (modulemd_dependencies_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdDependencies, modulemd_dependencies, MODULEMD, DEPENDENCIES, GObject)


/**
 * modulemd_dependencies_new:
 *
 * Returns: (transfer full): A new #ModulemdDependencies object. This must be
 * freed with g_object_unref().
 *
 * Since: 1.0
 */
ModulemdDependencies *
modulemd_dependencies_new (void);


/**
 * modulemd_dependencies_add_buildrequires:
 * @module: The module name
 * @streams: (array zero-terminated): The list of streams for this module
 *
 * Add a set of modules and their streams that are required to build another
 * dependent module. The matrix of streams and module names will be calculated
 * by the build-system. If the listed provided module name is already present,
 * the streams will be added (with deduplication).
 *
 * Since: 1.0
 */
void
modulemd_dependencies_add_buildrequires (ModulemdDependencies *self,
                                         const gchar *module,
                                         const gchar **streams);


/**
 * modulemd_dependencies_add_buildrequires_single:
 * @module: The module name
 * @stream: The stream for this module
 *
 * Add a single stream of a module that is required to build another dependent
 * module. The matrix of streams and module names will be calculated by the
 * build-system. If the listed provided module name is already present, the
 * streams will be added (with deduplication).
 *
 * Since: 1.0
 */
void
modulemd_dependencies_add_buildrequires_single (ModulemdDependencies *self,
                                                const gchar *module,
                                                const gchar *stream);


/**
 * modulemd_dependencies_set_buildrequires:
 * @buildrequires: (nullable) (element-type utf8 ModulemdSimpleSet) (transfer none): The
 * requirements to build this module.
 *
 * Sets the 'buildrequires' property.
 *
 * Since: 1.0
 */
void
modulemd_dependencies_set_buildrequires (ModulemdDependencies *self,
                                         GHashTable *buildrequires);


/**
 * modulemd_dependencies_get_buildrequires:
 *
 * Retrieves the "buildrequires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A hash
 * table containing the "buildrequires" property. Returns NULL if there are no
 * buildrequires set.
 *
 * Deprecated: 1.1
 * Use peek_buildrequires() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_dependencies_peek_buildrequires)
GHashTable *
modulemd_dependencies_get_buildrequires (ModulemdDependencies *self);


/**
 * modulemd_dependencies_peek_buildrequires:
 *
 * Retrieves the "buildrequires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A hash
 * table containing the "buildrequires" property. Returns NULL if there are no
 * buildrequires set.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_dependencies_peek_buildrequires (ModulemdDependencies *self);


/**
 * modulemd_dependencies_dup_buildrequires:
 *
 * Retrieves a copy of the "buildrequires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer container): A hash
 * table containing the "buildrequires" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_dependencies_dup_buildrequires (ModulemdDependencies *self);


/**
 * modulemd_dependencies_add_requires:
 * @module: The module name
 * @streams: (array zero-terminated=1): The list of streams for this module
 *
 * Add a single stream of a module that is required to build another dependent
 * module. The matrix of streams and module names will be calculated by the
 * build-system. If the listed provided module name is already present, the
 * streams will be added (with deduplication).
 *
 * Since: 1.0
 */
void
modulemd_dependencies_add_requires (ModulemdDependencies *self,
                                    const gchar *module,
                                    const gchar **streams);


/**
 * modulemd_dependencies_add_requires_single:
 * @module: The module name
 * @stream: The stream for this module
 *
 * Add a set of modules and their streams that are required at runtime by a
 * dependent module. The matrix of streams and module names will be calculated
 * by the build-system. If the listed provided module name is already present,
 * the streams will be added (with deduplication).
 *
 * Since: 1.0
 */
void
modulemd_dependencies_add_requires_single (ModulemdDependencies *self,
                                           const gchar *module,
                                           const gchar *stream);


/**
 * modulemd_dependencies_set_requires:
 * @requires: (nullable) (element-type utf8 ModulemdSimpleSet) (transfer none):
 * The runtime requirements for this module.
 *
 * Sets the 'requires' property.
 *
 * Since: 1.0
 */
void
modulemd_dependencies_set_requires (ModulemdDependencies *self,
                                    GHashTable *requires);


/**
 * modulemd_dependencies_get_requires:
 *
 * Retrieves the "requires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A hash
 * table containing the "requires" property.
 *
 * Deprecated: 1.1
 * Use peek_requires() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_dependencies_peek_requires)
GHashTable *
modulemd_dependencies_get_requires (ModulemdDependencies *self);


/**
 * modulemd_dependencies_peek_requires:
 *
 * Retrieves the "requires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A hash
 * table containing the "requires" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_dependencies_peek_requires (ModulemdDependencies *self);


/**
 * modulemd_dependencies_dup_requires:
 *
 * Retrieves a copy of the "requires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer container): A hash
 * table containing the "requires" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_dependencies_dup_requires (ModulemdDependencies *self);


/**
 * modulemd_dependencies_copy:
 * @dest: (out): A reference to the destination #ModulemdDependencies
 *
 * This function will copy the contents of this #ModulemdDependencies to @dest.
 * If the dereferenced pointer is NULL, a new #ModulemdDependencies will be
 * allocated.
 *
 * If the dereferenced pointer is not NULL, it will replace the contents of
 * @dest. All existing internal variables will be freed.
 *
 * In either case, the caller is responsible for calling g_object_unref() later
 * to free it.
 *
 * If @self is NULL, no changes will occur to @dest.
 *
 * Since: 1.0
 */
void
modulemd_dependencies_copy (ModulemdDependencies *self,
                            ModulemdDependencies **dest);

G_END_DECLS

#endif /* MODULEMD_DEPENDENCIES_H */
