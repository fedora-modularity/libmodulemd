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

#include "modulemd-buildopts.h"
#include "modulemd-modulestream.h"
#include "modulemd-defaults.h"

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-improvedmodule
 * @title: Modulemd.ImprovedModule
 * @short_description: Collects all information about a module: all of its
 * streams, defaults, etc.
 */

#define MODULEMD_TYPE_IMPROVEDMODULE (modulemd_improvedmodule_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdImprovedModule,
                      modulemd_improvedmodule,
                      MODULEMD,
                      IMPROVEDMODULE,
                      GObject)


/**
 * modulemd_improvedmodule_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdImprovedModule. This
 * object must be freed with g_object_unref().
 *
 * Since: 1.6
 */
ModulemdImprovedModule *
modulemd_improvedmodule_new (const gchar *name);


/**
 * modulemd_improvedmodule_add_stream:
 * @stream: (transfer none) (not nullable): A #ModulemdModuleStream of this
 * module.
 *
 * Add a #ModulemdModuleStream to this module. If this stream name is already in
 * use, this function will overwrite the existing value. If the module name does
 * not match, this function will silently ignore this stream.
 *
 * Since : 1.6
 */
void
modulemd_improvedmodule_add_stream (ModulemdImprovedModule *self,
                                    ModulemdModuleStream *stream);

/**
 * modulemd_improvedmodule_get_stream_by_name:
 * @stream_name: The name of the stream to retrieve.
 *
 * Returns: (transfer full): A #ModulemModuleStream representing the requested
 * module stream. NULL if the stream name was not found. If there is more than
 * one version and/or context for this stream, it will return one in a
 * non-deterministic manner. This function remains for backwards-compatibility,
 * but it should never be used.
 *
 * Since: 1.6
 *
 * Deprecated: 1.7
 * Use get_stream_by_nsvc() or get_streams_by_name() instead.
 */
MMD_DEPRECATED_FOR (modulemd_improvedmodule_get_stream_by_nsvc)
ModulemdModuleStream *
modulemd_improvedmodule_get_stream_by_name (ModulemdImprovedModule *self,
                                            const gchar *stream_name);


/**
 * modulemd_improvedmodule_get_stream_by_nsvc:
 * @nsvc: The NSVC of the stream to retrieve.
 *
 * Returns: (transfer full): A #ModulemModuleStream representing the requested
 * module stream. NULL if the stream (name, version, context) was not found.
 *
 * Since: 1.7
 */
ModulemdModuleStream *
modulemd_improvedmodule_get_stream_by_nsvc (ModulemdImprovedModule *self,
                                            const gchar *nsvc);


/**
 * modulemd_improvedmodule_get_streams:
 *
 * Returns: (element-type utf8 ModulemdModuleStream) (transfer container): A
 * #GHashTable containing all #ModulemModuleStream objects for this module.
 * This hash table must be freed with g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_improvedmodule_get_streams (ModulemdImprovedModule *self);

/**
 * modulemd_improvedmodule_get_streams_by_name:
 * @stream_name: The name of the stream to retrieve
 *
 * Returns: (element-type ModulemdModuleStream) (transfer container): A
 * #GPtrArray containing all #ModulemModuleStream objects for this module which
 * matches the requested stream_name. Returns NULL if no streams matched this
 * name.
 * This #GPtrArray must be freed with g_ptr_array_unref().
 *
 * Since: 1.6
 */
GPtrArray *
modulemd_improvedmodule_get_streams_by_name (ModulemdImprovedModule *self,
                                             const gchar *stream_name);


/**
 * modulemd_improvedmodule_set_name:
 * @module_name: (transfer none) (not nullable): The name of this module.
 *
 * Sets the module name.
 *
 * Since: 1.6
 */
void
modulemd_improvedmodule_set_name (ModulemdImprovedModule *self,
                                  const gchar *module_name);


/**
 * modulemd_improvedmodule_get_name:
 *
 * Gets the name of this module.
 *
 * Returns: (transfer full): The name of this module. This value must be freed
 * with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_improvedmodule_get_name (ModulemdImprovedModule *self);


/**
 * modulemd_improvedmodule_peek_name: (skip)
 *
 * Gets the name of this module.
 *
 * Returns: (transfer none): The name of this module. This value must be not be
 * modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_improvedmodule_peek_name (ModulemdImprovedModule *self);


/**
 * modulemd_improvedmodule_set_defaults:
 * @defaults: (transfer none) (nullable): A #ModulemdDefaults object describing
 * the defaults for this module.
 *
 * Set the default stream and profiles for this module. Makes no changes if the
 * defaults do not apply to this module.
 *
 * Since: 1.6
 */
void
modulemd_improvedmodule_set_defaults (ModulemdImprovedModule *self,
                                      ModulemdDefaults *defaults);


/**
 * modulemd_improvedmodule_get_defaults:
 *
 * Returns the #ModulemdDefaults object for this module.
 *
 * Returns: (transfer full): a #ModulemdDefaults object if set, NULL otherwise.
 * This object must be freed with g_object_unref().
 *
 * Since: 1.6
 */
ModulemdDefaults *
modulemd_improvedmodule_get_defaults (ModulemdImprovedModule *self);


/**
 * modulemd_improvedmodule_peek_defaults: (skip)
 *
 * Returns the #ModulemdDefaults object for this module.
 *
 * Returns: (transfer none): a #ModulemdDefaults object if set, NULL otherwise.
 * This object must be not be modified or freed.
 *
 * Since: 1.6
 */
ModulemdDefaults *
modulemd_improvedmodule_peek_defaults (ModulemdImprovedModule *self);


/**
 * modulemd_improvedmodule_copy:
 *
 * Make a copy of this module.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdImprovedModule that is
 * a copy of the one passed in.
 *
 * Since: 1.6
 */
ModulemdImprovedModule *
modulemd_improvedmodule_copy (ModulemdImprovedModule *self);


/**
 * modulemd_improvedmodule_dump:
 * @yaml_file: A string containing the path to the output file
 *
 * Writes this module out to a YAML document on disk.
 *
 * Since: 1.6
 */
void
modulemd_improvedmodule_dump (ModulemdImprovedModule *self,
                              const gchar *yaml_file,
                              GError **error);


/**
 * modulemd_improvedmodule_dumps:
 *
 * Writes this module out to a YAML document string.
 *
 * Return value: (transfer full): A string containing a YAML representation of
 * this module and all of its streams. This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_improvedmodule_dumps (ModulemdImprovedModule *self, GError **error);

G_END_DECLS
