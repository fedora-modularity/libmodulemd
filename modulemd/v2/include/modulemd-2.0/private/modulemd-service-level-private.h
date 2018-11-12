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
 * @short_description: #Modulemd.ServiceLevel methods that should be used only
 * by internal consumers.
 */

/**
 * modulemd_service_level_parse_yaml:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * Service Level entry in the YAML document.
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
modulemd_service_level_parse_yaml (yaml_parser_t *parser, GError **error);
