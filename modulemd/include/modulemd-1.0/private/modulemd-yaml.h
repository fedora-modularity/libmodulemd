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

#ifndef MODULEMD_YAML_H
#define MODULEMD_YAML_H

#include "modulemd.h"
#include <glib.h>
#include <yaml.h>

G_BEGIN_DECLS

#define MODULEMD_YAML_ERROR modulemd_yaml_error_quark ()
GQuark
modulemd_yaml_error_quark (void);

enum ModulemdYamlError
{
  MODULEMD_YAML_ERROR_OPEN,
  MODULEMD_YAML_ERROR_PROGRAMMING,
  MODULEMD_YAML_ERROR_UNPARSEABLE,
  MODULEMD_YAML_ERROR_PARSE,
  MODULEMD_YAML_ERROR_EMIT
};

const gchar *
mmd_yaml_get_event_name (yaml_event_type_t type);

typedef gboolean (*ModulemdParsingFunc) (yaml_parser_t *parser,
                                         GObject **object,
                                         guint64 version,
                                         GError **error);

#define YAML_PARSER_PARSE_WITH_ERROR_RETURN(parser, event, _error, msg)       \
  do                                                                          \
    {                                                                         \
      if (!yaml_parser_parse (parser, event))                                 \
        {                                                                     \
          g_debug (msg);                                                      \
          g_set_error_literal (_error,                                        \
                               MODULEMD_YAML_ERROR,                           \
                               MODULEMD_YAML_ERROR_UNPARSEABLE,               \
                               msg);                                          \
          result = FALSE;                                                     \
          goto error;                                                         \
        }                                                                     \
      g_debug ("Parser event: %s", mmd_yaml_get_event_name ((event)->type));  \
    }                                                                         \
  while (0)

#define YAML_PARSER_PARSE_WITH_EXIT(parser, event, _error)                    \
  do                                                                          \
    {                                                                         \
      if (!yaml_parser_parse (parser, event))                                 \
        {                                                                     \
          g_debug ("Parser error");                                           \
          g_set_error_literal (_error,                                        \
                               MODULEMD_YAML_ERROR,                           \
                               MODULEMD_YAML_ERROR_UNPARSEABLE,               \
                               "Parser error");                               \
          return FALSE;                                                       \
        }                                                                     \
      g_debug ("Parser event: %s", mmd_yaml_get_event_name ((event)->type));  \
    }                                                                         \
  while (0)

