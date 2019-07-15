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

#include "modulemd-service-level.h"

/**
 * SECTION: modulemd-service-level-private
 * @title: Modulemd.ServiceLevel (Private)
 * @stability: Private
 * @short_description: #ModulemdServiceLevel methods that should be used only
 * by internal consumers.
 */

/**
 * modulemd_service_level_parse_yaml:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * Service Level entry in the YAML document.
 * @name: (in): The name of this service level.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdServiceLevel object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.0
 */
ModulemdServiceLevel *
modulemd_service_level_parse_yaml (yaml_parser_t *parser,
                                   const gchar *name,
                                   gboolean strict,
                                   GError **error);

/**
 * modulemd_service_level_emit_yaml:
 * @self: This #ModulemdServiceLevel object.
 * @emitter: (inout): A libyaml emitter object positioned where a Service Level
 * entry belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the service level was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_service_level_emit_yaml (ModulemdServiceLevel *self,
                                  yaml_emitter_t *emitter,
                                  GError **error);

/**
 * modulemd_service_level_equals_wrapper:
 * @a: A const void pointer.
 * @b: A const void pointer.
 *
 * Returns: TRUE, if both arguments are pointers to equivalent
 * #ModulemdServiceLevel objects. FALSE, otherwise.
 *
 * Since: 2.5
 */
gboolean
modulemd_service_level_equals_wrapper (const void *a, const void *b);
