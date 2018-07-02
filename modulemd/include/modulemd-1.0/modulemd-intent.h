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

#include "modulemd.h"

#include "modulemd-simpleset.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-intent
 * @title: Modulemd.Intent
 * @short_description: Overrides #ModulemdDefaults when a specific system
 * "purpose" has been set.
 */

#define MODULEMD_TYPE_INTENT (modulemd_intent_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdIntent, modulemd_intent, MODULEMD, INTENT, GObject)


/**
 * modulemd_intent_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdIntent object. This must
 * be freed with g_object_unref().
 *
 * Since: 1.5
 */
ModulemdIntent *
modulemd_intent_new (const gchar *name);


/**
 * modulemd_intent_set_intent_name:
 * @name: (not nullable): The module name to which these defaults apply
 *
 * Sets the "module-name" property.
 *
 * Since: 1.5
 */
void
modulemd_intent_set_intent_name (ModulemdIntent *self, const gchar *name);


/**
 * modulemd_intent_peek_intent_name:
 *
 * Retrieves the module name to which these defaults apply.
 *
 * Returns: a string containing the "module-name" property. This string must
 * not be modified or freed. If you need to do so, use
 * modulemd_intent_dup_intent_name() instead.
 *
 * Since: 1.5
 */
const gchar *
modulemd_intent_peek_intent_name (ModulemdIntent *self);


/**
 * modulemd_intent_dup_intent_name:
 *
 * Retrieves the module name to which these defaults apply.
 *
 * Returns: a string containing the "module-name" property. This string must be
 * freed with g_free() when the caller is done with it.
 *
 * Since: 1.5
 */
gchar *
modulemd_intent_dup_intent_name (ModulemdIntent *self);


/**
 * modulemd_intent_set_default_stream:
 * @stream: The default stream for this module
 *
 * Sets the "default-stream" property.
 *
 * Since: 1.5
 */
void
modulemd_intent_set_default_stream (ModulemdIntent *self, const gchar *stream);


/**
 * modulemd_intent_peek_default_stream:
 *
 * Retrieves the default stream.
 *
 * Returns: a string containing the "default-stream" property. This string
 * must not be modified or freed. If you need to do so, use
 * modulemd_intent_dup_default_stream() instead.
 *
 * Since: 1.5
 */
const gchar *
modulemd_intent_peek_default_stream (ModulemdIntent *self);


/**
 * modulemd_intent_dup_default_stream:
 *
 * Retrieves the default stream.
 *
 * Returns: a string containing the "default-stream" property. This string must
 * be freed with g_free() when the caller is done with it.
 *
 * Since: 1.5
 */
gchar *
modulemd_intent_dup_default_stream (ModulemdIntent *self);


/**
 * modulemd_intent_set_profiles_for_stream:
 * @stream: The name of the stream getting default profiles
 * @profiles: (array zero-terminated=1) (transfer none): The set of profile
 * names to install by default when installing this stream of the module.
 *
 * Since: 1.5
 */
void
modulemd_intent_set_profiles_for_stream (ModulemdIntent *self,
                                         const gchar *stream,
                                         gchar **profiles);

/**
 * modulemd_intent_assign_profiles_for_stream:
 * @stream: The name of the stream getting default profiles
 * @profiles: A #ModulemdSimpleSet of profile names to install by default when
 * installing this stream of the module.
 *
 * Since: 1.5
 */
void
modulemd_intent_assign_profiles_for_stream (ModulemdIntent *self,
                                            const gchar *stream,
                                            ModulemdSimpleSet *profiles);


/**
 * modulemd_intent_set_profile_defaults:
 * @profile_defaults: (nullable) (element-type utf8 ModulemdSimpleSet) (transfer none):
 *
 * Assigns the hash table of streams and their default profiles
 *
 * Since: 1.5
 */
void
modulemd_intent_set_profile_defaults (ModulemdIntent *self,
                                      GHashTable *profile_defaults);


/**
 * modulemd_intent_dup_profiles_for_stream:
 * @stream: The name of the stream from which to retrieve defaults
 *
 * Returns: (array zero-terminated=1) (transfer full): A zero-terminated array
 * of strings that provides the list of profiles that should be installed by
 * default when this stream is specified.
 *
 * Since: 1.5
 */
gchar **
modulemd_intent_dup_profiles_for_stream (ModulemdIntent *self,
                                         const gchar *stream);


/**
 * modulemd_intent_peek_profile_defaults:
 *
 * Retrieves a hash table of the profile defaults.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A GHashTable
 * containing the set of profile defaults for streams of this module. This hash
 * table is maintained by the ModulemdIntent object and must not be freed or
 * modified. If modification is necessary, use
 * modulemd_intent_dup_profile_defaults() instead.
 *
 * Since: 1.5
 */
GHashTable *
modulemd_intent_peek_profile_defaults (ModulemdIntent *self);


/**
 * modulemd_intent_dup_profile_defaults:
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
 * Since: 1.5
 */
GHashTable *
modulemd_intent_dup_profile_defaults (ModulemdIntent *self);


/**
 * modulemd_intent_copy:
 *
 * Makes a deep copy of a #ModulemdIntent
 *
 * Returns: (transfer full): A deep copy of the #ModulemdIntent
 *
 * Since: 1.5
 */
ModulemdIntent *
modulemd_intent_copy (ModulemdIntent *self);

G_END_DECLS
