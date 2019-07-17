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

#include <inttypes.h>
#include "modulemd.h"
#include "modulemd-prioritizer.h"
#include "private/modulemd-util.h"


GQuark
modulemd_prioritizer_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-prioritizer-error-quark");
}

struct _ModulemdPrioritizer
{
  GObject parent_instance;

  GHashTable *priorities;
};

G_DEFINE_TYPE (ModulemdPrioritizer, modulemd_prioritizer, G_TYPE_OBJECT)

ModulemdPrioritizer *
modulemd_prioritizer_new (void)
{
  return g_object_new (MODULEMD_TYPE_PRIORITIZER, NULL);
}

static void
modulemd_prioritizer_finalize (GObject *object)
{
  ModulemdPrioritizer *self = (ModulemdPrioritizer *)object;

  g_clear_pointer (&self->priorities, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_prioritizer_parent_class)->finalize (object);
}

static void
modulemd_prioritizer_class_init (ModulemdPrioritizerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_prioritizer_finalize;

  /* This class has no public properties */
  object_class->get_property = NULL;
  object_class->set_property = NULL;
}

static GPtrArray *
_deduplicate_module_streams (const GPtrArray *first,
                             const GPtrArray *second,
                             GError **error);
static GPtrArray *
_latest_module_streams (const GPtrArray *streams, GError **error);

static void
_modulemd_ptr_array_unref (gpointer ptr)
{
  g_ptr_array_unref ((GPtrArray *)ptr);
}

static void
modulemd_prioritizer_init (ModulemdPrioritizer *self)
{
  self->priorities = g_hash_table_new_full (
    g_int64_hash, g_int64_equal, g_free, _modulemd_ptr_array_unref);
}


