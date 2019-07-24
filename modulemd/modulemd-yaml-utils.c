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

#include "modulemd.h"
#include "private/modulemd-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

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

static GVariant *
mmd_variant_from_scalar (const gchar *scalar)
{
  GVariant *value = NULL;

  /* Treat "TRUE" and "FALSE" as boolean values */
  if (g_strcmp0 (scalar, "TRUE") == 0)
    {
      value = g_variant_new_boolean (TRUE);
    }
  else if (g_strcmp0 (scalar, "FALSE") == 0)
    {
      value = g_variant_new_boolean (FALSE);
    }

  else
    {
      /* Any value we don't handle specifically becomes a string */
      value = g_variant_new_string (scalar);
    }

  return value;
}

gboolean
parse_raw_yaml_mapping (yaml_parser_t *parser,
                        GVariant **variant,
                        GError **error)
{
  gboolean result = FALSE;
  gboolean done = FALSE;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  GVariantDict *dict = NULL;
  GVariant *value = NULL;
  gchar *key = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering parse_raw_yaml_mapping");

  dict = g_variant_dict_new (NULL);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT:
          /* We've processed the whole dictionary */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* We assume that all mapping keys are scalars */
          key = g_strdup ((const gchar *)event.data.scalar.value);

          YAML_PARSER_PARSE_WITH_ERROR_RETURN (
            parser, &value_event, error, "Parser error");

          switch (value_event.type)
            {
            case YAML_SCALAR_EVENT:
              value = mmd_variant_from_scalar (
                (const gchar *)value_event.data.scalar.value);
              break;

            case YAML_MAPPING_START_EVENT:
              if (!parse_raw_yaml_mapping (parser, &value, error))
                {
                  MMD_YAML_ERROR_EVENT_RETURN (
                    error, event, "Failed to parse mapping value");
                }
              break;

            case YAML_SEQUENCE_START_EVENT:
              if (!parse_raw_yaml_sequence (parser, &value, error))
                {
                  MMD_YAML_ERROR_EVENT_RETURN (
                    error, event, "Failed to parse sequence value");
                }
              break;

            default:
              /* We received a YAML event we shouldn't expect at this level */
              MMD_YAML_ERROR_EVENT_RETURN (
                error, event, "Unexpected YAML event in raw mapping");
              break;
            }

          yaml_event_delete (&value_event);
          g_variant_dict_insert_value (dict, key, value);
          g_clear_pointer (&key, g_free);

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_EVENT_RETURN (
            error, event, "Unexpected YAML event in raw mapping");
          break;
        }

      yaml_event_delete (&event);
    }


  *variant = g_variant_dict_end (dict);
  result = TRUE;
error:
  g_free (key);
  g_variant_dict_unref (dict);

  g_debug ("TRACE: exiting parse_raw_yaml_mapping");
  return result;
}

gboolean
parse_raw_yaml_sequence (yaml_parser_t *parser,
                         GVariant **variant,
                         GError **error)
{
  gboolean result = FALSE;
  gboolean done = FALSE;
  MMD_INIT_YAML_EVENT (event);
  GVariant *value = NULL;
  GVariant **array = NULL;
  gchar *key = NULL;
  gsize count = 0;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering parse_raw_yaml_sequence");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_SEQUENCE_END_EVENT:
          /* We've processed the whole sequence */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          value = g_variant_new_variant (
            mmd_variant_from_scalar ((const gchar *)event.data.scalar.value));
          break;

        case YAML_MAPPING_START_EVENT:
          if (!parse_raw_yaml_mapping (parser, &value, error))
            {
              MMD_YAML_ERROR_EVENT_RETURN (
                error, event, "Failed to parse mapping value");
            }
          break;

        case YAML_SEQUENCE_START_EVENT:
          if (!parse_raw_yaml_sequence (parser, &value, error))
            {
              MMD_YAML_ERROR_EVENT_RETURN (
                error, event, "Failed to parse sequence value");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_EVENT_RETURN (
            error, event, "Unexpected YAML event in raw sequence");
          break;
        }

      yaml_event_delete (&event);

      if (!done)
        {
          count++;
          array = g_realloc_n (array, count, sizeof (GVariant *));
          array[count - 1] = value;
          value = NULL;
        }

      yaml_event_delete (&event);
    }

  *variant = g_variant_new_array (G_VARIANT_TYPE_VARIANT, array, count);

  result = TRUE;
