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
 * SECTION: modulemd-profile
 * @title: Modulemd.Profile
 * @stability: stable
 * @short_description: Stores profile information for a module stream.
 */

#define MODULEMD_TYPE_PROFILE (modulemd_profile_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdProfile, modulemd_profile, MODULEMD, PROFILE, GObject)


/**
 * modulemd_profile_equals:
 * @self_1: A #ModulemdProfile
 * @self_2: A #ModulemdProfile
 *
 * Returns: TRUE if all elements of self_1 and self_2 are equal. FALSE, otherwise. 
 *
 * Since: 2.2
 */
gboolean
modulemd_profile_equals (ModulemdProfile *self_1, ModulemdProfile *self_2);


/**
 * modulemd_profile_new:
 * @name: (not nullable): The name of this profile.
 *
 * Returns: (transfer full): A newly-allocated #ModuleProfile. This object must
 * be freed with g_object_unref().
 *
 * Since: 2.0
 */
ModulemdProfile *
modulemd_profile_new (const gchar *name);


/**
 * modulemd_profile_copy:
 * @self: This #ModulemdProfile
 *
 * Create a copy of this #ModulemdProfile object.
 *
 * Returns: (transfer full): a copied #ModulemdProfile object
 *
 * Since: 2.0
 */
ModulemdProfile *
modulemd_profile_copy (ModulemdProfile *self);


/**
 * modulemd_profile_get_name:
 * @self: This #ModulemdProfile
 *
 * Returns: (transfer none): The name of this profile.
 *
 * Since: 2.0
 */
const gchar *
modulemd_profile_get_name (ModulemdProfile *self);


/**
 * modulemd_profile_set_description:
 * @self: This #ModulemdProfile
 * @description: (nullable): The description of this profile in the C locale.
 *
 * Since: 2.0
 */
void
modulemd_profile_set_description (ModulemdProfile *self,
                                  const gchar *description);


/**
 * modulemd_profile_get_description:
 * @self: This #ModulemdProfile
 * @locale: (in) (nullable): The name of the locale to use when translating
 * the string. If NULL, it will determine the locale with a system call to
 * setlocale(LC_MESSAGES, NULL) and return the that. If the caller wants the
 * untranslated string, they should pass "C" for the locale.
 *
 * Returns: (transfer none): The description of this profile translated into
 * the language specified by the locale if it is available, otherwise it
 * returns the C.UTF-8 original.
 *
 * Since: 2.0
 */
const gchar *
modulemd_profile_get_description (ModulemdProfile *self, const gchar *locale);


/**
 * modulemd_profile_add_rpm:
 * @self: This #ModulemdProfile
 * @rpm: The name of a binary RPM that should be installed when this profile is
 * selected for installation.
 *
 * Since: 2.0
 */
void
modulemd_profile_add_rpm (ModulemdProfile *self, const gchar *rpm);


/**
 * modulemd_profile_remove_rpm:
 * @self: This #ModulemdProfile
 * @rpm: The name of a binary RPM to remove from this profile.
 *
 * Since: 2.0
 */
void
modulemd_profile_remove_rpm (ModulemdProfile *self, const gchar *rpm);


/**
 * modulemd_profile_clear_rpms:
 * @self: This #ModulemdProfile
 *
 * Remove all RPMs from this profile
 *
 * Since: 2.5
 */
void
modulemd_profile_clear_rpms (ModulemdProfile *self);


/**
 * modulemd_profile_get_rpms_as_strv: (rename-to modulemd_profile_get_rpms)
 * @self: This #ModulemdProfile
 *
 * Returns: (transfer full): An ordered list of binary RPMS that would be
 * installed when this profile is selected for installation.
 *
 * Since: 2.0
 */
gchar **
modulemd_profile_get_rpms_as_strv (ModulemdProfile *self);

G_END_DECLS
