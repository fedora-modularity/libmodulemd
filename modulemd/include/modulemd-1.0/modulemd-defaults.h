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

#ifndef MODULEMD_DEFAULTS_H
#define MODULEMD_DEFAULTS_H

#include <stdio.h>

#include "modulemd-intent.h"
#include "modulemd-simpleset.h"

#include <glib-object.h>

enum
{
  MD_DEFAULTS_VERSION_UNSET = 0,

  MD_DEFAULTS_VERSION_1 = 1,

  MD_DEFAULTS_VERSION_MAX = G_MAXUINT64
};

#define MD_DEFAULTS_VERSION_LATEST MD_DEFAULTS_VERSION_1

G_BEGIN_DECLS

/**
 * SECTION: modulemd-defaults
 * @title: Modulemd.Defaults
 * @short_description: Represents the default stream, profiles and other global
 * data for a module.
 */

#define MODULEMD_DEFAULTS_ERROR modulemd_defaults_error_quark ()
GQuark
modulemd_defaults_error_quark (void);

enum ModulemdDefaultsError
{
  MODULEMD_DEFAULTS_ERROR_MISSING_CONTENT,
  MODULEMD_DEFAULTS_ERROR_CONFLICTING_STREAMS,
  MODULEMD_DEFAULTS_ERROR_CONFLICTING_PROFILES,
  MODULEMD_DEFAULTS_ERROR_CONFLICTING_INTENT_STREAM,
  MODULEMD_DEFAULTS_ERROR_CONFLICTING_INTENT_PROFILE
};

#define MODULEMD_TYPE_DEFAULTS (modulemd_defaults_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdDefaults, modulemd_defaults, MODULEMD, DEFAULTS, GObject)


/**
 * modulemd_defaults_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdDefaults object. This
 * must be freed with g_object_unref().
 *
 * Since: 1.1
 */
ModulemdDefaults *
modulemd_defaults_new (void);


/**
 * modulemd_defaults_set_version:
 * @version: The metadata file format version
 *
 * Sets the version of the metadata in use.
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_version (ModulemdDefaults *self, guint64 version);


/**
 * modulemd_defaults_peek_version:
 *
 * Retrieves the metadata file format version.
 *
 * Returns: a 64-bit unsigned integer containing the file format version.
 *
 * Since: 1.1
 */
guint64
modulemd_defaults_peek_version (ModulemdDefaults *self);


/**
 * modulemd_defaults_set_module_name:
 * @name: The module name to which these defaults apply
 *
 * Sets the "module-name" property.
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_module_name (ModulemdDefaults *self, const gchar *name);


/**
 * modulemd_defaults_peek_module_name:
 *
 * Retrieves the module name to which these defaults apply.
 *
 * Returns: a string containing the "module-name" property. This string must
 * not be modified or freed. If you need to do so, use
 * modulemd_defaults_dup_module_name() instead.
 *
 * Since: 1.1
 */
const gchar *
modulemd_defaults_peek_module_name (ModulemdDefaults *self);


/**
 * modulemd_defaults_dup_module_name:
 *
 * Retrieves the module name to which these defaults apply.
 *
 * Returns: a string containing the "module-name" property. This string must be
 * freed with g_free() when the caller is done with it.
 *
 * Since: 1.1
 */
gchar *
modulemd_defaults_dup_module_name (ModulemdDefaults *self);


/**
 * modulemd_defaults_set_default_stream:
 * @stream: The default stream for this module
 *
 * Sets the "default-stream" property.
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_default_stream (ModulemdDefaults *self,
                                      const gchar *stream);


/**
 * modulemd_defaults_peek_default_stream:
 *
 * Retrieves the default stream.
 *
 * Returns: a string containing the "default-stream" property. This string
 * must not be modified or freed. If you need to do so, use
 * modulemd_defaults_dup_default_stream() instead.
 *
 * Since: 1.1
 */
const gchar *
modulemd_defaults_peek_default_stream (ModulemdDefaults *self);


