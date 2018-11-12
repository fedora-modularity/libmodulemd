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

#include <glib.h>
#include <locale.h>

G_BEGIN_DECLS

/**
 * SECTION: test-utils
 * @title: Internal Unit Test Utilities
 * @stability: private
 * @short_description: Utility functions for use with unit tests.
 */


/**
 * parser_skip_headers:
 * @parser: (inout): A libyaml parser object that has been initialized with an
 * input source.
 *
 * This function will advance the parser object past the initial STREAM_START,
 * DOCUMENT_START and MAPPING_START events to the first real entry in the first
 * document of the YAML stream. This is intended for unit tests to move the
 * parser location to the start of the object representation to be tested.
 *
 * Since: 2.0
 */
void
parser_skip_headers (yaml_parser_t *parser);

G_END_DECLS
