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

#include "config.h"
#include "modulemd.h"
#include "modulemd-module.h"
#include "private/modulemd-improvedmodule-private.h"
#include "private/modulemd-yaml.h"
#include "private/modulemd-util.h"
#include <glib.h>


GPtrArray *
modulemd_objects_from_file (const gchar *yaml_file, GError **error)
{
  return modulemd_objects_from_file_ext (yaml_file, NULL, error);
}


GPtrArray *
modulemd_objects_from_file_ext (const gchar *yaml_file,
                                GPtrArray **failures,
                                GError **error)
{
  g_autoptr (GPtrArray) data = NULL;
  GPtrArray *compat_data = NULL;
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!parse_yaml_file (yaml_file, &data, failures, error))
    {
      return NULL;
    }

  /* For backwards-compatibility, we need to return Modulemd.Module objects,
   * not Modulemd.ModuleStream objects
   */
  compat_data = convert_modulestream_to_module (data);

  return compat_data;
}


GHashTable *
modulemd_index_from_file (const gchar *yaml_file,
                          GPtrArray **failures,
                          GError **error)
{
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return parse_module_index_from_file (yaml_file, failures, error);
}


GPtrArray *
modulemd_objects_from_stream (FILE *stream, GError **error)
{
  return modulemd_objects_from_stream_ext (stream, NULL, error);
}


GPtrArray *
modulemd_objects_from_stream_ext (FILE *stream,
                                  GPtrArray **failures,
                                  GError **error)
{
  g_autoptr (GPtrArray) data = NULL;
  GPtrArray *compat_data = NULL;
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!parse_yaml_stream (stream, &data, failures, error))
    {
      return NULL;
    }

  /* For backwards-compatibility, we need to return Modulemd.Module objects,
   * not Modulemd.ModuleStream objects
   */
  compat_data = convert_modulestream_to_module (data);

  return compat_data;
}


GHashTable *
modulemd_index_from_stream (FILE *yaml_stream,
                            GPtrArray **failures,
                            GError **error)
{
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return parse_module_index_from_stream (yaml_stream, failures, error);
}


GPtrArray *
modulemd_objects_from_string (const gchar *yaml_string, GError **error)
{
  return modulemd_objects_from_string_ext (yaml_string, NULL, error);
}


GPtrArray *
modulemd_objects_from_string_ext (const gchar *yaml_string,
                                  GPtrArray **failures,
                                  GError **error)
{
  g_autoptr (GPtrArray) data = NULL;
  GPtrArray *compat_data = NULL;
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!parse_yaml_string (yaml_string, &data, failures, error))
    {
      return NULL;
    }

  /* For backwards-compatibility, we need to return Modulemd.Module objects,
   * not Modulemd.ModuleStream objects
   */
  compat_data = convert_modulestream_to_module (data);

  return compat_data;
}


GHashTable *
modulemd_index_from_string (const gchar *yaml_string,
                            GPtrArray **failures,
                            GError **error)
{
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return parse_module_index_from_string (yaml_string, failures, error);
}


gboolean
modulemd_dump_index (GHashTable *index, const gchar *yaml_file, GError **error)
{
  g_autoptr (GPtrArray) objects = _modulemd_index_serialize (index, error);
  if (!objects)
    {
      g_debug ("Serialization of index failed: %s", (*error)->message);
      return FALSE;
    }

  return emit_yaml_file (objects, yaml_file, error);
}


gchar *
modulemd_dumps_index (GHashTable *index, GError **error)
{
  gboolean result;
  gchar *yaml = NULL;

  g_autoptr (GPtrArray) objects = _modulemd_index_serialize (index, error);
  if (!objects)
    {
      g_debug ("Serialization of index failed: %s", (*error)->message);
      return FALSE;
    }

  result = emit_yaml_string (objects, &yaml, error);
  if (!result)
    {
      g_debug ("Emitting YAML string failed: %s", (*error)->message);
      return NULL;
    }

  return yaml;
}


void
modulemd_dump (GPtrArray *objects, const gchar *yaml_file, GError **error)
{
  g_return_if_fail (error == NULL || *error == NULL);

  emit_yaml_file (objects, yaml_file, error);
}


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


const gchar *
modulemd_get_version (void)
{
  return LIBMODULEMD_VERSION;
}