/**
 * modulemd_defaults_dup_default_stream:
 *
 * Retrieves the default stream.
 *
 * Returns: a string containing the "default-stream" property. This string must
 * be freed with g_free() when the caller is done with it.
 *
 * Since: 1.1
 */
gchar *
modulemd_defaults_dup_default_stream (ModulemdDefaults *self);


/**
 * modulemd_defaults_set_profiles_for_stream:
 * @stream: The name of the stream getting default profiles
 * @profiles: (array zero-terminated=1) (transfer none): The set of profile
 * names to install by default when installing this stream of the module.
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_profiles_for_stream (ModulemdDefaults *self,
                                           const gchar *stream,
                                           gchar **profiles);


/**
 * modulemd_defaults_assign_profiles_for_stream:
 * @stream: The name of the stream getting default profiles
 * @profiles: A #ModulemdSimpleSet of profile names to install by default when
 * installing this stream of the module.
 *
 * Since: 1.1
 */
void
modulemd_defaults_assign_profiles_for_stream (ModulemdDefaults *self,
                                              const gchar *stream,
                                              ModulemdSimpleSet *profiles);


/**
 * modulemd_defaults_set_profile_defaults:
 * @profile_defaults: (nullable) (element-type utf8 ModulemdSimpleSet) (transfer none):
 *
 * Assigns the hash table of streams and their default profiles
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_profile_defaults (ModulemdDefaults *self,
                                        GHashTable *profile_defaults);


/**
 * modulemd_defaults_dup_profiles_for_stream:
 * @stream: The name of the stream from which to retrieve defaults
 *
 * Returns: (array zero-terminated=1) (transfer full): A zero-terminated array
 * of strings that provides the list of profiles that should be installed by
 * default when this stream is specified.
 *
 * Since: 1.1
 */
gchar **
modulemd_defaults_dup_profiles_for_stream (ModulemdDefaults *self,
                                           const gchar *stream);


/**
 * modulemd_defaults_peek_profile_defaults:
 *
 * Retrieves a hash table of the profile defaults.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A GHashTable
 * containing the set of profile defaults for streams of this module. This hash
 * table is maintained by the ModulemdDefaults object and must not be freed or
 * modified. If modification is necessary, use
 * modulemd_defaults_dup_profile_defaults() instead.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_defaults_peek_profile_defaults (ModulemdDefaults *self);


/**
 * modulemd_defaults_dup_profile_defaults:
 *
 * Retrieves a copy of the hash table of profile defaults.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer container): A
 * GHashTable containing the set of profile defaults for streams of this
 * module. This hash table is a copy and must be freed with
 * g_hash_table_unref() when the caller is finished with it. The elements it
 * contains are maintained by the hash table and will be automatically freed
 * when their key is removed or the hash table is freed.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_defaults_dup_profile_defaults (ModulemdDefaults *self);


/**
 * modulemd_defaults_add_intent:
 * @intent: (transfer none) (not nullable): The #ModulemdIntent to add to the
 * intents table.
 *
 * Adds an intent object to the hash table.
 *
 * Since: 1.5
 */
void
modulemd_defaults_add_intent (ModulemdDefaults *self, ModulemdIntent *intent);


/**
 * modulemd_defaults_set_intents:
 * @intents: (element-type utf8 ModulemdIntent) (nullable) (transfer none):
 * A #GHashTable containing defaults for individual system intents.
 *
 * Since: 1.5
 */
void
modulemd_defaults_set_intents (ModulemdDefaults *self, GHashTable *intents);


/**
 * modulemd_defaults_peek_intents:
 *
 * Get a pointer to the intents hash table. The returned table is managed by the
 * #ModulemdDefaults object and must not be modified or freed.
 *
 * Returns: (element-type utf8 ModulemdIntent) (transfer none): A pointer to the
 * intents hash table.
 *
 * Since: 1.5
 */
GHashTable *
modulemd_defaults_peek_intents (ModulemdDefaults *self);


