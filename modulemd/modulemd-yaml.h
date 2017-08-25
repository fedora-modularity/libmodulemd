/* modulemd-yaml.h
 *
 * Copyright (C) 2017 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MODULEMD_YAML_H
#define MODULEMD_YAML_H

#include <yaml.h>
#include "modulemd.h"

#define MODULEMD_YAML_ERROR modulemd_yaml_error_quark ()
GQuark
modulemd_yaml_error_quark (void);

enum ModulemdYamlError
{
  MODULEMD_YAML_ERROR_OPEN,
  MODULEMD_YAML_ERROR_PARSE,
  MODULEMD_YAML_ERROR_EMIT
};

#define YAML_PARSER_PARSE_WITH_ERROR_RETURN(parser, event, error, msg)        \
  do                                                                          \
    {                                                                         \
      if (!yaml_parser_parse (parser, event))                                 \
        {                                                                     \
          g_set_error_literal (                                               \
            error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE, msg);      \
          goto error;                                                         \
        }                                                                     \
      g_debug ("Parser event: %u", (event)->type);                            \
    }                                                                         \
  while (0)

#define MMD_YAML_ERROR_RETURN_RETHROW(error, msg)                             \
  do                                                                          \
    {                                                                         \
      g_message (msg);                                                        \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define MMD_YAML_ERROR_RETURN(error, msg)                                     \
  do                                                                          \
    {                                                                         \
      g_message (msg);                                                        \
      g_set_error_literal (                                                   \
        error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE, msg);          \
      g_debug ("Error occurred while parsing event %u", event.type);          \
      goto error;                                                             \
    }                                                                         \
  while (0)

ModulemdModule **
parse_yaml_file (const gchar *path, GError **error);

ModulemdModule **
parse_yaml_string (const gchar *yaml, GError **error);

gboolean
emit_yaml_file (ModulemdModule **modules, const gchar *path, GError **error);

gboolean
emit_yaml_string (ModulemdModule **modules, gchar **_yaml, GError **error);

gboolean
parse_raw_yaml_mapping (yaml_parser_t *parser,
                        GVariant **variant,
                        GError **error);

gboolean
parse_raw_yaml_sequence (yaml_parser_t *parser,
                         GVariant **variant,
                         GError **error);

#endif
