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


GQuark
modulemd_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-error-quark");
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


GHashTable *
modulemd_hash_table_deep_set_copy (GHashTable *orig)
{
  GHashTable *new;
  GHashTableIter iter;
  gpointer key, value;

  g_return_val_if_fail (orig, NULL);

  new = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  g_hash_table_iter_init (&iter, orig);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_add (new, g_strdup ((const gchar *)key));
    }

  return new;
}


GHashTable *
modulemd_hash_table_deep_str_set_copy (GHashTable *orig)
{
  GHashTable *new;
  GHashTableIter iter;
  gpointer key, value;

  g_return_val_if_fail (orig, NULL);

  new = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_hash_table_unref);

  g_hash_table_iter_init (&iter, orig);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_insert (new,
                           g_strdup ((const gchar *)key),
                           modulemd_hash_table_deep_set_copy (value));
    }

  return new;
}


GHashTable *
modulemd_hash_table_deep_str_str_set_copy (GHashTable *orig)
{
  GHashTable *new;
  GHashTableIter iter;
  gpointer key, value;

  g_return_val_if_fail (orig, NULL);

  new = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_hash_table_unref);

  g_hash_table_iter_init (&iter, orig);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_insert (new,
                           g_strdup ((const gchar *)key),
                           modulemd_hash_table_deep_str_set_copy (value));
    }

  return new;
}


gboolean
modulemd_hash_table_sets_are_equal (GHashTable *a, GHashTable *b)
{
  g_autoptr (GPtrArray) set_a = NULL;
  g_autoptr (GPtrArray) set_b = NULL;


  if (g_hash_table_size (a) != g_hash_table_size (b))
    {
      /* If they have a different number of strings in the set, they can't
       * be identical.
       */
      return FALSE;
    }

  set_a = modulemd_ordered_str_keys (a, modulemd_strcmp_sort);
  set_b = modulemd_ordered_str_keys (b, modulemd_strcmp_sort);

  for (guint i = 0; i < set_a->len; i++)
    {
      /* These are guaranteed to be returned ordered, so we can
       * assume that any difference at any index means that the
       * lists are not identical.
       */
      if (!g_str_equal (g_ptr_array_index (set_a, i),
                        g_ptr_array_index (set_b, i)))
        {
          /* No match, so this simpleset is not equal */
          return FALSE;
        }
    }

  /* If we made it here, everything must have matched */
  return TRUE;
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


void
modulemd_hash_table_unref (void *table)
{
  if (!table)
    return;

  g_hash_table_unref ((GHashTable *)table);
}

GVariant *
modulemd_variant_deep_copy (GVariant *variant)
{
  const GVariantType *data_type = g_variant_get_type (variant);
  gsize data_size = g_variant_get_size (variant);
  gconstpointer data = g_variant_get_data (variant);

  return g_variant_ref_sink (
    g_variant_new_from_data (data_type, data, data_size, TRUE, NULL, NULL));
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
