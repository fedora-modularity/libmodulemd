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

#include "modulemd-defaults.h"
#include "modulemd-deprecated.h"
#include "modulemd-module-stream.h"
#include "modulemd-translation.h"
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-module
 * @title: Modulemd.Module
 * @stability: stable
 * @short_description: Collects all information about a module: all of its streams, defaults, etc.
 */

#define MODULEMD_TYPE_MODULE (modulemd_module_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdModule, modulemd_module, MODULEMD, MODULE, GObject)


/**
 * modulemd_module_get_stream_names_as_strv: (rename-to modulemd_module_get_stream_names)
 * @self: This #ModulemdModule object.
 *
 * Returns: (transfer full): An ordered #GStrv list of stream names in this
 * module.
 *
 * Since: 2.6
 */
GStrv
modulemd_module_get_stream_names_as_strv (ModulemdModule *self);


/**
 * modulemd_module_copy:
 * @self: (in): This #ModulemdModule object.
 *
 * Returns: (transfer full): A deep copy of this #ModulemdModule object.
 *
 * Since: 2.0
 */
ModulemdModule *
modulemd_module_copy (ModulemdModule *self);


/**
 * modulemd_module_validate:
 * @self: (in): This #ModulemdModule object.
 * @error: (out): A #GError containing the reason the object failed validation.
 * NULL if the validation passed.
 *
 * Returns: TRUE if validation passed, FALSE and sets @error if failed.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_validate (ModulemdModule *self, GError **error);


/**
 * modulemd_module_get_module_name:
 * @self: This #ModulemdModule object.
 *
 * Returns: (transfer none): The module name.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_get_module_name (ModulemdModule *self);


/**
 * modulemd_module_get_all_streams:
 * @self: This #ModulemdModule object.
 *
 * Returns: (transfer none) (element-type ModulemdModuleStream): A list of all available stream objects associated with
 * this module. There may be multiple streams with the same name and different version and
 * context. The order of items in this list is not guaranteed.
 *
 * Since: 2.0
 */
GPtrArray *
modulemd_module_get_all_streams (ModulemdModule *self);


/**
 * modulemd_module_get_streams_by_stream_name_as_list: (rename-to modulemd_module_get_streams_by_stream_name)
 * @self: This #ModulemdModule object.
 * @stream_name: The name of the stream to retrieve.
 *
 * Returns: (transfer container) (element-type ModulemdModuleStream): A list of all available stream objects associated with a
 * particular stream name, sorted highest to lowest by the version. The same version may have
 * more than one associated context.
 *
 * Since: 2.0
 */
GPtrArray *
modulemd_module_get_streams_by_stream_name_as_list (ModulemdModule *self,
                                                    const gchar *stream_name);


/**
 * modulemd_module_get_stream_by_NSVC:
 * @self: This #ModulemdModule object.
 * @stream_name: The name of the stream to retrieve.
 * @version: The version of the stream to retrieve.
 * @context: The context of the stream to retrieve.
 *
 * Returns: (transfer none): The requested stream object or NULL if no match was found.
 *
 * Since: 2.0
 * Deprecated: 2.2: Use modulemd_module_get_stream_by_NSVCA() instead.
 */
MMD_DEPRECATED_FOR (modulemd_module_get_stream_by_NSVCA)
ModulemdModuleStream *
modulemd_module_get_stream_by_NSVC (ModulemdModule *self,
                                    const gchar *stream_name,
                                    guint64 version,
                                    const gchar *context);


/**
 * modulemd_module_search_streams:
 * @self: This #ModulemdModule object.
 * @stream_name: The name of the stream to retrieve.
 * @version: The version of the stream to retrieve. If set to zero,
 * the version is not included in the search.
 * @context: (nullable): The context of the stream to retrieve. If NULL, the
 * context is not included in the search.
 * @arch: (nullable): The processor architecture of the stream to retrieve. If
 * NULL, the architecture is not included in the search.
 *
 * Returns: (transfer container) (element-type ModulemdModuleStream): The list
 * of stream objects matching the requested parameters. This function cannot
 * fail, but it may return a zero-length list if no matches were found. The
 * returned streams will be in a predictable order, sorted first by stream
 * name, then by version (highest to lowest), then by context and finally by
 * architecture.
 *
 * Since: 2.5
 */
GPtrArray *
modulemd_module_search_streams (ModulemdModule *self,
                                const gchar *stream_name,
                                guint64 version,
                                const gchar *context,
                                const gchar *arch);

