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

gboolean
modulemd_simpleset_contains (ModulemdSimpleSet *self, const gchar *value)
{
  return g_hash_table_contains (self->set, value);
}

guint
modulemd_simpleset_size (ModulemdSimpleSet *self)
{
  return g_hash_table_size (self->set);
}

static gboolean
modulemd_simpleset_remove_from_array (gpointer key,
                                      gpointer value,
                                      gpointer user_data)
{
  GPtrArray *array;
  gchar *item;
  g_return_val_if_fail (key, FALSE);
  g_return_val_if_fail (user_data, FALSE);

  array = (GPtrArray *)user_data;

  for (gsize i = 0; i < array->len; i++)
    {
      item = g_ptr_array_index (array, i);
      if (g_strcmp0 ((gchar *)key, item) == 0)
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
 * @set: (array zero-terminated=0) (element-type utf8): Extensible metadata block
 *
 * Make the contents of the set equal to an array of strings. This function
 * will trigger a signal only if the resulting set is different. It does not
 * guarantee any order to the resulting set, only that it will be unique.
 */
void
modulemd_simpleset_set (ModulemdSimpleSet *self, GPtrArray *set)
{
  gboolean do_notify = FALSE;
  guint num_removed;
  gchar *item;

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
  for (gsize i = 0; i < set->len; i++)
    {
      item = g_ptr_array_index (set, i);
      if (g_hash_table_add (self->set, g_strdup (item)))
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
 * Retrieves the set as a #GPtrArray of strings
 *
 * Returns: (array zero-terminated=0) (element-type utf8) (transfer container):
 * A list representing a set of string values.
 */
GPtrArray *
modulemd_simpleset_get (ModulemdSimpleSet *self)
{
  g_return_val_if_fail (MODULEMD_IS_SIMPLESET (self), NULL);

  return _modulemd_ordered_str_keys (self->set, _modulemd_strcmp_sort);
}

void
modulemd_simpleset_add (ModulemdSimpleSet *self, const gchar *value)
{
  if (g_hash_table_add (self->set, g_strdup (value)))
    {
      /* This key didn't previously exist */
      g_object_notify_by_pspec (G_OBJECT (self), set_properties[SET_PROP_SET]);
    }
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
      g_value_set_boxed (value, modulemd_simpleset_get (self));
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

  /**
     * ModulemdSimpleSet:set: (type GLib.PtrArray(utf8)) (transfer container)
     */
  set_properties[SET_PROP_SET] =
    g_param_spec_boxed ("set",
                        "The set represented as an array of strings.",
                        "An ordered list of unique strings in this set",
                        G_TYPE_ARRAY,
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
