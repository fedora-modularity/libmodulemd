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
#include <yaml.h>

#include "modulemd-translation-entry.h"

/**
 * SECTION: modulemd-translation-entry-private
 * @title: Modulemd.TranslationEntry (Private)
 * @stability: Private
 * @short_description: #ModulemdTranslationEntry methods that should be used only
 * by internal consumers.
 */

/**
 * modulemd_translation_entry_parse_yaml:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * Translation Entry's mapping entry in the YAML document.
 * @locale: (int): A string with the locale for the current translation entry.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdTranslationEntry object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.0
 */
ModulemdTranslationEntry *
modulemd_translation_entry_parse_yaml (yaml_parser_t *parser,
                                       const gchar *locale,
                                       gboolean strict,
                                       GError **error);

/**
 * modulemd_translation_entry_emit_yaml:
 * @self: This #ModulemdTranslationEntry object.
 * @emitter: (inout): A libyaml emitter object positioned where a Translation Entry
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the translation entry was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_translation_entry_emit_yaml (ModulemdTranslationEntry *self,
                                      yaml_emitter_t *emitter,
                                      GError **error);
