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
 * SECTION: modulemd-translation-entry
 * @title: Modulemd.TranslationEntry
 * @stability: stable
 * @short_description: Contains the translated strings of a module stream for a specific locale.
 */

#define MODULEMD_TYPE_TRANSLATION_ENTRY                                       \
  (modulemd_translation_entry_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdTranslationEntry,
                      modulemd_translation_entry,
                      MODULEMD,
                      TRANSLATION_ENTRY,
                      GObject)

/**
 * modulemd_translation_entry_new:
 * @locale: (not nullable): The locale for this translation entry.
 * It must correspond to the format specified by libc locale names.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdTranslationEntry. This
 * object must be freed with g_object_unref().
 *
 * Since: 2.0
 */
ModulemdTranslationEntry *
modulemd_translation_entry_new (const gchar *locale);


/**
 * modulemd_translation_entry_copy:
 * @self: This #ModulemdTranslationEntry
 *
 * Create a copy of this #ModulemdTranslationEntry object.
 *
 * Returns: (transfer full): a copied #ModulemdTranslationEntry object
 *
 * Since: 2.0
 */
ModulemdTranslationEntry *
modulemd_translation_entry_copy (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_get_locale:
 * @self: This #ModulemdTranslationEntry
 *
 * Get the locale of this translation entry.
 *
 * Returns: (transfer none): The locale of this translation entry. This is a pointer
 * to the internal memory location and must not be freed.
 *
 * Since: 2.0
 */
const gchar *
modulemd_translation_entry_get_locale (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_set_summary:
 * @self: This #ModulemdTranslationEntry
 * @summary: (nullable): The summary of this module translated appropriately for this locale.
 *
 * Since: 2.0
 */
void
modulemd_translation_entry_set_summary (ModulemdTranslationEntry *self,
                                        const gchar *summary);


/**
 * modulemd_translation_entry_get_summary:
 * @self: This #ModulemdTranslationEntry
 *
 * Get the summary of this translation entry.
 *
 * Returns: (transfer none): The summary of this module stream translated into the language specified by locale.
 *
 * Since: 2.0
 */
const gchar *
modulemd_translation_entry_get_summary (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_set_description:
 * @self: This #ModulemdTranslationEntry
 * @description: (nullable): The description of this module stream translated into the language specified by locale.
 *
 * Since: 2.0
 */
void
modulemd_translation_entry_set_description (ModulemdTranslationEntry *self,
                                            const char *description);


/**
 * modulemd_translation_entry_get_description:
 * @self: This #ModulemdTranslationEntry
 *
 * Get the description of this translation entry.
 *
 * Returns: (transfer none): The description of this module stream translated into the language specified by locale.
 *
 * Since: 2.0
 */
const gchar *
modulemd_translation_entry_get_description (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_get_profiles_as_strv: (rename-to modulemd_translation_entry_get_profiles)
 * @self: This #ModuleTranslationEntry
 *
 * Get a list of profiles that have descriptions.
 *
 * Returns: (transfer full): An ordered list of profiles for which descriptions have been translated for this locale.
 *
 * Since: 2.0
 */
gchar **
modulemd_translation_entry_get_profiles_as_strv (
  ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_set_profile_description:
 * @self: This #ModuleTranslationEntry
 * @profile_name: The name of the profile.
 * @profile_description: (nullable): The translated description of the profile.
 *
 * Adds a profile name translation
 *
 * Since: 2.0
 */
void
modulemd_translation_entry_set_profile_description (
  ModulemdTranslationEntry *self,
  const gchar *profile_name,
  const gchar *profile_description);


/**
 * module_translation_entry_get_profile_description:
 * @self: This #ModuleTranslationEntry
 * @profile_name: The name of the profile whose description is being translated.
 *
 * Returns: (transfer none): The description for the specified profile.
 *
 * Since: 2.0
 */
const gchar *
modulemd_translation_entry_get_profile_description (
  ModulemdTranslationEntry *self, const gchar *profile_name);

G_END_DECLS
