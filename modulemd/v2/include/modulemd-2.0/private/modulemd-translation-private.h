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

/**
 * SECTION: modulemd-profile-private
 * @title: Modulemd.Profile (Private)
 * @stability: Private
 * @short_description: #Modulemd.Profile methods that should be used only
 * by internal consumers.
 */


guint64
modulemd_translation_get_version (ModulemdTranslation *self);

const gchar *
modulemd_translation_get_module_name (ModulemdTranslation *self);

const gchar *
modulemd_translation_get_module_stream (ModulemdTranslation *self);

guint64
modulemd_translation_get_modified (ModulemdTranslation *self);


/**
 * modulemd_translation_parse_yaml:
 * @data: (in): A YAML string in the format of those returned from
 * modulemd_yaml_parse_data()
 * @version: (in): The "version" attribute from the modulemd_yaml_parse_data()
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
modulemd_translation_parse_yaml (const gchar *data,
                                 guint64 version,
                                 GError **error);

/**
 * modulemd_translation_emit_yaml:
 * @self: This #ModulemdTranslation
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
