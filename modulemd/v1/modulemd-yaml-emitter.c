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
#include <glib.h>
#include <glib/gstdio.h>
#include <yaml.h>
#include <errno.h>
#include <inttypes.h>
#include "private/modulemd-private.h"
#include "private/modulemd-yaml.h"
#include "private/modulemd-util.h"

gboolean
emit_yaml (yaml_emitter_t *emitter, GPtrArray *objects, GError **error);

gboolean
emit_yaml_file (GPtrArray *objects, const gchar *path, GError **error)
{
  MODULEMD_INIT_TRACE
  g_autoptr (FILE) yaml_file = NULL;
  g_auto (yaml_emitter_t) emitter;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (objects, FALSE);

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
      return FALSE;
    }

  yaml_emitter_set_output_file (&emitter, yaml_file);

  if (!emit_yaml (&emitter, objects, error))
    {
      return FALSE;
    }

  return TRUE;
}

gboolean
emit_yaml_string (GPtrArray *objects, gchar **_yaml, GError **error)
{
  MODULEMD_INIT_TRACE
  g_auto (yaml_emitter_t) emitter;
  g_autoptr (modulemd_yaml_string) yaml_string = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (objects, FALSE);

  yaml_string = g_malloc0_n (1, sizeof (modulemd_yaml_string));

  yaml_emitter_initialize (&emitter);

  yaml_emitter_set_output (&emitter, _write_yaml_string, (void *)yaml_string);

  if (!emit_yaml (&emitter, objects, error))
    {
      return FALSE;
    }

  /* Steal the final string to return */
  *_yaml = yaml_string->str;
  yaml_string->str = NULL;

  return TRUE;
}


gboolean
emit_yaml (yaml_emitter_t *emitter, GPtrArray *objects, GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  GObject *object = NULL;

  yaml_emitter_set_unicode (emitter, TRUE);

  yaml_stream_start_event_initialize (&event, YAML_UTF8_ENCODING);
  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Error starting stream");

  for (gsize i = 0; i < objects->len; i++)
    {
      object = g_ptr_array_index (objects, i);

      /* Write out the YAML */
      if (MODULEMD_IS_MODULE (object))
        {
          if (!_emit_modulestream (
                emitter,
                modulemd_module_peek_modulestream (MODULEMD_MODULE (object)),
                error))
            {
              g_debug ("Could not emit module stream YAML: %s",
                       (*error)->message);
              return FALSE;
            }
        }
      else if (MODULEMD_IS_MODULESTREAM (object))
        {
          if (!_emit_modulestream (
                emitter, MODULEMD_MODULESTREAM (object), error))
            {
              g_debug ("Could not emit YAML: %s", (*error)->message);
              return FALSE;
            }
        }
      else if (MODULEMD_IS_DEFAULTS (object))
        {
          if (!_emit_defaults (emitter, MODULEMD_DEFAULTS (object), error))
            {
              g_debug ("Could not emit YAML: %s", (*error)->message);
              return FALSE;
            }
        }
      else if (MODULEMD_IS_TRANSLATION (object))
        {
          if (!_emit_translation (
                emitter, MODULEMD_TRANSLATION (object), error))
            {
              g_debug ("Could not emit translation YAML: %s",
                       (*error)->message);
              return FALSE;
            }
        }
      /* Emitters for other types go here */
      /* else if (document->type == <...>) */
      else
        {
          /* Unknown document type */
          g_set_error (error,
                       MODULEMD_YAML_ERROR,
                       MODULEMD_YAML_ERROR_PARSE,
                       "Unknown document type: %s",
                       G_OBJECT_TYPE_NAME (object));
          return FALSE;
        }
    }

  yaml_stream_end_event_initialize (&event);
  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Error ending stream");

  return TRUE;
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
  gchar **array = modulemd_simpleset_dup (set);
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
