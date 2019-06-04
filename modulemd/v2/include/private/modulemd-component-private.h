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

#include "modulemd-component.h"

/**
 * SECTION: modulemd-component-private
 * @title: Modulemd.Component (Private)
 * @stability: Private
 * @short_description: #ModulemdComponent methods that should be used only
 * by internal consumers
 */


/**
 * modulemd_component_parse_buildafter:
 * @self: This #ModulemdComponent
 * @parser: (inout): A libyaml parser object positioned just after the
 * "buildafter" key in a #ModulemdComponent section of a YAML document.
 * @error: (out): A #GError that will return the reason for a parse failure.
 *
 * Returns: TRUE if the buildafter list could be parsed successfully.
 *
 * Since: 2.2
 */
gboolean
modulemd_component_parse_buildafter (ModulemdComponent *self,
                                     yaml_parser_t *parser,
                                     GError **error);

/**
 * modulemd_component_parse_buildonly:
 * @self: This #ModulemdComponent
 * @parser: (inout): A libyaml parser object positioned just after the
 * "buildonly" key in a #ModulemdComponent section of a YAML document.
 * @error: (out): A #GError that will return the reason for a parse failure.
 *
 * Returns: TRUE if the buildafter list could be parsed successfully.
 *
 * Since: 2.2
 */
gboolean
modulemd_component_parse_buildonly (ModulemdComponent *self,
                                    yaml_parser_t *parser,
                                    GError **error);

/**
 * modulemd_component_has_buildafter:
 * @self: This #ModulemdComponent
 *
 * Returns: Whether one or more buildafter entries have been added to this
 * component.
 *
 * Since: 2.2
 */
gboolean
modulemd_component_has_buildafter (ModulemdComponent *self);


/**
 * modulemd_component_get_buildafter_internal:
 * @self: This #ModulemdComponentModule
 *
 * Returns: The internal hash table representing the set of buildafter
 * dependencies.
 *
 * Since: 2.2
 */
GHashTable *
modulemd_component_get_buildafter_internal (ModulemdComponent *self);


/**
 * modulemd_component_emit_yaml_start:
 * @self: This #ModulemdComponent
 * @emitter: (inout): A libyaml emitter object positioned where Component start
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission error.
 *
 * Returns: TRUE if the component header was emitted succesfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_component_emit_yaml_start (ModulemdComponent *self,
                                    yaml_emitter_t *emitter,
                                    GError **error);

/**
 * modulemd_component_emit_yaml_build_common:
 * @self: This #ModulemdComponent
 * @emitter: (inout): A libyaml emitter object positioned where a Component's
 * buildorder, buildafter and/or buildonly item(s) should appear in the YAML
 * document.
 * @error: (out): A #GError that will return the reason for an emission error.
 *
 * Returns: TRUE if the component buildorder was emitted succesfully. FALSE and
 * sets @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.2
 */
gboolean
modulemd_component_emit_yaml_build_common (ModulemdComponent *self,
                                           yaml_emitter_t *emitter,
                                           GError **error);

/**
 * modulemd_component_equals_wrapper:
 * @a: A void pointer
 * @b: A void pointer
 *
 * Returns: TRUE, if both the pointers are equal. FALSE, otherwise
 *
 * Since: 2.5
 */
gboolean
modulemd_component_equals_wrapper (const void *a, const void *b);