#define MMD_YAML_ERROR_RETURN_RETHROW(_error, msg)                            \
  do                                                                          \
    {                                                                         \
      g_debug (msg);                                                          \
      result = FALSE;                                                         \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define MMD_ERROR_RETURN_FULL(_error, type, msg)                              \
  do                                                                          \
    {                                                                         \
      g_debug (msg);                                                          \
      g_set_error_literal (_error, MODULEMD_YAML_ERROR, type, msg);           \
      result = FALSE;                                                         \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define MMD_YAML_ERROR_RETURN(_error, msg)                                    \
  do                                                                          \
    {                                                                         \
      g_debug (msg);                                                          \
      g_set_error_literal (                                                   \
        _error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE, msg);         \
      result = FALSE;                                                         \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define YAML_EMITTER_EMIT_WITH_ERROR_RETURN(emitter, event, _error, msg)      \
  do                                                                          \
    {                                                                         \
      if (!yaml_emitter_emit (emitter, event))                                \
        {                                                                     \
          g_debug ("Error: %s - event type: %s",                              \
                   msg,                                                       \
                   mmd_yaml_get_event_name ((event)->type));                  \
          g_set_error_literal (                                               \
            _error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_EMIT, msg);      \
          result = FALSE;                                                     \
          goto error;                                                         \
        }                                                                     \
      g_debug ("Emitter event: %s", mmd_yaml_get_event_name ((event)->type)); \
    }                                                                         \
  while (0)

#define MMD_EMIT_WITH_EXIT(emitter, event, _error, ...)                       \
  do                                                                          \
    {                                                                         \
      g_debug ("Emitter event: %s", mmd_yaml_get_event_name ((event)->type)); \
      if (!yaml_emitter_emit (emitter, event))                                \
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

#define MMD_YAML_EMITTER_ERROR_RETURN(_error, msg)                            \
  do                                                                          \
    {                                                                         \
      g_debug (msg);                                                          \
      g_set_error_literal (                                                   \
        _error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_EMIT, msg);          \
      result = FALSE;                                                         \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define MMD_YAML_EMIT_SCALAR(event, scalar, style)                            \
  do                                                                          \
    {                                                                         \
      yaml_scalar_event_initialize (event,                                    \
                                    NULL,                                     \
                                    NULL,                                     \
                                    (yaml_char_t *)scalar,                    \
                                    (int)strlen (scalar),                     \
                                    1,                                        \
                                    1,                                        \
                                    style);                                   \
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (                                   \
        emitter, event, error, "Error writing scalar");                       \
      g_clear_pointer (&scalar, g_free);                                      \
    }                                                                         \
  while (0)

#define MMD_EMIT_SCALAR(event, scalar, style)                                 \
  do                                                                          \
    {                                                                         \
      yaml_scalar_event_initialize (event,                                    \
                                    NULL,                                     \
                                    NULL,                                     \
                                    (yaml_char_t *)scalar,                    \
                                    (int)strlen (scalar),                     \
                                    1,                                        \
                                    1,                                        \
                                    style);                                   \
      MMD_EMIT_WITH_EXIT (                                                    \
        emitter, event, error, "Error writing scalar \"%s\"", scalar);        \
      g_clear_pointer (&scalar, g_free);                                      \
    }                                                                         \
  while (0)

#define MMD_YAML_EMIT_STR_STR_DICT(event, name, value, style)                 \
  do                                                                          \
    {                                                                         \
      yaml_scalar_event_initialize (event,                                    \
                                    NULL,                                     \
                                    NULL,                                     \
                                    (yaml_char_t *)name,                      \
                                    (int)strlen (name),                       \
                                    1,                                        \
                                    1,                                        \
                                    YAML_PLAIN_SCALAR_STYLE);                 \
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (                                   \
        emitter, event, error, "Error writing name");                         \
      g_clear_pointer (&name, g_free);                                        \
                                                                              \
      yaml_scalar_event_initialize (event,                                    \
                                    NULL,                                     \
                                    NULL,                                     \
                                    (yaml_char_t *)value,                     \
                                    (int)strlen (value),                      \
                                    1,                                        \
                                    1,                                        \
                                    style);                                   \
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (                                   \
        emitter, event, error, "Error writing value");                        \
      g_clear_pointer (&value, g_free);                                       \
    }                                                                         \
  while (0)

#define MMD_EMIT_STR_STR_DICT(event, _name, _value, style)                    \
  do                                                                          \
    {                                                                         \
      yaml_scalar_event_initialize (event,                                    \
                                    NULL,                                     \
                                    NULL,                                     \
                                    (yaml_char_t *)_name,                     \
                                    (int)strlen (_name),                      \
                                    1,                                        \
                                    1,                                        \
                                    YAML_PLAIN_SCALAR_STYLE);                 \
      MMD_EMIT_WITH_EXIT (emitter, event, error, "Error writing name");       \
      g_clear_pointer (&_name, g_free);                                       \
                                                                              \
      yaml_scalar_event_initialize (event,                                    \
                                    NULL,                                     \
                                    NULL,                                     \
                                    (yaml_char_t *)_value,                    \
                                    (int)strlen (_value),                     \
                                    1,                                        \
                                    1,                                        \
                                    style);                                   \
      MMD_EMIT_WITH_EXIT (emitter, event, error, "Error writing value");      \
      g_clear_pointer (&_value, g_free);                                      \
    }                                                                         \
  while (0)

#define MMD_YAML_NOEVENT_ERROR_RETURN(_error, msg)                            \
  do                                                                          \
    {                                                                         \
      g_debug (msg);                                                          \
      g_set_error_literal (                                                   \
        _error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE, msg);         \
      result = FALSE;                                                         \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define MMD_YAML_SET_ERROR(_error, ...)                                       \
  do                                                                          \
    {                                                                         \
      g_debug (__VA_ARGS__);                                                  \
      g_set_error (                                                           \
        _error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE, __VA_ARGS__); \
    }                                                                         \
  while (0)

#define MMD_EMITTER_SET_ERROR(_error, ...)                                    \
  do                                                                          \
    {                                                                         \
      g_debug (__VA_ARGS__);                                                  \
      g_set_error (                                                           \
        _error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_EMIT, __VA_ARGS__);  \
    }                                                                         \
  while (0)

ModulemdModule **
mmd_yaml_dup_modules (GPtrArray *objects);

gboolean
parse_yaml_file (const gchar *path,
                 GPtrArray **data,
                 GPtrArray **failures,
                 GError **error);

GHashTable *
parse_module_index_from_file (const gchar *path,
                              GPtrArray **failures,
                              GError **error);

gboolean
parse_yaml_string (const gchar *yaml,
                   GPtrArray **data,
                   GPtrArray **failures,
                   GError **error);

GHashTable *
parse_module_index_from_string (const gchar *yaml,
                                GPtrArray **failures,
                                GError **error);

gboolean
parse_yaml_stream (FILE *stream,
                   GPtrArray **data,
                   GPtrArray **failures,
                   GError **error);

GHashTable *
parse_module_index_from_stream (FILE *iostream,
                                GPtrArray **failures,
                                GError **error);


gboolean
emit_yaml_file (GPtrArray *objects, const gchar *path, GError **error);

gboolean
emit_yaml_string (GPtrArray *objects, gchar **_yaml, GError **error);

typedef struct _modulemd_yaml_string
{
  char *str;
  size_t len;
} modulemd_yaml_string;

int
_write_yaml_string (void *data, unsigned char *buffer, size_t size);

void
modulemd_yaml_string_free (modulemd_yaml_string *yaml_string);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (FILE, fclose);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (modulemd_yaml_string,
                               modulemd_yaml_string_free);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC (yaml_event_t, yaml_event_delete);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC (yaml_parser_t, yaml_parser_delete);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC (yaml_emitter_t, yaml_emitter_delete);

#define MMD_INIT_YAML_EVENT(_event)                                           \
  g_auto (yaml_event_t) _event;                                               \
  memset (&(_event), 0, sizeof (yaml_event_t));

gboolean
parse_raw_yaml_mapping (yaml_parser_t *parser,
                        GVariant **variant,
                        GError **error);

gboolean
parse_raw_yaml_sequence (yaml_parser_t *parser,
                         GVariant **variant,
                         GError **error);

gboolean
emit_yaml_variant (yaml_emitter_t *emitter, GVariant *variant, GError **error);

/* == Common Parsing Functions == */

gboolean
_parse_modulemd_date (yaml_parser_t *parser, GDate **_date, GError **error);

gboolean
_simpleset_from_sequence (yaml_parser_t *parser,
                          ModulemdSimpleSet **_set,
                          GError **error);

gboolean
_hashtable_from_mapping (yaml_parser_t *parser,
                         GHashTable **_htable,
                         GError **error);

gboolean
_parse_skip (yaml_parser_t *parser, GError **error);


gboolean
_emit_modulemd_simpleset (yaml_emitter_t *emitter,
                          ModulemdSimpleSet *set,
                          yaml_sequence_style_t style,
                          GError **error);

gboolean
_emit_modulemd_hashtable (yaml_emitter_t *emitter,
                          GHashTable *htable,
                          yaml_scalar_style_t style,
                          GError **error);
gboolean
_emit_modulemd_variant_hashtable (yaml_emitter_t *emitter,
                                  GHashTable *htable,
                                  GError **error);


/* == ModulemdModule Parser == */

gboolean
_parse_module_stream (yaml_parser_t *parser,
                      GObject **object,
                      guint64 version,
                      GError **error);

/* == ModulemdDefaults Parser == */
gboolean
_parse_defaults (yaml_parser_t *parser,
                 GObject **object,
                 guint64 version,
                 GError **error);

/* == ModulemdTranslation Parser == */
gboolean
_parse_translation (yaml_parser_t *parser,
                    GObject **object,
                    guint64 version,
                    GError **error);

/* == ModulemdModule Emitter == */
gboolean
_emit_modulestream (yaml_emitter_t *emitter,
                    ModulemdModuleStream *module,
                    GError **error);


/* == ModulemdDefaults Emitter == */
gboolean
_emit_defaults (yaml_emitter_t *emitter,
                ModulemdDefaults *defaults,
                GError **error);

/* == ModulemdTranslation Emitter == */
gboolean
_emit_translation (yaml_emitter_t *emitter,
                   ModulemdTranslation *translation,
                   GError **error);

G_END_DECLS

#endif /* MODULEMD_YAML_H */
