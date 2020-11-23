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

#include <fnmatch.h>
#include <string.h>
#include <inttypes.h>

#include "modulemd-errors.h"
#include "modulemd-module-stream.h"
#include "private/glib-extensions.h"
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
  gpointer key;
  gpointer value;

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
  gpointer key;
  gpointer value;

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
  gpointer key;
  gpointer value;

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
  gpointer key;
  gpointer value;

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
modulemd_hash_table_sets_are_equal_wrapper (const void *a, const void *b)
{
  return modulemd_hash_table_sets_are_equal ((GHashTable *)a, (GHashTable *)b);
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

/* Processes the set of keys first, then calls a unique compare function to handle any kind of value*/
gboolean
modulemd_hash_table_equals (GHashTable *a,
                            GHashTable *b,
                            GEqualFunc compare_func)
{
  g_autoptr (GPtrArray) set_a = NULL;
  g_autoptr (GPtrArray) set_b = NULL;
  int i = 0;
  gchar *key = NULL;
  gchar *value_a = NULL;
  gchar *value_b = NULL;

  /*Check size*/
  if (g_hash_table_size (a) != g_hash_table_size (b))
    {
      return FALSE;
    }

  /*Equality check on keys of hashtable*/
  set_a = modulemd_ordered_str_keys (a, modulemd_strcmp_sort);
  set_b = modulemd_ordered_str_keys (b, modulemd_strcmp_sort);

  for (i = 0; i < set_a->len; i++)
    {
      if (!g_str_equal (g_ptr_array_index (set_a, i),
                        g_ptr_array_index (set_b, i)))
        {
          return FALSE;
        }
    }

  /*Equality check for each value of all keys of hashtable*/
  for (i = 0; i < set_a->len; i++)
    {
      key = g_ptr_array_index (set_a, i);
      value_a = g_hash_table_lookup (a, key);
      value_b = g_hash_table_lookup (b, key);

      if (!compare_func (value_a, value_b))
        {
          return FALSE;
        }
    }

  return TRUE;
}


gint
modulemd_hash_table_compare (GHashTable *a,
                             GHashTable *b,
                             GCompareFunc value_compare_func)
{
  g_autoptr (GPtrArray) set_a = NULL;
  g_autoptr (GPtrArray) set_b = NULL;
  int i = 0;
  gchar *key = NULL;
  gchar *value_a = NULL;
  gchar *value_b = NULL;
  gint cmp;

  /* Get the ordered list of keys from each hashtable. */
  set_a = modulemd_ordered_str_keys (a, modulemd_strcmp_sort);
  set_b = modulemd_ordered_str_keys (b, modulemd_strcmp_sort);

  for (i = 0; i < set_a->len; i++)
    {
      /* If we ran out of elements in the second set, it is shorter and thus the
       * lesser one.
       */
      if (i >= set_b->len)
        {
          return 1;
        }

      /* Compare the keys. */
      cmp =
        g_strcmp0 (g_ptr_array_index (set_a, i), g_ptr_array_index (set_b, i));
      if (cmp != 0)
        {
          return cmp;
        }

      /* If the keys match, compare the values if needed. */
      if (value_compare_func)
        {
          key = g_ptr_array_index (set_a, i);
          value_a = g_hash_table_lookup (a, key);
          value_b = g_hash_table_lookup (b, key);
          cmp = value_compare_func (value_a, value_b);
          if (cmp != 0)
            {
              return cmp;
            }
        }
    }

  /* If everything has been the same so far but there are more elements in the
   * second set, it is longer and thus the greater one.
   */
  if (set_a->len < set_b->len)
    {
      return -1;
    }

  /* If we made it this far, they are equal. */
  return 0;
}

gint
modulemd_strcmp_sort (gconstpointer a, gconstpointer b)
{
  return g_strcmp0 (*(const gchar **)a, *(const gchar **)b);
}

gint
modulemd_strcmp_wrapper (gconstpointer a, gconstpointer b)
{
  return g_strcmp0 ((gchar *)a, (gchar *)b);
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

GStrv
modulemd_ordered_str_keys_as_strv (GHashTable *htable)
{
  GPtrArray *keys = modulemd_ordered_str_keys (htable, modulemd_strcmp_sort);
  // Add the NULL sentinel
  g_ptr_array_add (keys, NULL);
  // Store the pdata for returning after we free the container
  GStrv result = (GStrv)keys->pdata;
  g_ptr_array_free (keys, FALSE);
  return result;
}


void
modulemd_hash_table_unref (void *table)
{
  if (!table)
    {
      return;
    }

  g_hash_table_unref ((GHashTable *)table);
}


static void
destroy_variant_data (gpointer data)
{
  g_free (data);
}


GVariant *
modulemd_variant_deep_copy (GVariant *variant)
{
  const GVariantType *data_type = g_variant_get_type (variant);
  gsize data_size = g_variant_get_size (variant);
  gpointer data = g_malloc0 (data_size);

  g_variant_store (variant, data);

  return g_variant_ref_sink (g_variant_new_from_data (
    data_type, data, data_size, FALSE, destroy_variant_data, data));
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

  i--;
  /* Process the version */
  while (i >= tmp)
    {
      if (*i == ':')
        {
          break;
        }
      if (*i == '-')
        {
          /* '-' between version and epoch is not allowed */
          return FALSE;
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


gboolean
modulemd_boolean_equals (gboolean a, gboolean b)
{
  /*
   * There is no validation when assigning to a gboolean variable and so it
   * could contain any value represented by a gint. Thus, each value needs to
   * be canonicalized before comparing for equality.
   */
  if (!!a == !!b)
    {
      return TRUE;
    }

  return FALSE;
}

gboolean
modulemd_is_glob_pattern (const char *pattern)
{
  g_return_val_if_fail (pattern, FALSE);

  return strpbrk (pattern, "*[?") != NULL;
}

gint
compare_streams (gconstpointer a, gconstpointer b)
{
  int cmp = 0;
  guint64 a_ver;
  guint64 b_ver;
  ModulemdModuleStream *a_ = *(ModulemdModuleStream **)a;
  ModulemdModuleStream *b_ = *(ModulemdModuleStream **)b;

  /* Sort alphabetically by module name */
  cmp = g_strcmp0 (modulemd_module_stream_get_module_name (a_),
                   modulemd_module_stream_get_module_name (b_));
  if (cmp != 0)
    {
      return cmp;
    }

  /* Sort alphabetically by stream name */
  cmp = g_strcmp0 (modulemd_module_stream_get_stream_name (a_),
                   modulemd_module_stream_get_stream_name (b_));
  if (cmp != 0)
    {
      return cmp;
    }

  /* Sort by the version, highest first */
  a_ver = modulemd_module_stream_get_version (a_);
  b_ver = modulemd_module_stream_get_version (b_);
  if (b_ver > a_ver)
    {
      return 1;
    }
  if (a_ver > b_ver)
    {
      return -1;
    }

  /* Sort alphabetically by context */
  cmp = g_strcmp0 (modulemd_module_stream_get_context (a_),
                   modulemd_module_stream_get_context (b_));
  if (cmp != 0)
    {
      return cmp;
    }

  /* Sort alphabetically by architecture */
  cmp = g_strcmp0 (modulemd_module_stream_get_arch (a_),
                   modulemd_module_stream_get_arch (b_));

  return cmp;
}


gboolean
modulemd_fnmatch (const gchar *pattern, const gchar *string)
{
  if (!pattern)
    {
      return TRUE;
    }

  if (!string)
    {
      return FALSE;
    }

  return !fnmatch (pattern, string, 0);
}

gboolean
modulemd_rpm_match (gpointer key, gpointer UNUSED (value), gpointer user_data)
{
  return modulemd_fnmatch (user_data, key);
}


guint64
modulemd_iso8601date_to_guint64 (const gchar *iso8601)
{
  struct tm tm = { 0 };
  char *s = strptime (iso8601, "%FT%H:%MZ", &tm);
  if (s == NULL || *s != '\0')
    {
      return 0;
    }

  char buf[32];
  strftime (buf, sizeof (buf), "%Y%m%d%H%M", &tm);

  return g_ascii_strtoull (buf, NULL, 0);
}


gchar *
modulemd_guint64_to_iso8601date (guint64 date)
{
  char date_str[32];
  sprintf (date_str, "%" PRIu64, date);
  struct tm tm = { 0 };
  char *s = strptime (date_str, "%Y%m%d%H%M", &tm);
  if (s == NULL || *s != '\0')
    {
      return NULL;
    }

  gchar *buf = g_malloc0_n (32, sizeof (gchar));
  strftime (buf, 32, "%FT%H:%MZ", &tm);

  return buf;
}


#ifndef HAVE_EXTEND_AND_STEAL

#ifndef MIN_ARRAY_SIZE
#define MIN_ARRAY_SIZE 16
#endif

typedef volatile gint gatomicrefcount;

/*
 * GPtrArray:
 * @pdata: points to the array of pointers, which may be moved when the
 *     array grows
 * @len: number of pointers in the array
 *
 * Contains the public fields of a pointer array.
 */
struct _GRealPtrArray
{
  gpointer *pdata;
  guint len;
  guint alloc;
  gatomicrefcount ref_count;
  GDestroyNotify element_free_func;
};

typedef struct _GRealPtrArray GRealPtrArray;


/* Returns the smallest power of 2 greater than n, or n if
 * such power does not fit in a guint
 */
static guint
g_nearest_pow (guint num)
{
  guint n = num - 1;

  g_assert (num > 0);

  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
#if SIZEOF_INT == 8
  n |= n >> 32;
#endif

  return n + 1;
}

static void
g_ptr_array_maybe_expand (GRealPtrArray *array, guint len)
{
  /* Detect potential overflow */
  if (G_UNLIKELY ((G_MAXUINT - array->len) < len))
    g_error ("adding %u to array would overflow", len);

  if ((array->len + len) > array->alloc)
    {
      guint old_alloc = array->alloc;
      array->alloc = g_nearest_pow (array->len + len);
      array->alloc = MAX (array->alloc, MIN_ARRAY_SIZE);
      array->pdata =
        g_realloc (array->pdata, sizeof (gpointer) * array->alloc);
      if (G_UNLIKELY (g_mem_gc_friendly))
        for (; old_alloc < array->alloc; old_alloc++)
          array->pdata[old_alloc] = NULL;
    }
}

void
g_ptr_array_extend (GPtrArray *array_to_extend,
                    GPtrArray *array,
                    GCopyFunc func,
                    gpointer user_data)
{
  GRealPtrArray *rarray_to_extend = (GRealPtrArray *)array_to_extend;
  gsize i;

  g_return_if_fail (array_to_extend != NULL);
  g_return_if_fail (array != NULL);

  g_ptr_array_maybe_expand (rarray_to_extend, array->len);

  if (func != NULL)
    {
      for (i = 0; i < array->len; i++)
        rarray_to_extend->pdata[i + rarray_to_extend->len] =
          func (array->pdata[i], user_data);
    }
  else if (array->len > 0)
    {
      memcpy (rarray_to_extend->pdata + rarray_to_extend->len,
              array->pdata,
              array->len * sizeof (*array->pdata));
    }

  rarray_to_extend->len += array->len;
}

void
g_ptr_array_extend_and_steal (GPtrArray *array_to_extend, GPtrArray *array)
{
  gpointer *pdata;

  g_ptr_array_extend (array_to_extend, array, NULL, NULL);

  /* Get rid of @array without triggering the GDestroyNotify attached
   * to the elements moved from @array to @array_to_extend. */
  pdata = g_steal_pointer (&array->pdata);
  array->len = 0;
  g_ptr_array_unref (array);
  g_free (pdata);
}
#endif
