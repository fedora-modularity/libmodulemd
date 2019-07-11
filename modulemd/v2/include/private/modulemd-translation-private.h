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

#include "modulemd-profile.h"
#include "modulemd-subdocument-info.h"

/**
 * SECTION: modulemd-translation-private
 * @title: Modulemd.Translation (Private)
 * @stability: Private
 * @short_description: #ModulemdTranslation methods that should be used only
 * by internal consumers.
 */


/**
 * modulemd_translation_get_version:
 * @self: (in): This #ModulemdTranslation object.
 *
 * Returns: The metadata version of this #ModulemdTranslation object.
 *
 * Since: 2.0
 */
guint64
modulemd_translation_get_version (ModulemdTranslation *self);

/**
 * modulemd_translation_get_module_name:
 * @self: (in): This #ModulemdTranslation object.
 *
 * Returns: The module name to which this #ModulemdTranslation object applies.
 *
 * Since: 2.0
 */
const gchar *
modulemd_translation_get_module_name (ModulemdTranslation *self);

/**
 * modulemd_translation_get_module_stream:
 * @self: (in): This #ModulemdTranslation object.
 *
 * Returns: The stream name to which this #ModulemdTranslation object applies.
 *
 * Since: 2.0
 */
const gchar *
modulemd_translation_get_module_stream (ModulemdTranslation *self);

/**
 * modulemd_translation_get_modified:
 * @self: (in): This #ModulemdTranslation object.
 *
 * Returns: The last modified time of this #ModulemdTranslation object
 * represented as a 64-bit integer (such as 201807011200).
 *
 * Since: 2.0
 */
guint64
modulemd_translation_get_modified (ModulemdTranslation *self);


/**
 * modulemd_translation_parse_yaml:
 * @subdoc: (in): A #ModulemdSubdocumentInfo representing a translation
 * document
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdTranslation object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.0
 */
ModulemdTranslation *
modulemd_translation_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                 gboolean strict,
                                 GError **error);

/**
 * modulemd_translation_emit_yaml:
 * @self: This #ModulemdTranslation object.
 * @emitter: (inout): A libyaml emitter object positioned where Translation data
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the translation data was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_translation_emit_yaml (ModulemdTranslation *self,
                                yaml_emitter_t *emitter,
                                GError **error);
