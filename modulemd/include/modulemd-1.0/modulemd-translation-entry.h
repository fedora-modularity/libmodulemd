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

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-translation-entry
 * @title: Modulemd.TranslationEntry
 * @short_description: Contains the translated strings for a particular module
 * stream.
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
 * @locale: (transfer none) (not nullable): The locale for this translation. It
 * must correspond to the format specified by [libc locale names](
 * https://www.gnu.org/software/libc/manual/html_node/Locale-Names.html).
 *
 * Since: 1.6
 */
ModulemdTranslationEntry *
modulemd_translation_entry_new (const gchar *locale);


/**
 * modulemd_translation_entry_copy:
 *
 * Returns: (transfer full): A deep copy of this translation entry. This must be
 * freed with g_object_unref().
 *
 * Since: 1.6
 */
ModulemdTranslationEntry *
modulemd_translation_entry_copy (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_set_locale:
 * @locale: (transfer none) (not nullable): The locale
 *
 * This function sets the locale for this translation. It must correspond to
 * the format specified by [libc locale names](
 * https://www.gnu.org/software/libc/manual/html_node/Locale-Names.html).
 *
 * Since: 1.6
 */
void
modulemd_translation_entry_set_locale (ModulemdTranslationEntry *self,
                                       const gchar *locale);

/**
 * modulemd_translation_entry_get_locale:
 *
 * Get the locale of this entry.
 *
 * Returns: (transfer full): A string describing the locale of this entry,
 * corresponding to [libc locale names](
 * https://www.gnu.org/software/libc/manual/html_node/Locale-Names.html).
 * This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_translation_entry_get_locale (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_peek_locale: (skip)
 *
 * Peek at the locale of this entry.
 *
 * Returns: (transfer none): A string describing the locale of this entry,
 * corresponding to [libc locale names](
 * https://www.gnu.org/software/libc/manual/html_node/Locale-Names.html).
 * This string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_translation_entry_peek_locale (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_set_summary:
 * @summary: (transfer none) (not nullable): The summary
 *
 * This function sets the translation of the summary of this module.
 *
 * Since: 1.6
 */
void
modulemd_translation_entry_set_summary (ModulemdTranslationEntry *self,
                                        const gchar *summary);

/**
 * modulemd_translation_entry_get_summary:
 *
 * Get the summary of this entry.
 *
 * Returns: (transfer full): A translation of the summary of the module
 * appropriate to the locale. This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_translation_entry_get_summary (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_peek_summary: (skip)
 *
 * Peek at the summary of this entry.
 *
 * Returns: (transfer none): A translation of the summary of the module
 * appropriate to the locale. This string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_translation_entry_peek_summary (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_set_description:
 * @description: (transfer none) (not nullable): The description
 *
 * This function sets the translation of the description of this module.
 *
 * Since: 1.6
 */
void
modulemd_translation_entry_set_description (ModulemdTranslationEntry *self,
                                            const gchar *description);

/**
 * modulemd_translation_entry_get_description:
 *
 * Get the description of this entry.
 *
 * Returns: (transfer full): A translation of the module description,
 * appropriate to the locale. This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_translation_entry_get_description (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_peek_description: (skip)
 *
 * Peek at the description of this entry.
 *
 * Returns: (transfer none): A translation of the module description,
 * appropriate to the locale. This string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_translation_entry_peek_description (ModulemdTranslationEntry *self);


/**
 * modulemd_translation_entry_set_profile_description:
 * @profile_name: (transfer none) (not nullable): The name of the stream profile
 * @profile_description: (transfer none) (nullable): A translation of the
 * profile description appropriate to the locale. If set to NULL, this will
 * remove the profile description from this #ModulemdTranslationEntry.
 *
 * Since: 1.6
 */
void
modulemd_translation_entry_set_profile_description (
  ModulemdTranslationEntry *self,
  const gchar *profile_name,
  const gchar *profile_description);


/**
 * modulemd_translation_entry_get_profile_description:
 * @profile_name: (transfer none) (not nullable): The name of the stream profile
 *
 * Returns: (transfer full): The translated description of the requested
 * profile. This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_translation_entry_get_profile_description (
  ModulemdTranslationEntry *self, const gchar *profile_name);


/**
 * modulemd_translation_entry_peek_profile_description: (skip)
 * @profile_name: (transfer none) (not nullable): The name of the stream profile
 *
 * Returns: (transfer none): The translated description of the requested
 * profile. This string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_translation_entry_peek_profile_description (
  ModulemdTranslationEntry *self, const gchar *profile_name);


/**
 * modulemd_translation_entry_get_all_profile_descriptions:
 *
 * Returns: (transfer container) (element-type utf8 utf8): The complete set of
 * profile descriptions, indexed by profile name. This must be freed with
 * g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_translation_entry_get_all_profile_descriptions (
  ModulemdTranslationEntry *self);

G_END_DECLS
