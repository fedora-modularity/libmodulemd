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

#include "modulemd-component-rpm.h"

/**
 * SECTION: modulemd-component-rpm-private
 * @title: Modulemd.ComponentRpm (Private)
 * @stability: Private
 * @short_description: #ModulemdComponentRpm methods that should be used only
 * by internal consumers
 */

/**
 * modulemd_component_rpm_parse_yaml:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * ComponentRpm's mapping entry in the YAML document.
 * @name: (in): A string with the name of the component.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for parsing error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdComponentRpm object
 * read from the YAML. NULL if a parse error occured and sets @error
 * appropriately.
 *
 * Since: 2.0
 */
ModulemdComponentRpm *
modulemd_component_rpm_parse_yaml (yaml_parser_t *parser,
                                   const gchar *name,
                                   gboolean strict,
                                   GError **error);


/**
 * modulemd_component_rpm_emit_yaml:
 * @self: This #ModulemdComponentRpm
 * @emitter: (inout): A libyaml emitter object positioned where Rpm Component
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission error.
 *
 * Returns: TRUE if the rpm component was emitted succesfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_component_rpm_emit_yaml (ModulemdComponentRpm *self,
                                  yaml_emitter_t *emitter,
                                  GError **error);
