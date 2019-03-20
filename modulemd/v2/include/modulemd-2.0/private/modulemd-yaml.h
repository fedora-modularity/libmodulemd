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
#include "modulemd-subdocument-info.h"

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
      if ((_event)->type == YAML_SCALAR_EVENT)                                \
        g_debug ("Parser event: %s: %s",                                      \
                 mmd_yaml_get_event_name ((_event)->type),                    \
                 (const gchar *)event.data.scalar.value);                     \
      else                                                                    \
        {                                                                     \
          g_debug ("Parser event: %s",                                        \
                   mmd_yaml_get_event_name ((_event)->type));                 \
        }                                                                     \
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

#define MMD_SET_PARSED_YAML_STRING(_parser, _error, _fn, _obj)                \
  do                                                                          \
    {                                                                         \
      GError *_nested_error = NULL;                                           \
      g_autofree gchar *_scalar =                                             \
        modulemd_yaml_parse_string (_parser, &_nested_error);                 \
      if (!_scalar)                                                           \
        {                                                                     \
          g_propagate_error (_error, _nested_error);                          \
          return NULL;                                                        \
        }                                                                     \
      _fn (_obj, _scalar);                                                    \
      g_clear_pointer (&_scalar, g_free);                                     \
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
 * modulemd_yaml_parse_bool:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * int64 scalar entry.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: A boolean representing the parsed value. Returns FALSE if a parse
 * error occured and sets @error appropriately.
 *
 * Since: 2.2
 */
gboolean
modulemd_yaml_parse_bool (yaml_parser_t *parser, GError **error);


/**
 * modulemd_yaml_parse_int64:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * int64 scalar entry.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A 64-bit signed integer representing the parsed
 * value. Returns 0 if a parse error occured and sets @error appropriately.
 *
 * Since: 2.0
 */
gint64
modulemd_yaml_parse_int64 (yaml_parser_t *parser, GError **error);


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
 * modulemd_yaml_parse_string_set_from_map:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * map containing a single key which is a sequence with string scalars.
 * @key: (in): The key in a single-key mapping whose contents should be
 * returned as a string set.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Function for retrieving a string set from a single-key map such as
 * data.artifacts, data.api or data.filter from a module stream document.
 *
 * Returns: (transfer full): A newly-allocated GHashtTable * representing the
 * parsed values. All parsed sequence entries are added as keys in the
 * hashtable. NULL if a parse error occured and sets @error appropriately.
 *
 * Since: 2.0
 */
GHashTable *
modulemd_yaml_parse_string_set_from_map (yaml_parser_t *parser,
                                         const gchar *key,
                                         gboolean strict,
                                         GError **error);


/**
 * modulemd_yaml_parse_string_string_map:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * map containing a scalar/scalar key/value pairs.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Function for retrieving a hash table from a str/str map such as
 * data.dependencies in ModuleStreamV1.
 *
 * Returns: (transfer full): A newly-allocated GHashtTable * representing the
 * parsed values. NULL if a parse error occured and sets @error appropriately.
 *
 * Since: 2.0
 */
GHashTable *
modulemd_yaml_parse_string_string_map (yaml_parser_t *parser, GError **error);


/**
 * modulemd_yaml_parse_document_type:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * yaml subdocument immediately prior to a YAML_DOCUMENT_START_EVENT.
 *
 * Reads through a YAML subdocument to retrieve the document type, metadata
 * version and the data section.
 *
 * Returns: (transfer full): A SubdocumentInfo with information on the parse results.
 *
 * Since: 2.0
 */
ModulemdSubdocumentInfo *
modulemd_yaml_parse_document_type (yaml_parser_t *parser);


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

/**
 * modulemd_yaml_emit_variant:
 * @emitter: (inout): A libyaml emitter object that is positioned where the
 * variant should occur.
 * @variant: (in): The variant to emit. It must be either a boolean, string,
 * array or dictionary.
 * @error: (out): A #GError that will return the reason for failing to emit.
 *
 * Returns: TRUE if the variant emitted succesfully. FALSE if an error was
 * encountered and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
modulemd_yaml_emit_variant (yaml_emitter_t *emitter,
                            GVariant *variant,
                            GError **error);


/**
 * mmd_variant_from_scalar:
 * @scalar: (in): A string or boolean value to read into a #GVariant
 *
 * Returns: (transfer full): A new, floating #GVariant representing a string
 * or boolean value matching the scalar passed in.
 *
 * Since: 2.0
 */
GVariant *
mmd_variant_from_scalar (const gchar *scalar);


/**
 * mmd_variant_from_mapping:
 * @parser: (inout): A YAML parser positioned just after a MAPPING_START
 * @error: (out): A #GError that will return the reason for failing to parse.
 *
 * Returns: (transfer full): A new, floating #GVariant representing a hash
 * table with string keys and #GVariant values.
 *
 * Since: 2.0
 */
GVariant *
mmd_variant_from_mapping (yaml_parser_t *parser, GError **error);


/**
 * mmd_variant_from_sequence:
 * @parser: (inout): A YAML parser positioned just after a SEQUENCE_START
 * @error: (out): A #GError that will return the reason for failing to parse.
 *
 * Returns: (transfer full): A new, floating #GVariant representing a list of
 * #GVariant values.
 *
 * Since: 2.0
 */
GVariant *
mmd_variant_from_sequence (yaml_parser_t *parser, GError **error);


/**
 * skip_unknown_yaml:
 * @parser: (inout): A YAML parser positioned just after an unexpected map key
 * @error: (out): A #GError that will return the reason for failing to parse.
 *
 * This function is used to skip a section of YAML that contains unknown keys.
 * The intent here is that it will allow libmodulemd to be forward-compatible
 * with new, backwards-compatible changes in the metadata format. This function
 * will advance @parser to just before the next key in the map.
 *
 * Returns: TRUE if the parser was able to skip the unknown values safely.
 * FALSE and sets @error appropriately if the document was malformed YAML.
 */
gboolean
skip_unknown_yaml (yaml_parser_t *parser, GError **error);


#define SKIP_UNKNOWN(_parser, _returnval, ...)                                \
  do                                                                          \
    {                                                                         \
      g_debug (__VA_ARGS__);                                                  \
      if (strict)                                                             \
        {                                                                     \
          MMD_YAML_ERROR_EVENT_EXIT_FULL (                                    \
            error, event, _returnval, __VA_ARGS__);                           \
        }                                                                     \
                                                                              \
      if (!skip_unknown_yaml (_parser, error))                                \
        return _returnval;                                                    \
      break;                                                                  \
    }                                                                         \
  while (0)


/* A set of macros for simple emitting of common elements */
#define NON_EMPTY_TABLE(table) (g_hash_table_size (table) != 0)

#define NON_EMPTY_ARRAY(array) (array->len != 0)


#define EMIT_SCALAR_FULL(emitter, error, value, style)                        \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_scalar (emitter, value, style, error))                 \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

#define EMIT_SCALAR(emitter, error, value)                                    \
  EMIT_SCALAR_FULL (emitter, error, value, YAML_PLAIN_SCALAR_STYLE)

#define EMIT_KEY_VALUE_FULL(emitter, error, key, value, style)                \
  do                                                                          \
    {                                                                         \
      if (value == NULL)                                                      \
        {                                                                     \
          g_set_error (error,                                                 \
                       MODULEMD_YAML_ERROR,                                   \
                       MODULEMD_YAML_ERROR_EMIT,                              \
                       "Value for key %s was NULL on emit",                   \
                       key);                                                  \
          return FALSE;                                                       \
        }                                                                     \
      EMIT_SCALAR (emitter, error, key);                                      \
      EMIT_SCALAR_FULL (emitter, error, value, style);                        \
    }                                                                         \
  while (0)

#define EMIT_KEY_VALUE(emitter, error, key, value)                            \
  EMIT_KEY_VALUE_FULL (emitter, error, key, value, YAML_PLAIN_SCALAR_STYLE)


#define EMIT_KEY_VALUE_IF_SET(emitter, error, key, value)                     \
  do                                                                          \
    {                                                                         \
      if (value != NULL)                                                      \
        {                                                                     \
          EMIT_KEY_VALUE (emitter, error, key, value);                        \
        }                                                                     \
    }                                                                         \
  while (0)

#define EMIT_MAPPING_START_WITH_STYLE(emitter, error, style)                  \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_start_mapping (emitter, style, error))                 \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

#define EMIT_MAPPING_START(emitter, error)                                    \
  EMIT_MAPPING_START_WITH_STYLE (emitter, error, YAML_BLOCK_MAPPING_STYLE)

#define EMIT_MAPPING_END(emitter, error)                                      \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_end_mapping (emitter, error))                          \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

#define EMIT_SEQUENCE_START_WITH_STYLE(emitter, error, style)                 \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_start_sequence (emitter, style, error))                \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

#define EMIT_SEQUENCE_START(emitter, error)                                   \
  EMIT_SEQUENCE_START_WITH_STYLE (emitter, error, YAML_BLOCK_SEQUENCE_STYLE)

#define EMIT_SEQUENCE_END(emitter, error)                                     \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_end_sequence (emitter, error))                         \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

#define EMIT_HASHTABLE_VALUES_IF_NON_EMPTY(                                   \
  emitter, error, key, table, emitfn)                                         \
  do                                                                          \
    {                                                                         \
      if (NON_EMPTY_TABLE (table))                                            \
        {                                                                     \
          EMIT_SCALAR (emitter, error, key);                                  \
          EMIT_MAPPING_START (emitter, error);                                \
          gsize i;                                                            \
          g_autoptr (GPtrArray) keys =                                        \
            modulemd_ordered_str_keys (table, modulemd_strcmp_sort);          \
          for (i = 0; i < keys->len; i++)                                     \
            {                                                                 \
              if (!emitfn (                                                   \
                    g_hash_table_lookup (table, g_ptr_array_index (keys, i)), \
                    emitter,                                                  \
                    error))                                                   \
                return FALSE;                                                 \
            }                                                                 \
          EMIT_MAPPING_END (emitter, error);                                  \
        }                                                                     \
    }                                                                         \
  while (0)

