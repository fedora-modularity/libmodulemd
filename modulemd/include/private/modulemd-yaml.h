/*
 * This file is part of libmodulemd
 * Copyright (C) 2017-2020 Stephen Gallagher
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

#include "modulemd-errors.h"
#include "modulemd-service-level.h"
#include "modulemd-subdocument-info.h"
#include "private/modulemd-util.h"

G_BEGIN_DECLS


/**
 * SECTION: modulemd-yaml
 * @title: YAML Manipulation Tools
 * @stability: private
 * @short_description: Provides private YAML utilities for internal use.
 */


/**
 * ModulemdYamlDocumentTypeEnum:
 * @MODULEMD_YAML_DOC_UNKNOWN: Represents an unknown YAML document type.
 * @MODULEMD_YAML_DOC_MODULESTREAM: Represents a `modulemd` (see
 * #ModulemdModuleStream) YAML document type.
 * @MODULEMD_YAML_DOC_DEFAULTS: Represents a `modulemd-defaults` (see
 * #ModulemdDefaultsV1) YAML document type.
 * @MODULEMD_YAML_DOC_TRANSLATIONS: Represents a `modulemd-translations` (see
 * #ModulemdTranslation) YAML document type.
 * @MODULEMD_YAML_DOC_PACKAGER: Represents a `modulemd-packager` document.
 * V2 is a subset of #ModulemdModuleStreamV2 containing only the attributes that a
 * package maintainer should modify. V3 (see #ModulemdPackagerV3) is a new YAML
 * document type. Since: 2.9
 * @MODULEMD_YAML_DOC_OBSOLETES: Represents a `modulemd-obsoletes` document (see
 * #ModulemdObsoletes) YAML document type. Since: 2.10
 *
 * Since: 2.0
 */
typedef enum
{
  MODULEMD_YAML_DOC_UNKNOWN = 0,
  MODULEMD_YAML_DOC_MODULESTREAM,
  MODULEMD_YAML_DOC_DEFAULTS,
  MODULEMD_YAML_DOC_TRANSLATIONS,
  MODULEMD_YAML_DOC_PACKAGER,
  MODULEMD_YAML_DOC_OBSOLETES
} ModulemdYamlDocumentTypeEnum;

/**
 * modulemd_yaml_string:
 * @str: A pointer to a block of memory containing YAML.
 * @len: The number of bytes currently in use in @str.
 *
 * #modulemd_yaml_string is an internal representation of an arbitrary length
 * YAML string.
 *
 * Since: 2.0
 */
typedef struct _modulemd_yaml_string
{
  char *str;
  size_t len;
} modulemd_yaml_string;

/**
 * write_yaml_string:
 * @data: (inout): A void pointer to a #modulemd_yaml_string object.
 * @buffer: (in): YAML text to append to @data.
 * @size: (in): The number of bytes from @buffer to append to @data.
 *
 * Additionally memory for @data is automatically allocated if necessary.
 *
 * Since: 2.0
 */
int
write_yaml_string (void *data, unsigned char *buffer, size_t size);

/**
 * modulemd_yaml_string_free:
 * @yaml_string: (inout): A pointer to a #modulemd_yaml_string to be freed.
 *
 * Since: 2.0
 */
void
modulemd_yaml_string_free (modulemd_yaml_string *yaml_string);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (FILE, fclose);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (modulemd_yaml_string,
                               modulemd_yaml_string_free);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC (yaml_event_t, yaml_event_delete);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC (yaml_parser_t, yaml_parser_delete);

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC (yaml_emitter_t, yaml_emitter_delete);

/**
 * mmd_yaml_get_event_name:
 * @type: (in): A libyaml event type.
 *
 * Returns: The string representation for @type.
 *
 * Since: 2.0
 */
const gchar *
mmd_yaml_get_event_name (yaml_event_type_t type);

/**
 * MMD_INIT_YAML_PARSER:
 * @_parser: (out): A variable name to use for the new parser object.
 *
 * This convenience macro allocates and initializes a new libyaml parser object
 * named @_parser.
 *
 * Since: 2.0
 */
#define MMD_INIT_YAML_PARSER(_parser)                                         \
  g_auto (yaml_parser_t) _parser;                                             \
  yaml_parser_initialize (&_parser)

/**
 * MMD_INIT_YAML_EMITTER:
 * @_emitter: (out): A variable name to use for the new emitter object.
 *
 * This convenience macro allocates and initializes a new libyaml emitter
 * object named @_emitter.
 *
 * Since: 2.0
 */
#define MMD_INIT_YAML_EMITTER(_emitter)                                       \
  g_auto (yaml_emitter_t) _emitter;                                           \
  yaml_emitter_initialize (&_emitter)

/**
 * MMD_INIT_YAML_EVENT:
 * @_event: (out): A variable name to use for the new event object.
 *
 * This convenience macro allocates and initializes a new libyaml event object
 * named @_event.
 *
 * Since: 2.0
 */
#define MMD_INIT_YAML_EVENT(_event)                                           \
  g_auto (yaml_event_t) _event;                                               \
  memset (&(_event), 0, sizeof (yaml_event_t))

