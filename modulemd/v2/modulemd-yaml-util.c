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
#include <yaml.h>
#include <inttypes.h>
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"


GQuark
modulemd_yaml_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-yaml-error-quark");
}


void
modulemd_yaml_string_free (modulemd_yaml_string *yaml_string)
{
  g_clear_pointer (&yaml_string->str, g_free);
  g_clear_pointer (&yaml_string, g_free);
}


int
write_yaml_string (void *data, unsigned char *buffer, size_t size)
{
  modulemd_yaml_string *yaml_string = (modulemd_yaml_string *)data;
  gsize total;

  if (!g_size_checked_add (&total, yaml_string->len, size + 1))
    {
      return 0;
    }

  yaml_string->str = g_realloc_n (yaml_string->str, total, sizeof (char));

  memcpy (yaml_string->str + yaml_string->len, buffer, size);
  yaml_string->len += size;
  yaml_string->str[yaml_string->len] = '\0';

  return 1;
}


const gchar *
mmd_yaml_get_event_name (yaml_event_type_t type)
{
  switch (type)
    {
    case YAML_NO_EVENT: return "YAML_NO_EVENT";

    case YAML_STREAM_START_EVENT: return "YAML_STREAM_START_EVENT";

    case YAML_STREAM_END_EVENT: return "YAML_STREAM_END_EVENT";

    case YAML_DOCUMENT_START_EVENT: return "YAML_DOCUMENT_START_EVENT";

    case YAML_DOCUMENT_END_EVENT: return "YAML_DOCUMENT_END_EVENT";

    case YAML_ALIAS_EVENT: return "YAML_ALIAS_EVENT";

    case YAML_SCALAR_EVENT: return "YAML_SCALAR_EVENT";

    case YAML_SEQUENCE_START_EVENT: return "YAML_SEQUENCE_START_EVENT";

    case YAML_SEQUENCE_END_EVENT: return "YAML_SEQUENCE_END_EVENT";

    case YAML_MAPPING_START_EVENT: return "YAML_MAPPING_START_EVENT";

    case YAML_MAPPING_END_EVENT: return "YAML_MAPPING_END_EVENT";
    }

  /* Should be unreachable */
  return "Unknown YAML Event";
}


gboolean
mmd_emitter_start_stream (yaml_emitter_t *emitter, GError **error)
{
  int ret;
  MMD_INIT_YAML_EVENT (event);

  yaml_emitter_set_unicode (emitter, TRUE);

  ret = yaml_stream_start_event_initialize (&event, YAML_UTF8_ENCODING);
  if (!ret)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EVENT_INIT,
                   "Could not initialize the stream start event");
      return FALSE;
    }

  MMD_EMIT_WITH_EXIT (
    emitter, &event, error, "Could not start the YAML stream");

  return TRUE;
}


gboolean
mmd_emitter_end_stream (yaml_emitter_t *emitter, GError **error)
{
  int ret;
  MMD_INIT_YAML_EVENT (event);

  ret = yaml_stream_end_event_initialize (&event);
  if (!ret)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EVENT_INIT,
                   "Could not initialize the stream end event");
      return FALSE;
    }

  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Could not end the YAML stream");

  return TRUE;
}


gboolean
mmd_emitter_start_document (yaml_emitter_t *emitter, GError **error)
{
  int ret;
  MMD_INIT_YAML_EVENT (event);

  ret = yaml_document_start_event_initialize (&event, NULL, NULL, NULL, 0);
  if (!ret)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EVENT_INIT,
                   "Could not initialize the document start event");
      return FALSE;
    }

  MMD_EMIT_WITH_EXIT (
    emitter, &event, error, "Could not start the YAML document");

  return TRUE;
}


gboolean
mmd_emitter_end_document (yaml_emitter_t *emitter, GError **error)
{
  int ret;
  MMD_INIT_YAML_EVENT (event);

  ret = yaml_document_end_event_initialize (&event, 0);
  if (!ret)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EVENT_INIT,
                   "Could not initialize the document end event");
      return FALSE;
    }

  MMD_EMIT_WITH_EXIT (
    emitter, &event, error, "Could not end the YAML document");

  return TRUE;
}


gboolean
mmd_emitter_start_mapping (yaml_emitter_t *emitter,
                           yaml_mapping_style_t style,
                           GError **error)
{
  int ret;
  MMD_INIT_YAML_EVENT (event);

  ret = yaml_mapping_start_event_initialize (&event, NULL, NULL, 1, style);
  if (!ret)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EVENT_INIT,
                   "Could not initialize the mapping start event");
      return FALSE;
    }

  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Could not start the mapping");

  return TRUE;
}