#define EMIT_HASHTABLE_KEY_VALUES_IF_NON_EMPTY(emitter, error, key, table)    \
  do                                                                          \
    {                                                                         \
      if (NON_EMPTY_TABLE (table))                                            \
        {                                                                     \
          EMIT_SCALAR (emitter, error, key);                                  \
          EMIT_MAPPING_START (emitter, error);                                \
          gsize i;                                                            \
          g_autoptr (GPtrArray) keys =                                        \
            modulemd_ordered_str_keys (table, modulemd_strcmp_sort);          \
          for (i = 0; i < keys->len; i++)                                     \
            {                                                                 \
              EMIT_SCALAR (emitter, error, g_ptr_array_index (keys, i));      \
              EMIT_SCALAR (                                                   \
                emitter,                                                      \
                error,                                                        \
                g_hash_table_lookup (table, g_ptr_array_index (keys, i)));    \
            }                                                                 \
          EMIT_MAPPING_END (emitter, error);                                  \
        }                                                                     \
    }                                                                         \
  while (0)

#define EMIT_STRING_SET(emitter, error, key, table)                           \
  do                                                                          \
    {                                                                         \
      if (!NON_EMPTY_TABLE (table))                                           \
        {                                                                     \
          g_set_error (error,                                                 \
                       MODULEMD_YAML_ERROR,                                   \
                       MODULEMD_YAML_ERROR_EMIT,                              \
                       "String set for key %s was empty on emit",             \
                       key);                                                  \
          return FALSE;                                                       \
        }                                                                     \
      EMIT_STRING_SET_FULL (                                                  \
        emitter, error, key, table, YAML_BLOCK_SEQUENCE_STYLE);               \
    }                                                                         \
  while (0)

