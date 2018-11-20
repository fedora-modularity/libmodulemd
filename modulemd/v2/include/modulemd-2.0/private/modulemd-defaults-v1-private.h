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
#include <yaml.h>
#include "modulemd-defaults-v1.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-defaults-v1-private
 * @title: Modulemd.DefaultsV1 (Private)
 * @stability: private
 * @short_description: #ModulemdDefault methods that should only be used by
 * internal consumers.
 */

/**
 * modulemd_defaults_v1_parse_yaml:
 * @data: (in): A YAML string in the format of those returned from
 * modulemd_yaml_parse_data()
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdDefaultsV1 object read
 * from the YAML. NULL if a parse or validation error occurred and sets @error
 * appropriately.
 *
 * Since: 2.0
 */
ModulemdDefaultsV1 *
modulemd_defaults_v1_parse_yaml (const gchar *data, GError **error);


/**
 * modulemd_defaults_v1_emit_yaml:
 * @self: This #ModulemdDefaultsV1
 * @emitter: (inout): A libyaml emitter object positioned where a Defaults (v1)
 * data section belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the #ModulemdDefaults was emitted successfully. FALSE and
 * sets @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_defaults_v1_emit_yaml (ModulemdDefaultsV1 *self,
                                yaml_emitter_t *emitter,
                                GError **error);

G_END_DECLS