/**
 * MMD_INIT_YAML_STRING:
 * @_emitter: (inout): A libyaml emitter object.
 * @_string: (out): A variable name to use for the new yaml string object.
 *
 * This convenience macro allocates and initializes a new yaml string object
 * named @_string and associates it as the output target for the libyaml
 * emitter object @_emitter.
 *
 * Since: 2.0
 */
#define MMD_INIT_YAML_STRING(_emitter, _string)                               \
  g_autoptr (modulemd_yaml_string) _string =                                  \
    g_malloc0_n (1, sizeof (modulemd_yaml_string));                           \
  yaml_emitter_set_output (_emitter, write_yaml_string, (void *)_string)

/**
 * MMD_REINIT_YAML_STRING:
 * @_emitter: (inout): A libyaml emitter object to reinitialize.
 * @_string: (inout): A variable name to reinitialize with a new yaml string
 * object.
 *
 * This convenience macro deletes then initializes a new libyaml emitter named
 * @_emitter, then deletes, reallocates, and initializes a new yaml string
 * object named @_string and associates it as the output target for @_emitter.
 *
 * Since: 2.0
 */
#define MMD_REINIT_YAML_STRING(_emitter, _string)                             \
  yaml_emitter_delete (_emitter);                                             \
  yaml_emitter_initialize (_emitter);                                         \
  g_clear_pointer (&_string, modulemd_yaml_string_free);                      \
  _string = g_malloc0_n (1, sizeof (modulemd_yaml_string));                   \
  yaml_emitter_set_output (_emitter, write_yaml_string, (void *)_string)

/**
 * YAML_PARSER_PARSE_WITH_EXIT_FULL:
 * @_parser: (inout): A libyaml parser object positioned at the beginning of an
 * event.
 * @_returnval: (in): The value to return in case of a parsing error.
 * @_event: (out): Returns the libyaml event that was parsed.
 * @_error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * DIRECT USE OF THIS MACRO SHOULD BE AVOIDED. This is the internal
 * implementation for %YAML_PARSER_PARSE_WITH_EXIT_BOOL,
 * %YAML_PARSER_PARSE_WITH_EXIT_INT, and %YAML_PARSER_PARSE_WITH_EXIT which
 * should be used instead.
 *
 * Returns: Continues on if parsing of the event was successful. Returns
 * @_returnval if a parse error occurred and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define YAML_PARSER_PARSE_WITH_EXIT_FULL(_parser, _returnval, _event, _error) \
  do                                                                          \
    {                                                                         \
      if (!yaml_parser_parse (_parser, _event))                               \
        {                                                                     \
          g_debug ("Parser error");                                           \
          g_set_error_literal (_error,                                        \
                               MODULEMD_YAML_ERROR,                           \
                               MMD_YAML_ERROR_UNPARSEABLE,                    \
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

/**
 * YAML_PARSER_PARSE_WITH_EXIT_BOOL:
 * @_parser: (inout): A libyaml parser object positioned at the beginning of an
 * event.
 * @_event: (out): Returns the libyaml event that was parsed.
 * @_error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: Continues on if parsing of the event was successful. Returns
 * FALSE if a parse error occurred and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define YAML_PARSER_PARSE_WITH_EXIT_BOOL(_parser, _event, _error)             \
  YAML_PARSER_PARSE_WITH_EXIT_FULL (_parser, FALSE, _event, _error)

/**
 * YAML_PARSER_PARSE_WITH_EXIT_INT:
 * @_parser: (inout): A libyaml parser object positioned at the beginning of an
 * event.
 * @_event: (out): Returns the libyaml event that was parsed.
 * @_error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: Continues on if parsing of the event was successful. Returns
 * 0 if a parse error occurred and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define YAML_PARSER_PARSE_WITH_EXIT_INT(_parser, _event, _error)              \
  YAML_PARSER_PARSE_WITH_EXIT_FULL (_parser, 0, _event, _error)

/**
 * YAML_PARSER_PARSE_WITH_EXIT:
 * @_parser: (inout): A libyaml parser object positioned at the beginning of an
 * event.
 * @_event: (out): Returns the libyaml event that was parsed.
 * @_error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: Continues on if parsing of the event was successful. Returns
 * NULL if a parse error occurred and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define YAML_PARSER_PARSE_WITH_EXIT(_parser, _event, _error)                  \
  YAML_PARSER_PARSE_WITH_EXIT_FULL (_parser, NULL, _event, _error)


/**
 * MMD_EMIT_WITH_EXIT_FULL:
 * @_emitter: (inout): A libyaml emitter object positioned where @_event
 * belongs in the YAML document.
 * @_returnval: (in): The value to return in case of an output error.
 * @_event: (in): The libyaml event to be emitted.
 * @_error: (out): A #GError that will return the reason for an output error.
 * @...: (in): Additional argument(s) to pass to g_set_error() when setting
 * @_error in case of failure.
 *
 * DIRECT USE OF THIS MACRO SHOULD BE AVOIDED. This is the internal
 * implementation for %MMD_EMIT_WITH_EXIT and %MMD_EMIT_WITH_EXIT_PTR which
 * should be used instead.
 *
 * Returns: Continues on if emitting of the event was successful. Returns
 * @_returnval if an output error occurred and sets @_error appropriately.
 *
 * Since: 2.0
 */
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
          g_set_error (                                                       \
            _error, MODULEMD_YAML_ERROR, MMD_YAML_ERROR_EMIT, __VA_ARGS__);   \
          return _returnval;                                                  \
        }                                                                     \
    }                                                                         \
  while (0)

