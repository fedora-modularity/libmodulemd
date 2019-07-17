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
#include <string.h>
#include "private/modulemd-util.h"
#include "private/modulemd-improvedmodule-private.h"
#include <locale.h>


GQuark
modulemd_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-error-quark");
}

GHashTable *
_modulemd_hash_table_deep_str_copy (GHashTable *orig)
{
  GHashTable *new;
  GHashTableIter iter;
  gpointer key, value;

  g_return_val_if_fail (orig, NULL);

  new = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  g_hash_table_iter_init (&iter, orig);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_insert (
        new, g_strdup ((const gchar *)key), g_strdup ((const gchar *)value));
    }

  return new;
}

GHashTable *
_modulemd_hash_table_deep_obj_copy (GHashTable *orig)
{
  GHashTable *new;
  GHashTableIter iter;
  gpointer key, value;

  g_return_val_if_fail (orig, NULL);

  new =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  g_hash_table_iter_init (&iter, orig);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_insert (
        new, g_strdup ((const gchar *)key), g_object_ref ((GObject *)value));
    }

  return new;
}

GHashTable *
_modulemd_hash_table_deep_variant_copy (GHashTable *orig)
{
  GHashTable *new;
  GHashTableIter iter;
  gpointer key, value;

  g_return_val_if_fail (orig, NULL);

  new = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_variant_unref);

  g_hash_table_iter_init (&iter, orig);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_insert (
        new, g_strdup ((const gchar *)key), g_variant_ref ((GVariant *)value));
    }

  return new;
}

gint
_modulemd_strcmp_sort (gconstpointer a, gconstpointer b)
{
  return g_strcmp0 (*(const gchar **)a, *(const gchar **)b);
}

GPtrArray *
_modulemd_ordered_str_keys (GHashTable *htable, GCompareFunc compare_func)
{
  GPtrArray *keys;
  GHashTableIter iter;
  gpointer key, value;

  keys = g_ptr_array_new_full (g_hash_table_size (htable), g_free);

  g_hash_table_iter_init (&iter, htable);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_ptr_array_add (keys, g_strdup ((const gchar *)key));
    }
  g_ptr_array_sort (keys, compare_func);

  return keys;
}

static gint
_modulemd_int64_sort (gconstpointer a, gconstpointer b)
{
  gint retval = *(const gint64 *)a - *(const gint64 *)b;
  return retval;
}

GList *
_modulemd_ordered_int64_keys (GHashTable *htable)
{
  g_autoptr (GList) hash_keys = NULL;
  GList *unsorted_keys = NULL;
  GList *sorted_keys = NULL;

  /* Get the keys from the hash table */
  hash_keys = g_hash_table_get_keys (htable);

  /* Make a copy of the keys that we can modify */
  unsorted_keys = g_list_copy (hash_keys);

  /* Sort the keys numerically */
  sorted_keys = g_list_sort (unsorted_keys, _modulemd_int64_sort);

  return sorted_keys;
}

void
modulemd_variant_unref (void *ptr)
{
  g_variant_unref ((GVariant *)ptr);
}

gboolean
modulemd_validate_nevra (const gchar *nevra)
{
  g_autofree gchar *tmp = g_strdup (nevra);
  gsize len = strlen (nevra);
  gchar *i;
  gchar *endptr;

  /* Since the "name" portion of a NEVRA can have an infinite number of
   * hyphens, we need to parse from the end backwards.
   */
  i = &tmp[len - 1];

  /* Everything after the last '.' must be the architecture */
  while (i >= tmp)
    {
      if (*i == '.')
        {
          break;
        }
      i--;
    }

  if (i < tmp)
    {
      /* We hit the start of the string without hitting '.' */
      return FALSE;
    }

  /*
   * TODO: Compare the architecture suffix with a list of known-valid ones
   * This needs to come from an external source that's kept up to date or
   * this will regularly break.
   */

  /* Process the "release" tag */
  while (i >= tmp)
    {
      if (*i == '-')
        {
          break;
        }
      i--;
    }

  if (i < tmp)
    {
      /* We hit the start of the string without hitting '-' */
      return FALSE;
    }

  /* No need to validate Release; it's fairly arbitrary */

  /* Process the version */
  while (i >= tmp)
    {
      if (*i == ':')
        {
          break;
        }
      i--;
    }
  if (i < tmp)
    {
      /* We hit the start of the string without hitting ':' */
      return FALSE;
    }

  /* Process the epoch */
  *i = '\0';
  while (i >= tmp)
    {
      if (*i == '-')
        {
          break;
        }
      i--;
    }
  if (i < tmp)
    {
      /* We hit the start of the string without hitting '-' */
      return FALSE;
    }

  /* Validate that this section is a number */
  g_ascii_strtoll (i, &endptr, 10);
  if (endptr == i)
    {
      /* String conversion failed */
      return FALSE;
    }

  /* No need to specifically parse the name section here */

  return TRUE;
}