#define EMIT_STRING_SET_IF_NON_EMPTY(emitter, error, key, table)              \
  do                                                                          \
    {                                                                         \
      if (NON_EMPTY_TABLE (table))                                            \
        {                                                                     \
          EMIT_STRING_SET (emitter, error, key, table);                       \
        }                                                                     \
    }                                                                         \
  while (0)

#define EMIT_STRING_SET_FULL(emitter, error, key, table, sequence_style)      \
  do                                                                          \
    {                                                                         \
      EMIT_SCALAR (emitter, error, key);                                      \
      EMIT_SEQUENCE_START_WITH_STYLE (emitter, error, sequence_style);        \
      gsize i;                                                                \
      g_autoptr (GPtrArray) keys =                                            \
        modulemd_ordered_str_keys (table, modulemd_strcmp_sort);              \
      for (i = 0; i < keys->len; i++)                                         \
        {                                                                     \
          EMIT_SCALAR (emitter, error, g_ptr_array_index (keys, i));          \
        }                                                                     \
      EMIT_SEQUENCE_END (emitter, error);                                     \
    }                                                                         \
  while (0)

#define EMIT_ARRAY_VALUES(emitter, error, key, array, emitfn)                 \
  do                                                                          \
    {                                                                         \
      if (!NON_EMPTY_ARRAY (array))                                           \
        {                                                                     \
          g_set_error (error,                                                 \
                       MODULEMD_YAML_ERROR,                                   \
                       MODULEMD_YAML_ERROR_EMIT,                              \
                       "Array for key %s was empty on emit",                  \
                       key);                                                  \
          return FALSE;                                                       \
        }                                                                     \
      EMIT_SCALAR (emitter, error, key);                                      \
      EMIT_SEQUENCE_START (emitter, error);                                   \
      gsize i;                                                                \
      for (i = 0; i < array->len; i++)                                        \
        {                                                                     \
          if (!emitfn (g_ptr_array_index (array, i), emitter, error))         \
            return FALSE;                                                     \
        }                                                                     \
      EMIT_SEQUENCE_END (emitter, error);                                     \
    }                                                                         \
  while (0)

#define EMIT_ARRAY_VALUES_IF_NON_EMPTY(emitter, error, key, array, emitfn)    \
  do                                                                          \
    {                                                                         \
      if (NON_EMPTY_ARRAY (array))                                            \
        {                                                                     \
          EMIT_ARRAY_VALUES (emitter, error, key, array, emitfn);             \
        }                                                                     \
    }                                                                         \
  while (0)

G_END_DECLS