/**
 * modulemd_defaults_dup_intents:
 *
 * Get a copy of the intents hash table. The returned table is managed by the
 * caller and must be freed with g_hash_table_unref()
 *
 * Returns: (element-type utf8 ModulemdIntent) (transfer container): A copy of
 * the intents hash table.
 *
 * Since: 1.5
 */
GHashTable *
modulemd_defaults_dup_intents (ModulemdDefaults *self);


/**
 * modulemd_defaults_set_modified:
 *
 * Sets the modified field for these defaults.
 *
 * Since: 1.8
 */
void
modulemd_defaults_set_modified (ModulemdDefaults *self, guint64 modified);


/**
 * modulemd_defaults_get_modified:
 *
 * Returns: The modified field for these defaults.
 *
 * Since: 1.8
 */
guint64
modulemd_defaults_get_modified (ModulemdDefaults *self);


/**
 * modulemd_defaults_new_from_file:
 * @yaml_file: A YAML file containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.2
 */
ModulemdDefaults *
modulemd_defaults_new_from_file (const gchar *yaml_file, GError **error);


/**
 * modulemd_defaults_new_from_file_ext:
 * @yaml_file: A YAML file containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.4
 */
ModulemdDefaults *
modulemd_defaults_new_from_file_ext (const gchar *yaml_file,
                                     GPtrArray **failures,
                                     GError **error);


/**
 * modulemd_defaults_new_from_string:
 * @yaml_string: A YAML string containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.2
 */
ModulemdDefaults *
modulemd_defaults_new_from_string (const gchar *yaml_string, GError **error);


/**
 * modulemd_defaults_new_from_string_ext:
 * @yaml_string: A YAML string containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.4
 */
ModulemdDefaults *
modulemd_defaults_new_from_string_ext (const gchar *yaml_string,
                                       GPtrArray **failures,
                                       GError **error);


/**
 * modulemd_defaults_new_from_stream:
 * @stream: A YAML stream containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.4
 */
ModulemdDefaults *
modulemd_defaults_new_from_stream (FILE *stream, GError **error);


/**
 * modulemd_defaults_new_from_stream_ext:
 * @stream: A YAML stream containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.4
 */
ModulemdDefaults *
modulemd_defaults_new_from_stream_ext (FILE *stream,
                                       GPtrArray **failures,
                                       GError **error);


/**
 * modulemd_defaults_dump:
 * @file_path: File path for exporting the YAML representation of this defaults
 * object
 *
 * Exports the YAML representation of this defaults object to a file.
 *
 * Since: 1.1
 */
void
modulemd_defaults_dump (ModulemdDefaults *self, const gchar *file_path);


/**
 * modulemd_defaults_dumps:
 * @yaml_string: (out): File path for exporting the YAML representation of this defaults
 * object
 *
 * Exports the YAML representation of this defaults object to a string. The
 * caller is responsible for calling g_free() on this string when they are
 * finished with it.
 *
 * Since: 1.1
 */
void
modulemd_defaults_dumps (ModulemdDefaults *self, gchar **yaml_string);


/**
 * modulemd_defaults_copy:
 * Returns a deep-copy of the defaults object.
 *
 * Returns: (transfer full): A deep-copied #ModulemdDefaults object. This
 * object must be freed with g_object_unref().
 *
 * Since: 1.3
 */
ModulemdDefaults *
modulemd_defaults_copy (ModulemdDefaults *self);


/**
 * modulemd_defaults_merge:
 * @first: A #ModulemdDefaults object providing the base for the merge.
 * @second: A #ModulemdDefaults object being merged onto @first.
 * @override: In the case of a conflict, should @second completely replace the
 * contents of @first.
 *
 * Returns: (transfer full): A merged or replaced #ModulemdDefaults object. In
 * case of unresolvable merge, NULL will be returned and an error will be set.
 * This object must be freed with g_object_unref().
 *
 * Since: 1.3
 */
ModulemdDefaults *
modulemd_defaults_merge (ModulemdDefaults *first,
                         ModulemdDefaults *second,
                         gboolean override,
                         GError **error);

G_END_DECLS

#endif /* MODULEMD_DEFAULTS_H */