/**
 * MMD_EMIT_WITH_EXIT:
 * @_emitter: (inout): A libyaml emitter object positioned where @_event
 * belongs in the YAML document.
 * @_event: (in): The libyaml event to be emitted.
 * @_error: (out): A #GError that will return the reason for an output error.
 * @...: (in): Additional argument(s) to pass to g_set_error() when setting
 * @_error in case of failure.
 *
 * Returns: Continues on if emitting of the event was successful. Returns
 * FALSE if an output error occurred and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define MMD_EMIT_WITH_EXIT(_emitter, _event, _error, ...)                     \
  MMD_EMIT_WITH_EXIT_FULL (_emitter, FALSE, _event, _error, __VA_ARGS__)

/**
 * MMD_EMIT_WITH_EXIT_PTR:
 * @_emitter: (inout): A libyaml emitter object positioned where @_event
 * belongs in the YAML document.
 * @_event: (in): The libyaml event to be emitted.
 * @_error: (out): A #GError that will return the reason for an output error.
 * @...: (in): Additional argument(s) to pass to g_set_error() when setting
 * @_error in case of failure.
 *
 * Returns: Continues on if emitting of the event was successful. Returns
 * NULL if an output error occurred and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define MMD_EMIT_WITH_EXIT_PTR(_emitter, _event, _error, ...)                 \
  MMD_EMIT_WITH_EXIT_FULL (_emitter, NULL, _event, _error, __VA_ARGS__)

/**
 * MMD_YAML_ERROR_EVENT_EXIT_FULL:
 * @_error: (out): A #GError that will return the reason for the error.
 * @_errorcode: (in): The exact error code that should be set on @_error.
 * @_event: (in): The libyaml event for which an error is to be reported.
 * @_returnval: (in): The error value to return.
 * @...: (in): Additional argument(s) to pass to g_set_error() when setting
 * @_error.
 *
 * DIRECT USE OF THIS MACRO SHOULD BE AVOIDED. This is the internal
 * implementation for %MMD_YAML_ERROR_EVENT_EXIT,
 * %MMD_YAML_ERROR_EVENT_EXIT_BOOL, and %MMD_YAML_ERROR_EVENT_EXIT_INT which
 * should be used instead.
 *
 * Returns: Returns @_returnval and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define MMD_YAML_ERROR_EVENT_EXIT_FULL(                                       \
  _error, _errorcode, _event, _returnval, ...)                                \
  do                                                                          \
    {                                                                         \
      g_autofree gchar *formatted = g_strdup_printf (__VA_ARGS__);            \
      g_autofree gchar *formatted2 =                                          \
        g_strdup_printf ("%s [line %zu col %zu]",                             \
                         formatted,                                           \
                         _event.start_mark.line + 1,                          \
                         _event.start_mark.column + 1);                       \
      g_debug ("%s", formatted2);                                             \
      g_set_error (                                                           \
        _error, MODULEMD_YAML_ERROR, _errorcode, "%s", formatted2);           \
      return _returnval;                                                      \
    }                                                                         \
  while (0)

/**
 * MMD_YAML_ERROR_EVENT_EXIT:
 * @_error: (out): A #GError that will return the reason for the error.
 * @_event: (in): The libyaml event for which an error is to be reported.
 * @...: (in): Additional argument(s) to pass to g_set_error() when setting
 * @_error.
 *
 * Returns: Returns NULL and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define MMD_YAML_ERROR_EVENT_EXIT(_error, _event, ...)                        \
  MMD_YAML_ERROR_EVENT_EXIT_FULL (                                            \
    _error, MMD_YAML_ERROR_PARSE, _event, NULL, __VA_ARGS__)

/**
 * MMD_YAML_ERROR_EVENT_EXIT_BOOL:
 * @_error: (out): A #GError that will return the reason for the error.
 * @_event: (in): The libyaml event for which an error is to be reported.
 * @...: (in): Additional argument(s) to pass to g_set_error() when setting
 * @_error.
 *
 * Returns: Returns FALSE and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define MMD_YAML_ERROR_EVENT_EXIT_BOOL(_error, _event, ...)                   \
  MMD_YAML_ERROR_EVENT_EXIT_FULL (                                            \
    _error, MMD_YAML_ERROR_PARSE, _event, FALSE, __VA_ARGS__)

/**
 * MMD_YAML_ERROR_EVENT_EXIT_INT:
 * @_error: (out): A #GError that will return the reason for the error.
 * @_event: (in): The libyaml event for which an error is to be reported.
 * @...: (in): Additional argument(s) to pass to g_set_error() when setting
 * @_error.
 *
 * Returns: Returns 0 and sets @_error appropriately.
 *
 * Since: 2.0
 */
