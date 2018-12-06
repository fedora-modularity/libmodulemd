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
#include "modulemd-module.h"
#include "modulemd-translation.h"
#include "modulemd-subdocument-info.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-module-index
 * @title: Modulemd.ModuleIndex
 * @stability: stable
 * @short_description: A simplistic wrapper around GHashTable to ensure
 * type-safety for [Modulemd.Module](Modulemd.Module.md) objects.
 */

#define MODULEMD_TYPE_MODULE_INDEX (modulemd_module_index_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdModuleIndex, modulemd_module_index, MODULEMD, MODULE_INDEX, GObject)


/**
 * modulemd_module_index_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleIndex object.
 *
 * Since: 2.0
 */
ModulemdModuleIndex *
modulemd_module_index_new (void);


/**
 * modulemd_module_index_update_from_file:
 * @self: This #ModulemdModuleIndex object
 * @yaml_file: (in): A YAML file containing the module metadata and other
 * related information such as default streams.
 * @failures: (out) (element-type ModulemdSubdocumentInfo) (transfer container):
 * An array containing any subdocuments from the YAML file that failed to parse.
 * See #ModulemdSubdocumentInfo for more details.
 * @error: (out): A GError containing additional information if this function
 * fails.
 *
 * Returns: TRUE if the update was successful. Returns FALSE and sets failures
 * and error approriately if any of the YAML subdocuments were invalid or if
 * there was a parse error.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_index_update_from_file (ModulemdModuleIndex *self,
                                        const gchar *yaml_file,
                                        GPtrArray **failures,
                                        GError **error);


/**
 * modulemd_module_index_update_from_string:
 * @self: This #ModulemdModuleIndex object
 * @yaml_string: (in): A YAML string containing the module metadata and other
 * related information such as default streams.
 * @failures: (out) (element-type ModulemdSubdocumentInfo) (transfer container):
 * An array containing any subdocuments from the YAML file that failed to parse.
 * See #ModulemdSubdocumentInfo for more details.
 * @error: (out): A GError containing additional information if this function
 * fails.
 *
 * Returns: TRUE if the update was successful. Returns FALSE and sets failures
 * and error approriately if any of the YAML subdocuments were invalid or if
 * there was a parse error.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_index_update_from_string (ModulemdModuleIndex *self,
                                          const gchar *yaml_string,
                                          GPtrArray **failures,
                                          GError **error);


/**
 * modulemd_module_index_update_from_stream: (skip)
 * @self: This #ModulemdModuleIndex object
 * @yaml_stream: (in): A YAML stream containing the module metadata and other
 * related information such as default streams.
 * @failures: (out) (element-type ModulemdSubdocumentInfo) (transfer container):
 * An array containing any subdocuments from the YAML file that failed to parse.
 * See #ModulemdSubdocumentInfo for more details.
 * @error: (out): A GError containing additional information if this function
 * fails.
 *
 * Returns: TRUE if the update was successful. Returns FALSE and sets failures
 * and error approriately if any of the YAML subdocuments were invalid or if
 * there was a parse error.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_index_update_from_stream (ModulemdModuleIndex *self,
                                          FILE *yaml_stream,
                                          GPtrArray **failures,
                                          GError **error);


/**
 * modulemd_module_index_dump_to_string:
 * @self: This #ModulemdModuleIndex
 * @error: (out): A #GError containing the reason the function failed, NULL if
 * the function succeeded.
 *
 * Returns: (transfer full): A YAML representation of the index as a string. In
 * the event of an error, sets error appropriately and returns NULL.
 *
 * Since: 2.0
 */
gchar *
modulemd_module_index_dump_to_string (ModulemdModuleIndex *self,
                                      GError **error);

/**
 * modulemd_module_index_dump_to_stream: (skip)
 * @self: This #ModulemdModuleIndex
 * @yaml_stream: (in): A stream to write the module metadata and other related
 * information to.
 * @error: (out): A #GError containing the reason the function failed, NULL if
 * the function succeeded.
 *
 * Returns: TRUE if written successfully, FALSE and sets error appropriately in
 * the event of an error.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_index_dump_to_stream (ModulemdModuleIndex *self,
                                      FILE *yaml_stream,
                                      GError **error);


/**
 * modulemd_module_index_get_module_names_as_strv: (rename-to modulemd_module_index_get_module_names)
 * @self: This #ModulemdModuleIndex
 *
 * Returns: (transfer full): An ordered list of string keys in this index.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_index_get_module_names_as_strv (ModulemdModuleIndex *self);


/**
 * modulemd_module_index_get_module:
 * @self: This #ModulemdModuleIndex
 * @module_name: The module name to look up in the index.
 *
 * Returns: (transfer none): The #ModulemdModule object matching the provided
 * module name or NULL if the key was not present in the index.
 *
 * Since: 2.0
 */
ModulemdModule *
modulemd_module_index_get_module (ModulemdModuleIndex *self,
                                  const gchar *module_name);


/**
 * modulemd_module_index_add_module_stream:
 * @self: This #ModulemdModuleIndex
 * @stream: The #ModulemdModuleStream to add to the index. The stream added
 * must have a module name and stream name set on it or it will be rejected.
 * @error: (out): A #GError containing the reason the #ModulemdModuleStream
 * object could not be added or NULL if the function succeeded.
 *
 * Returns: TRUE if the #ModulemdModule was added succesfully. If the stream
 * already existed in the index, it will be replaced by the new one. On
 * failure, returns FALSE and sets error appropriately.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_index_add_module_stream (ModulemdModuleIndex *self,
                                         ModulemdModuleStream *stream,
                                         GError **error);


/**
 * modulemd_module_index_add_defaults:
 * @self: This #ModulemdModuleIndex
 * @defaults: The #ModulemdDefaults object to add to the index.
 * @error: (out): A #GError containing the reason the #ModulemdDefaults object
 * could not be added or NULL if the function succeeded.
 *
 * Returns: TRUE if the #ModulemdDefaults was added succesfully. If the defaults
 * already existed in the index, it will be replaced by the new one. On failure,
 * returns FALSE and sets error approriately.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_index_add_defaults (ModulemdModuleIndex *self,
                                    ModulemdDefaults *defaults,
                                    GError **error);


/**
 * modulemd_index_add_translation:
 * @self: This #ModulemdModuleIndex
 * @translation: The #ModulemdTranslation object to add to the index.
 * @error: (out): A #GError containing the reason the #ModulemdTranslation
 * object could not be added or NULL if the function succeeded.
 *
 * Returns: TRUE if the #ModulemdTranslation was added succesfully. If the
 * translation already existed in the index, it will be replaced by the new one.
 * On failure, returns FALSE and sets error appropriately.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_index_add_translation (ModulemdModuleIndex *self,
                                       ModulemdTranslation *translation,
                                       GError **error);

/**
 * modulemd_module_index_get_defaults_mdversion:
 * @self: This #ModulemdModuleIndex
 *
 * Returns: The metadata version of #ModulemdDefaults in use for this index.
 *
 * Since: 2.0
 */
ModulemdDefaultsVersionEnum
modulemd_module_index_get_defaults_mdversion (ModulemdModuleIndex *self);


/**
 * modulemd_module_index_get_stream_mdversion:
 * @self: This #ModulemdModuleIndex
 *
 * Returns: The metadata version of #ModulemdModuleStream in use for this
 * index.
 *
 * Since: 2.0
 */
ModulemdModuleStreamVersionEnum
modulemd_module_index_get_stream_mdversion (ModulemdModuleIndex *self);

G_END_DECLS