gboolean
mmd_emitter_end_mapping (yaml_emitter_t *emitter, GError **error)
{
  int ret;
  MMD_INIT_YAML_EVENT (event);

  ret = yaml_mapping_end_event_initialize (&event);
  if (!ret)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EVENT_INIT,
                   "Could not initialize the mapping end event");
      return FALSE;
    }

  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Could not end the mapping");

  return TRUE;
}


gboolean
mmd_emitter_start_sequence (yaml_emitter_t *emitter,
                            yaml_sequence_style_t style,
                            GError **error)
{
  int ret;
  MMD_INIT_YAML_EVENT (event);

  ret = yaml_sequence_start_event_initialize (&event, NULL, NULL, 1, style);
  if (!ret)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EVENT_INIT,
                   "Could not initialize the sequence start event");
      return FALSE;
    }

  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Could not start the sequence");

  return TRUE;
}


gboolean
mmd_emitter_end_sequence (yaml_emitter_t *emitter, GError **error)
{
  int ret;
  MMD_INIT_YAML_EVENT (event);

  ret = yaml_sequence_end_event_initialize (&event);
  if (!ret)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EVENT_INIT,
                   "Could not initialize the sequence end event");
      return FALSE;
    }

  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Could not end the sequence");

  return TRUE;
}


gboolean
mmd_emitter_scalar (yaml_emitter_t *emitter,
                    const gchar *scalar,
                    yaml_scalar_style_t style,
                    GError **error)
{
  int ret;
  MMD_INIT_YAML_EVENT (event);

  g_debug ("SCALAR: %s", scalar);
  ret = yaml_scalar_event_initialize (&event,
                                      NULL,
                                      NULL,
                                      (yaml_char_t *)scalar,
                                      (int)strlen (scalar),
                                      1,
                                      1,
                                      style);
  if (!ret)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EVENT_INIT,
                   "Could not initialize the scalar event");
      return FALSE;
    }

  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Could not emit scalar value");

  return TRUE;
}


gboolean
mmd_emitter_strv (yaml_emitter_t *emitter,
                  yaml_sequence_style_t seq_style,
                  const GStrv list,
                  GError **error)
{
  int ret;
  g_autoptr (GError) nested_error = NULL;
  MMD_INIT_YAML_EVENT (event);
  int numentries = g_strv_length (list);

  ret = mmd_emitter_start_sequence (emitter, seq_style, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to emit list start: ");
      return FALSE;
    }

  for (int i = 0; i < numentries; i++)
    {
      ret = mmd_emitter_scalar (
        emitter, list[i], YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error, nested_error, "Failed to emit list entry: ");
          return FALSE;
        }
    }

  ret = mmd_emitter_end_sequence (emitter, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to emit list end: ");
      return FALSE;
    }
  return TRUE;
}


GDate *
modulemd_yaml_parse_date (yaml_parser_t *parser, GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  g_auto (GStrv) strv = NULL;

  YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
  if (event.type != YAML_SCALAR_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (error, event, "Date was not a scalar");
    }

  g_debug ("Parsing scalar: %s", (const gchar *)event.data.scalar.value);

  strv = g_strsplit ((const gchar *)event.data.scalar.value, "-", 4);

  if (!strv[0] || !strv[1] || !strv[2])
    {
      MMD_YAML_ERROR_EVENT_EXIT (
        error, event, "Date not in the form YYYY-MM-DD");
    }

  return g_date_new_dmy (g_ascii_strtoull (strv[2], NULL, 10), /* Day */
                         g_ascii_strtoull (strv[1], NULL, 10), /* Month */
                         g_ascii_strtoull (strv[0], NULL, 10)); /* Year */
}

gchar *
modulemd_yaml_parse_string (yaml_parser_t *parser, GError **error)
{
  MMD_INIT_YAML_EVENT (event);

  YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
  if (event.type != YAML_SCALAR_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (error, event, "String was not a scalar");
    }

  g_debug ("Parsing scalar: %s", (const gchar *)event.data.scalar.value);

  return g_strdup ((const gchar *)event.data.scalar.value);
}


gint64
modulemd_yaml_parse_int64 (yaml_parser_t *parser, GError **error)
{
  MMD_INIT_YAML_EVENT (event);

  YAML_PARSER_PARSE_WITH_EXIT_INT (parser, &event, error);
  if (event.type != YAML_SCALAR_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_INT (error, event, "String was not a scalar");
    }

  return g_ascii_strtoll ((const gchar *)event.data.scalar.value, NULL, 10);
}