gboolean
modulemd_prioritizer_add (ModulemdPrioritizer *self,
                          GPtrArray *objects,
                          gint64 priority,
                          GError **error)
{
  GPtrArray *current_objects = NULL;
  g_autoptr (GPtrArray) concat_objects = NULL;
  g_autoptr (GPtrArray) deduped_objects = NULL;
  g_autoptr (GPtrArray) merged_objects = NULL;
  g_autoptr (ModulemdSimpleSet) nsvcs = modulemd_simpleset_new ();
  g_autofree gchar *nsvc = NULL;
  gint64 *prio = NULL;
  gsize i;

  g_return_val_if_fail (MODULEMD_IS_PRIORITIZER (self), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (priority < MODULEMD_PRIORITIZER_PRIORITY_MIN)
    {
      g_set_error (error,
                   MODULEMD_PRIORITIZER_ERROR,
                   MODULEMD_PRIORITIZER_PRIORITY_OUT_OF_RANGE,
                   "Priority %" PRIi64 " below the minimum value %d",
                   priority,
                   MODULEMD_PRIORITIZER_PRIORITY_MIN);
      return FALSE;
    }
  else if (priority > MODULEMD_PRIORITIZER_PRIORITY_MAX)
    {
      g_set_error (error,
                   MODULEMD_PRIORITIZER_ERROR,
                   MODULEMD_PRIORITIZER_PRIORITY_OUT_OF_RANGE,
                   "Priority %" PRIi64 " above the maximum value %d",
                   priority,
                   MODULEMD_PRIORITIZER_PRIORITY_MAX);
      return FALSE;
    }

  prio = g_new0 (gint64, 1);
  *prio = priority;

  current_objects = g_hash_table_lookup (self->priorities, prio);

  /* All values at the same priority level can be concatenated together
   * because the merge routine will handle it cleanly that way and with
   * less memory-usage. Any duplication will be cleaned up as part of the
   * merge logic. For performance reasons, we'll assume the common case that
   * there is no deduplication to happen and allocate enough memory for all
   * of the possible elements of the array up front.
   */
  if (current_objects)
    {
      concat_objects = g_ptr_array_new_full (
        current_objects->len + objects->len, g_object_unref);
    }
  else
    {
      concat_objects = g_ptr_array_new_full (objects->len, g_object_unref);
    }

  if (current_objects)
    {
      for (i = 0; i < current_objects->len; i++)
        {
          g_ptr_array_add (
            concat_objects,
            g_object_ref (g_ptr_array_index (current_objects, i)));
        }
    }

  deduped_objects =
    _deduplicate_module_streams (concat_objects, objects, error);
  if (!deduped_objects)
    {
      /* Something went wrong. Return the error here. */
      g_clear_pointer (&prio, g_free);
      return FALSE;
    }

  merged_objects =
    modulemd_merge_defaults (deduped_objects, NULL, FALSE, error);
  if (!merged_objects)
    {
      /* Something went wrong. Return the error here. */
      g_clear_pointer (&prio, g_free);
      return FALSE;
    }

  /* Replace or insert the new objects at this priority level */
  g_hash_table_replace (
    self->priorities, prio, g_ptr_array_ref (merged_objects));

  return TRUE;
}

gboolean
modulemd_prioritizer_add_index (ModulemdPrioritizer *self,
                                GHashTable *index,
                                gint64 priority,
                                GError **error)
{
  g_autoptr (GPtrArray) objects = NULL;

  g_return_val_if_fail (MODULEMD_IS_PRIORITIZER (self), FALSE);
  g_return_val_if_fail (index, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  objects = _modulemd_index_serialize (index, error);
  if (!objects)
    return FALSE;

  return modulemd_prioritizer_add (self, objects, priority, error);
}


GPtrArray *
modulemd_prioritizer_resolve (ModulemdPrioritizer *self, GError **error)
{
  g_autoptr (GPtrArray) current = NULL;
  g_autoptr (GPtrArray) prev = NULL;
  g_autoptr (GPtrArray) tmp = NULL;
  g_autoptr (GPtrArray) deduped_objects = NULL;
  g_autoptr (GPtrArray) latest_objects = NULL;
  g_autoptr (GList) priority_levels = NULL;
  GList *current_level = NULL;

  g_return_val_if_fail (MODULEMD_IS_PRIORITIZER (self), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  priority_levels = _modulemd_ordered_int64_keys (self->priorities);

  if (!priority_levels)
    {
      /* Nothing has been added to the resolver. */
      g_set_error (error,
                   MODULEMD_PRIORITIZER_ERROR,
                   MODULEMD_PRIORITIZER_NOTHING_TO_PRIORITIZE,
                   "No module objects have been added to the prioritizer. Use "
                   "modulemd_prioritizer_add() first.");
      return NULL;
    }

  /* Go through the merge from highest priority down to lowest. */
  current_level = g_list_last (priority_levels);

  current = g_ptr_array_ref (
    g_hash_table_lookup (self->priorities, current_level->data));

  while (current_level->prev)
    {
      prev = g_ptr_array_ref (
        g_hash_table_lookup (self->priorities, current_level->prev->data));

      /* Merge the values, replacing any conflicts */
      tmp = modulemd_merge_defaults (prev, current, TRUE, error);
      if (!tmp)
        {
          /* Something went wrong with the merge. Return the error */
          return NULL;
        }

      /* Deduplicate after the merge */
      deduped_objects = _deduplicate_module_streams (tmp, NULL, error);
      if (!deduped_objects)
        {
          /* Something went wrong. Return the error here. */
          return NULL;
        }

      g_clear_pointer (&current, g_ptr_array_unref);
      current = g_ptr_array_ref (deduped_objects);
      g_clear_pointer (&tmp, g_ptr_array_unref);
      g_clear_pointer (&deduped_objects, g_ptr_array_unref);
      g_clear_pointer (&prev, g_ptr_array_unref);

      current_level = current_level->prev;
    }

  /* Ensure that we have only the highest Version for each
   * (module_name, module_stream, context) object in the list
   */
  latest_objects = _latest_module_streams (current, error);

  /* TODO: Sort items */

  return g_ptr_array_ref (latest_objects);
}

static GPtrArray *
_deduplicate_module_streams (const GPtrArray *first,
                             const GPtrArray *second,
                             GError **error)
{
  GObject *object = NULL;
  g_autoptr (GPtrArray) deduplicated = NULL;
  g_autoptr (ModulemdSimpleSet) nsvcs = modulemd_simpleset_new ();
  g_autofree gchar *nsvc = NULL;
  gssize reserved_size, i;

  g_return_val_if_fail (first, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  /* Assume the common case that there are no duplicates and preallocate
   * space to hold the entire set.
   */
  reserved_size = first->len;
  if (second)
    {
      reserved_size += second->len;
    }

  deduplicated = g_ptr_array_new_full (reserved_size, g_object_unref);

  /* We check the second list here as a preventative measure. In a proper
   * implementation of this, we'd do a full check of the module stream entries
   * to ensure that they don't have the same NSVC but different content. For
   * now, however, we'll just assume that the second list has the right data
   * since it's likely to be newer.
   */
  if (second)
    for (i = 0; i < second->len; i++)
      {
        object = g_ptr_array_index (second, i);

        if (MODULEMD_IS_MODULE (object))
          {
            nsvc = modulemd_module_dup_nsvc (MODULEMD_MODULE (object));
          }
        else if (MODULEMD_IS_MODULESTREAM (object))
          {
            nsvc =
              modulemd_modulestream_get_nsvc (MODULEMD_MODULESTREAM (object));
          }
        if (nsvc)
          {
            if (modulemd_simpleset_contains (nsvcs, nsvc))
              {
                /* We've seen this NSVC before; skip it */
                continue;
              }

            /* Save this NSVC so we don't add it twice */
            modulemd_simpleset_add (nsvcs, nsvc);
            g_clear_pointer (&nsvc, g_free);
          }

        g_ptr_array_add (deduplicated, g_object_ref (object));
      }

  /* For the 'first' list, go through in reverse order, because this may be
   * called after the modulemd_merge_defaults() routine has already
   * concatenated the higher-priority list. This will ensure that the
   * other list wins any merge disputes.
   */
  for (i = (gsize)first->len - 1; i >= 0; i--)
    {
      object = g_ptr_array_index (first, i);

      if (MODULEMD_IS_MODULE (object))
        {
          nsvc = modulemd_module_dup_nsvc (MODULEMD_MODULE (object));
        }
      else if (MODULEMD_IS_MODULESTREAM (object))
        {
          nsvc =
            modulemd_modulestream_get_nsvc (MODULEMD_MODULESTREAM (object));
        }

      if (nsvc)
        {
          if (modulemd_simpleset_contains (nsvcs, nsvc))
            {
              /* We've seen this NSVC before; skip it */
              continue;
            }

          /* Save this NSVC so we don't add it twice */
          modulemd_simpleset_add (nsvcs, nsvc);
          g_clear_pointer (&nsvc, g_free);
        }

      g_ptr_array_add (deduplicated, g_object_ref (object));
    }

  return g_ptr_array_ref (deduplicated);
}


static GPtrArray *
_latest_module_streams (const GPtrArray *streams, GError **error)
{
  GObject *item = NULL;
  GObject *prev_item = NULL;
  g_autoptr (GPtrArray) prio_streams = NULL;
  g_autoptr (GHashTable) nsc_index = NULL;
  g_autofree gchar *nsc = NULL;
  guint64 version, prev_version;
  gsize i, idx;
  GHashTableIter iter;
  gpointer key;

  g_return_val_if_fail (streams, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  prio_streams = g_ptr_array_new_full (streams->len, g_object_unref);

  nsc_index = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  for (i = 0; i < streams->len; i++)
    {
      item = g_ptr_array_index (streams, i);
      g_return_val_if_fail (item && G_IS_OBJECT (item), NULL);

      if (!MODULEMD_IS_MODULE (item) && !MODULEMD_IS_MODULESTREAM (item))
        {
          /* No special handling for other object types */
          g_ptr_array_add (prio_streams, g_object_ref (item));
          continue;
        }

      if (MODULEMD_IS_MODULE (item))
        {
          nsc = g_strdup_printf (
            "%s:%s:%s",
            modulemd_module_peek_name (MODULEMD_MODULE (item)),
            modulemd_module_peek_stream (MODULEMD_MODULE (item)),
            modulemd_module_peek_context (MODULEMD_MODULE (item)));
          version = modulemd_module_get_version (MODULEMD_MODULE (item));
        }

      else if (MODULEMD_IS_MODULESTREAM (item))
        {
          nsc = g_strdup_printf (
            "%s:%s:%s",
            modulemd_modulestream_peek_name (MODULEMD_MODULESTREAM (item)),
            modulemd_modulestream_peek_stream (MODULEMD_MODULESTREAM (item)),
            modulemd_modulestream_peek_context (MODULEMD_MODULESTREAM (item)));
          version =
            modulemd_modulestream_get_version (MODULEMD_MODULESTREAM (item));
        }

      if (g_hash_table_lookup_extended (nsc_index, nsc, NULL, (void *)&idx))
        {
          prev_item = g_ptr_array_index (streams, idx);

          if (MODULEMD_IS_MODULE (item))
            {
              prev_version =
                modulemd_module_get_version (MODULEMD_MODULE (prev_item));
            }
          else if (MODULEMD_IS_MODULESTREAM (item))
            {
              prev_version = modulemd_modulestream_get_version (
                MODULEMD_MODULESTREAM (prev_item));
            }
          else
            {
              /* This should never happen */
              g_return_val_if_reached (NULL);
            }

          if (version > prev_version)
            {
              /* Update the hash table with the index of the newer version */
              g_hash_table_replace (nsc_index, g_strdup (nsc), (void *)i);
            }
        }
      else
        {
          /* This is the first time we've seen this name, stream and context */
          g_hash_table_replace (nsc_index, g_strdup (nsc), (void *)i);
        }

      g_clear_pointer (&nsc, g_free);
    }


  g_hash_table_iter_init (&iter, nsc_index);
  while (g_hash_table_iter_next (&iter, &key, (void *)&idx))
    {
      g_ptr_array_add (prio_streams,
                       g_object_ref (g_ptr_array_index (streams, idx)));
    }

  return g_ptr_array_ref (prio_streams);
}


GHashTable *
modulemd_prioritizer_resolve_index (ModulemdPrioritizer *self, GError **error)
{
  g_autoptr (GPtrArray) objects = NULL;

  g_return_val_if_fail (MODULEMD_IS_PRIORITIZER (self), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  objects = modulemd_prioritizer_resolve (self, error);
  if (!objects)
    return NULL;

  return module_index_from_data (objects, error);
}
