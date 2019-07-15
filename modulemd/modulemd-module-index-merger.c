/*
 * This file is part of libmodulemd
 * Copyright (C) 2018 Stephen Gallagher
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#include <glib.h>
#include "modulemd-defaults.h"
#include "modulemd-module.h"
#include "modulemd-module-index.h"
#include "modulemd-module-index-merger.h"
#include "modulemd-module-stream.h"
#include "private/modulemd-module-index-private.h"
#include "private/modulemd-util.h"


typedef struct _priorities
{
  gint16 priority;
  GPtrArray *index_array;
} MergerPriorities;

struct _ModulemdModuleIndexMerger
{
  GObject parent_instance;

  ModulemdModuleIndex *merged;

  GPtrArray *priority_levels; /* <MergerPriorities> */
};

G_DEFINE_TYPE (ModulemdModuleIndexMerger,
               modulemd_module_index_merger,
               G_TYPE_OBJECT)

ModulemdModuleIndexMerger *
modulemd_module_index_merger_new (void)
{
  return g_object_new (MODULEMD_TYPE_MODULE_INDEX_MERGER, NULL);
}

static void
modulemd_module_index_merger_finalize (GObject *object)
{
  ModulemdModuleIndexMerger *self = (ModulemdModuleIndexMerger *)object;

  g_clear_pointer (&self->priority_levels, g_ptr_array_unref);
  g_clear_object (&self->merged);

  G_OBJECT_CLASS (modulemd_module_index_merger_parent_class)
    ->finalize (object);
}

static void
modulemd_module_index_merger_get_property (GObject *object,
                                           guint prop_id,
                                           GValue *value,
                                           GParamSpec *pspec)
{
  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_module_index_merger_set_property (GObject *object,
                                           guint prop_id,
                                           const GValue *value,
                                           GParamSpec *pspec)
{
  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_module_index_merger_class_init (ModulemdModuleIndexMergerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_module_index_merger_finalize;
  object_class->get_property = modulemd_module_index_merger_get_property;
  object_class->set_property = modulemd_module_index_merger_set_property;
}


static void
merger_priorities_free (void *ptr)
{
  MergerPriorities *level = (MergerPriorities *)ptr;

  g_clear_pointer (&level->index_array, g_ptr_array_unref);
  g_free (level);
}


static void
modulemd_module_index_merger_init (ModulemdModuleIndexMerger *self)
{
  self->priority_levels =
    g_ptr_array_new_with_free_func (merger_priorities_free);
}


static GPtrArray *
get_or_create_index_array (GPtrArray *priorities, gint32 priority)
{
  MergerPriorities *level = NULL;

  for (guint i = 0; i < priorities->len; i++)
    {
      level = g_ptr_array_index (priorities, i);

      if (level->priority == priority)
        {
          return level->index_array;
        }

      if (level->priority > priority)
        {
          /* We encountered a higher priority, so insert this new one
           * just before it
           */
          level = g_new0 (MergerPriorities, 1);
          level->priority = priority;
          level->index_array = g_ptr_array_new_full (1, g_object_unref);
          g_ptr_array_insert (priorities, i, level);
          return level->index_array;
        }
    }

  /* Went through the list and didn't encounter a higher value. Add this
   * to the end
   */
  level = g_new0 (MergerPriorities, 1);
  level->priority = priority;
  level->index_array = g_ptr_array_new_full (1, g_object_unref);
  g_ptr_array_add (priorities, level);
  return level->index_array;
}


void
modulemd_module_index_merger_associate_index (ModulemdModuleIndexMerger *self,
                                              ModulemdModuleIndex *index,
                                              gint32 priority)
{
  MODULEMD_INIT_TRACE ();
  GPtrArray *index_array = NULL;

  g_return_if_fail (MODULEMD_IS_MODULE_INDEX_MERGER (self));
  g_return_if_fail (MODULEMD_IS_MODULE_INDEX (index));
  g_return_if_fail (priority >= 0 && priority <= 1000);

  index_array = get_or_create_index_array (self->priority_levels, priority);

  g_ptr_array_add (index_array, g_object_ref (index));
}


ModulemdModuleIndex *
modulemd_module_index_merger_resolve (ModulemdModuleIndexMerger *self,
                                      GError **error)
{
  return modulemd_module_index_merger_resolve_ext (self, FALSE, error);
}

ModulemdModuleIndex *
modulemd_module_index_merger_resolve_ext (ModulemdModuleIndexMerger *self,
                                          gboolean strict_default_streams,
                                          GError **error)
{
  MODULEMD_INIT_TRACE ();
  g_autoptr (ModulemdModuleIndex) thislevel = NULL;
  g_autoptr (ModulemdModuleIndex) final = NULL;
  g_autoptr (GError) nested_error = NULL;
  GPtrArray *indexes = NULL;
  MergerPriorities *priority_level;

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX_MERGER (self), NULL);

  final = modulemd_module_index_new ();

  for (guint i = 0; i < self->priority_levels->len; i++)
    {
      priority_level = g_ptr_array_index (self->priority_levels, i);
      g_debug ("Handling Priority Level: %" G_GINT32_FORMAT,
               priority_level->priority);

      /* At each level, process through the attached ModuleIndex objects and
       * merge them together, then into the lower levels.
       */
      thislevel = modulemd_module_index_new ();
      indexes = priority_level->index_array;

      for (guint j = 0; j < indexes->len; j++)
        {
          /* Merge each ModuleIndex at this priority level into 'thislevel' */
          if (!modulemd_module_index_merge (g_ptr_array_index (indexes, j),
                                            thislevel,
                                            FALSE,
                                            strict_default_streams,
                                            &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return NULL;
            }
        }


      /* Merge 'thislevel' into 'final' with override=True */
      if (!modulemd_module_index_merge (
            thislevel, final, TRUE, strict_default_streams, &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return NULL;
        }

      g_clear_object (&thislevel);
    }
  return g_steal_pointer (&final);
}
