/* modulemd-yaml-emitter.c
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
#include <inttypes.h>
#include "modulemd.h"
#include "modulemd-yaml.h"
#include "modulemd-util.h"

gboolean
emit_yaml_file (GPtrArray *objects, const gchar *path, GError **error)
{
  gboolean result = FALSE;
  FILE *yaml_file = NULL;
  yaml_emitter_t emitter;
  yaml_event_t event;
  GObject *object;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (objects, FALSE);

  g_debug ("TRACE: entering emit_yaml_file");

  yaml_emitter_initialize (&emitter);

  errno = 0;
  yaml_file = g_fopen (path, "wb");
  if (!yaml_file)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_OPEN,
                   "Failed to open file: %s",
                   g_strerror (errno));
      goto error;
    }

  yaml_emitter_set_output_file (&emitter, yaml_file);

  yaml_stream_start_event_initialize (&event, YAML_UTF8_ENCODING);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error starting stream");

  for (gsize i = 0; i < objects->len; i++)
    {
      object = g_ptr_array_index (objects, i);

      /* Write out the YAML */
      if (G_OBJECT_TYPE (object) == MODULEMD_TYPE_MODULE)
        {
          if (!_emit_modulemd (&emitter, MODULEMD_MODULE (object), error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not emit YAML");
            }
        }
    }

  yaml_stream_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error ending stream");

  result = TRUE;

error:
  yaml_emitter_delete (&emitter);
  if (yaml_file)
    {
      fclose (yaml_file);
    }

  g_debug ("TRACE: exiting emit_yaml_file");
  return result;
}

gboolean
emit_yaml_string (GPtrArray *objects, gchar **_yaml, GError **error)
{
  gboolean result = FALSE;
  yaml_emitter_t emitter;
  yaml_event_t event;
  struct modulemd_yaml_string *yaml_string = NULL;
  GObject *object;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (objects, FALSE);

  g_debug ("TRACE: entering emit_yaml_string");

  yaml_string = g_malloc0_n (1, sizeof (struct modulemd_yaml_string));

  yaml_emitter_initialize (&emitter);

  yaml_emitter_set_output (&emitter, _write_yaml_string, (void *)yaml_string);

  yaml_stream_start_event_initialize (&event, YAML_UTF8_ENCODING);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error starting stream");

  for (gsize i = 0; i < objects->len; i++)
    {
      object = g_ptr_array_index (objects, i);

      /* Write out the YAML */
      if (MODULEMD_IS_MODULE (object))
        {
          if (!_emit_modulemd (&emitter, MODULEMD_MODULE (object), error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not emit YAML");
            }
        }
      else if (MODULEMD_IS_DEFAULTS (object))
        {
          if (!_emit_defaults (&emitter, MODULEMD_DEFAULTS (object), error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not emit YAML");
            }
        }
      /* Emitters for other types go here */
      /* else if (document->type == <...>) */
      else
        {
          /* Unknown document type */
          g_set_error_literal (error,
                               MODULEMD_YAML_ERROR,
                               MODULEMD_YAML_ERROR_PARSE,
                               "Unknown document type");
          result = FALSE;
          goto error;
        }
    }

  yaml_stream_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error ending stream");

  *_yaml = yaml_string->str;

  result = TRUE;

error:
  yaml_emitter_delete (&emitter);
  if (!result)
    {
      g_clear_pointer (&yaml_string->str, g_free);
    }
  g_free (yaml_string);

  g_debug ("TRACE: exiting emit_yaml_string");
  return result;
}


gboolean
_emit_modulemd_simpleset (yaml_emitter_t *emitter,
                          ModulemdSimpleSet *set,
                          yaml_sequence_style_t style,
                          GError **error)
{
  gboolean result = FALSE;
  gsize i;
  yaml_event_t event;
  gchar **array = modulemd_simpleset_get (set);
  gchar *item;

  g_debug ("TRACE: entering _emit_modulemd_simpleset");

  yaml_sequence_start_event_initialize (&event, NULL, NULL, 1, style);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting simpleset sequence");

  for (i = 0; array[i]; i++)
    {
      item = g_strdup (array[i]);
      MMD_YAML_EMIT_SCALAR (&event, item, YAML_PLAIN_SCALAR_STYLE);
    }

  yaml_sequence_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending simpleset sequence");

  result = TRUE;
error:
  for (i = 0; array[i]; i++)
    {
      g_free (array[i]);
    }
  g_free (array);

  g_debug ("TRACE: exiting _emit_modulemd_simpleset");
  return result;
}


gboolean
_emit_modulemd_hashtable (yaml_emitter_t *emitter,
                          GHashTable *htable,
                          yaml_scalar_style_t style,
                          GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  GPtrArray *keys;
  GHashTableIter iter;
  gpointer key, value;
  gchar *name;
  gchar *val;

  g_debug ("TRACE: entering _emit_modulemd_hashtable");

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting hashtable mapping");

  keys = g_ptr_array_new_full (g_hash_table_size (htable), g_free);

  g_hash_table_iter_init (&iter, htable);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_ptr_array_add (keys, g_strdup ((const gchar *)key));
    }
  g_ptr_array_sort (keys, _modulemd_strcmp_sort);


  for (gsize i = 0; i < keys->len; i++)
    {
      name = g_strdup (g_ptr_array_index (keys, i));
      val = g_strdup (g_hash_table_lookup (htable, name));
      MMD_YAML_EMIT_STR_STR_DICT (&event, name, val, style);
    }
  g_ptr_array_unref (keys);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending hashtable mapping");

  result = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_hashtable");
  return result;
}

gboolean
_emit_modulemd_variant_hashtable (yaml_emitter_t *emitter,
                                  GHashTable *htable,
                                  GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  GPtrArray *keys;
  GHashTableIter iter;
  gpointer key, value;
  gchar *name;
  GVariant *val;

  g_debug ("TRACE: entering _emit_modulemd_variant_hashtable");

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting variant hashtable mapping");

  keys = g_ptr_array_new_full (g_hash_table_size (htable), g_free);

  g_hash_table_iter_init (&iter, htable);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_ptr_array_add (keys, g_strdup ((const gchar *)key));
    }
  g_ptr_array_sort (keys, _modulemd_strcmp_sort);


  for (gsize i = 0; i < keys->len; i++)
    {
      /* Write out the key as a scalar */
      name = g_strdup (g_ptr_array_index (keys, i));
      val = g_hash_table_lookup (htable, name);
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      /* Write out the values as a variant, recursing as needed */
      if (!emit_yaml_variant (emitter, val, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error writing arbitrary mapping");
        }
    }
  g_ptr_array_unref (keys);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending variant hashtable mapping");

  result = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_variant_hashtable");
  return result;
}
