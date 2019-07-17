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

#ifndef MODULEMD_PROFILE_H
#define MODULEMD_PROFILE_H

#include "modulemd-deprecated.h"
#include "modulemd-prioritizer.h"
#include "modulemd-simpleset.h"

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-profile
 * @title: Modulemd.Profile
 * @short_description: Stores profile information for a module stream.
 */

#define MODULEMD_TYPE_PROFILE modulemd_profile_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdProfile, modulemd_profile, MODULEMD, PROFILE, GObject)


/**
 * modulemd_profile_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdProfile. This object
 * must be freed with g_object_unref().
 *
 * Since: 1.0
 */
ModulemdProfile *
modulemd_profile_new (void);


/**
 * modulemd_profile_set_description:
 * @description: (nullable): the profile description.
 *
 * Sets the "description" property.
 *
 * Since: 1.0
 */
void
modulemd_profile_set_description (ModulemdProfile *self,
                                  const gchar *description);


/**
 * modulemd_profile_get_description:
 *
 * Retrieves the profile description.
 *
 * Returns: A string containing the "description" property.
 *
 * Deprecated: 1.1
 * Use peek_description() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_profile_peek_description)
const gchar *
modulemd_profile_get_description (ModulemdProfile *self);

/**
 * modulemd_profile_get_localized_description:
 * @locale: (transfer none) (nullable): Specify the locale for the description.
 * If NULL is passed, it will attempt to use the LC_MESSAGES locale. If "C" is
 * passed or if the locale has no translation available, it will treat it as
 * untranslated.
 *
 * Returns: (transfer full): A string containing the "description" property,
 * translated into the language specified by @locale if possible. This string
 * must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_profile_get_localized_description (ModulemdProfile *self,
                                            const gchar *locale);


/**
 * modulemd_profile_peek_description:
 *
 * Retrieves the profile description.
 *
 * Returns: A string containing the "description" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_profile_peek_description (ModulemdProfile *self);


/**
 * modulemd_profile_dup_description:
 *
 * Retrieves a copy of the profile description.
 *
 * Returns: A copy of the string containing the "description" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_profile_dup_description (ModulemdProfile *self);


/**
 * modulemd_profile_set_name:
 * @name: (nullable): the profile name.
 *
 * Sets the "name" property.
 *
 * Since: 1.0
 */
void
modulemd_profile_set_name (ModulemdProfile *self, const gchar *name);


/**
 * modulemd_profile_get_name:
 *
 * Retrieves the profile name.
 *
 * Returns: A string containing the "name" property.
 *
 * Deprecated: 1.1
 * Use peek_name() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_profile_peek_name)
const gchar *
modulemd_profile_get_name (ModulemdProfile *self);


/**
 * modulemd_profile_peek_name:
 *
 * Retrieves the profile name.
 *
 * Returns: A string containing the "name" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_profile_peek_name (ModulemdProfile *self);


/**
 * modulemd_profile_dup_name:
 *
 * Retrieves a copy of the profile name.
 *
 * Returns: A copy of string containing the "name" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_profile_dup_name (ModulemdProfile *self);


/**
 * modulemd_profile_set_rpms:
 * @rpms: (nullable): A #ModulemdSimpleSet: The rpms to be installed by this
 * profile.
 *
 * Assigns the set of RPMs that will be installed when this profile is
 * activated.
 *
 * Since: 1.0
 */
void
modulemd_profile_set_rpms (ModulemdProfile *self, ModulemdSimpleSet *rpms);


/**
 * modulemd_profile_get_rpms:
 *
 * Retrieves the "rpms" for this profile
 *
 * Returns: (transfer none): a #SimpleSet containing the set of RPMs in the
 * "rpms" property.
 *
 * Deprecated: 1.1
 * Use peek_rpms() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_profile_peek_rpms)
ModulemdSimpleSet *
modulemd_profile_get_rpms (ModulemdProfile *self);


/**
 * modulemd_profile_peek_rpms:
 *
 * Retrieves the "rpms" for this profile
 *
 * Returns: (transfer none): a #SimpleSet containing the set of RPMs in the
 * "rpms" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_profile_peek_rpms (ModulemdProfile *self);


/**
 * modulemd_profile_dup_rpms:
 *
 * Retrieves a copy of the "rpms" for this profile
 *
 * Returns: (transfer full): a #SimpleSet containing the set of RPMs in the
 * "rpms" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_profile_dup_rpms (ModulemdProfile *self);


/**
 * modulemd_profile_add_rpm:
 * @rpm: (transfer none) (not nullable): An RPM that will be installed as part
 * of this profile.
 *
 * Since: 1.1
 */
void
modulemd_profile_add_rpm (ModulemdProfile *self, const gchar *rpm);


/**
 * modulemd_profile_remove_rpm:
 * @rpm: (transfer none) (not nullable): An RPM that will no longer be installed
 * as part of this profile.
 *
 * Since: 1.1
 */
void
modulemd_profile_remove_rpm (ModulemdProfile *self, const gchar *rpm);


/**
 * modulemd_profile_copy:
 *
 * Creates a copy of this profile
 *
 * Returns: (transfer full): a copy of this #ModulemdProfile
 *
 * Since: 1.1
 */
ModulemdProfile *
modulemd_profile_copy (ModulemdProfile *self);

G_END_DECLS

#endif /* MODULEMD_PROFILE_H */
