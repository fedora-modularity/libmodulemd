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

#include "modulemd-defaults.h"
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-defaults-v1
 * @title: Modulemd.DefaultsV1
 * @stability: stable
 * @short_description: Object representing a defaults document (version 1)
 */

#define MODULEMD_TYPE_DEFAULTS_V1 (modulemd_defaults_v1_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdDefaultsV1,
                      modulemd_defaults_v1,
                      MODULEMD,
                      DEFAULTS_V1,
                      ModulemdDefaults)


/**
 * modulemd_defaults_v1_new:
 * @module_name: (in): The name of the module to which these defaults apply.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdDefaultsV1 object.
 *
 * Since: 2.0
 */
ModulemdDefaultsV1 *
modulemd_defaults_v1_new (const gchar *module_name);


/**
 * modulemd_defaults_v1_set_default_stream:
 * @self: (in): This #ModulemdDefaultsV1 object.
 * @default_stream: (in) (nullable): The name of the default stream for this
 * module. If NULL, it will remove the default stream.
 * @intent: (in) (nullable): If non-NULL, this indicates the system-intent to
 * apply this default stream. If NULL, it will be added as common defaults.
 *
 * Set the default stream for this module.
 *
 * Since: 2.0
 */
void
modulemd_defaults_v1_set_default_stream (ModulemdDefaultsV1 *self,
                                         const gchar *default_stream,
                                         const gchar *intent);


/**
 * modulemd_defaults_v1_get_default_stream:
 * @self: (in): This #ModulemdDefaultsV1 object.
 * @intent: (in) (nullable): The name of the system intent whose default stream
 * will be retrieved. If left NULL or the specified intent has no different
 * default, it will return the generic default stream for this module.
 *
 * Returns: (transfer none): The name of the default stream for this module.
 *
 * Since: 2.0
 */
const gchar *
modulemd_defaults_v1_get_default_stream (ModulemdDefaultsV1 *self,
                                         const gchar *intent);


/**
 * modulemd_defaults_v1_get_streams_with_default_profiles_as_strv: (rename-to modulemd_defaults_v1_get_streams_with_default_profiles)
 * @self: (in): This #ModulemdDefaultsV1 object.
 * @intent: (in) (nullable): The name of the system intent whose stream
 * profiles will be retrieved. If left NULL or the specified intent has no
 * separate defaults for this module, it will return the generic stream
 * profiles.
 *
 * Returns: (transfer full): A sorted #GStrv list of unique stream names for
 * which default profiles have been assigned.
 *
 * Since: 2.0
 */
GStrv
modulemd_defaults_v1_get_streams_with_default_profiles_as_strv (
  ModulemdDefaultsV1 *self, const gchar *intent);


/**
 * modulemd_defaults_v1_add_default_profile_for_stream:
 * @self: (in): This #ModulemdDefaultsV1 object.
 * @stream_name: (in): The name of the module stream to which to add this
 * default profile.
 * @profile_name: (in): The name of the default profile to add.
 * @intent: (in) (nullable): The name of the system intent to add profile
 * defaults to. If NULL, this sets the generic fallback profiles for the
 * stream.
 *
 * Add a profile that will be installed for this stream if none are explicitly
 * specified by the user. This function may be called any number of times for
 * the same stream and will deduplicate input.
 *
 * Since: 2.0
 */
void
modulemd_defaults_v1_add_default_profile_for_stream (ModulemdDefaultsV1 *self,
                                                     const gchar *stream_name,
                                                     const gchar *profile_name,
                                                     const gchar *intent);


/**
 * modulemd_defaults_v1_set_empty_default_profiles_for_stream:
 * @self: (in): This #ModulemdDefaultsV1 object.
 * @stream_name: (in): The name of the module stream for which to empty
 * default profiles.
 * @intent: (in) (nullable): The name of the system intent from which to clear
 * the profile defaults for this stream.
 *
 * Sets the default profiles for @stream_name to the empty set. When output to
 * a file, it will appear as `stream_name: []`.
 *
 * Since: 2.0
 */
void
modulemd_defaults_v1_set_empty_default_profiles_for_stream (
  ModulemdDefaultsV1 *self, const gchar *stream_name, const gchar *intent);

/**
 * modulemd_defaults_v1_remove_default_profiles_for_stream:
 * @self: (in): This #ModulemdDefaultsV1 object.
 * @stream_name: (in): The name of the module stream from which to remove
 * default profiles.
 * @intent: (in) (nullable): The name of the system intent from which to remove
 * the profile defaults for this stream.
 *
 * Removes this stream from the list of profiles entirely. It will not appear
 * in the output document.
 *
 * Since: 2.0
 */
void
modulemd_defaults_v1_remove_default_profiles_for_stream (
  ModulemdDefaultsV1 *self, const gchar *stream_name, const gchar *intent);


/**
 * modulemd_defaults_v1_get_default_profiles_for_stream_as_strv: (rename-to modulemd_defaults_v1_get_default_profiles_for_stream)
 * @self: (in): This #ModulemdDefaultsV1 object.
 * @stream_name: (in): The name of the string to retrieve the default profiles
 * for.
 * @intent: (in) (nullable): The name of the system intent from which to
 * retrieve the profile defaults for this stream.
 *
 * Returns: (transfer full): A sorted #GStrv list of unique profiles to be
 * installed by default for this stream. NULL, if this stream_name is not
 * present in the defaults.
 *
 * Since: 2.0
 */
GStrv
modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
  ModulemdDefaultsV1 *self, const gchar *stream_name, const gchar *intent);

G_END_DECLS