#define MMD_YAML_ERROR_EVENT_EXIT_INT(_error, _event, ...)                    \
  MMD_YAML_ERROR_EVENT_EXIT_FULL (                                            \
    _error, MMD_YAML_ERROR_PARSE, _event, 0, __VA_ARGS__)

/**
 * MMD_SET_PARSED_YAML_STRING:
 * @_parser: (inout): A libyaml parser object positioned at the beginning of an
 * expected string event.
 * @_error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 * @_fn: (in): A setter method for a property of object @_obj to be called with
 * the successfully parsed string.
 * @_obj: (inout): The object that is to store the parsed string via its @_fn
 * setter method.
 *
 * This convenience macro can be used when a YAML string (scalar) event is
 * expected, and that string is to be stored in a property of libmodulemd
 * object @_obj via setter method @_fn.
 *
 * Returns: Continues on if parsing of the event was successful. Returns
 * NULL if a parse error occurred and sets @_error appropriately.
 *
 * Since: 2.0
 */
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

/**
 * mmd_emitter_start_stream:
 * @emitter: (inout): A libyaml emitter object that will be positioned at the
 * beginning of a new YAML output stream.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: TRUE if the YAML stream was started successfully. Returns FALSE if
 * an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_start_stream (yaml_emitter_t *emitter, GError **error);

/**
 * mmd_emitter_end_stream:
 * @emitter: (inout): A libyaml emitter object that is positioned at the end of
 * a YAML output stream to be closed.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: TRUE if the YAML stream was closed successfully. Returns FALSE if
 * an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_end_stream (yaml_emitter_t *emitter, GError **error);


/**
 * mmd_emitter_start_document:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new YAML document will be written.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Emits a YAML document start header line.
 *
 * Returns: TRUE if the YAML document was started successfully. Returns FALSE
 * if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_start_document (yaml_emitter_t *emitter, GError **error);

/**
 * mmd_emitter_end_document:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a YAML document terminator will be written.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Emits a YAML document termination line.
 *
 * Returns: TRUE if the YAML document was terminated successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_end_document (yaml_emitter_t *emitter, GError **error);


/**
 * mmd_emitter_start_mapping:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new mapping will be written.
 * @style: (in): The YAML mapping style for the output.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: TRUE if the YAML mapping was started successfully. Returns FALSE if
 * an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_start_mapping (yaml_emitter_t *emitter,
                           yaml_mapping_style_t style,
                           GError **error);

/**
 * mmd_emitter_end_mapping:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a mapping ending will be written.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: TRUE if the YAML mapping was ended successfully. Returns FALSE if
 * an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_end_mapping (yaml_emitter_t *emitter, GError **error);


/**
 * mmd_emitter_start_sequence:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new sequence will be written.
 * @style: (in): The YAML sequence style for the output.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: TRUE if the YAML sequence was started successfully. Returns FALSE
 * if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_start_sequence (yaml_emitter_t *emitter,
                            yaml_sequence_style_t style,
                            GError **error);

/**
 * mmd_emitter_end_sequence:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a sequence ending will be written.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: TRUE if the YAML sequence was ended successfully. Returns FALSE if
 * an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_end_sequence (yaml_emitter_t *emitter, GError **error);


/**
 * mmd_emitter_scalar:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a scalar will be written.
 * @scalar: (in): The scalar (string) to be written.
 * @style: (in): The YAML scalar style for the output.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: TRUE if the YAML scalar was written successfully. Returns FALSE if
 * an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_scalar (yaml_emitter_t *emitter,
                    const gchar *scalar,
                    yaml_scalar_style_t style,
                    GError **error);


/**
 * mmd_emitter_scalar_string:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a scalar will be written.
 * @scalar: (in) (nullable): The scalar string to be written. If the string
 * looks like a number or is empty or undefined, it will be explicitly quoted.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: TRUE if the YAML scalar was written successfully. Returns FALSE if
 * an error occurred and sets @error appropriately.
 *
 * Since: 2.15
 */
gboolean
mmd_emitter_scalar_string (yaml_emitter_t *emitter,
                           const gchar *scalar,
                           GError **error);


