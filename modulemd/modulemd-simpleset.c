/* modulemd-simpleset.c
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
#include <string.h>
#include "modulemd-simpleset.h"
#include "modulemd-util.h"

enum
{
  SET_PROP_0,

  SET_PROP_SET,

  SET_N_PROPERTIES
};

static GParamSpec *set_properties[SET_N_PROPERTIES] = {
  NULL,
};

struct _ModulemdSimpleSet
{
  GObject parent_instance;

  GHashTable *set;
};

G_DEFINE_TYPE (ModulemdSimpleSet, modulemd_simpleset, G_TYPE_OBJECT)

/**
 * modulemd_simpleset_contains:
 * @value: A string to compare against the set
 *
 * Returns: TRUE if the value existed in the set.
 *
 * Since: 1.0
 */
gboolean
modulemd_simpleset_contains (ModulemdSimpleSet *self, const gchar *value)
{
  g_return_val_if_fail (MODULEMD_IS_SIMPLESET (self), 0);

  return g_hash_table_contains (self->set, value);
}


/**
 * modulemd_simpleset_size:
 *
 * Returns: The number of elements in the set
 *
 * Since: 1.0
 */
guint
modulemd_simpleset_size (ModulemdSimpleSet *self)
{
  g_return_val_if_fail (MODULEMD_IS_SIMPLESET (self), 0);

  return g_hash_table_size (self->set);
}

static gboolean
modulemd_simpleset_remove_from_array (gpointer key,
                                      gpointer value,
                                      gpointer user_data)
{
  gchar **array;
  g_return_val_if_fail (key, FALSE);
  g_return_val_if_fail (user_data, FALSE);

  array = (gchar **)user_data;

  for (gsize i = 0; array[i]; i++)
    {
      if (g_strcmp0 ((gchar *)key, array[i]) == 0)
        {
          /* This value should stay in the set */
          return FALSE;
        }
    }

  /* This value wasn't in the array, so remove it */
  return TRUE;
}


/**
 * modulemd_simpleset_set:
 * @set: (array zero-terminated=1) (transfer none): Extensible metadata block
 *
 * Make the contents of the set equal to an array of strings. This function
 * will trigger a signal only if the resulting set is different. It does not
 * guarantee any order to the resulting set, only that it will be unique.
 *
 * Since: 1.0
 */
void
modulemd_simpleset_set (ModulemdSimpleSet *self, gchar **set)
{
  gboolean do_notify = FALSE;
  guint num_removed;

  g_return_if_fail (MODULEMD_IS_SIMPLESET (self));
  g_return_if_fail (set);

  /* Remove any values that are not part of the new set */
  num_removed = g_hash_table_foreach_remove (
    self->set, modulemd_simpleset_remove_from_array, set);
  if (num_removed > 0)
    {
      /* At least one value was removed, so we will need to notify */
      do_notify = TRUE;
    }

  /* Add in the whole new set to make sure we have everything */
  for (gsize i = 0; set[i]; i++)
    {
      if (g_hash_table_add (self->set, g_strdup (set[i])))
        {
          /* This key didn't previously exist */
          do_notify = TRUE;
        }
    }

  if (do_notify)
    {
      g_object_notify_by_pspec (G_OBJECT (self), set_properties[SET_PROP_SET]);
    }
}

/**
 * modulemd_simpleset_get:
 *
 * Retrieves the set as an array of strings
 *
 * Returns: (array zero-terminated=1) (transfer full):
 * A list representing a set of string values.
 *
 * Deprecated: 1.1
 * Use dup() instead.
 * This function was inconsistent with the other get() methods in libmodulemd
 * and should have been called dup() all along.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_simpleset_dup)
gchar **
modulemd_simpleset_get (ModulemdSimpleSet *self)
{
  return modulemd_simpleset_dup (self);
}


/**
 * modulemd_simpleset_dup:
 *
 * Retrieves the set as an array of strings
 *
 * Returns: (array zero-terminated=1) (transfer full):
 * A list representing a set of string values.
 *
 * Since: 1.1
 */
gchar **
modulemd_simpleset_dup (ModulemdSimpleSet *self)
{
  GPtrArray *sorted_keys = NULL;
  gchar **keys = NULL;
  g_return_val_if_fail (MODULEMD_IS_SIMPLESET (self), NULL);

  sorted_keys = _modulemd_ordered_str_keys (self->set, _modulemd_strcmp_sort);

  keys = g_malloc0_n (sorted_keys->len + 1, sizeof (char *));
  for (gsize i = 0; i < sorted_keys->len; i++)
    {
      keys[i] = g_strdup (g_ptr_array_index (sorted_keys, i));
    }
  keys[sorted_keys->len] = NULL;
  g_ptr_array_unref (sorted_keys);

  return keys;
}


