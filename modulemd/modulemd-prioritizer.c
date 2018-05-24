/* modulemd-prioritizer.c
 *
 * Copyright (C) 2018 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <inttypes.h>
#include "modulemd.h"
#include "modulemd-util.h"

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


/**
 * modulemd_prioritizer_add:
 * @objects: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * module-related objects loaded from a modulemd YAML stream.
 * @priority: The priority of the YAML stream these were loaded from. Items at
 * the same priority level will attempt to merge on conflict. Items at higher
 * priority levels will replace on conflict. Valid values are 0 - 1000.
 *
 * Returns: TRUE if the objects could be added without generating a conflict at
 * this priority level. If a conflict was detected, this function returns FALSE
 * and error is set. The internal state is undefined in the case of an error.
 *
 * Since: 1.3
 */
gboolean
modulemd_prioritizer_add (ModulemdPrioritizer *self,
                          GPtrArray *objects,
                          gint64 priority,
                          GError **error)
{
  GPtrArray *current_objects = NULL;
  g_autoptr (GPtrArray) concat_objects = NULL;
  g_autoptr (GPtrArray) merged_objects = NULL;
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

  for (i = 0; i < objects->len; i++)
    {
      g_ptr_array_add (concat_objects,
                       g_object_ref (g_ptr_array_index (objects, i)));
    }

  merged_objects =
    modulemd_merge_defaults (concat_objects, NULL, FALSE, error);
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


/**
 * modulemd_prioritizer_resolve:
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of module-related objects with all priorities resolved. This
 * object must be freed with g_ptr_array_unref().
 *
 * Since: 1.3
 */
GPtrArray *
modulemd_prioritizer_resolve (ModulemdPrioritizer *self, GError **error)
{
  g_autoptr (GPtrArray) current = NULL;
  g_autoptr (GPtrArray) next = NULL;
  g_autoptr (GPtrArray) tmp = NULL;
  g_autoptr (GList) priority_levels = NULL;
  GList *current_level = NULL;

  g_return_val_if_fail (MODULEMD_IS_PRIORITIZER (self), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  current_level = priority_levels =
    _modulemd_ordered_int64_keys (self->priorities);

  if (!current_level)
    {
      /* Nothing has been added to the resolver. */
      g_set_error (error,
                   MODULEMD_PRIORITIZER_ERROR,
                   MODULEMD_PRIORITIZER_NOTHING_TO_PRIORITIZE,
                   "No module objects have been added to the prioritizer. Use "
                   "modulemd_prioritizer_add() first.");
      return NULL;
    }

  current = g_ptr_array_ref (
    g_hash_table_lookup (self->priorities, priority_levels->data));

  while (current_level->next)
    {
      next = g_ptr_array_ref (
        g_hash_table_lookup (self->priorities, current_level->next->data));

      /* Merge the values, replacing any conflicts */
      tmp = modulemd_merge_defaults (current, next, TRUE, error);
      if (!tmp)
        {
          /* Something went wrong with the merge. Return the error */
          return NULL;
        }
      g_clear_pointer (&current, g_ptr_array_unref);
      current = g_ptr_array_ref (tmp);
      g_clear_pointer (&tmp, g_ptr_array_unref);
      g_clear_pointer (&next, g_ptr_array_unref);

      current_level = current_level->next;
    }

  return g_ptr_array_ref (current);
}