/**
 * mmd_emitter_strv:
 * @emitter: (inout): A libyaml emitter object positioned at the start of where
 * a string sequence will be written.
 * @seq_style: (in): The YAML sequence style for the output.
 * @list: (in): A list that will be emitted to the YAML emitter.
 * @error: (out): A #GError that will return the reason for an emitting error.
 *
 * Returns: TRUE if the sequence emitted successfully. FALSE if an error was
 * encountered and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
mmd_emitter_strv (yaml_emitter_t *emitter,
                  yaml_sequence_style_t seq_style,
                  GStrv list,
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
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * string scalar entry.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated `gchar *` representing the
 * parsed value. NULL if a parse error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
gchar *
modulemd_yaml_parse_string (yaml_parser_t *parser, GError **error);


/**
 * modulemd_yaml_parse_bool:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * boolean scalar entry.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: A boolean representing the parsed value. Returns FALSE if a parse
 * error occurred and sets @error appropriately.
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
 * value. Returns 0 if a parse error occurred and sets @error appropriately.
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
 * value. Returns 0 if a parse error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
guint64
modulemd_yaml_parse_uint64 (yaml_parser_t *parser, GError **error);


/**
 * modulemd_yaml_parse_string_set:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * sequence with string scalars.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #GHashTable * representing the
 * parsed value. All parsed sequence entries are added as keys in the
 * hashtable. NULL if a parse error occurred and sets @error appropriately.
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
 * Returns: (transfer full): A newly-allocated #GHashTable * representing the
 * parsed values. All parsed sequence entries are added as keys in the
 * hashtable. NULL if a parse error occurred and sets @error appropriately.
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
 * Returns: (transfer full): A newly-allocated #GHashTable * representing the
 * parsed values. NULL if a parse error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
GHashTable *
modulemd_yaml_parse_string_string_map (yaml_parser_t *parser, GError **error);


/**
 * modulemd_yaml_parse_nested_set:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * map containing scalar keys with string set values.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Function for retrieving a hash table from a str/string-set map such as
 * data.dependencies in #ModulemdModuleStreamV2.
 *
 * Returns: (transfer full): A newly-allocated #GHashTable * representing the
 * parsed values. NULL if a parse error occurred and sets @error appropriately.
 *
 * Since: 2.10
 */
GHashTable *
modulemd_yaml_parse_nested_set (yaml_parser_t *parser, GError **error);

/**
 * modulemd_yaml_emit_nested_set:
 * @emitter: (inout): A libyaml emitter object that is positioned where a nested
 * set (a map containing scalar keys with string set values) should occur.
 * @table: (in): The nested set to emit.
 * @error: (out): A #GError that will return the reason for failing to emit.
 *
 * Returns: TRUE if the nested set emitted successfully. FALSE if an error was
 * encountered and sets @error appropriately.
 *
 * Since: 2.10
 */
gboolean
modulemd_yaml_emit_nested_set (yaml_emitter_t *emitter,
                               GHashTable *table,
                               GError **error);

/**
 * modulemd_yaml_parse_document_type:
 * @parser: (inout): A libyaml parser object positioned at the beginning of a
 * yaml subdocument immediately prior to a `YAML_DOCUMENT_START_EVENT`.
 *
 * Reads through a YAML subdocument to retrieve the document type, metadata
 * version and the data section.
 *
 * Returns: (transfer full): A #ModulemdSubdocumentInfo with information on
 * the parse results.
 *
 * Since: 2.0
 */
ModulemdSubdocumentInfo *
modulemd_yaml_parse_document_type (yaml_parser_t *parser);


/**
 * modulemd_yaml_emit_document_headers:
 * @emitter: (inout): A libyaml emitter object that is positioned where the
 * `YAML_DOCUMENT_START_EVENT` should occur (so this must be after either a
 * `YAML_STREAM_START_EVENT` or `YAML_DOCUMENT_END_EVENT`).
 * @doctype: (in): The document type (see #ModulemdYamlDocumentTypeEnum)
 * @mdversion: (in): The metadata version for this document
 * @error: (out): A #GError that will return the reason for failing to emit.
 *
 * Creates the YAML header and returns @emitter positioned just before the
 * `YAML_MAPPING_START` for the "data:" section.
 *
 * Returns: TRUE if the document emitted successfully. FALSE if an error was
 * encountered and sets @error appropriately.
 *
 * Since: 2.0
 */
