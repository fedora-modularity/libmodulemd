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
 * @self_1: A #ModulemdProfile object.
 * @self_2: A #ModulemdProfile object.
 *
 * Returns: TRUE, if all elements of @self_1 and @self_2 are equal. FALSE,
 * otherwise.
 *
 * Since: 2.2
 */
gboolean
modulemd_profile_equals (ModulemdProfile *self_1, ModulemdProfile *self_2);


/**
 * modulemd_profile_new:
 * @name: (not nullable): The name of this profile.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdProfile object. This
 * object must be freed with g_object_unref().
 *
 * Since: 2.0
 */
ModulemdProfile *
modulemd_profile_new (const gchar *name);


/**
 * modulemd_profile_copy:
 * @self: This #ModulemdProfile object.
 *
 * Create a copy of this #ModulemdProfile object.
 *
 * Returns: (transfer full): The copied #ModulemdProfile object.
 *
 * Since: 2.0
 */
ModulemdProfile *
modulemd_profile_copy (ModulemdProfile *self);


/**
 * modulemd_profile_get_name:
 * @self: This #ModulemdProfile object.
 *
 * Returns: (transfer none): The name of this profile.
 *
 * Since: 2.0
 */
const gchar *
modulemd_profile_get_name (ModulemdProfile *self);


/**
 * modulemd_profile_set_description:
 * @self: This #ModulemdProfile object.
 * @description: (nullable): The untranslated description of this profile.
 *
 * Since: 2.0
 */
void
modulemd_profile_set_description (ModulemdProfile *self,
                                  const gchar *description);


/**
 * modulemd_profile_get_description:
 * @self: This #ModulemdProfile object.
 * @locale: (in) (nullable): The name of the locale to use when translating
 * the string. If NULL, it will determine the locale with a system call to
 * `setlocale(LC_MESSAGES, NULL)` and return that. If the caller wants the
 * untranslated string, they should pass `"C"` for the locale.
 *
 * Returns: (transfer none): The description of this profile translated into
 * the language specified by the locale if it is available, otherwise it
 * returns the C.UTF-8 original. Translation information is managed by the
 * #ModulemdTranslation and #ModulemdTranslationEntry objects.
 *
 * Since: 2.0
 */
const gchar *
modulemd_profile_get_description (ModulemdProfile *self, const gchar *locale);


/**
 * modulemd_profile_set_default:
 * @self: This #ModulemdProfile object.
 *
 * Calling this function indicates that this profile should be considered one
 * of the default profiles for the associated stream.
 *
 * Since: 2.10
 */
void
modulemd_profile_set_default (ModulemdProfile *self);


/**
 * modulemd_profile_unset_default:
 * @self: This #ModulemdProfile object.
 *
 * Calling this function indicates that this profile should not be considered
 * one of the default profiles for this stream. This is the normal state of
 * a #ModulemdProfile and thus this function is usually unnecessary. It has no
 * effect if @self is already non-default.
 *
 * Since: 2.10
 */
void
modulemd_profile_unset_default (ModulemdProfile *self);


/**
 * modulemd_profile_is_default:
 * @self: This #ModulemdProfile object.
 *
 * Returns: TRUE if this profile is a default for the associated stream. FALSE
 * otherwise.
 *
 * Since: 2.10
 */
gboolean
modulemd_profile_is_default (ModulemdProfile *self);


/**
 * modulemd_profile_add_rpm:
 * @self: This #ModulemdProfile object.
 * @rpm: The name of a binary RPM that should be installed when this profile is
 * selected for installation.
 *
 * Since: 2.0
 */
void
modulemd_profile_add_rpm (ModulemdProfile *self, const gchar *rpm);


/**
 * modulemd_profile_remove_rpm:
 * @self: This #ModulemdProfile object.
 * @rpm: The name of a binary RPM to remove from this profile.
 *
 * Since: 2.0
 */
void
modulemd_profile_remove_rpm (ModulemdProfile *self, const gchar *rpm);


/**
 * modulemd_profile_clear_rpms:
 * @self: This #ModulemdProfile object.
 *
 * Remove all RPMs from this profile.
 *
 * Since: 2.5
 */
void
modulemd_profile_clear_rpms (ModulemdProfile *self);


/**
 * modulemd_profile_get_rpms_as_strv: (rename-to modulemd_profile_get_rpms)
 * @self: This #ModulemdProfile object.
 *
 * Returns: (transfer full): An ordered #GStrv list of binary RPMS that would be
 * installed when this profile is selected for installation.
 *
 * Since: 2.0
 */
GStrv
modulemd_profile_get_rpms_as_strv (ModulemdProfile *self);

G_END_DECLS
