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
#include "modulemd-translation-entry.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-translation
 * @title: Modulemd.Translation
 * @stability: stable
 * @short_description: Translation information for a module stream.
 */

#define MODULEMD_TYPE_TRANSLATION (modulemd_translation_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdTranslation, modulemd_translation, MODULEMD, TRANSLATION, GObject)

/**
 * modulemd_translation_new:
 * @version: The metadata version of this #ModulemdTranslation.
 * @module_name: The name of the module to which these translations apply.
 * @module_stream: The name of the module stream to which these translations apply.
 * @modified: The last modified time represented as a 64-bit integer (such as 201807011200)
 *
 * Returns: (transfer full): A newly-allocated #ModuleTranslation. This object must be freed with g_object_unref().
 *
 * Since: 2.0
 */
ModulemdTranslation *
modulemd_translation_new (guint64 version,
                          const gchar *module_name,
                          const gchar *module_stream,
                          guint64 modified);


/**
 * modulemd_translation_copy:
 * @self: This #ModulemdTranslation
 *
 * Create a copy of this #ModulemdTranslation object.
 *
 * Returns: (transfer full): a copied #ModulemdTranslation object
 *
 * Since: 2.0
 */
ModulemdTranslation *
modulemd_translation_copy (ModulemdTranslation *self);


/**
 * modulemd_translation_validate:
 * @self: This #ModulemdTranslation
 * @error: (out): If the object is not valid, it will return the reason.
 *
 * This method ensures that the translation is internally consistent for usage or dumping to YAML. It will be run implicitly prior to emitting YAML. This is not a complete linter, merely a sanity check that the values are not impossible.
 *
 * Since: 2.0
 */
gboolean
modulemd_translation_validate (ModulemdTranslation *self, GError **error);


/**
 * modulemd_translation_set_modified:
 * @self: This #ModulemdTranslation
 * @modified: The last modified time represented as a 64-bit integer (such as 201807011200)
 *
 * Since: 2.0
 */
void
modulemd_translation_set_modified (ModulemdTranslation *self,
                                   guint64 modified);


/**
 * modulemd_translation_get_locales_as_strv: (rename-to modulemd_translation_get_locales)
 * @self: This #ModulemdTranslation
 *
 * Returns: (transfer full): An ordered list of locales known to this Modulemd.Translation.
 *
 * Since: 2.0
 */
GStrv
modulemd_translation_get_locales_as_strv (ModulemdTranslation *self);


/**
 * modulemd_translation_set_translation_entry:
 * @self: This #ModulemdTranslation
 * @translation_entry: A set of translations of this module stream for a particular locale.
 *
 * Since: 2.0
 */
void
modulemd_translation_set_translation_entry (
  ModulemdTranslation *self, ModulemdTranslationEntry *translation_entry);


/**
 * modulemd_translation_get_translation_entry:
 * @self: This #ModulemdTranslation
 * @locale: The locale of the translation to retrieve.
 *
 * Returns: (transfer none): The translation entry for the requested locale, or NULL if the locale was unknown.
 *
 * Since: 2.0
 */
ModulemdTranslationEntry *
modulemd_translation_get_translation_entry (ModulemdTranslation *self,
                                            const gchar *locale);