gboolean
modulemd_yaml_emit_document_headers (yaml_emitter_t *emitter,
                                     ModulemdYamlDocumentTypeEnum doctype,
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
 * Returns: TRUE if the variant emitted successfully. FALSE if an error was
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
 * @parser: (inout): A YAML parser positioned just after a `MAPPING_START`
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
 * @parser: (inout): A YAML parser positioned just after a `SEQUENCE_START`
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
 * mmd_parse_xmd:
 * @parser: (inout): A YAML parser positioned just after an 'xmd' mapping key.
 * @error: (out): A #GError that will return the reason for failing to parse.
 *
 * Returns: (transfer full): A new, floating #GVariant representing the parsed
 * XMD (eXtensible MetaData).
 *
 * Since: 2.10
 */
GVariant *
mmd_parse_xmd (yaml_parser_t *parser, GError **error);


/**
 * skip_unknown_yaml:
 * @parser: (inout): A YAML parser positioned just after an unexpected map key.
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


/**
 * SKIP_UNKNOWN:
 * @_parser: (inout): A YAML parser positioned just after an unexpected map key.
 * @_returnval: (in): The error value to return.
 * @...: (in): Additional argument(s) to pass to g_set_error() when setting
 * `error`.
 *
 * This convenience macro is a wrapper around skip_unknown_yaml() used to skip
 * a section of YAML that contains unknown keys.
 *
 * NOTE: Local variables `strict` and `error` are expected to be present in the
 * code from which this macro is used.
 *
 * Returns: If `strict` is TRUE or skip_unknown_yaml() fails, @_returnval is
 * returned and `error` is set appropriately.
 *
 * Since: 2.0
 */
#define SKIP_UNKNOWN(_parser, _returnval, ...)                                \
  do                                                                          \
    {                                                                         \
      g_debug (__VA_ARGS__);                                                  \
      if (strict)                                                             \
        {                                                                     \
          MMD_YAML_ERROR_EVENT_EXIT_FULL (error,                              \
                                          MMD_YAML_ERROR_UNKNOWN_ATTR,        \
                                          event,                              \
                                          _returnval,                         \
                                          __VA_ARGS__);                       \
        }                                                                     \
                                                                              \
      if (!skip_unknown_yaml (_parser, error))                                \
        return _returnval;                                                    \
      break;                                                                  \
    }                                                                         \
  while (0)


/* A set of macros for simple emitting of common elements */
/**
 * NON_EMPTY_TABLE:
 * @table: A #GHashTable.
 *
 * Returns: FALSE if @table is empty, otherwise TRUE.
 *
 * Since: 2.0
 */
#define NON_EMPTY_TABLE(table) (g_hash_table_size (table) != 0)

/**
 * NON_EMPTY_ARRAY:
 * @array: A #GPtrArray.
 *
 * Returns: FALSE if @array is empty, otherwise TRUE.
 *
 * Since: 2.0
 */
#define NON_EMPTY_ARRAY(array) (array->len != 0)

/**
 * EMIT_SCALAR_FULL:
 * @emitter: (inout): A libyaml emitter object positioned where a scalar
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an output error.
 * @value: (in): The scalar (string) to be written.
 * @style: (in): The YAML scalar style for the output.
 *
 * Emits scalar @value using style @style.
 *
 * Returns: Continues on if the YAML scalar was written successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.1
 */
#define EMIT_SCALAR_FULL(emitter, error, value, style)                        \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_scalar (emitter, value, style, error))                 \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

/**
 * EMIT_SCALAR:
 * @emitter: (inout): A libyaml emitter object positioned where a scalar
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an output error.
 * @value: (in): The scalar (string) to be written.
 *
 * Emits scalar @value using style `YAML_PLAIN_SCALAR_STYLE`.
 *
 * Returns: Continues on if the YAML scalar was written successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_SCALAR(emitter, error, value)                                    \
  EMIT_SCALAR_FULL (emitter, error, value, YAML_PLAIN_SCALAR_STYLE)

/**
 * EMIT_SCALAR_STRING:
 * @emitter: (inout): A libyaml emitter object positioned where a scalar
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an output error.
 * @value: (in): The scalar (string) to be written.
 *
 * Emits a string @value. Using style `YAML_DOUBLE_QUOTED_SCALAR_STYLE` style
 * if the @value is empty or looks like a number. Otherwise, using
 * `YAML_PLAIN_SCALAR_STYLE` style. This autoquoting of number-like
 * strings is in place to prevent other YAML applications from trimming
 * trailing null digits and to force them handle the values as a string (e.g.
 * "1.0" will be serialized as "1.0" instead of 1.0 which some applications
 * interpret as 1. We do not always quote to keep the YAML file concise and
 * similar to previous serialization styles.
 *
 * Returns: Continues on if the YAML scalar was written successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.15
 */
#define EMIT_SCALAR_STRING(emitter, error, value)                             \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_scalar_string ((emitter), (value), (error)))           \
        return FALSE;                                                         \
    }                                                                         \
  while (0)


/**
 * EMIT_KEY_VALUE_FULL:
 * @emitter: (inout): A libyaml emitter object positioned where a scalar
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an output error.
 * @key: (in): The key (string) to be written.
 * @value: (in): The scalar (string) to be written.
 * @style: (in): The YAML scalar style for the output.
 *
 * Emits key/value pair (@key: @value) using style @style.
 *
 * NOTE: This macro outputs both a key and a value for that key, thus it must
 * only be used from within a YAML mapping.
 *
 * Returns: Continues on if the YAML key/value pair was written successfully.
 * Returns FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.1
 */
#define EMIT_KEY_VALUE_FULL(emitter, error, key, value, style)                \
  do                                                                          \
    {                                                                         \
      if (value == NULL)                                                      \
        {                                                                     \
          g_set_error (error,                                                 \
                       MODULEMD_YAML_ERROR,                                   \
                       MMD_YAML_ERROR_EMIT,                                   \
                       "Value for key %s was NULL on emit",                   \
                       key);                                                  \
          return FALSE;                                                       \
        }                                                                     \
      EMIT_SCALAR (emitter, error, key);                                      \
      EMIT_SCALAR_FULL (emitter, error, value, style);                        \
    }                                                                         \
  while (0)