guint64
modulemd_yaml_parse_uint64 (yaml_parser_t *parser, GError **error)
{
  MMD_INIT_YAML_EVENT (event);

  YAML_PARSER_PARSE_WITH_EXIT_INT (parser, &event, error);
  if (event.type != YAML_SCALAR_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_INT (error, event, "String was not a scalar");
    }

  g_debug ("Parsing scalar: %s", (const gchar *)event.data.scalar.value);

  return g_ascii_strtoull ((const gchar *)event.data.scalar.value, NULL, 10);
}


GHashTable *
modulemd_yaml_parse_string_set (yaml_parser_t *parser, GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_list = FALSE;
  g_autoptr (GHashTable) result =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_SEQUENCE_START_EVENT: in_list = TRUE; break;

        case YAML_SEQUENCE_END_EVENT:
          if (!in_list)
            MMD_YAML_ERROR_EVENT_EXIT (error, event, "Unexpected end of list");
          in_list = FALSE;
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          g_debug ("Parsing scalar: %s",
                   (const gchar *)event.data.scalar.value);
          g_hash_table_add (result,
                            g_strdup ((const gchar *)event.data.scalar.value));

          if (!in_list)
            {
              /* We got a scalar instead of a sequence. Treat it as a list with
               * a single entry
               */
              done = TRUE;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error, event, "Unspected YAML event in list");
          break;
        }
      yaml_event_delete (&event);
    }

  return g_steal_pointer (&result);
}


static gboolean
modulemd_yaml_parse_data (yaml_parser_t *parser,
                          yaml_emitter_t *emitter,
                          GError **error);


static gboolean
modulemd_yaml_parse_document_type_internal (
  yaml_parser_t *parser,
  enum ModulemdYamlDocumentType *_doctype,
  guint64 *_mdversion,
  yaml_emitter_t *emitter,
  GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean had_data = FALSE;
  enum ModulemdYamlDocumentType doctype = MODULEMD_YAML_DOC_UNKNOWN;
  guint64 mdversion = 0;
  g_autofree gchar *doctype_scalar = NULL;
  g_autofree gchar *mdversion_string = NULL;
  g_autoptr (GError) nested_error = NULL;

  if (!mmd_emitter_start_stream (emitter, &nested_error))
    {
      g_propagate_prefixed_error (
        error, nested_error, "Error emitting stream: ");
      return FALSE;
    }

  /*
   * We should assume the initial document start is consumed by the Index.
   * But we still emit it.
   */
  if (!mmd_emitter_start_document (emitter, error))
    return FALSE;

  /* The second event must be the mapping start */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Document did not start with a mappping");
    }
  MMD_EMIT_WITH_EXIT_FULL (
    emitter, FALSE, &event, error, "Error starting mapping");
  yaml_event_delete (&event);

  /* Now process through the document top-level */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT:
          if (!mmd_emitter_end_mapping (emitter, error))
            return FALSE;
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!mmd_emitter_scalar (emitter,
                                   (const gchar *)event.data.scalar.value,
                                   YAML_PLAIN_SCALAR_STYLE,
                                   error))
            return FALSE;

          if (g_str_equal (event.data.scalar.value, "document"))
            {
              if (doctype != MODULEMD_YAML_DOC_UNKNOWN)
                {
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error, event, "Document type encountered twice.");
                }

              doctype_scalar =
                modulemd_yaml_parse_string (parser, &nested_error);
              if (!doctype_scalar)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
              if (!mmd_emitter_scalar (emitter,
                                       (const gchar *)doctype_scalar,
                                       YAML_PLAIN_SCALAR_STYLE,
                                       error))
                return FALSE;

              if (g_str_equal (doctype_scalar, "modulemd"))
                {
                  doctype = MODULEMD_YAML_DOC_MODULESTREAM;
                }
              else if (g_str_equal (doctype_scalar, "modulemd-defaults"))
                {
                  doctype = MODULEMD_YAML_DOC_DEFAULTS;
                }
              else if (g_str_equal (doctype_scalar, "modulemd-translations"))
                {
                  doctype = MODULEMD_YAML_DOC_TRANSLATIONS;
                }
              else
                {
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error, event, "Document type %s unknown.", doctype_scalar);
                }

              g_clear_pointer (&doctype_scalar, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "version"))
            {
              if (mdversion != 0)
                {
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error, event, "Metadata version encountered twice.");
                }

              mdversion = modulemd_yaml_parse_uint64 (parser, &nested_error);
              if (nested_error)
                {
                  /* If we got a parsing error, report it. Otherwise, continue
                   * and we'll catch the invalid mdversion further on
                   */
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
              mdversion_string = g_strdup_printf ("%" PRIu64, mdversion);
              if (!mmd_emitter_scalar (
                    emitter, mdversion_string, YAML_PLAIN_SCALAR_STYLE, error))
                return FALSE;
            }
          else if (g_str_equal (event.data.scalar.value, "data"))
            {
              had_data = TRUE;
              if (!modulemd_yaml_parse_data (parser, emitter, &nested_error))
                return FALSE;
            }

          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in document metadata.");
        }

      yaml_event_delete (&event);
    }

  /* The final event must be the document end */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_DOCUMENT_END_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Document did not end. It just goes on forever...");
    }
  MMD_EMIT_WITH_EXIT_FULL (
    emitter, FALSE, &event, error, "Error ending document");
  yaml_event_delete (&event);

  if (!mmd_emitter_end_stream (emitter, error))
    return FALSE;

  if (doctype == MODULEMD_YAML_DOC_UNKNOWN)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                           "No document type specified");
      return FALSE;
    }

  if (!mdversion)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                           "No metadata version specified");
      return FALSE;
    }

  if (!had_data)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                           "No data section provided");
      return FALSE;
    }

  *_doctype = doctype;
  *_mdversion = mdversion;

  return TRUE;
}


