/* modulemd-yaml-parser.c
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

#include <glib.h>
#include <glib/gstdio.h>
#include <yaml.h>
#include <errno.h>
#include "modulemd.h"
#include "modulemd-yaml.h"
#include "modulemd-util.h"

GQuark
modulemd_yaml_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-yaml-error-quark");
}


struct yaml_subdocument
{
  GType type;
  guint64 version;
  GObject *subdocument;
  char *yaml;
};


static gboolean
_parse_yaml (yaml_parser_t *parser, GPtrArray **data, GError **error);

static gboolean
_read_yaml_and_type (yaml_parser_t *parser,
                     gchar **yaml,
                     GType *type,
                     guint64 *version,
                     GError **error);

static gboolean
_parse_subdocument (struct yaml_subdocument *subdocument,
                    ModulemdParsingFunc parse_func,
                    GObject **data,
                    guint64 version,
                    GError **error);


gboolean
parse_yaml_file (const gchar *path, GPtrArray **data, GError **error)
{
  gboolean result = FALSE;
  FILE *yaml_file = NULL;
  yaml_parser_t parser;

  g_debug ("TRACE: entering parse_yaml_file");

  if (error == NULL || *error != NULL)
    {
      MMD_ERROR_RETURN_FULL (
        error, MODULEMD_YAML_ERROR_PROGRAMMING, "GError is initialized.");
      goto error;
    }

  if (!path)
    {
      MMD_ERROR_RETURN_FULL (
        error, MODULEMD_YAML_ERROR_PROGRAMMING, "Path not supplied.");
      goto error;
    }

  yaml_parser_initialize (&parser);

  errno = 0;
  yaml_file = g_fopen (path, "rb");
  if (!yaml_file)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_OPEN,
                   "Failed to open file: %s",
                   g_strerror (errno));
      goto error;
    }

  yaml_parser_set_input_file (&parser, yaml_file);

  if (!_parse_yaml (&parser, data, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not parse YAML");
    }

  result = TRUE;

error:
  yaml_parser_delete (&parser);
  if (yaml_file)
    {
      fclose (yaml_file);
    }
  g_debug ("TRACE: exiting parse_yaml_file");
  return result;
}

gboolean
parse_yaml_string (const gchar *yaml, GPtrArray **data, GError **error)
{
  gboolean result = FALSE;
  yaml_parser_t parser;

  g_debug ("TRACE: entering parse_yaml_string");

  if (error == NULL || *error != NULL)
    {
      MMD_ERROR_RETURN_FULL (
        error, MODULEMD_YAML_ERROR_PROGRAMMING, "GError is initialized.");
      goto error;
    }

  if (!yaml)
    {
      MMD_ERROR_RETURN_FULL (
        error, MODULEMD_YAML_ERROR_PROGRAMMING, "String not supplied.");
      goto error;
    }

  yaml_parser_initialize (&parser);

  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml, strlen (yaml));

  if (!_parse_yaml (&parser, data, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not parse YAML");
    }

  result = TRUE;

error:
  yaml_parser_delete (&parser);

  g_debug ("TRACE: exiting parse_yaml_string");
  return result;
}


static void
modulemd_subdocument_free (gpointer mem)
{
  struct yaml_subdocument *document = (struct yaml_subdocument *)mem;
  g_clear_pointer (&document->subdocument, g_object_unref);
  g_clear_pointer (&document->yaml, g_free);
  g_free (document);
}


static gboolean
_parse_yaml (yaml_parser_t *parser, GPtrArray **data, GError **error)
{
  gboolean result = FALSE;
  gboolean done = FALSE;
  yaml_event_t event;
  struct yaml_subdocument *document = NULL;
  GPtrArray *subdocuments = NULL;
  GPtrArray *objects = NULL;
  GObject *object = NULL;
  g_debug ("TRACE: entering _parse_yaml");

  /* Read through the complete stream once, separating subdocuments and
   * identifying their types
   */
  subdocuments = g_ptr_array_new_full (1, modulemd_subdocument_free);
  objects = g_ptr_array_new_full (1, g_object_unref);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_STREAM_START_EVENT:
          /* The beginning of the YAML stream */
          break;

        case YAML_STREAM_END_EVENT:
          /* All of the subdocuments have been processed */
          done = TRUE;
          break;

        case YAML_DOCUMENT_START_EVENT:
          /* New document to process */
          document = g_new0 (struct yaml_subdocument, 1);

          if (!_read_yaml_and_type (parser,
                                    &document->yaml,
                                    &document->type,
                                    &document->version,
                                    error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (
                error, "Parse error during preprocessing");
            }

          /* Add all valid documents to the list */
          if (document->type != G_TYPE_INVALID)
            {
              g_ptr_array_add (subdocuments, document);
            }
          else
            {
              modulemd_subdocument_free (document);
            }
          document = NULL;
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN_RETHROW (
            error, "Unexpected YAML event during preprocessing");
          break;
        }
    }

  /* Iterate through the subdocuments and process them by type */
  for (gsize i = 0; i < subdocuments->len; i++)
    {
      document = g_ptr_array_index (subdocuments, i);
      if (document->type == MODULEMD_TYPE_MODULE)
        {
          result = _parse_subdocument (
            document, _parse_modulemd, &object, document->version, error);

          if (result)
            {
              g_ptr_array_add (objects, object);
            }
        }
      /* Parsers for other types go here */
      /* else if (document->type == <...>) */
      else
        {
          /* Unknown document type */
          g_set_error_literal (error,
                               MODULEMD_YAML_ERROR,
                               MODULEMD_YAML_ERROR_PARSE,
                               "Unknown document type");
          result = FALSE;
        }

      if (!result)
        {
          if ((*error)->code == MODULEMD_YAML_ERROR_UNPARSEABLE)
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error,
                                             "Error processing subdocuments");
            }
          g_message ("Invalid document [%s]. Skipping it.", (*error)->message);
          g_clear_error (error);
        }
    }

  if (data)
    {
      *data = objects;
    }
  result = TRUE;

