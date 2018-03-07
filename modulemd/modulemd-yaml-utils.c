/* modulemd-yaml-utils.c
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

#include "modulemd.h"
#include "modulemd-yaml.h"

int
_write_yaml_string (void *data, unsigned char *buffer, size_t size)
{
  struct modulemd_yaml_string *yaml_string =
    (struct modulemd_yaml_string *)data;

  yaml_string->str =
    g_realloc_n (yaml_string->str, yaml_string->len + size + 1, sizeof (char));

  memcpy (yaml_string->str + yaml_string->len, buffer, size);
  yaml_string->len += size;
  yaml_string->str[yaml_string->len] = '\0';

  return 1;
}

gboolean
parse_raw_yaml_mapping (yaml_parser_t *parser,
                        GVariant **variant,
                        GError **error)
{
  gboolean ret = FALSE;
  gboolean done = FALSE;
  yaml_event_t event;
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
            parser, &event, error, "Parser error");

          switch (event.type)
            {
            case YAML_SCALAR_EVENT:
              value =
                g_variant_new_string ((const gchar *)event.data.scalar.value);
              break;

            case YAML_MAPPING_START_EVENT:
              if (!parse_raw_yaml_mapping (parser, &value, error))
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse mapping value");
                }
              break;

            case YAML_SEQUENCE_START_EVENT:
              if (!parse_raw_yaml_sequence (parser, &value, error))
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse sequence value");
                }
              break;

            default:
              /* We received a YAML event we shouldn't expect at this level */
              MMD_YAML_ERROR_RETURN (error,
                                     "Unexpected YAML event in raw mapping");
              break;
            }
          g_variant_dict_insert_value (dict, key, value);
          g_clear_pointer (&key, g_free);

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error,
                                 "Unexpected YAML event in raw mapping");
          break;
        }
    }


  *variant = g_variant_dict_end (dict);
  ret = TRUE;
error:
  g_free (key);
  g_variant_dict_unref (dict);

  g_debug ("TRACE: exiting parse_raw_yaml_mapping");
  return ret;
}

gboolean
parse_raw_yaml_sequence (yaml_parser_t *parser,
                         GVariant **variant,
                         GError **error)
{
  gboolean ret = FALSE;
  gboolean done = FALSE;
  yaml_event_t event;
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
            g_variant_new_string ((const gchar *)event.data.scalar.value));
          break;

        case YAML_MAPPING_START_EVENT:
          if (!parse_raw_yaml_mapping (parser, &value, error))
            {
              MMD_YAML_ERROR_RETURN (error, "Failed to parse mapping value");
            }
          break;

        case YAML_SEQUENCE_START_EVENT:
          if (!parse_raw_yaml_sequence (parser, &value, error))
            {
              MMD_YAML_ERROR_RETURN (error, "Failed to parse sequence value");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error,
                                 "Unexpected YAML event in raw sequence");
          break;
        }

      if (!done)
        {
          count++;
          array = g_realloc_n (array, count, sizeof (GVariant *));
          array[count - 1] = value;
          value = NULL;
        }
    }

  *variant = g_variant_new_array (G_VARIANT_TYPE_VARIANT, array, count);

  ret = TRUE;
error:
  g_free (array);
  g_free (key);

  g_debug ("TRACE: exiting parse_raw_yaml_sequence");
  return ret;
}

gboolean
emit_yaml_variant (yaml_emitter_t *emitter, GVariant *variant, GError **error)
{
  gboolean ret = FALSE;
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
      MMD_YAML_ERROR_RETURN (error, "Unhandled variant type");
    }

  ret = TRUE;

error:
  g_free (scalar);
  if (value)
    {
      g_variant_unref (value);
    }

  return ret;
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
    }

  return modules;
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
