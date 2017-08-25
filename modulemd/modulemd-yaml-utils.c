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
  g_free (key);

  g_debug ("TRACE: exiting parse_raw_yaml_sequence");
  return ret;
}
