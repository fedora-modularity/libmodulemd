/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat, Inc.
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
#include "modulemd-2.0/modulemd-build-config.h"
#include "modulemd-2.0/modulemd-buildopts.h"
#include "private/modulemd-yaml.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-build-config
 * @title: Modulemd.BuildConfig
 * @stability: private
 * @short_description: Internal representation of a module build configuration
 */


/**
 * modulemd_build_config_parse_yaml:
 * @parser: A #yaml_parser_t positioned at the start of a configuration
 * entry of a ModulemdPackager v3 YAML document.
 * @strict: Whether to ignore unknown keys in the YAML
 * @error: (out): A #GError explaining any failure to complete the parsing
 *
 * Returns: (transfer full): A newly-constructed #ModulemdBuildConfig object
 * populated from the data in the provided YAML. Returns NULL and sets @error
 * appropriately if the document couldn't be parsed successfully or failed
 * validation.
 *
 * Since: 2.11
 */
ModulemdBuildConfig *
modulemd_build_config_parse_yaml (yaml_parser_t *parser,
                                  gboolean strict,
                                  GError **error);


/**
 * modulemd_build_config_emit_yaml:
 * @self: This #ModulemdBuildConfig object.
 * @emitter: (inout): A libyaml emitter object positioned where a BuidConfig
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the BuildConfig was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.11
 */
gboolean
modulemd_build_config_emit_yaml (ModulemdBuildConfig *self,
                                 yaml_emitter_t *emitter,
                                 GError **error);

G_END_DECLS
