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

#include "modulemd-dependencies.h"

/**
 * SECTION: modulemd-dependencies-private
 * @title: Modulemd.Dependencies (Private)
 * @stability: Private
 * @short_description: #Modulemd.Dependencies methods that should be used only
 * by internal consumers.
 */

/**
 * modulemd_dependencies_parse_yaml:
 * @parser: (inout): A libyaml parser object positioned at a sequence entry for
 * a Dependencies object.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdDependencies object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.0
 */
ModulemdDependencies *
modulemd_dependencies_parse_yaml (yaml_parser_t *parser, GError **error);

/**
 * modulemd_dependenciesemitter_yaml:
 * @self: This #ModulemdDependencies
 * @emitter: (inout): A libyaml emitter object positioned where a dependencies instance
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the dependencies object was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_dependencies_emit_yaml (ModulemdDependencies *self,
                                 yaml_emitter_t *emitter,
                                 GError **error);


gboolean
modulemd_dependencies_validate (ModulemdDependencies *self, GError **error);