error:
  if (!result)
    {
      g_clear_pointer (&objects, g_ptr_array_free);
    }
  g_clear_pointer (&document, modulemd_subdocument_free);

  return result;
}

static gboolean
_read_yaml_and_type (yaml_parser_t *parser,
                     gchar **yaml,
                     GType *type,
                     guint64 *version,
                     GError **error)
{
  gboolean result = FALSE;
  gboolean done = FALSE;
  gsize depth = 0;
  struct modulemd_yaml_string *yaml_string = NULL;
  yaml_event_t event;
  yaml_event_t value_event;
  yaml_emitter_t emitter;

  g_debug ("TRACE: entering _read_yaml_and_type");

  /* In case we don't encounter a "document" type, default to INVALID */
  *type = G_TYPE_INVALID;

  yaml_string = g_malloc0_n (1, sizeof (struct modulemd_yaml_string));
  yaml_emitter_initialize (&emitter);

  yaml_emitter_set_output (&emitter, _write_yaml_string, (void *)yaml_string);

  yaml_stream_start_event_initialize (&event, YAML_UTF8_ENCODING);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error starting stream");

  yaml_document_start_event_initialize (&event, NULL, NULL, NULL, 0);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error starting document");


  while (!done)
    {
      value_event.type = YAML_NO_EVENT;
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_DOCUMENT_END_EVENT: done = TRUE; break;

        case YAML_SEQUENCE_START_EVENT:
        case YAML_MAPPING_START_EVENT: depth++; break;

        case YAML_SEQUENCE_END_EVENT:
        case YAML_MAPPING_END_EVENT: depth--; break;

        case YAML_SCALAR_EVENT:
          if (depth == 1)
            {
              /* If we're in the root of the document, check for the
               * document type and version
               */

              if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                              "document"))
                {
                  if ((*type) != G_TYPE_INVALID)
                    {
                      /* We encountered document-type twice in the same
                       * document root mapping. This shouldn't ever happen
                       */
                      MMD_YAML_ERROR_RETURN (error, "Document type set twice");
                    }

                  YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                    parser, &value_event, error, "Parser error");

                  if (value_event.type != YAML_SCALAR_EVENT)
                    {
                      MMD_YAML_ERROR_RETURN (error,
                                             "Error parsing document type");
                    }

                  if (g_strcmp0 ((const gchar *)value_event.data.scalar.value,
                                 "modulemd") == 0)
                    {
                      *type = MODULEMD_TYPE_MODULE;
                    }
                  /* Handle additional types here */

                  g_debug ("Document type: %d", *type);
                }

              else if (g_strcmp0 ((const gchar *)event.data.scalar.value,
                                  "version") == 0)
                {
                  if ((*version) != 0)
                    {
                      /* We encountered document-version twice in the same
                       * document root mapping. This shouldn't ever happen
                       */
                      MMD_YAML_ERROR_RETURN (error,
                                             "Document version set twice");
                    }

                  YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                    parser, &value_event, error, "Parser error");

                  if (value_event.type != YAML_SCALAR_EVENT)
                    {
                      MMD_YAML_ERROR_RETURN (error,
                                             "Error parsing document version");
                    }

                  *version = g_ascii_strtoull (
                    (const gchar *)value_event.data.scalar.value, NULL, 10);

                  g_debug ("Document version: %d", *version);
                }
            }
          break;

        default:
          /* Just fall through here. */
          break;
        }

      /* Copy this event to the string */
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        &emitter, &event, error, "Error storing YAML event");

      if (value_event.type != YAML_NO_EVENT)
        {
          /* Copy this event to the string */
          YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
            &emitter, &value_event, error, "Error storing YAML event");
        }
    }

  yaml_stream_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error ending stream");

  *yaml = yaml_string->str;
  yaml_string->str = NULL;

  result = TRUE;
