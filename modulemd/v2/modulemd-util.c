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

#include "private/modulemd-util.h"


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


GHashTable *
modulemd_hash_table_deep_str_copy (GHashTable *orig)
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

gint
modulemd_strcmp_sort (gconstpointer a, gconstpointer b)
{
  return g_strcmp0 (*(const gchar **)a, *(const gchar **)b);
}

GPtrArray *
modulemd_ordered_str_keys (GHashTable *htable, GCompareFunc compare_func)
{
  GPtrArray *keys;
  GHashTableIter iter;
  gpointer key;

  keys = g_ptr_array_new_full (g_hash_table_size (htable), g_free);

  g_hash_table_iter_init (&iter, htable);
  while (g_hash_table_iter_next (&iter, &key, NULL))
    {
      g_ptr_array_add (keys, g_strdup ((const gchar *)key));
    }
  g_ptr_array_sort (keys, compare_func);

  return keys;
}

gchar **
modulemd_ordered_str_keys_as_strv (GHashTable *htable)
{
  GPtrArray *keys = modulemd_ordered_str_keys (htable, modulemd_strcmp_sort);
  // Add the NULL sentinel
  g_ptr_array_add (keys, NULL);
  // Store the pdata for returning after we free the container
  gchar **result = (gchar **)keys->pdata;
  g_ptr_array_free (keys, FALSE);
  return result;
}
