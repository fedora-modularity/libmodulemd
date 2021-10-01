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

#pragma once

#include <glib.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-util
 * @title: Modulemd Utility Functions
 * @stability: private
 * @short_description: Provides private utility functions for use within
 * libmodulemd.
 */

#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__ ((__unused__))
#else
#define UNUSED(x) UNUSED_##x
#endif

#ifdef __GNUC__
#define FALLTHROUGH __attribute__ ((fallthrough));
#else
#define FALLTHROUGH
#endif

/**
 * modulemd_tracer:
 * @function_name: The name of the function to be tracked by this
 * #modulemd_tracer.
 *
 * Since: 2.0
 */
typedef struct _modulemd_tracer
{
  gchar *function_name;
} modulemd_tracer;

/**
 * modulemd_trace_init:
 * @function_name: The name of the function being traced.
 *
 * Returns: (transfer full): A newly-allocated #modulemd_tracer object.
 *
 * Allocates and returns a new #modulemd_tracer that tracks the provided
 * @function_name. Also writes a g_debug() trace message indicating
 * @function_name has been entered.
 *
 * DIRECT USE OF THIS FUNCTION SHOULD BE AVOIDED. Instead use
 * %MODULEMD_INIT_TRACE--which makes use of this function as part of its
 * internal implementation.
 *
 * Since: 2.0
 */
modulemd_tracer *
modulemd_trace_init (const gchar *function_name);

/**
 * modulemd_trace_free:
 * @tracer: A #modulemd_tracer object representing a function being traced.
 *
 * Writes a g_debug() trace message indicating the function name associated
 * with @tracer is being exited and frees @tracer.
 *
 * DIRECT USE OF THIS FUNCTION SHOULD BE AVOIDED. Instead use
 * %MODULEMD_INIT_TRACE--which makes use of this function as part of its
 * internal implementation.
 *
 * Since: 2.0
 */
void
modulemd_trace_free (modulemd_tracer *tracer);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (modulemd_tracer, modulemd_trace_free);

/**
 * MODULEMD_INIT_TRACE:
 *
 * When used at the beginning of a function, automatically writes g_debug()
 * trace messages when entering and leaving that function. Makes use of
 * modulemd_trace_init() and modulemd_trace_free().
 *
 * This macro manages the memory of the hidden #modulemd_tracer object it
 * creates, so the caller should not attempt to modify that object in any way.
 *
 * Since: 2.0
 */
#define MODULEMD_INIT_TRACE()                                                 \
  g_autoptr (modulemd_tracer) tracer = modulemd_trace_init (__func__);        \
  do                                                                          \
    {                                                                         \
      (void)(tracer);                                                         \
    }                                                                         \
  while (0)

G_END_DECLS

/**
 * modulemd_hash_table_deep_str_copy:
 * @orig: A #GHashTable to copy, containing string keys and string values.
 *
 * Returns: (transfer full): A newly-allocated #GHashTable containing a deep
 * copy of both the keys and values from @orig.
 *
 * Since: 2.0
 */
GHashTable *
modulemd_hash_table_deep_str_copy (GHashTable *orig);

/**
 * modulemd_hash_table_deep_set_copy:
 * @orig: A #GHashTable to copy, containing string keys.
 *
 * Returns: (transfer full): A newly-allocated #GHashTable containing a deep
 * copy of the keys from @orig. The values from @orig are ignored, and the
 * values in the copy are set the same as the corresponding keys so the
 * returned #GHashTable can be used as a set.
 *
 * Since: 2.0
 */
GHashTable *
modulemd_hash_table_deep_set_copy (GHashTable *orig);

/**
 * modulemd_hash_table_deep_str_set_copy:
 * @orig: A #GHashTable to copy, containing string keys and #GHashTable values.
 *
 * Returns: (transfer full): A newly-allocated #GHashTable containing a deep
 * copy of the keys from @orig. The corresponding #GHashTable value for each
 * key is deep copied via modulemd_hash_table_deep_str_copy() for use as a set.
 *
 * Since: 2.0
 */
GHashTable *
modulemd_hash_table_deep_str_set_copy (GHashTable *orig);

/**
 * modulemd_hash_table_deep_str_str_set_copy:
 * @orig: A #GHashTable to copy, containing string keys and #GHashTable values
 * that are nested two levels deep.
 *
 * Returns: (transfer full): A newly-allocated #GHashTable containing a deep
 * copy of the keys from @orig. Each corresponding #GHashTable value is deep
 * copied, with the second level #GHashTable copied for use as a set as
 * described in modulemd_hash_table_deep_str_copy().
 *
 * Since: 2.0
 */
