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
#include <string.h>
#include "modulemd-simpleset.h"
#include "private/modulemd-util.h"


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
  g_return_val_if_fail (MODULEMD_IS_SIMPLESET (self), 0);

  return g_hash_table_contains (self->set, value);
}


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


gchar **
modulemd_simpleset_get (ModulemdSimpleSet *self)
{
  return modulemd_simpleset_dup (self);
}


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


void
modulemd_simpleset_add (ModulemdSimpleSet *self, const gchar *value)
{
  if (g_hash_table_add (self->set, g_strdup (value)))
    {
      /* This key didn't previously exist */
      g_object_notify_by_pspec (G_OBJECT (self), set_properties[SET_PROP_SET]);
    }
}


void
modulemd_simpleset_remove (ModulemdSimpleSet *self, const gchar *value)
{
  gchar *key = g_strdup (value);
  if (g_hash_table_remove (self->set, key))
    {
      /* This key existed */
      g_object_notify_by_pspec (G_OBJECT (self), set_properties[SET_PROP_SET]);
    }
  g_free (key);
}


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


gboolean
modulemd_simpleset_is_equal (ModulemdSimpleSet *self, ModulemdSimpleSet *other)
{
  g_auto (GStrv) set_a = NULL;
  g_auto (GStrv) set_b = NULL;
  gsize i = 0;

  if (modulemd_simpleset_size (self) != modulemd_simpleset_size (other))
    {
      /* If they have a different number of strings in the set, they can't
       * be identical.
       */
      return FALSE;
    }

  set_a = modulemd_simpleset_dup (self);
  set_b = modulemd_simpleset_dup (other);

  for (i = 0; set_a[i] && set_b[i]; i++)
    {
      /* These are guaranteed to be returned ordered, so we can
       * assume that any difference at any index means that the
       * lists are not identical.
       */
      if (g_strcmp0 (set_a[i], set_b[i]))
        {
          /* No match, so this simpleset is not equal */
          return FALSE;
        }
    }

  /* If we made it here, everything must have matched */
  return TRUE;
}


/**
 * modulemd_simpleset_validate_contents:
 * @func: (scope call): a #SimpleSetValidationFn that will be run against each
 * entry in the set.
 * @failures: (array zero-terminated=1) (element-type utf8) (out) (nullable):
 * An array of strings in the set that failed the validation function. If this
 * returns non-NULL, it must be freed with g_ptr_array_unref().
 *
 * Returns: TRUE if all members of the set passed the validation. If any of the
 * set fails validation, they will be returned via failures.
 *
 * Since: 1.4
 */
gboolean
modulemd_simpleset_validate_contents (ModulemdSimpleSet *self,
                                      ModulemdSimpleSetValidationFn func,
                                      GPtrArray **failures)
{
  gboolean passing = TRUE;
  GHashTableIter iter;
  g_autoptr (GPtrArray) _failed = NULL;
  gpointer key, value;

  _failed = g_ptr_array_new_with_free_func (g_free);

  g_hash_table_iter_init (&iter, self->set);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      if (!func ((const gchar *)key))
        {
          passing = FALSE;
          g_ptr_array_add (_failed, g_strdup (key));
        }
    }

  if (!passing && failures)
    {
      *failures = g_ptr_array_ref (_failed);
    }

  return passing;
}