error:
  g_free (array);
  g_free (key);

  g_debug ("TRACE: exiting parse_raw_yaml_sequence");
  return result;
}


static gboolean
skip_unknown_yaml_mapping (yaml_parser_t *parser, GError **error);
static gboolean
skip_unknown_yaml_sequence (yaml_parser_t *parser, GError **error);


gboolean
skip_unknown_yaml (yaml_parser_t *parser, GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  MODULEMD_INIT_TRACE

  /* This function is called when an unknown key appears in a mapping.
   * Read the next event and then skip to the end of it.
   */

  YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

  switch (event.type)
    {
    case YAML_SCALAR_EVENT:
      /* If we get a scalar key, we can just return here */
      break;

    case YAML_MAPPING_START_EVENT:
      return skip_unknown_yaml_mapping (parser, error);

    case YAML_SEQUENCE_START_EVENT:
      return skip_unknown_yaml_sequence (parser, error);

    default:
      /* We received a YAML event we shouldn't expect at this level */
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_PARSE,
                   "Unexpected YAML event %s in skip_unknown_yaml()",
                   mmd_yaml_get_event_name (event.type));
      return FALSE;
    }

  return TRUE;
}


static gboolean
skip_unknown_yaml_sequence (yaml_parser_t *parser, GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  gsize depth = 0;
  gboolean done = FALSE;

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_SCALAR_EVENT: break;

        case YAML_MAPPING_START_EVENT:
        case YAML_SEQUENCE_START_EVENT: depth++; break;

        case YAML_MAPPING_END_EVENT: depth--; break;

        case YAML_SEQUENCE_END_EVENT:
          if (depth == 0)
            {
              done = TRUE;
              break;
            }

          depth--;
          break;


        default:
          /* We received a YAML event we shouldn't expect at this level */
          g_set_error (
            error,
            MODULEMD_YAML_ERROR,
            MODULEMD_YAML_ERROR_PARSE,
            "Unexpected YAML event %s in skip_unknown_yaml_sequence()",
            mmd_yaml_get_event_name (event.type));
          return FALSE;
        }

      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
skip_unknown_yaml_mapping (yaml_parser_t *parser, GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  gsize depth = 0;
  gboolean done = FALSE;

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_SCALAR_EVENT: break;

        case YAML_MAPPING_START_EVENT:
        case YAML_SEQUENCE_START_EVENT: depth++; break;

        case YAML_SEQUENCE_END_EVENT: depth--; break;

        case YAML_MAPPING_END_EVENT:
          if (depth == 0)
            {
              done = TRUE;
              break;
            }

          depth--;
          break;


        default:
          /* We received a YAML event we shouldn't expect at this level */
          g_set_error (
            error,
            MODULEMD_YAML_ERROR,
            MODULEMD_YAML_ERROR_PARSE,
            "Unexpected YAML event %s in skip_unknown_yaml_sequence()",
            mmd_yaml_get_event_name (event.type));
          return FALSE;
        }

      yaml_event_delete (&event);
    }

  return TRUE;
}


