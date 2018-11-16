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
mmd_emitter_strv (yaml_emitter_t *emitter, const GStrv list, GError **error)
{
  int ret;
  g_autoptr (GError) nested_error = NULL;
  MMD_INIT_YAML_EVENT (event);
  int numentries = g_strv_length (list);

  ret = mmd_emitter_start_sequence (
    emitter, YAML_BLOCK_SEQUENCE_STYLE, &nested_error);
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

  return g_strdup ((const gchar *)event.data.scalar.value);
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
          g_hash_table_add (result,
                            g_strdup ((const gchar *)event.data.scalar.value));
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
