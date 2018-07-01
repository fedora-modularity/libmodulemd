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
#include "modulemd-translation-entry.h"
#include <glib-object.h>

G_BEGIN_DECLS

enum
{
  MD_TRANSLATION_VERSION_UNSET = 0,

  MD_TRANSLATION_VERSION_1 = 1,

  MD_TRANSLATION_VERSION_MAX = G_MAXUINT64
};

#define MODULEMD_TRANSLATION_ERROR modulemd_translation_error_quark ()
GQuark
modulemd_translation_error_quark (void);

enum ModulemdTranslationError
{
  MODULEMD_TRANSLATION_ERROR_MISSING_CONTENT,
};

#define MD_TRANSLATION_VERSION_LATEST MD_TRANSLATION_VERSION_1

/**
 * SECTION: modulemd-translation
 * @title: Modulemd.Translation
 * @short_description: Translation information for a module stream
 */

#define MODULEMD_TYPE_TRANSLATION (modulemd_translation_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdTranslation, modulemd_translation, MODULEMD, TRANSLATION, GObject)


/**
 * modulemd_translation_new:
 *
 * Creates a new, uninitialized #ModulemdTranslation object. Use
 * .import_from_*() to initialize from an existing source.
 *
 * Since: 1.6
 */
ModulemdTranslation *
modulemd_translation_new (void);


/**
 * modulemd_translation_new_full:
 * @module_name: (transfer none): The name of the module to which these
 * translations apply.
 * @module_stream: (transfer none): The name of the stream to which these
 * translations apply.
 * @mdversion: The metadata version of the document.
 * @modified: The last modified time represented as a 64-bit integer (such as
 * 201807011200)
 *
 * Creates a new #ModulemdTranslation object and initializes its basic
 * information.
 *
 * Since: 1.6
 */
ModulemdTranslation *
modulemd_translation_new_full (const gchar *module_name,
                               const gchar *module_stream,
                               guint64 mdversion,
                               guint64 modified);

/**
 * modulemd_translation_copy:
 *
 * Make a deep copy of a #ModulemdTranslation.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdTranslation identical to
 * the one passed in. This object must be freed with g_object_unref()
 *
 * Since: 1.6
 */
ModulemdTranslation *
modulemd_translation_copy (ModulemdTranslation *self);


/**
 * modulemd_translation_import_from_file:
 * @yaml_file: (transfer none) (not nullable): The path to a YAML file that
 * contains one modulemd-translations subdocument.
 *
 * Returns: TRUE if the YAML document was a valid modulemd-translations document
 * and populates @self with its contents. If any error occurs, @error is set
 * appropriately and the function returns FALSE.
 *
 *  Since: 1.6
 */
gboolean
modulemd_translation_import_from_file (ModulemdTranslation *self,
                                       const gchar *yaml_file,
                                       GError **error);


/**
 * modulemd_translation_import_from_string:
 * @yaml: (transfer none) (not nullable): A string that contains one
 * modulemd-translations YAML subdocument.
 *
 * Returns: TRUE if the YAML document was a valid modulemd-translations document
 * and populates @self with its contents. If any error occurs, @error is set
 * appropriately and the function returns FALSE.
 *
 *  Since: 1.6
 */
gboolean
modulemd_translation_import_from_string (ModulemdTranslation *self,
                                         const gchar *yaml,
                                         GError **error);


/**
 * modulemd_translation_import_from_stream:
 * @yaml_stream: (transfer none) (not nullable): A YAML stream that contains one
 * modulemd-translations YAML subdocument.
 *
 * Returns: TRUE if the YAML document was a valid modulemd-translations document
 * and populates @self with its contents. If any error occurs, @error is set
 * appropriately and the function returns FALSE.
 *
 *  Since: 1.6
 */
gboolean
modulemd_translation_import_from_stream (ModulemdTranslation *self,
                                         FILE *yaml_stream,
                                         GError **error);


/**
 * modulemd_translation_dump:
 * @yaml_file: A string containing the path to the output file
 *
 * Writes this module stream translation out to a YAML document on disk.
 *
 * Returns: False if the file could not be written and sets @error.
 *
 * Since: 1.6
 */
