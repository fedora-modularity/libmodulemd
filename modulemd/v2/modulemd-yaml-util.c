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
_write_yaml_string (void *data, unsigned char *buffer, size_t size)
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
