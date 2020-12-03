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

#include "modulemd-buildopts.h"

/**
 * SECTION: modulemd-buildopts-private
 * @title: Modulemd.Buildopts (Private)
 * @stability: Private
 * @short_description: #ModulemdBuildopts methods that should be used only
 * by internal consumers.
 */

/**
 * modulemd_buildopts_parse_yaml:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * Buildopts entry in the YAML document.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdBuildopts object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.0
 */
ModulemdBuildopts *
modulemd_buildopts_parse_yaml (yaml_parser_t *parser,
                               gboolean strict,
                               GError **error);

/**
 * modulemd_buildopts_emit_yaml:
 * @self: This #ModulemdBuildopts object.
 * @emitter: (inout): A libyaml emitter object positioned where a Buildopts
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the buildopts was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_buildopts_emit_yaml (ModulemdBuildopts *self,
                              yaml_emitter_t *emitter,
                              GError **error);


/**
 * modulemd_buildopts_compare:
 * @self_1: (in): A #ModulemdBuildopts object.
 * @self_2: (in): A #ModulemdBuildopts object.
 *
 * Returns: Less than zero if @self_1 sorts less than @self_2, zero for equal,
 * greater than zero if @self_1 is greater than @self_2.
 *
 * Since: 2.10
 */
gint
modulemd_buildopts_compare (ModulemdBuildopts *self_1,
                            ModulemdBuildopts *self_2);