gboolean
modulemd_translation_dump (ModulemdTranslation *self,
                           const gchar *yaml_file,
                           GError **error);


/**
 * modulemd_translation_dumps:
 *
 * Writes this module out to a YAML document string.
 *
 * Returns: (transfer full): The serialized YAML representation of this module
 * stream translation or NULL and sets @error appropriately. The returned string
 * must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_translation_dumps (ModulemdTranslation *self, GError **error);


/**
 * modulemd_translation_set_mdversion:
 * @version: The version of the modulemd-translation format in use
 *
 * Since: 1.6
 */
void
modulemd_translation_set_mdversion (ModulemdTranslation *self,
                                    guint64 version);


/**
 * modulemd_translation_get_mdversion:
 *
 * Returns: The version of the modulemd-translation format in use
 *
 * Since: 1.6
 */
guint64
modulemd_translation_get_mdversion (ModulemdTranslation *self);


/**
 * modulemd_translation_set_module_name:
 * @module_name: (transfer none) (not nullable): The module name to which these
 * translations apply.
 *
 * Since: 1.6
 */
void
modulemd_translation_set_module_name (ModulemdTranslation *self,
                                      const gchar *module_name);


/**
 * modulemd_translation_get_module_name:
 *
 * Returns: (transfer full): The name of the module to which these translations
 * apply. This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_translation_get_module_name (ModulemdTranslation *self);


/**
 * modulemd_translation_peek_module_name: (skip)
 *
 * Returns: (transfer none): The name of the module to which these translations
 * apply. This string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_translation_peek_module_name (ModulemdTranslation *self);


/**
 * modulemd_translation_set_module_stream:
 * @module_stream: (transfer none) (not nullable): The module stream to which
 * these translations apply.
 *
 * Since: 1.6
 */
void
modulemd_translation_set_module_stream (ModulemdTranslation *self,
                                        const gchar *module_stream);


/**
 * modulemd_translation_get_module_stream:
 *
 * Returns: (transfer full): The name of the module stream to which these
 * translations apply. This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_translation_get_module_stream (ModulemdTranslation *self);


/**
 * modulemd_translation_peek_module_stream: (skip)
 *
 * Returns: (transfer none): The name of the module stream to which these
 * translations apply. This string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_translation_peek_module_stream (ModulemdTranslation *self);


/**
 * modulemd_translation_set_modified:
 * @modified: A 64-bit unsigned integer. Use YYYYMMDDHHMM to easily identify the
 * last modification time. Use UTC for consistency.
 * When merging, entries with a newer 'modified' value will override any
 * earlier values.
 *
 * Since: 1.6
 */
void
modulemd_translation_set_modified (ModulemdTranslation *self,
                                   guint64 modified);


/**
 * modulemd_translation_get_modified:
 *
 * Returns: A 64-bit unsigned integer representing the last modified time.
 *
 * Since: 1.6
 */
guint64
modulemd_translation_get_modified (ModulemdTranslation *self);


/**
 * modulemd_translation_add_entry:
 * @entry: (transfer none) (not nullable): A set of translations of this module
 * stream for a particular language.
 *
 * Since: 1.6
 */
void
modulemd_translation_add_entry (ModulemdTranslation *self,
                                ModulemdTranslationEntry *entry);


/**
 * modulemd_translation_get_entry_by_locale:
 * @locale: (transfer none) (not nullable): The locale of the translation to
 * retrieve. It must correspond to the format specified by [libc locale names](
 * https://www.gnu.org/software/libc/manual/html_node/Locale-Names.html).
 *
 * Returns: (transfer full): A #ModulemdTranslationEntry containing the
 * translations for the requested locale. This object must be freed with
 * g_object_unref().
 *
 * Since: 1.6
 */
ModulemdTranslationEntry *
modulemd_translation_get_entry_by_locale (ModulemdTranslation *self,
                                          const gchar *locale);


/**
 * modulemd_translation_get_locales:
 *
 * Returns: (transfer container) (element-type utf8): A sorted list of locales
 * known to this #ModulemdTranslation.
 *
 * Since: 1.6
 */
GPtrArray *
modulemd_translation_get_locales (ModulemdTranslation *self);


G_END_DECLS
