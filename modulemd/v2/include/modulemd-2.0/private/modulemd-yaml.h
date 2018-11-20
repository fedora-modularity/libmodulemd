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

enum ModulemdYamlDocumentType
{
  MODULEMD_YAML_DOC_UNKNOWN = 0,
  MODULEMD_YAML_DOC_MODULESTREAM,
  MODULEMD_YAML_DOC_DEFAULTS,
  MODULEMD_YAML_DOC_TRANSLATIONS
};

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
  yaml_parser_initialize (&_parser)

#define MMD_INIT_YAML_EMITTER(_emitter)                                       \
  g_auto (yaml_emitter_t) _emitter;                                           \
  yaml_emitter_initialize (&_emitter)

#define MMD_INIT_YAML_EVENT(_event)                                           \
  g_auto (yaml_event_t) _event;                                               \
  memset (&(_event), 0, sizeof (yaml_event_t))

#define MMD_INIT_YAML_STRING(_emitter, _string)                               \
  g_autoptr (modulemd_yaml_string) yaml_string =                              \
    g_malloc0_n (1, sizeof (modulemd_yaml_string));                           \
  yaml_emitter_set_output (_emitter, write_yaml_string, (void *)yaml_string)

#define MMD_REINIT_YAML_STRING(_emitter, _string)                             \
  yaml_emitter_delete (_emitter);                                             \
  yaml_emitter_initialize (_emitter);                                         \
  g_clear_pointer (&_string, modulemd_yaml_string_free);                      \
  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));               \
  yaml_emitter_set_output (_emitter, write_yaml_string, (void *)yaml_string)

#define YAML_PARSER_PARSE_WITH_EXIT_FULL(_parser, _returnval, _event, _error) \
  do                                                                          \
    {                                                                         \
      if (!yaml_parser_parse (_parser, _event))                               \
        {                                                                     \
          g_debug ("Parser error");                                           \
          g_set_error_literal (_error,                                        \
                               MODULEMD_YAML_ERROR,                           \
                               MODULEMD_YAML_ERROR_UNPARSEABLE,               \
                               "Parser error");                               \
          return _returnval;                                                  \
        }                                                                     \
      g_debug ("Parser event: %s", mmd_yaml_get_event_name ((_event)->type)); \
    }                                                                         \
  while (0)

#define YAML_PARSER_PARSE_WITH_EXIT_BOOL(_parser, _event, _error)             \
  YAML_PARSER_PARSE_WITH_EXIT_FULL (_parser, FALSE, _event, _error)
#define YAML_PARSER_PARSE_WITH_EXIT_INT(_parser, _event, _error)              \
  YAML_PARSER_PARSE_WITH_EXIT_FULL (_parser, 0, _event, _error)
#define YAML_PARSER_PARSE_WITH_EXIT(_parser, _event, _error)                  \
  YAML_PARSER_PARSE_WITH_EXIT_FULL (_parser, NULL, _event, _error)


#define MMD_EMIT_WITH_EXIT_FULL(_emitter, _returnval, _event, _error, ...)    \
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
          return _returnval;                                                  \
        }                                                                     \
    }                                                                         \
  while (0)

#define MMD_EMIT_WITH_EXIT(_emitter, _event, _error, ...)                     \
  MMD_EMIT_WITH_EXIT_FULL (_emitter, FALSE, _event, _error, __VA_ARGS__)
#define MMD_EMIT_WITH_EXIT_PTR(_emitter, _event, _error, ...)                 \
  MMD_EMIT_WITH_EXIT_FULL (_emitter, NULL, _event, _error, __VA_ARGS__)

#define MMD_YAML_ERROR_EVENT_EXIT_FULL(_error, _event, _returnval, ...)       \
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
      return _returnval;                                                      \
    }                                                                         \
  while (0)

#define MMD_YAML_ERROR_EVENT_EXIT(_error, _event, ...)                        \
  MMD_YAML_ERROR_EVENT_EXIT_FULL (_error, _event, NULL, __VA_ARGS__)