/**
 * EMIT_KEY_VALUE:
 * @emitter: (inout): A libyaml emitter object positioned where a scalar
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an output error.
 * @key: (in): The key (string) to be written.
 * @value: (in): The scalar (string) to be written.
 *
 * Emits key/value pair (@key: @value) using style `YAML_PLAIN_SCALAR_STYLE`.
 *
 * NOTE: This macro outputs both a key and a value for that key, thus it must
 * only be used from within a YAML mapping.
 *
 * Returns: Continues on if the YAML key/value pair was written successfully.
 * Returns FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_KEY_VALUE(emitter, error, key, value)                            \
  EMIT_KEY_VALUE_FULL (emitter, error, key, value, YAML_PLAIN_SCALAR_STYLE)


/**
 * EMIT_KEY_VALUE_IF_SET:
 * @emitter: (inout): A libyaml emitter object positioned where a scalar
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an output error.
 * @key: (in): The key (string) to be written.
 * @value: (in): The scalar (string) to be written.
 *
 * Emits key/value pair (@key: @value) only if @value is not NULL.
 *
 * NOTE: This macro outputs both a key and a value for that key, thus it must
 * only be used from within a YAML mapping.
 *
 * Returns: Continues on if @value is NULL or the YAML key/value pair was
 * written successfully. Returns FALSE if an error occurred and sets @error
 * appropriately.
 *
 * Since: 2.0
 */
#define EMIT_KEY_VALUE_IF_SET(emitter, error, key, value)                     \
  do                                                                          \
    {                                                                         \
      if (value != NULL)                                                      \
        {                                                                     \
          EMIT_KEY_VALUE (emitter, error, key, value);                        \
        }                                                                     \
    }                                                                         \
  while (0)

/**
 * EMIT_MAPPING_START_WITH_STYLE:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new mapping will be written.
 * @error: (out): A #GError that will return the reason for any error.
 * @style: (in): The YAML mapping style for the output.
 *
 * Returns: Continues on if the YAML mapping was started successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_MAPPING_START_WITH_STYLE(emitter, error, style)                  \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_start_mapping (emitter, style, error))                 \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

/**
 * EMIT_MAPPING_START:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new mapping will be written.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: Continues on if the YAML mapping was started successfully using
 * style `YAML_BLOCK_MAPPING_STYLE`. Returns FALSE if an error occurred and
 * sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_MAPPING_START(emitter, error)                                    \
  EMIT_MAPPING_START_WITH_STYLE (emitter, error, YAML_BLOCK_MAPPING_STYLE)

/**
 * EMIT_MAPPING_END:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a mapping ending will be written.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: Continues on if the YAML mapping was ended successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_MAPPING_END(emitter, error)                                      \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_end_mapping (emitter, error))                          \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

/**
 * EMIT_SEQUENCE_START_WITH_STYLE:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new sequence will be written.
 * @error: (out): A #GError that will return the reason for any error.
 * @style: (in): The YAML sequence style for the output.
 *
 * Returns: Continues on if the YAML sequence was started successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_SEQUENCE_START_WITH_STYLE(emitter, error, style)                 \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_start_sequence (emitter, style, error))                \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

/**
 * EMIT_SEQUENCE_START:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new sequence will be written.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: Continues on if the YAML sequence was started successfully using
 * style `YAML_BLOCK_SEQUENCE_STYLE`. Returns FALSE if an error occurred and
 * sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_SEQUENCE_START(emitter, error)                                   \
  EMIT_SEQUENCE_START_WITH_STYLE (emitter, error, YAML_BLOCK_SEQUENCE_STYLE)

/**
 * EMIT_SEQUENCE_END:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a sequence ending will be written.
 * @error: (out): A #GError that will return the reason for any error.
 *
 * Returns: Continues on if the YAML sequence was ended successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_SEQUENCE_END(emitter, error)                                     \
  do                                                                          \
    {                                                                         \
      if (!mmd_emitter_end_sequence (emitter, error))                         \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

/**
 * EMIT_HASHTABLE_VALUES_IF_NON_EMPTY:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new mapping will be written.
 * @error: (out): A #GError that will return the reason for any error.
 * @key: (in): The name to be used as the identifier for the output mapping.
 * @table: (in): The #GHashTable that is to be output.
 * @emitfn: (in): A function used to emit each of the hash table values.
 *
 * Does nothing if the hash table @table is empty. Otherwise, calls @emitfn
 * to emit each of the values from @table identified as @key.
 *
 * NOTE: This macro outputs both a key and a sub-mapping value for that key,
 * thus it must only be used from within a YAML mapping.
 *
 * Returns: Continues on if the YAML mapping was output successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
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

/**
 * EMIT_HASHTABLE_KEY_VALUES_IF_NON_EMPTY:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new mapping will be written.
 * @error: (out): A #GError that will return the reason for any error.
 * @key: (in): The name to be used as the identifier for the output mapping.
 * @table: (in): The #GHashTable that is to be output. Both the keys and values
 * must be strings.
 *
 * Does nothing if the hash table @table is empty. Otherwise, outputs a YAML
 * mapping with the key/value pairs from @table identified as @key.
 *
 * NOTE: This macro outputs both a key and a sub-mapping value for that key,
 * thus it must only be used from within a YAML mapping.
 *
 * Returns: Continues on if the YAML mapping was output successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
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

/**
 * EMIT_STRING_SET:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new sequence will be written.
 * @error: (out): A #GError that will return the reason for any error.
 * @key: (in): The name to be used as the identifier for the output sequence.
 * @table: (in): The #GHashTable that is to be output. The keys are expected to
 * be a set() of strings.
 *
 * Raises an error if the hash table @table is empty. Otherwise, outputs a YAML
 * sequence using style `YAML_BLOCK_SEQUENCE_STYLE` with the keys from @table
 * identified as @key.
 *
 * NOTE: This macro outputs both a key and an array value for that key, thus it
 * must only be used from within a YAML mapping.
 *
 * Returns: Continues on if the YAML sequence was output successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_STRING_SET(emitter, error, key, table)                           \
  do                                                                          \
    {                                                                         \
      if (!NON_EMPTY_TABLE (table))                                           \
        {                                                                     \
          g_set_error (error,                                                 \
                       MODULEMD_YAML_ERROR,                                   \
                       MMD_YAML_ERROR_EMIT,                                   \
                       "String set for key %s was empty on emit",             \
                       key);                                                  \
          return FALSE;                                                       \
        }                                                                     \
      EMIT_STRING_SET_FULL (                                                  \
        emitter, error, key, table, YAML_BLOCK_SEQUENCE_STYLE);               \
    }                                                                         \
  while (0)

/**
 * EMIT_STRING_SET_IF_NON_EMPTY:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new sequence will be written.
 * @error: (out): A #GError that will return the reason for any error.
 * @key: (in): The name to be used as the identifier for the output sequence.
 * @table: (in): The #GHashTable that is to be output. The keys are expected to
 * be a set() of strings.
 *
 * Does nothing if the hash table @table is empty. Otherwise, outputs a YAML
 * sequence using style `YAML_BLOCK_SEQUENCE_STYLE` with the keys from @table
 * identified as @key.
 *
 * NOTE: This macro outputs both a key and an array value for that key, thus it
 * must only be used from within a YAML mapping.
 *
 * Returns: Continues on if the YAML sequence was output successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_STRING_SET_IF_NON_EMPTY(emitter, error, key, table)              \
  do                                                                          \
    {                                                                         \
      if (NON_EMPTY_TABLE (table))                                            \
        {                                                                     \
          EMIT_STRING_SET (emitter, error, key, table);                       \
        }                                                                     \
    }                                                                         \
  while (0)

/**
 * EMIT_STRING_SET_FULL:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new sequence will be written.
 * @error: (out): A #GError that will return the reason for any error.
 * @key: (in): The name to be used as the identifier for the output sequence.
 * @table: (in): The #GHashTable that is to be output. The keys are expected to
 * be a set() of strings.
 * @sequence_style: (in): The YAML sequence style for the output.
 *
 * Outputs a YAML sequence with the keys from @table identified as @key.
 *
 * NOTE: This macro outputs both a key and an array value for that key, thus it
 * must only be used from within a YAML mapping.
 *
 * Returns: Continues on if the YAML sequence was output successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.1
 */
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
          EMIT_SCALAR_STRING (emitter, error, g_ptr_array_index (keys, i));   \
        }                                                                     \
      EMIT_SEQUENCE_END (emitter, error);                                     \
    }                                                                         \
  while (0)