GHashTable *
modulemd_hash_table_deep_str_str_set_copy (GHashTable *orig);

/**
 * modulemd_hash_table_sets_are_equal_wrapper:
 * @a: A void pointer.
 * @b: A void pointer.
 *
 * Returns: TRUE if both @a and @b (considered as #GHashTable<!-- -->s) contain
 * an identical set of keys, FALSE if they differ.
 *
 * Only the keys of @a and @b are compared. The values are ignored.
 *
 * Since: 2.2
 */
gboolean
modulemd_hash_table_sets_are_equal_wrapper (const void *a, const void *b);

/**
 * modulemd_hash_table_sets_are_equal:
 * @a: A #GHashTable object.
 * @b: A #GHashTable object.
 *
 * Returns: TRUE if both @a and @b contain an identical set of keys, FALSE if
 * they differ.
 *
 * Only the keys of @a and @b are compared. The values are ignored.
 *
 * Since: 2.0
 */
gboolean
modulemd_hash_table_sets_are_equal (GHashTable *a, GHashTable *b);

/**
 * modulemd_hash_table_equals:
 * @a: A #GHashTable object.
 * @b: A #GHashTable object.
 * @compare_func: A #GEqualFunc function that is called to determine the
 * equivalence of pairs of #GHashTable values.
 *
 * Returns: TRUE if both @a and @b contain identical keys and identical
 * corresponding values (as determined by @compare_func), FALSE if they
 * differ.
 *
 * Since: 2.2
 */
gboolean
modulemd_hash_table_equals (GHashTable *a,
                            GHashTable *b,
                            GEqualFunc compare_func);

/*
 * modulemd_hash_table_compare:
 * @a: (in): A #GHashTable object.
 * @b: (in): A #GHashTable object.
 * @value_compare_func: (in): A #GCompareFunc function that is called to compare
 * pairs of #GHashTable values for sorting. If NULL, only the #GHashTable keys
 * are compared.
 *
 * Using guidance from
 * https://docs.python.org/3/tutorial/datastructures.html#comparing-sequences-and-other-types
 *
 * Returns: Less than zero if @a sorts less than @b, zero for equal, greater than
 * zero if @a is greater than @b.
 *
 * Since: 2.10
 */
gint
modulemd_hash_table_compare (GHashTable *a,
                             GHashTable *b,
                             GCompareFunc value_compare_func);

/**
 * modulemd_strcmp_sort:
 * @a: A #gconstpointer.
 * @b: A #gconstpointer.
 *
 * Returns: 0 if @a and @b are pointers to identical strings, a negative value
 * if @a is less than @b, and a positive value if @a is greater than @b.
 *
 * Since: 2.0
 */
gint
modulemd_strcmp_sort (gconstpointer a, gconstpointer b);

/**
 * modulemd_strcmp_wrapper:
 * @a: (in): A #gconstpointer.
 * @b: (in): A #gconstpointer.
 *
 * Returns: 0 if @a and @b are identical strings, a negative value if @a is less
 * than @b, and a positive value if @a is greater than @b.
 *
 * Since: 2.10
 */
gint
modulemd_strcmp_wrapper (gconstpointer a, gconstpointer b);

/**
 * modulemd_ordered_str_keys:
 * @htable: A #GHashTable.
 * @compare_func: A #GCompareFunc function that is called to determine the
 * equivalence of pairs of #GHashTable keys from @htable. This should almost
 * always be passed as modulemd_strcmp_sort().
 *
 * Returns: (transfer container): A #GPtrArray of the keys from @htable sorted
 * according to @compare_func.
 *
 * Since: 2.0
 */
GPtrArray *
modulemd_ordered_str_keys (GHashTable *htable, GCompareFunc compare_func);

/**
 * modulemd_ordered_str_keys_as_strv:
 * @htable: A #GHashTable.
 *
 * Returns: (transfer full): A #GStrv list of the keys from @htable sorted
 * according to modulemd_strcmp_sort().
 *
 * Since: 2.0
 */
GStrv
modulemd_ordered_str_keys_as_strv (GHashTable *htable);