error:
  g_clear_pointer (&yaml_string->str, g_free);
  g_clear_pointer (&yaml_string, g_free);

  g_debug ("TRACE: exiting _read_yaml_and_type");
  return result;
}

static gboolean
_parse_subdocument (struct yaml_subdocument *subdocument,
                    ModulemdParsingFunc parse_func,
                    GObject **data,
                    guint64 version,
                    GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gboolean done = FALSE;
  GObject *object = NULL;
  yaml_parser_t parser;

  yaml_parser_initialize (&parser);
  yaml_parser_set_input_string (&parser,
                                (const unsigned char *)subdocument->yaml,
                                strlen (subdocument->yaml));

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        &parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_STREAM_START_EVENT:
          /* Starting the stream here */
          break;

        case YAML_DOCUMENT_START_EVENT:
          if (!parse_func (&parser, &object, version, error))
            {
              g_message ("Invalid [%s] document [%s].",
                         g_type_name (subdocument->type),
                         (*error)->message);
              goto error;
            }
          break;

        case YAML_DOCUMENT_END_EVENT:
          /* This document is complete. */
          break;

        case YAML_STREAM_END_EVENT: done = TRUE; break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event at toplevel");
          break;
        }

      yaml_event_delete (&event);
    }

  *data = object;
  result = TRUE;

error:
  g_debug ("TRACE: exiting _parse_yaml");
  return result;
}