/**
 * modulemd_simpleset_add
 * @value: A new string to add to the set
 *
 * This routine will add a new value to the set if it is not already present.
 *
 * Since: 1.0
 */
void
modulemd_simpleset_add (ModulemdSimpleSet *self, const gchar *value)
{
  if (g_hash_table_add (self->set, g_strdup (value)))
    {
      /* This key didn't previously exist */
      g_object_notify_by_pspec (G_OBJECT (self), set_properties[SET_PROP_SET]);
    }
}

/**
 * modulemd_simpleset_remove
 * @value: A string to remove from the set
 *
 * This routine will remove a value from the set if it is present.
 */
void
modulemd_simpleset_remove (ModulemdSimpleSet *self, const gchar *value)
{
  if (g_hash_table_remove (self->set, g_strdup (value)))
    {
      /* This key existed */
      g_object_notify_by_pspec (G_OBJECT (self), set_properties[SET_PROP_SET]);
    }
}


/**
 * modulemd_simpleset_copy
 * @dest: (out) (transfer full): A reference to the destination #ModulemdSimpleSet
 *
 * This function will copy the contents of this #ModulemdSimpleSet to @dest.
 * If the dereferenced pointer is NULL, a new #ModulemdSimpleSet will be
 * allocated.
 *
 * If the dereferenced pointer is not NULL, it will replace the contents of
 * @dest. All existing values in the set will be freed.
 *
 * In either case, the caller is responsible for calling g_object_unref()
 * later to free it.
 *
 * Since: 1.0
 */
void
modulemd_simpleset_copy (ModulemdSimpleSet *self, ModulemdSimpleSet **dest)
{
  gchar **keys = NULL;

  g_return_if_fail (!self || MODULEMD_IS_SIMPLESET (self));
  g_return_if_fail (dest);
  g_return_if_fail (*dest == NULL ||
                    (*dest != NULL && MODULEMD_IS_SIMPLESET (*dest)));

  /* Allocate a SimpleSet if needed */
  if (*dest == NULL)
    {
      *dest = modulemd_simpleset_new ();
    }

  if (self)
    {
      /* Get the set of keys so we can just use the set() function */
      keys = (gchar **)g_hash_table_get_keys_as_array (self->set, NULL);
    }
  else
    {
      /* If the source is NULL, treat it as empty */
      keys = g_new0 (gchar *, 1);
    }

  /* set() them. This will also handle the object notification */
  modulemd_simpleset_set (*dest, keys);

  g_free (keys);
}


static void
modulemd_simpleset_set_property (GObject *gobject,
                                 guint property_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdSimpleSet *self = MODULEMD_SIMPLESET (gobject);

  switch (property_id)
    {
    case SET_PROP_SET:
      modulemd_simpleset_set (self, g_value_get_boxed (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_simpleset_get_property (GObject *gobject,
                                 guint property_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdSimpleSet *self = MODULEMD_SIMPLESET (gobject);

  switch (property_id)
    {
    case SET_PROP_SET:
      g_value_take_boxed (value, modulemd_simpleset_dup (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_simpleset_finalize (GObject *gobject)
{
  ModulemdSimpleSet *self = (ModulemdSimpleSet *)gobject;

  g_clear_pointer (&self->set, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_simpleset_parent_class)->finalize (gobject);
}

static void
modulemd_simpleset_class_init (ModulemdSimpleSetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = modulemd_simpleset_set_property;
  object_class->get_property = modulemd_simpleset_get_property;

  object_class->finalize = modulemd_simpleset_finalize;

  set_properties[SET_PROP_SET] =
    g_param_spec_boxed ("set",
                        "The set represented as an array of strings.",
                        "An ordered list of unique strings in this set",
                        G_TYPE_STRV,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (
    object_class, SET_N_PROPERTIES, set_properties);
}

static void
modulemd_simpleset_init (ModulemdSimpleSet *self)
{
  /* Allocate the hash table */
  /* Free only once, since the key and value will be the same */
  self->set = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}

ModulemdSimpleSet *
modulemd_simpleset_new (void)
{
  return g_object_new (MODULEMD_TYPE_SIMPLESET, NULL);
}