/**
 * modulemd_variant_deep_copy:
 * @variant: A #GVariant opaque data structure.
 *
 * Returns: (transfer full): A newly-allocated deep copy of @variant.
 *
 * Since: 2.0
 */
GVariant *
modulemd_variant_deep_copy (GVariant *variant);

/**
 * modulemd_hash_table_unref:
 * @table: (nullable): A void pointer.
 *
 * Decrements the reference count of @table (considered as a #GHashTable). If
 * the reference count drops to 0, all keys, values, and memory associated with
 * @table will be released.
 *
 * The main purpose of this function is to provide a GDestroyNotify() function
 * for #GPtrArray, #GHashTable, and similar object types.
 *
 * Since: 2.0
 */
void
modulemd_hash_table_unref (void *table);

/**
 * modulemd_validate_nevra:
 * @nevra: A NEVRA (Name, Epoch, Version, Release, Architecture) string.
 *
 * Returns: TRUE if @nevra is in proper N-E:V-R.A format, FALSE otherwise.
 *
 * Since: 2.0
 */
gboolean
modulemd_validate_nevra (const gchar *nevra);

/**
 * modulemd_boolean_equals:
 * @a: A #gboolean value.
 * @b: A #gboolean value.
 *
 * Since a #gboolean could contain any value represented by a #gint, @a and @b
 * are compared for logical equivalence.
 *
 * Returns: TRUE if @a and @b are logically equal, FALSE otherwise.
 *
 * Since: 2.7
 */
gboolean
modulemd_boolean_equals (gboolean a, gboolean b);


/**
 * modulemd_is_glob_pattern:
 * @pattern: (nullable) A string to check for glob patterns as defined by
 * [glob(7)[(https://www.mankier.com/7/glob)
 *
 * Returns: TRUE if @pattern contains any globbing characters. FALSE
 * otherwise.
 *
 * Since: 2.9
 */
gboolean
modulemd_is_glob_pattern (const char *pattern);


/**
 * compare_streams:
 * @a: The first #ModulemdModuleStream to sort
 * @b: The second #ModulemdModuleStream to sort
 *
 * Sorting function for GPtrArrays of #ModulemdModuleStream objects.
 *
 * Since: 2.9
 */
gint
compare_streams (gconstpointer a, gconstpointer b);

/**
 * modulemd_fnmatch:
 * @pattern: (nullable) A string to check for glob patterns as defined by
 * [glob(7)[(https://www.mankier.com/7/glob)
 * @string: (nullable) A string to check for matches.
 *
 * A wrapper around fnmatch() for use with modulemd.
 *
 * Returns: #TRUE if @pattern matched for this string or if @pattern is NULL.
 * #FALSE if @pattern did not match or @string is NULL.
 */
gboolean
modulemd_fnmatch (const gchar *pattern, const gchar *string);


/**
 * modulemd_rpm_match:
 * @key: The key in the RPM artifacts hash table.
 * @value: The value in the RPM artifacts hash table (should be the same
 * as @key).
 * @user_data: A [glob](https://www.mankier.com/3/glob)
 * pattern to match against the NEVRA strings of the RPM artifacts in @self.
 *
 * This is a #GHRFunc for use with g_hash_table_find() to search for RPMs. It
 * is a wrapper around modulemd_fnmatch().
 *
 * Returns: TRUE if @key is a match for the pattern in @user_data.
 */
gboolean
modulemd_rpm_match (gpointer key, gpointer UNUSED (value), gpointer user_data);


/**
 * modulemd_iso8601date_to_guint64:
 * @iso8601: (in): string containing a date in ISO 8601 format.
 *
 * Returns: date represented as a 64-bit integer (such as 201807011200).
 *
 * Since: 2.10
 */
guint64
modulemd_iso8601date_to_guint64 (const gchar *iso8601);


/**
 * modulemd_guint64_to_iso8601date:
 * @date: (in): date represented as a 64-bit integer (such as 201807011200).
 *
 * Returns: (transfer full): A newly-allocated string containing a date in ISO 8601 format.
 *
 * Since: 2.10
 */
gchar *
modulemd_guint64_to_iso8601date (guint64 date);


/**
 * MODULEMD_REPLACE_SET:
 * @_dest: A reference to a #GHashTable.
 * @_set: (nullable): A reference to a #GHashTable.
 *
 * Frees the existing #GHashTable at @_dest. If @_set is not NULL, assigns a
 * deep copy of @_set's keys for use as a set.
 *
 * This helper function is intended for use in GOBJECT_copy() functions to
 * simplify copying internal set variables and avoid code duplication.
 *
 * Since: 2.0
 */