modulemd_tracer *
modulemd_trace_init (const gchar *function_name)
{
  modulemd_tracer *self = g_malloc0_n (1, sizeof (modulemd_tracer));
  self->function_name = g_strdup (function_name);

  g_debug ("TRACE: Entering %s", self->function_name);

  return self;
}


void
modulemd_trace_free (modulemd_tracer *tracer)
{
  g_debug ("TRACE: Exiting %s", tracer->function_name);
  g_clear_pointer (&tracer->function_name, g_free);
  g_free (tracer);
}


ModulemdTranslationEntry *
_get_locale_entry (ModulemdTranslation *translation, const gchar *_locale)
{
  ModulemdTranslationEntry *entry = NULL;
  g_autofree gchar *locale = NULL;

  if (!translation)
    return NULL;

  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (translation), NULL);

  if (_locale)
    {
      if (g_strcmp0 (_locale, "C") == 0 || g_strcmp0 (_locale, "C.UTF-8") == 0)
        {
          /* If the locale is "C" or "C.UTF-8", always return the standard value */
          return NULL;
        }

      locale = g_strdup (_locale);
    }
  else
    {
      /* If the locale was NULL, use the locale of this process */
      locale = g_strdup (setlocale (LC_MESSAGES, NULL));
    }

  entry = modulemd_translation_get_entry_by_locale (translation, locale);

  return entry;
}


GPtrArray *
_modulemd_index_serialize (GHashTable *index, GError **error)
{
  GHashTableIter iter;
  gpointer key, value;
  gsize i;
  g_autoptr (GPtrArray) objects = NULL;
  g_autoptr (GPtrArray) sub_objects = NULL;


  if (!index)
    {
      g_set_error (
        error, MODULEMD_ERROR, MODULEMD_ERROR_PROGRAMMING, "Index was NULL.");
      return NULL;
    }

  objects = g_ptr_array_new_with_free_func (g_object_unref);
  g_hash_table_iter_init (&iter, index);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      if (!value || !MODULEMD_IS_IMPROVEDMODULE (value))
        {
          g_set_error (error,
                       MODULEMD_ERROR,
                       MODULEMD_ERROR_PROGRAMMING,
                       "Index value was not a ModulemdImprovedModule.");
          return NULL;
        }

      sub_objects =
        modulemd_improvedmodule_serialize (MODULEMD_IMPROVEDMODULE (value));

      for (i = 0; i < sub_objects->len; i++)
        {
          g_ptr_array_add (objects,
                           g_object_ref (g_ptr_array_index (sub_objects, i)));
        }
      g_clear_pointer (&sub_objects, g_ptr_array_unref);
    }

  return g_ptr_array_ref (objects);
}


static ModulemdImprovedModule *
get_or_create_module_from_index (GHashTable *htable, const gchar *module_name)
{
  ModulemdImprovedModule *stored_module = NULL;
  ModulemdImprovedModule *module = NULL;

  stored_module = g_hash_table_lookup (htable, module_name);
  if (stored_module)
    {
      module = modulemd_improvedmodule_copy (stored_module);
    }
  else
    {
      /* This is the first encounter of this module */
      module = modulemd_improvedmodule_new (module_name);
    }
  return module;
}