#define MMD_YAML_ERROR_EVENT_EXIT_BOOL(_error, _event, ...)                   \
  MMD_YAML_ERROR_EVENT_EXIT_FULL (_error, _event, FALSE, __VA_ARGS__)

#define MMD_YAML_ERROR_EVENT_EXIT_INT(_error, _event, ...)                    \
  MMD_YAML_ERROR_EVENT_EXIT_FULL (_error, _event, 0, __VA_ARGS__)

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
 * mmd_emitter_strv:
 * @emitter: (intout): A yaml emitter object positioned at the beginning of a value place to emit a sequence.
 * @seq_style: (in): The YAML sequence style for the output.
 * @list: A list that will be emitted to the YAML emitter.
 * @error: (out): A #GError that will return the reason for an emitting error.
 *
 * Returns: A boolean whether emitting the sequence was succesful.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_strv (yaml_emitter_t *emitter,
                  yaml_sequence_style_t seq_style,
                  const GStrv list,
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


/**
 * modulemd_yaml_parse_string:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a string scalar entry.
 * @error: (out): A #GError that will return the reason for a parsing or validation error.
 *
 * Returns: (transfer full): A newly-allocated gchar * representing the parsed value.
 * NULL if a parse error occured and sets @error appropriately.
 *
 * Since: 2.0
 */
gchar *
modulemd_yaml_parse_string (yaml_parser_t *parser, GError **error);


/**
 * modulemd_yaml_parse_uint64:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * uint64 scalar entry.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A 64-bit unsigned integer representing the parsed
 * value. Returns 0 if a parse error occured and sets @error appropriately.
 *
 * Since: 2.0
 */
guint64
modulemd_yaml_parse_uint64 (yaml_parser_t *parser, GError **error);


/**
 * modulemd_yaml_parse_string_set:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a sequence with string scalars.
 * @error: (out): A #GError that will return the reason for a parsing or validation error.
 *
 * Returns: (transfer full): A newly-allocated GHashtTable * representing the parsed value.
 * All parsed sequence entries are added as keys in the hashtable.
 * NULL if a parse error occured and sets @error appropriately.
 *
 * Since: 2.0
 */
GHashTable *
modulemd_yaml_parse_string_set (yaml_parser_t *parser, GError **error);


/**
 * modulemd_yaml_parse_document_type:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * yaml subdocument immediately prior to a YAML_DOCUMENT_START_EVENT.
 * @doctype: (out): The type of document that was encountered.
 * @mdversion: (out): The metadata version of this document.
 * @data: (out) (transfer full): A string representation of the YAML under the
 * "data" section in the document.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Reads through a YAML subdocument to retrieve the document type, metadata
 * version and the data section.
 *
 * Returns: TRUE if the document parsed successfully. FALSE if an error was
 * encountered and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
modulemd_yaml_parse_document_type (yaml_parser_t *parser,
                                   enum ModulemdYamlDocumentType *doctype,
                                   guint64 *mdversion,
                                   gchar **data,
                                   GError **error);


/**
 * modulemd_yaml_emit_document_headers:
 * @emitter: (inout): A libyaml emitter object that is positioned where the
 * YAML_DOCUMENT_START_EVENT should occur (so this must be after either a
 * YAML_STREAM_START_EVENT or YAML_DOCUMENT_END_EVENT).
 * @doctype: (in): The document type (see #ModulemdYamlDocumentType)
 * @mdversion: (in): The metadata version for this document
 * @error: (out): A #GError that will return the reason for failing to emit.
 *
 * Creates the YAML header and returns @emitter positioned just before the
 * YAML_MAPPING_START for the "data:" section.
 *
 * Returns: TRUE if the document emitted successfully. FALSE if an error was
 * encountered and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
modulemd_yaml_emit_document_headers (yaml_emitter_t *emitter,
                                     enum ModulemdYamlDocumentType doctype,
                                     guint64 mdversion,
                                     GError **error);

G_END_DECLS
