/* modulemd-util.c
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

#include "modulemd-util.h"

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

void
modulemd_variant_unref (void *ptr)
{
  g_variant_unref ((GVariant *)ptr);
}