/**
 * modulemd_module_search_streams_by_glob:
 * @self: This #ModulemdModule object.
 * @stream_name: (nullable): The name of the stream to retrieve. If NULL, will
 * search all streams.
 * @version: (nullable): The version of the stream to retrieve. If NULL, will
 * search all versions.
 * @context: (nullable): The context of the stream to retrieve. If NULL, will
 * search all contexts.
 * @arch: (nullable): The processor architecture of the stream to retrieve. If
 * NULL, the architecture is not included in the search.
 *
 * All arguments to this method will be compared using
 * [fnmatch(3)](https://www.mankier.com/3/fnmatch).
 *
 * Returns: (transfer container) (element-type ModulemdModuleStream): The list
 * of stream objects matching all of the requested parameters. This function
 * cannot fail, but it may return a zero-length list if no matches were found.
 * The returned streams will be in a predictable order, sorted first by module
 * name, then stream name, then by version (highest first), then by context
 * and finally by architecture.
 *
 * Since: 2.9
 */
GPtrArray *
modulemd_module_search_streams_by_glob (ModulemdModule *self,
                                        const gchar *stream_name,
                                        const gchar *version,
                                        const gchar *context,
                                        const gchar *arch);

/**
 * modulemd_module_search_streams_by_nsvca_glob:
 * @self: This #ModulemdModule object.
 * @nsvca_pattern: (nullable): A [glob](https://www.mankier.com/3/glob)
 * pattern to match against the NSVCA strings of the #ModulemdModuleStream
 * objects in this module. If NULL, this will match all NSVCAs.
 *
 * Returns: (transfer container) (element-type ModulemdModuleStream): An array
 * of #ModulemdModuleStream objects whose NSVCA string matches the provided
 * pattern. This function cannot fail, but may return an array of zero entries
 * if the pattern did not match any streams. The returned streams will be in a
 * predictable order, sorted first by module name, then stream name, then by
 * version (highest first), then by context and finally by architecture.
 *
 * Since: 2.9
 */
GPtrArray *
modulemd_module_search_streams_by_nsvca_glob (ModulemdModule *self,
                                              const gchar *nsvca_pattern);


/**
 * modulemd_module_get_stream_by_NSVCA:
 * @self: This #ModulemdModule object.
 * @stream_name: The name of the stream to retrieve.
 * @version: The version of the stream to retrieve. If set to zero, the version
 * is not included in the search.
 * @context: (nullable): The context of the stream to retrieve. If NULL, the
 * context is not included in the search.
 * @arch: (nullable): The processor architecture of the stream to retrieve. If
 * NULL, the architecture is not included in the search.
 * @error: (out): A #GError indicating the reason this function failed to
 * retrieve exactly one #ModulemdModuleStream.
 *
 * Returns: (transfer none): The requested stream object. NULL and sets @error
 * appropriately if the provided information is not sufficient to return
 * exactly one #ModulemdModuleStream result.
 *
 * Since: 2.2
 */
ModulemdModuleStream *
modulemd_module_get_stream_by_NSVCA (ModulemdModule *self,
                                     const gchar *stream_name,
                                     guint64 version,
                                     const gchar *context,
                                     const gchar *arch,
                                     GError **error);


/**
 * modulemd_module_remove_streams_by_NSVCA:
 * @self: This #ModulemdModule object.
 * @stream_name: (not nullable): The name of the stream to remove.
 * @version: The version of the stream to remove. If set to zero, matches all
 * versions.
 * @context: (nullable): The context of the stream to remove. If NULL, matches
 * all stream contexts.
 * @arch: (nullable): The processor architecture of the stream to remove. If
 * NULL, matches all architectures.
 *
 * Remove one or more #ModulemdModuleStream objects from this #ModulemdModule
 * that match the provided parameters.
 *
 * Since: 2.3
 */
void
modulemd_module_remove_streams_by_NSVCA (ModulemdModule *self,
                                         const gchar *stream_name,
                                         guint64 version,
                                         const gchar *context,
                                         const gchar *arch);


/**
 * modulemd_module_remove_streams_by_name:
 * @self: This #ModulemdModule object.
 * @stream_name: (not nullable): The name of the stream to remove.
 *
 * Remove one or more #ModulemdModuleStream objects from this #ModulemdModule
 * that match the provided stream name.
 *
 * Since: 2.3
 */
#define modulemd_module_remove_streams_by_name(self, stream_name)             \
  modulemd_module_remove_streams_by_NSVCA (self, stream_name, 0, NULL, NULL)


/**
 * modulemd_module_get_defaults:
 * @self: This #ModulemdModule object.
 *
 * Returns: (transfer none): The defaults of this module.
 *
 * Since: 2.0
 */
ModulemdDefaults *
modulemd_module_get_defaults (ModulemdModule *self);


/**
 * modulemd_module_get_translation:
 * @self: This #ModulemdModule object.
 * @stream: The stream to look up translations for.
 *
 * Returns: (transfer none): The set of translations attached to streams.
 *
 * Since: 2.8
 */
ModulemdTranslation *
modulemd_module_get_translation (ModulemdModule *self, const gchar *stream);

G_END_DECLS
