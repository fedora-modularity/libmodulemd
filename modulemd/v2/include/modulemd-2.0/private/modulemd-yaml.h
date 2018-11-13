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
#include <yaml.h>

#include "modulemd-service-level.h"

G_BEGIN_DECLS


/**
 * SECTION: modulemd-yaml
 * @title: YAML Manipulation Tools
 * @stability: private
 * @short_description: Provides private YAML utilities for internal use.
 */


#define MODULEMD_YAML_ERROR modulemd_yaml_error_quark ()
GQuark
modulemd_yaml_error_quark (void);

enum ModulemdYamlError
{
  MODULEMD_YAML_ERROR_OPEN,
  MODULEMD_YAML_ERROR_PROGRAMMING,
  MODULEMD_YAML_ERROR_UNPARSEABLE,
  MODULEMD_YAML_ERROR_PARSE,
  MODULEMD_YAML_ERROR_EMIT,
  MODULEMD_YAML_ERROR_MISSING_REQUIRED,
  MODULEMD_YAML_ERROR_EVENT_INIT
};

typedef struct _modulemd_yaml_string
{
  char *str;
  size_t len;
} modulemd_yaml_string;

int
write_yaml_string (void *data, unsigned char *buffer, size_t size);

void
modulemd_yaml_string_free (modulemd_yaml_string *yaml_string);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (FILE, fclose);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (modulemd_yaml_string,
                               modulemd_yaml_string_free);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC (yaml_event_t, yaml_event_delete);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC (yaml_parser_t, yaml_parser_delete);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC (yaml_emitter_t, yaml_emitter_delete);

const gchar *
mmd_yaml_get_event_name (yaml_event_type_t type);

#define MMD_INIT_YAML_PARSER(_parser)                                         \
  g_auto (yaml_parser_t) _parser;                                             \
  yaml_parser_initialize (&_parser);

#define MMD_INIT_YAML_EMITTER(_emitter)                                       \
  g_auto (yaml_emitter_t) _emitter;                                           \
  yaml_emitter_initialize (&_emitter);

#define MMD_INIT_YAML_EVENT(_event)                                           \
  g_auto (yaml_event_t) _event;                                               \
  memset (&(_event), 0, sizeof (yaml_event_t));

#define MMD_INIT_YAML_STRING(_emitter, _string)                               \
  g_autoptr (modulemd_yaml_string) yaml_string =                              \
    g_malloc0_n (1, sizeof (modulemd_yaml_string));                           \
  yaml_emitter_set_output (_emitter, write_yaml_string, (void *)yaml_string);

#define YAML_PARSER_PARSE_WITH_EXIT(_parser, _event, _error)                  \
  do                                                                          \
    {                                                                         \
      if (!yaml_parser_parse (_parser, _event))                               \
        {                                                                     \
          g_debug ("Parser error");                                           \
          g_set_error_literal (_error,                                        \
                               MODULEMD_YAML_ERROR,                           \
                               MODULEMD_YAML_ERROR_UNPARSEABLE,               \
                               "Parser error");                               \
          return NULL;                                                        \
        }                                                                     \
      g_debug ("Parser event: %s", mmd_yaml_get_event_name ((_event)->type)); \
    }                                                                         \
  while (0)


#define MMD_EMIT_WITH_EXIT(_emitter, _event, _error, ...)                     \
  do                                                                          \
    {                                                                         \
      int _ret;                                                               \
      g_debug ("Emitter event: %s",                                           \
               mmd_yaml_get_event_name ((_event)->type));                     \
      _ret = yaml_emitter_emit (_emitter, _event);                            \
      (_event)->type = 0;                                                     \
      if (!_ret)                                                              \
        {                                                                     \
          g_debug (__VA_ARGS__);                                              \
          g_set_error (_error,                                                \
                       MODULEMD_YAML_ERROR,                                   \
                       MODULEMD_YAML_ERROR_EMIT,                              \
                       __VA_ARGS__);                                          \
          return FALSE;                                                       \
        }                                                                     \
    }                                                                         \
  while (0)


#define MMD_YAML_ERROR_EVENT_EXIT(_error, _event, ...)                        \
  do                                                                          \
    {                                                                         \
      g_autofree gchar *formatted = g_strdup_printf (__VA_ARGS__);            \
      g_autofree gchar *formatted2 =                                          \
        g_strdup_printf ("%s [line %zu col %zu]",                             \
                         formatted,                                           \
                         _event.start_mark.line + 1,                          \
                         _event.start_mark.column + 1);                       \
      g_debug ("%s", formatted2);                                             \
      g_set_error (_error,                                                    \
                   MODULEMD_YAML_ERROR,                                       \
                   MODULEMD_YAML_ERROR_PARSE,                                 \
                   "%s",                                                      \
                   formatted2);                                               \
      return NULL;                                                            \
    }                                                                         \
  while (0)


gboolean
mmd_emitter_start_stream (yaml_emitter_t *emitter, GError **error);

gboolean
mmd_emitter_end_stream (yaml_emitter_t *emitter, GError **error);


gboolean
mmd_emitter_start_document (yaml_emitter_t *emitter, GError **error);

gboolean
mmd_emitter_end_document (yaml_emitter_t *emitter, GError **error);


gboolean
mmd_emitter_start_mapping (yaml_emitter_t *emitter,
                           yaml_mapping_style_t style,
                           GError **error);

gboolean
mmd_emitter_end_mapping (yaml_emitter_t *emitter, GError **error);


gboolean
mmd_emitter_start_sequence (yaml_emitter_t *emitter,
                            yaml_sequence_style_t style,
                            GError **error);

gboolean
mmd_emitter_end_sequence (yaml_emitter_t *emitter, GError **error);


gboolean
mmd_emitter_scalar (yaml_emitter_t *emitter,
                    const gchar *scalar,
                    yaml_scalar_style_t style,
                    GError **error);


/**
 * modulemd_yaml_parse_date:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * date (YYYY-MM-DD) scalar entry.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #GDate representing the parsed
 * value. NULL if a parse or validation error occurred and sets @error
 * appropriately.
 *
 * Since: 2.0
 */
GDate *
modulemd_yaml_parse_date (yaml_parser_t *parser, GError **error);


G_END_DECLS