GHashTable *
module_index_from_data (GPtrArray *data, GError **error)
{
  GObject *item = NULL;
  gsize i = 0;
  gsize j = 0;
  g_autofree gchar *module_name = NULL;
  g_autofree gchar *stream_name = NULL;
  g_autoptr (ModulemdImprovedModule) module = NULL;
  ModulemdImprovedModule *stored_module = NULL;
  ModulemdModuleStream *stream = NULL;
  g_autoptr (GPtrArray) retrieved_streams = NULL;
  ModulemdModuleStream *retrieved_stream = NULL;
  ModulemdDefaults *defaults = NULL;
  g_autoptr (GHashTable) module_index = NULL;
  GError *merge_error = NULL;
  g_autoptr (GPtrArray) clean_data = NULL;
  g_autoptr (GPtrArray) translations =
    g_ptr_array_new_with_free_func (g_object_unref);
  ModulemdTranslation *translation = NULL;

  /* Deduplicate and merge any ModulemdDefaults objects in the list */
  clean_data = modulemd_merge_defaults (data, NULL, FALSE, &merge_error);
  if (!clean_data)
    {
      g_debug ("Error merging defaults: %s", merge_error->message);
      g_propagate_error (error, merge_error);
      return NULL;
    }

  module_index =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  /* Iterate through the data and add the entries to the module_index */
  for (i = 0; i < clean_data->len; i++)
    {
      item = g_ptr_array_index (clean_data, i);

      if (MODULEMD_IS_MODULESTREAM (item))
        {
          stream = MODULEMD_MODULESTREAM (item);
          module_name = modulemd_modulestream_get_name (stream);
          stream_name = modulemd_modulestream_get_stream (stream);

          if (!module_name || !stream_name)
            {
              g_set_error (error,
                           MODULEMD_ERROR,
                           MODULEMD_ERROR_MISSING_CONTENT,
                           "Module streams without a module name or stream "
                           "name may not be read into an index.");
              return NULL;
            }

          module = get_or_create_module_from_index (module_index, module_name);

          /* Add the stream to this module. Note: if the same stream name
           * appears in the data more than once, the last one encountered wins.
           */
          modulemd_improvedmodule_add_stream (module, stream);

          /* Save it back to the index */
          g_hash_table_replace (module_index,
                                g_strdup (module_name),
                                modulemd_improvedmodule_copy (module));
        }
      else if (MODULEMD_IS_DEFAULTS (item))
        {
          defaults = MODULEMD_DEFAULTS (item);
          module_name = modulemd_defaults_dup_module_name (defaults);
          module = get_or_create_module_from_index (module_index, module_name);

          /* Update the defaults. */
          modulemd_improvedmodule_set_defaults (module, defaults);

          /* Save it back to the index */
          g_hash_table_replace (module_index,
                                g_strdup (module_name),
                                modulemd_improvedmodule_copy (module));
        }
      else if (MODULEMD_IS_TRANSLATION (item))
        {
          /* Queue these up to process at the end, because we need to ensure
           * that the streams they are associated with have been added first
           */
          g_ptr_array_add (translations,
                           g_object_ref (MODULEMD_TRANSLATION (item)));
        }

      g_clear_pointer (&module, g_object_unref);
      g_clear_pointer (&module_name, g_free);
      g_clear_pointer (&stream_name, g_free);
    }

  /* Iterate through the translations and associate them to the appropriate
   * streams.
   */
  for (i = 0; i < translations->len; i++)
    {
      translation = g_ptr_array_index (translations, i);
      stored_module = g_hash_table_lookup (
        module_index, modulemd_translation_peek_module_name (translation));
      if (!stored_module)
        {
          /* No streams of this module were processed, so ignore this set of
           * translations.
           */
          continue;
        }

      retrieved_streams = modulemd_improvedmodule_get_streams_by_name (
        stored_module, modulemd_translation_peek_module_stream (translation));
      if (!retrieved_streams)
        {
          /* This stream of this module wasn't processed, so ignore this set of
           * translations.
           */
          continue;
        }

      for (j = 0; j < retrieved_streams->len; j++)
        {
          retrieved_stream = g_ptr_array_index (retrieved_streams, j);

          /* Assign this translation to the object.
           * Note: This will be ignored if there is a higher modified value already
           * assigned to this object.
           */
          modulemd_modulestream_set_translation (retrieved_stream,
                                                 translation);

          /* Save the updated stream back to the index */
          modulemd_improvedmodule_add_stream (stored_module, retrieved_stream);
        }
      g_clear_pointer (&retrieved_streams, g_ptr_array_unref);
    }


  return g_hash_table_ref (module_index);
}