ModulemdSubdocumentInfo *
modulemd_yaml_parse_document_type (yaml_parser_t *parser)
{
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);
  g_autoptr (ModulemdSubdocumentInfo) s = modulemd_subdocument_info_new ();
  enum ModulemdYamlDocumentType doctype = MODULEMD_YAML_DOC_UNKNOWN;
  guint64 mdversion = 0;
  g_autoptr (GError) error = NULL;

  if (!modulemd_yaml_parse_document_type_internal (
        parser, &doctype, &mdversion, &emitter, &error))
    {
      modulemd_subdocument_info_set_gerror (s, error);
    }

  modulemd_subdocument_info_set_doctype (s, doctype);
  modulemd_subdocument_info_set_mdversion (s, mdversion);
  modulemd_subdocument_info_set_yaml (s, yaml_string->str);

  return g_steal_pointer (&s);
}


static gboolean
modulemd_yaml_parse_data (yaml_parser_t *parser,
                          yaml_emitter_t *emitter,
                          GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gsize depth = 0;

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      /* Read in all the YAML from the data section */
      switch (event.type)
        {
        case YAML_SEQUENCE_START_EVENT:
        case YAML_MAPPING_START_EVENT: depth++; break;

        case YAML_SEQUENCE_END_EVENT:
        case YAML_MAPPING_END_EVENT:
          depth--;
          /* Fall through intentionally */

        default:
          if (depth == 0)
            done = TRUE;
          break;
        }

      /* Copy this event to the string */
      MMD_EMIT_WITH_EXIT_FULL (
        emitter, FALSE, &event, error, "Error storing YAML event");
    }

  return TRUE;
}


static const gchar *
modulemd_yaml_get_doctype_string (enum ModulemdYamlDocumentType doctype)
{
  switch (doctype)
    {
    case MODULEMD_YAML_DOC_MODULESTREAM: return "modulemd";

    case MODULEMD_YAML_DOC_DEFAULTS: return "modulemd-defaults";

    case MODULEMD_YAML_DOC_TRANSLATIONS: return "modulemd-translations";

    default: return NULL;
    }
}


gboolean
modulemd_yaml_emit_document_headers (yaml_emitter_t *emitter,
                                     enum ModulemdYamlDocumentType doctype,
                                     guint64 mdversion,
                                     GError **error)
{
  MODULEMD_INIT_TRACE ();
  const gchar *doctype_string = modulemd_yaml_get_doctype_string (doctype);
  g_autofree gchar *mdversion_string = g_strdup_printf ("%" PRIu64, mdversion);

  if (!mmd_emitter_start_document (emitter, error))
    return FALSE;

  if (!mmd_emitter_start_mapping (emitter, YAML_BLOCK_MAPPING_STYLE, error))
    return FALSE;

  if (!mmd_emitter_scalar (
        emitter, "document", YAML_PLAIN_SCALAR_STYLE, error))
    return FALSE;

  if (!mmd_emitter_scalar (
        emitter, doctype_string, YAML_PLAIN_SCALAR_STYLE, error))
    return FALSE;

  if (!mmd_emitter_scalar (emitter, "version", YAML_PLAIN_SCALAR_STYLE, error))
    return FALSE;

  if (!mmd_emitter_scalar (
        emitter, mdversion_string, YAML_PLAIN_SCALAR_STYLE, error))
    return FALSE;

  if (!mmd_emitter_scalar (emitter, "data", YAML_PLAIN_SCALAR_STYLE, error))
    return FALSE;

  return TRUE;
}
