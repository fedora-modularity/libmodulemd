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

#ifndef MODULEMD_H
#define MODULEMD_H

/**
 * SECTION: modulemd
 * @title: Modulemd
 * @short_description: Functions for managing Module Metadata
 */


#include <glib.h>
#include <glib-object.h>
#include <stdio.h>

#include "modulemd-buildopts.h"
#include "modulemd-component.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-defaults.h"
#include "modulemd-dependencies.h"
#include "modulemd-deprecated.h"
#include "modulemd-improvedmodule.h"
#include "modulemd-intent.h"
#include "modulemd-module.h"
#include "modulemd-modulestream.h"
#include "modulemd-prioritizer.h"
#include "modulemd-profile.h"
#include "modulemd-simpleset.h"
#include "modulemd-servicelevel.h"
#include "modulemd-subdocument.h"
#include "modulemd-translation.h"
#include "modulemd-translation-entry.h"

G_BEGIN_DECLS

/**
 * modulemd_get_version:
 *
 * Returns: The version of libmodulemd.
 *
 * Since: 1.5
 */
const gchar *
modulemd_get_version (void);


/**
 * modulemd_objects_from_file:
 * @yaml_file: A YAML file containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.2
 */
GPtrArray *
modulemd_objects_from_file (const gchar *yaml_file, GError **error);


/**
 * modulemd_objects_from_file_ext:
 * @yaml_file: A YAML file containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object). This array must be freed with g_ptr_array_unref().
 *
 * Since: 1.4
 */
GPtrArray *
modulemd_objects_from_file_ext (const gchar *yaml_file,
                                GPtrArray **failures,
                                GError **error);


/**
 * modulemd_index_from_file:
 * @yaml_string: A YAML file containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GHashTable of module data from a file.
 *
 * Returns: (element-type utf8 ModulemdImprovedModule) (transfer container):
 * A #GHashTable containing all of the subdocuments from a YAML file, indexed
 * by module name. This hash table must be freed with g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_index_from_file (const gchar *yaml_file,
                          GPtrArray **failures,
                          GError **error);


/**
 * modulemd_objects_from_string:
 * @yaml_string: A YAML string containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.2
 */
GPtrArray *
modulemd_objects_from_string (const gchar *yaml_string, GError **error);


/**
 * modulemd_objects_from_string_ext:
 * @yaml_string: A YAML string containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.4
 */
GPtrArray *
modulemd_objects_from_string_ext (const gchar *yaml_string,
                                  GPtrArray **failures,
                                  GError **error);


/**
 * modulemd_index_from_string:
 * @yaml_string: A YAML string containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML string that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GHashTable of module data from a string.
 *
 * Returns: (element-type utf8 ModulemdImprovedModule) (transfer container):
 * A #GHashTable containing all of the subdocuments from a YAML file, indexed
 * by module name. This hash table must be freed with g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_index_from_string (const gchar *yaml_string,
                            GPtrArray **failures,
                            GError **error);


/**
 * modulemd_objects_from_stream:
 * @stream: A YAML stream containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.4
 */
GPtrArray *
modulemd_objects_from_stream (FILE *stream, GError **error);


/**
 * modulemd_objects_from_stream_ext:
 * @stream: A YAML stream containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.4
 */
GPtrArray *
modulemd_objects_from_stream_ext (FILE *stream,
                                  GPtrArray **failures,
                                  GError **error);


/**
 * modulemd_index_from_stream:
 * @yaml_stream: A YAML stream containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML stream that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GHashTable of module data from a stream.
 *
 * Returns: (element-type utf8 ModulemdImprovedModule) (transfer container):
 * A #GHashTable containing all of the subdocuments from a YAML file, indexed
 * by module name. This hash table must be freed with g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_index_from_stream (FILE *yaml_stream,
                            GPtrArray **failures,
                            GError **error);


/**
 * modulemd_dump_index:
 * @index: (element-type utf8 ModulemdImprovedModule) (transfer none): The index
 * of #ModulemdImprovedModule objects to dump to a YAML file.
 * @yaml_file: (transfer none): The path to the file that should contain the
 * resulting YAML.
 *
 * Returns: TRUE if the file was written successfully. In the event of an error,
 * sets @error appropriately and returns FALSE.
 *
 * Since: 1.6
 */
gboolean
modulemd_dump_index (GHashTable *index,
                     const gchar *yaml_file,
                     GError **error);


/**
 * modulemd_dumps_index:
 * @index: (element-type utf8 ModulemdImprovedModule) (transfer none): The index
 * of #ModulemdImprovedModule objects to dump to a string.
 *
 * Returns: A YAML representation of the index as a string, which must be freed
 * with g_free(). In the event of an error, sets @error appropriately and
 * returns NULL.
 *
 * Since: 1.6
 */
gchar *
modulemd_dumps_index (GHashTable *index, GError **error);


/**
 * modulemd_dump:
 * @objects: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * modulemd or related objects to dump to YAML.
 * @yaml_file: The path to the file that should contain the resulting YAML
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Creates a file containing a series of YAML subdocuments, one per object
 * passed in.
 *
 * Since: 1.2
 */
void
modulemd_dump (GPtrArray *objects, const gchar *yaml_file, GError **error);


/**
 * modulemd_dumps:
 * @objects: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * modulemd or related objects to dump to YAML.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Creates a string containing a series of YAML subdocuments, one per object
 * passed in. This string must be freed with g_free() when no longer needed.
 *
 * Since: 1.2
 */
gchar *
modulemd_dumps (GPtrArray *objects, GError **error);


/**
 * modulemd_merge_defaults:
 * @first: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * modulemd-related objects.
 * @second: (array zero-terminated=1) (element-type GObject) (nullable):
 * Optional. A #GPtrArray of modulemd-related objects to be merged into the
 * first list.
 * @override: Whether entries in @second should override those of @first in the
 * event of a conflict or whether they should attempt to merge instead.
 *
 * This function will process the lists of objects, merging duplicated
 * modulemd-defaults objects as needed. If the object lists have different
 * priorities, the conflicting values will be replaced by the ones from the
 * higher-priority list rather than merged.
 *
 * This function will return an error if the merge cannot be completed safely.
 *
 * Returns: (element-type GObject) (transfer container): A list of
 * module-related objects with defaults deduplicated and merged. This array is
 * newly-allocated and must be freed with g_ptr_array_unref().
 *
 * Since: 1.3
 */
GPtrArray *
modulemd_merge_defaults (const GPtrArray *first,
                         const GPtrArray *second,
                         gboolean override,
                         GError **error);

G_END_DECLS

#endif /* MODULEMD_H */
