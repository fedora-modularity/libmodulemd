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

#include <glib.h>
#include <locale.h>
#include <yaml.h>

#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

void
parser_skip_headers (yaml_parser_t *parser)
{
  int result;
  MMD_INIT_YAML_EVENT (event);

  /* Advance the parser past STREAM_START, DOCUMENT_START and MAPPING_START */
  result = yaml_parser_parse (parser, &event);
  g_assert_cmpint (result, ==, 1);
  g_assert_cmpint (event.type, ==, YAML_STREAM_START_EVENT);

  result = yaml_parser_parse (parser, &event);
  g_assert_cmpint (result, ==, 1);
  g_assert_cmpint (event.type, ==, YAML_DOCUMENT_START_EVENT);

  result = yaml_parser_parse (parser, &event);
  g_assert_cmpint (result, ==, 1);
  g_assert_cmpint (event.type, ==, YAML_MAPPING_START_EVENT);
}
