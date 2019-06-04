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
#include "modulemd-subdocument-info.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-defaults-v1-private
 * @title: Modulemd.DefaultsV1 (Private)
 * @stability: private
 * @short_description: #ModulemdDefaults methods that should only be used by
 * internal consumers.
 */

#define DEFAULT_MERGE_CONFLICT "__merge_conflict__"

/**
 * modulemd_defaults_v1_parse_yaml:
 * @subdoc: (in): A #ModulemdSubdocumentInfo representing a defaults document
 * of metadata version 1.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
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
modulemd_defaults_v1_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                 gboolean strict,
                                 GError **error);


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


ModulemdDefaults *
modulemd_defaults_v1_merge (const gchar *module_name,
                            ModulemdDefaultsV1 *from,
                            ModulemdDefaultsV1 *into,
                            GError **error);

G_END_DECLS