gboolean
_parse_modulemd_date (yaml_parser_t *parser, GDate **_date, GError **error)
{
  gboolean ret = FALSE;
  gchar **strv = NULL;
  yaml_event_t event;

  YAML_PARSER_PARSE_WITH_ERROR_RETURN (parser, &event, error, "Parser error");
  if (event.type != YAML_SCALAR_EVENT)
    {
      MMD_YAML_ERROR_RETURN (error, "Failed to parse date");
    }

  strv = g_strsplit ((const gchar *)event.data.scalar.value, "-", 4);

  if (!strv[0] || !strv[1] || !strv[2])
    {
      MMD_YAML_ERROR_RETURN (error, "Date not in the form YYYY-MM-DD");
    }

  *_date = g_date_new_dmy (g_ascii_strtoull (strv[2], NULL, 10), /* Day */
                           g_ascii_strtoull (strv[1], NULL, 10), /* Month */
                           g_ascii_strtoull (strv[0], NULL, 10)); /* Year */

  ret = TRUE;

error:
  return ret;
}

gboolean
_simpleset_from_sequence (yaml_parser_t *parser,
                          ModulemdSimpleSet **_set,
                          GError **error)
{
  yaml_event_t event;
  gboolean started = FALSE;
  gboolean done = FALSE;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _simpleset_from_sequence");

  set = modulemd_simpleset_new ();


  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_SEQUENCE_START_EVENT:
          /* Sequence has begun */
          started = TRUE;
          break;

        case YAML_SEQUENCE_END_EVENT:
          /* Sequence has concluded. Return */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!started)
            {
              MMD_YAML_ERROR_RETURN (
                error, "Received scalar where sequence expected");
            }
          modulemd_simpleset_add (set, (const gchar *)event.data.scalar.value);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in sequence");
          break;
        }
    }

error:
  if (*error)
    {
      g_object_unref (set);
      return FALSE;
    }
  *_set = set;
  g_debug ("TRACE: exiting _simpleset_from_sequence");
  return TRUE;
}

gboolean
_hashtable_from_mapping (yaml_parser_t *parser,
                         GHashTable **_htable,
                         GError **error)
{
  yaml_event_t event;
  gboolean started = FALSE;
  gboolean done = FALSE;
  GHashTable *htable = NULL;
  gchar *name = NULL;
  gchar *value = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _hashtable_from_mapping");

  htable = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* The dictionary has begun */
          started = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          /* We've processed the whole dictionary */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!started)
            {
              MMD_YAML_ERROR_RETURN (error,
                                     "Received scalar where mapping expected");
            }
          name = g_strdup ((const gchar *)event.data.scalar.value);
          YAML_PARSER_PARSE_WITH_ERROR_RETURN (
            parser, &event, error, "Parser error");
          if (event.type != YAML_SCALAR_EVENT)
            {
              g_free (name);
              MMD_YAML_ERROR_RETURN (error,
                                     "Non-scalar value for dictionary.");
            }
          value = g_strdup ((const gchar *)event.data.scalar.value);

          /* Set this key and value to the hash table */
          g_hash_table_insert (htable, name, value);

          break;


        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in sequence");
          break;
        }
    }
  *_htable = g_hash_table_ref (htable);

error:
  g_hash_table_unref (htable);
  if (*error)
    {
      return FALSE;
    }

  g_debug ("TRACE: exiting _hashtable_from_mapping");
  return TRUE;
}

/* Helper function to skip over sections that aren't yet implemented */
gboolean
_parse_skip (yaml_parser_t *parser, GError **error)
{
  yaml_event_t event;
  gboolean result = FALSE;
  gboolean done = FALSE;
  gsize depth = 0;

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_DOCUMENT_END_EVENT: done = TRUE; break;

        case YAML_SEQUENCE_START_EVENT:
        case YAML_MAPPING_START_EVENT: depth++; break;

        case YAML_SEQUENCE_END_EVENT:
        case YAML_MAPPING_END_EVENT:
          depth--;

          if (depth <= 0)
            {
              /* We've come back up to the original level from which we
               * started
               */
              done = TRUE;
            }
          break;

        default:
          /* Just fall through here. */
          break;
        }
    }

  result = TRUE;
error:
  return result;
}
