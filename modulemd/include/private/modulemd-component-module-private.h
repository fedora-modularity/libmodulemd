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

#include "modulemd-component-module.h"

/**
 * SECTION: modulemd-component-module-private
 * @title: Modulemd.ComponentModule (Private)
 * @stability: Private
 * @short_description: #ModulemdComponentModule methods that should be used only
 * by internal consumers.
 */

/**
 * modulemd_component_module_parse_yaml:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * ComponentModule's mapping entry in the YAML document.
 * @name: (in): A string with the name of the component.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for parsing error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdComponentModule object
 * read from the YAML. NULL if a parse error occurred and sets @error
 * appropriately.
 *
 * Since: 2.0
 */
ModulemdComponentModule *
modulemd_component_module_parse_yaml (yaml_parser_t *parser,
                                      const gchar *name,
                                      gboolean strict,
                                      GError **error);


/**
 * modulemd_component_module_emit_yaml:
 * @self: This #ModulemdComponentModule object.
 * @emitter: (inout): A libyaml emitter object positioned where Module Component
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission error.
 *
 * Returns: TRUE if the module component was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_component_module_emit_yaml (ModulemdComponentModule *self,
                                     yaml_emitter_t *emitter,
                                     GError **error);