/**
 * EMIT_ARRAY_VALUES:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new sequence will be written.
 * @error: (out): A #GError that will return the reason for any error.
 * @key: (in): The name to be used as the identifier for the output sequence.
 * @array: (in): The #GPtrArray that is to be output.
 * @emitfn: (in): A function used to emit each of the array values.
 *
 * Raises an error if the array @array is empty. Otherwise, calls @emitfn
 * to emit each of the values from @array identified as @key.
 *
 * NOTE: This macro outputs both a key and an array value for that key, thus it
 * must only be used from within a YAML mapping.
 *
 * Returns: Continues on if the YAML sequence was output successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
#define EMIT_ARRAY_VALUES(emitter, error, key, array, emitfn)                 \
  do                                                                          \
    {                                                                         \
      if (!NON_EMPTY_ARRAY (array))                                           \
        {                                                                     \
          g_set_error (error,                                                 \
                       MODULEMD_YAML_ERROR,                                   \
                       MMD_YAML_ERROR_EMIT,                                   \
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

/**
 * EMIT_ARRAY_VALUES_IF_NON_EMPTY:
 * @emitter: (inout): A libyaml emitter object that is positioned at the start
 * of where a new sequence will be written.
 * @error: (out): A #GError that will return the reason for any error.
 * @key: (in): The name to be used as the identifier for the output sequence.
 * @array: (in): The #GPtrArray that is to be output.
 * @emitfn: (in): A function used to emit each of the array values.
 *
 * Does nothing if the array @array is empty. Otherwise, calls @emitfn to emit
 * each of the values from @array identified as @key.
 *
 * NOTE: This macro outputs both a key and an array value for that key, thus it
 * must only be used from within a YAML mapping.
 *
 * Returns: Continues on if the YAML sequence was output successfully. Returns
 * FALSE if an error occurred and sets @error appropriately.
 *
 * Since: 2.0
 */
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
