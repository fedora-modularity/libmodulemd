/* modulemd-common.c
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

#include "modulemd.h"
#include "modulemd-yaml.h"
#include "modulemd-util.h"
#include <glib.h>


/**
 * modulemd_objects_from_file:
 * @yaml_file: A YAML file containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.2
 */
GPtrArray *
modulemd_objects_from_file (const gchar *yaml_file, GError **error)
{
  GPtrArray *data = NULL;
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!parse_yaml_file (yaml_file, &data, error))
    {
      return NULL;
    }

  return data;
}


/**
 * modulemd_objects_from_stream:
 * @stream: A YAML stream containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.4
 */
GPtrArray *
modulemd_objects_from_stream (FILE *stream, GError **error)
{
  GPtrArray *data = NULL;
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!parse_yaml_stream (stream, &data, error))
    {
      return NULL;
    }

  return data;
}


/**
 * modulemd_objects_from_string:
 * @yaml_string: A YAML string containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.2
 */
GPtrArray *
modulemd_objects_from_string (const gchar *yaml_string, GError **error)
{
  GPtrArray *data = NULL;
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!parse_yaml_string (yaml_string, &data, error))
    {
      return NULL;
    }

  return data;
}


/**
 * modulemd_dump:
 * @objects: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * modulemd or related objects to dump to YAML.
 * @yaml_file: The path to the file that should contain the resulting YAML
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Creates a file containing a series of YAML subdocuments, one per object
 * passed in.
 *
 * Since: 1.2
 */
void
modulemd_dump (GPtrArray *objects, const gchar *yaml_file, GError **error)
{
  g_return_if_fail (error == NULL || *error == NULL);

  emit_yaml_file (objects, yaml_file, error);
}


/**
 * modulemd_dumps:
 * @objects: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * modulemd or related objects to dump to YAML.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Creates a string containing a series of YAML subdocuments, one per object
 * passed in. This string must be freed with g_free() when no longer needed.
 *
 * Since: 1.2
 */
gchar *
modulemd_dumps (GPtrArray *objects, GError **error)
{
  gchar *yaml_string = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!emit_yaml_string (objects, &yaml_string, error))
    {
      return NULL;
    }

  return yaml_string;
}

/**
 * modulemd_merge_defaults:
 * @first: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * modulemd-related objects.
 * @second: (array zero-terminated=1) (element-type GObject) (nullable):
 * Optional. A #GPtrArray of modulemd-related objects to be merged into the
 * first list.
 * @override: Whether entries in @second should override those of @first in the
 * event of a conflict or whether they should attempt to merge instead.
 *
 * This function will process the lists of objects, merging duplicated
 * modulemd-defaults objects as needed. If the object lists have different
 * priorities, the conflicting values will be replaced by the ones from the
 * higher-priority list rather than merged.
 *
 * This function will return an error if the merge cannot be completed safely.
 *
 * Returns: (element-type GObject) (transfer container): A list of
 * module-related objects with defaults deduplicated and merged. This array is
 * newly-allocated and must be freed with g_ptr_array_unref().
 *
 * Since: 1.3
 */
GPtrArray *
modulemd_merge_defaults (const GPtrArray *first,
                         const GPtrArray *second,
                         gboolean override,
                         GError **error)
{
  g_autoptr (GPtrArray) merge_base = NULL;
  g_autoptr (GPtrArray) merged = NULL;
  g_autoptr (GPtrArray) keys = NULL;
  GObject *object = NULL;
  g_autoptr (GHashTable) defaults =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  gchar *key = NULL;
  gpointer value = NULL;
  ModulemdDefaults *updated_defs = NULL;
  gsize i;

  merge_base = g_ptr_array_new_full (first->len, g_object_unref);
  for (i = 0; i < first->len; i++)
    {
      g_ptr_array_add (merge_base,
                       g_object_ref (g_ptr_array_index (first, i)));
    }

  if (second && !override)
    {
      /* If the second repo doesn't override the first, then we can just treat
       * them as concatenated together and save some processing time.
       */
      g_ptr_array_set_size (merge_base, first->len + second->len);
      for (i = 0; i < second->len; i++)
        {
          g_ptr_array_add (merge_base,
                           g_object_ref (g_ptr_array_index (second, i)));
        }
    }

  merged = g_ptr_array_new_full (merge_base->len, g_object_unref);
  for (i = 0; i < merge_base->len; i++)
    {
      object = g_ptr_array_index (merge_base, i);
      if (MODULEMD_IS_DEFAULTS (object))
        {
          key = modulemd_defaults_dup_module_name (MODULEMD_DEFAULTS (object));
          value = g_hash_table_lookup (defaults, key);
          if (!value)
            {
              /* This is the first time we've encountered the defaults for this
               * module.
               */
              g_hash_table_replace (defaults, key, g_object_ref (object));
            }
          else
            {
              /* We've seen this one before. Handle the merge */
              updated_defs =
                modulemd_defaults_merge (MODULEMD_DEFAULTS (value),
                                         MODULEMD_DEFAULTS (object),
                                         FALSE,
                                         error);

              if (!updated_defs)
                {
                  /* The merge failed. Raise the error */
                  g_clear_pointer (&key, g_free);
                  return NULL;
                }

              g_hash_table_replace (defaults, key, updated_defs);
            }
        }
      else
        {
          /* Not a default object, so just add it to the list */
          g_ptr_array_add (merged, g_object_ref (object));
        }
    }

  if (second && override)
    {
      /* The repos had different priorities, so after resolving any conflicts
       * from the base repo, we will set any needed updates and overrides here.
       */
      for (i = 0; i < second->len; i++)
        {
          object = g_ptr_array_index (second, i);
          if (MODULEMD_IS_DEFAULTS (object))
            {
              key =
                modulemd_defaults_dup_module_name (MODULEMD_DEFAULTS (object));
              value = g_hash_table_lookup (defaults, key);
              if (!value)
                {
                  /* This is the first time we've encountered the defaults for this
                   * module.
                   */
                  g_hash_table_replace (defaults, key, g_object_ref (object));
                }
              else
                {
                  /* We've seen this one before. Handle the merge */
                  updated_defs =
                    modulemd_defaults_merge (MODULEMD_DEFAULTS (value),
                                             MODULEMD_DEFAULTS (object),
                                             TRUE,
                                             error);

                  if (!updated_defs)
                    {
                      /* The merge failed. Raise the error */
                      return NULL;
                    }

                  g_hash_table_replace (defaults, key, updated_defs);
                }
            }
          else
            {
              /* Not a default object, so just add it to the list */
              g_ptr_array_add (merged, g_object_ref (object));
            }
        }
    }

  /* Add all of the defaults to the end of the list */
  keys = _modulemd_ordered_str_keys (defaults, _modulemd_strcmp_sort);
  for (i = 0; i < keys->len; i++)
    {
      value = g_hash_table_lookup (defaults, g_ptr_array_index (keys, i));
      g_ptr_array_add (merged, g_object_ref (MODULEMD_DEFAULTS (value)));
    }

  return g_ptr_array_ref (merged);
}