#define MODULEMD_REPLACE_SET(_dest, _set)                                     \
  do                                                                          \
    {                                                                         \
      if (_set)                                                               \
        {                                                                     \
          g_clear_pointer (&_dest, g_hash_table_unref);                       \
          _dest = modulemd_hash_table_deep_set_copy (_set);                   \
        }                                                                     \
      else                                                                    \
        {                                                                     \
          g_hash_table_remove_all (_dest);                                    \
        }                                                                     \
    }                                                                         \
  while (0)


/**
 * MODULEMD_SETTER_GETTER_STRING_EXT:
 * @is_static: static for private methods, or empty comment for public.
 * @ObjName: The name of the object type, in camel case.
 * @obj_name: The name of the object type, in lowercase with words separated
 * by '_'.
 * @OBJ_NAME: The name of the object type, in uppercase with words separated
 * by '_'.
 * @attr: The name of the object attribute, in lowercase.
 * @ATTR: The name of the object attribute, in uppercase.
 *
 * A convenience macro for defining standard set and get methods for a string
 * attribute of an object.
 *
 * This is the internal implementation for %MODULEMD_SETTER_GETTER_STRING and
 * %MODULEMD_SETTER_GETTER_STRING_STATIC which should be used instead.
 *
 * Since: 2.2
 */
#define MODULEMD_SETTER_GETTER_STRING_EXT(                                    \
  is_static, ObjName, obj_name, OBJ_NAME, attr, ATTR)                         \
  is_static void modulemd_##obj_name##_set_##attr (ObjName *self,             \
                                                   const gchar *attr)         \
  {                                                                           \
    g_return_if_fail (MODULEMD_IS_##OBJ_NAME (self));                         \
                                                                              \
    g_clear_pointer (&self->attr, g_free);                                    \
    self->attr = g_strdup (attr);                                             \
                                                                              \
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_##ATTR]);      \
  }                                                                           \
                                                                              \
                                                                              \
  is_static const gchar *modulemd_##obj_name##_get_##attr (ObjName *self)     \
  {                                                                           \
    g_return_val_if_fail (MODULEMD_IS_##OBJ_NAME (self), NULL);               \
                                                                              \
    return self->attr;                                                        \
  }

/**
 * MODULEMD_SETTER_GETTER_STRING:
 * @ObjName: The name of the object type, in camel case.
 * @obj_name: The name of the object type, in lowercase with words separated
 * by '_'.
 * @OBJ_NAME: The name of the object type, in uppercase with words separated
 * by '_'.
 * @attr: The name of the object attribute, in lowercase.
 * @ATTR: The name of the object attribute, in uppercase.
 *
 * A convenience macro for defining standard <emphasis>public</emphasis> set
 * and get methods for a string attribute of an object.
 *
 * Since: 2.2
 */
#define MODULEMD_SETTER_GETTER_STRING(                                        \
  ObjName, obj_name, OBJ_NAME, attr, ATTR)                                    \
  MODULEMD_SETTER_GETTER_STRING_EXT (                                         \
    /**/, ObjName, obj_name, OBJ_NAME, attr, ATTR)

/**
 * MODULEMD_SETTER_GETTER_STRING_STATIC:
 * @ObjName: The name of the object type, in camel case.
 * @obj_name: The name of the object type, in lowercase with words separated
 * by '_'.
 * @OBJ_NAME: The name of the object type, in uppercase with words separated
 * by '_'.
 * @attr: The name of the object attribute, in lowercase.
 * @ATTR: The name of the object attribute, in uppercase.
 *
 * A is a convenience macro for defining standard <emphasis>private</emphasis>
 * (static) set and get methods for a string attribute of an object.
 *
 * Since: 2.2
 */
#define MODULEMD_SETTER_GETTER_STRING_STATIC(                                 \
  ObjName, obj_name, OBJ_NAME, attr, ATTR)                                    \
  MODULEMD_SETTER_GETTER_STRING_EXT (                                         \
    static, ObjName, obj_name, OBJ_NAME, attr, ATTR)

/**
 * modulemd_str_set_new:
 *
 * A convenience macro to simplify the common operation of setting up a hash
 * table in libmodulemd for containing a set() of unique strings.
 *
 * Since: 2.10
 */
#define modulemd_str_set_new()                                                \
  g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL)