gboolean
emit_yaml_variant (yaml_emitter_t *emitter, GVariant *variant, GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *scalar = NULL;
  GVariantIter iter;
  GVariant *value = NULL;

  if (g_variant_is_of_type (variant, G_VARIANT_TYPE_STRING))
    {
      /* Print the string as a scalar */
      scalar = g_strdup (g_variant_get_string (variant, NULL));
      g_debug ("Printing scalar: %s", scalar);
      MMD_YAML_EMIT_SCALAR (&event, scalar, YAML_PLAIN_SCALAR_STYLE);
    }

  else if (g_variant_is_of_type (variant, G_VARIANT_TYPE_BOOLEAN))
    {
      /* Print the boolean as a scalar */
      if (g_variant_get_boolean (variant))
        {
          scalar = g_strdup ("TRUE");
        }
      else
        {
          scalar = g_strdup ("FALSE");
        }
      g_debug ("Printing boolean: %s", scalar);
      MMD_YAML_EMIT_SCALAR (&event, scalar, YAML_PLAIN_SCALAR_STYLE);
    }

  else if (g_variant_is_of_type (variant, G_VARIANT_TYPE_DICTIONARY))
    {
      /* Start the YAML mapping */
      yaml_mapping_start_event_initialize (
        &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error starting variant mapping");

      g_variant_iter_init (&iter, variant);
      while (g_variant_iter_next (&iter, "{sv}", &scalar, &value))
        {
          /* Loop through all entries in this dictionary and parse them
           * recursively through this function again
           */

          /* Print the key as a scalar */
          g_debug ("Printing scalar key: %s", scalar);
          MMD_YAML_EMIT_SCALAR (&event, scalar, YAML_PLAIN_SCALAR_STYLE);

          /* Recurse into the value */
          emit_yaml_variant (emitter, value, error);

          g_clear_pointer (&value, g_variant_unref);
        }

      /* Terminate the YAML mapping */
      yaml_mapping_end_event_initialize (&event);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error ending variant mapping");
    }

  else if (g_variant_is_of_type (variant, G_VARIANT_TYPE_ARRAY))
    {
      /* Start the YAML sequence */
      yaml_sequence_start_event_initialize (
        &event, NULL, NULL, 1, YAML_BLOCK_SEQUENCE_STYLE);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error starting variant sequence");

      g_variant_iter_init (&iter, variant);
      while (g_variant_iter_next (&iter, "v", &value))
        {
          /* Loop through all entries in this array and parse them
           * recursively through this function again
           */
          emit_yaml_variant (emitter, value, error);

          g_clear_pointer (&value, g_variant_unref);
        }

      /* Terminate the YAML sequence */
      yaml_sequence_end_event_initialize (&event);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error ending variant sequence");
    }

  else
    {
      g_debug ("Unhandled variant type: %s",
               g_variant_get_type_string (variant));
      event.type = YAML_NO_EVENT;
      MMD_YAML_ERROR_EVENT_RETURN (error, event, "Unhandled variant type");
    }

  result = TRUE;

error:
  g_free (scalar);
  if (value)
    {
      g_variant_unref (value);
    }

  return result;
}

ModulemdModule **
mmd_yaml_dup_modules (GPtrArray *objects)
{
  GObject *object = NULL;
  gsize module_count = 0;
  ModulemdModule **modules = NULL;

  g_return_val_if_fail (objects, NULL);

  /* Assume that all objects are modules */
  modules = g_new0 (ModulemdModule *, objects->len + 1);

  for (gsize i = 0; i < objects->len; i++)
    {
      object = g_ptr_array_index (objects, i);
      if (MODULEMD_IS_MODULE (object))
        {
          modules[module_count] =
            modulemd_module_copy (MODULEMD_MODULE (object));
          module_count++;
        }
      else if (MODULEMD_IS_MODULESTREAM (object))
        {
          modules[module_count] = modulemd_module_new_from_modulestream (
            MODULEMD_MODULESTREAM (object));
          module_count++;
        }
    }

  return modules;
}

GPtrArray *
mmd_yaml_convert_modulestreams (GPtrArray *objects)
{
  GPtrArray *compat_data = NULL;
  GObject *object = NULL;
  gsize i;

  compat_data = g_ptr_array_new_full (objects->len, g_object_unref);

  for (i = 0; i < objects->len; i++)
    {
      object = g_ptr_array_index (objects, i);
      if (MODULEMD_IS_MODULESTREAM (object))
        {
          g_ptr_array_add (objects,
                           modulemd_module_new_from_modulestream (
                             MODULEMD_MODULESTREAM (object)));
        }
      else
        {
          g_ptr_array_add (compat_data, g_object_ref (object));
        }
    }

  return compat_data;
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
