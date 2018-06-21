/* modulemd-defaults.c
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

#include "modulemd-defaults.h"
#include "modulemd-simpleset.h"
#include "private/modulemd-yaml.h"

GQuark
modulemd_defaults_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-defaults-error-quark");
}

struct _ModulemdDefaults
{
  GObject parent_instance;

  /* == Members == */
  guint64 version;
  gchar *module_name;
  gchar *default_stream;
  GHashTable *intents;
  GHashTable *profile_defaults;
};

G_DEFINE_TYPE (ModulemdDefaults, modulemd_defaults, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_VERSION,
  PROP_MODULE_NAME,
  PROP_DEFAULT_STREAM,
  PROP_INTENTS,
  PROP_PROFILE_DEFAULTS,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdDefaults *
modulemd_defaults_new (void)
{
  return g_object_new (MODULEMD_TYPE_DEFAULTS, NULL);
}

static void
modulemd_defaults_finalize (GObject *object)
{
  ModulemdDefaults *self = (ModulemdDefaults *)object;

  g_clear_pointer (&self->module_name, g_free);
  g_clear_pointer (&self->default_stream, g_free);
  g_clear_pointer (&self->intents, g_hash_table_unref);
  g_clear_pointer (&self->profile_defaults, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_defaults_parent_class)->finalize (object);
}


/**
 * modulemd_defaults_set_version:
 * @version: The metadata file format version
 *
 * Sets the version of the metadata in use.
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_version (ModulemdDefaults *self, guint64 version)
{
  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  if (self->version != version)
    {
      self->version = version;
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VERSION]);
    }
}


/**
 * modulemd_defaults_peek_version:
 *
 * Retrieves the metadata file format version.
 *
 * Returns: a 64-bit unsigned integer containing the file format version.
 *
 * Since: 1.1
 */
guint64
modulemd_defaults_peek_version (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, MD_DEFAULTS_VERSION_UNSET);

  return self->version;
}


/**
 * modulemd_defaults_set_module_name:
 * @name: The module name to which these defaults apply
 *
 * Sets the "module-name" property.
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_module_name (ModulemdDefaults *self, const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  if (g_strcmp0 (self->module_name, name) != 0)
    {
      g_free (self->module_name);
      self->module_name = g_strdup (name);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_NAME]);
    }
}


/**
 * modulemd_defaults_peek_module_name:
 *
 * Retrieves the module name to which these defaults apply.
 *
 * Returns: a string containing the "module-name" property. This string must
 * not be modified or freed. If you need to do so, use
 * modulemd_defaults_dup_module_name() instead.
 *
 * Since: 1.1
 */
const gchar *
modulemd_defaults_peek_module_name (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, NULL);

  return self->module_name;
}


/**
 * modulemd_defaults_dup_module_name:
 *
 * Retrieves the module name to which these defaults apply.
 *
 * Returns: a string containing the "module-name" property. This string must be
 * freed with g_free() when the caller is done with it.
 *
 * Since: 1.1
 */
gchar *
modulemd_defaults_dup_module_name (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, NULL);

  return g_strdup (self->module_name);
}


/**
 * modulemd_defaults_set_default_stream:
 * @stream: The default stream for this module
 *
 * Sets the "default-stream" property.
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_default_stream (ModulemdDefaults *self,
                                      const gchar *stream)
{
  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  if (g_strcmp0 (self->default_stream, stream) != 0)
    {
      g_free (self->default_stream);
      self->default_stream = g_strdup (stream);
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties[PROP_DEFAULT_STREAM]);
    }
}


/**
 * modulemd_defaults_peek_default_stream:
 *
 * Retrieves the default stream.
 *
 * Returns: a string containing the "default-stream" property. This string
 * must not be modified or freed. If you need to do so, use
 * modulemd_defaults_dup_default_stream() instead.
 *
 * Since: 1.1
 */
const gchar *
modulemd_defaults_peek_default_stream (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, NULL);

  return self->default_stream;
}


/**
 * modulemd_defaults_dup_default_stream:
 *
 * Retrieves the default stream.
 *
 * Returns: a string containing the "default-stream" property. This string must
 * be freed with g_free() when the caller is done with it.
 *
 * Since: 1.1
 */
gchar *
modulemd_defaults_dup_default_stream (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, NULL);

  return g_strdup (self->default_stream);
}


/**
 * modulemd_defaults_set_profiles_for_stream:
 * @stream: The name of the stream getting default profiles
 * @profiles: (array zero-terminated=1) (transfer none): The set of profile
 * names to install by default when installing this stream of the module.
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_profiles_for_stream (ModulemdDefaults *self,
                                           const gchar *stream,
                                           gchar **profiles)
{
  ModulemdSimpleSet *set = NULL;
  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  set = modulemd_simpleset_new ();
  modulemd_simpleset_set (set, profiles);

  g_hash_table_replace (self->profile_defaults, g_strdup (stream), set);
  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_PROFILE_DEFAULTS]);
}


/**
 * modulemd_defaults_assign_profiles_for_stream:
 * @stream: The name of the stream getting default profiles
 * @profiles: A #ModulemdSimpleSet of profile names to install by default when
 * installing this stream of the module.
 *
 * Since: 1.1
 */
void
modulemd_defaults_assign_profiles_for_stream (ModulemdDefaults *self,
                                              const gchar *stream,
                                              ModulemdSimpleSet *profiles)
{
  ModulemdSimpleSet *set = NULL;
  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  modulemd_simpleset_copy (profiles, &set);

  g_hash_table_replace (self->profile_defaults, g_strdup (stream), set);
  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_PROFILE_DEFAULTS]);
}


/**
 * modulemd_defaults_set_profile_defaults:
 * @profile_defaults: (nullable) (element-type utf8 ModulemdSimpleSet) (transfer none):
 *
 * Assigns the hash table of streams and their default profiles
 *
 * Since: 1.1
 */
void
modulemd_defaults_set_profile_defaults (ModulemdDefaults *self,
                                        GHashTable *profile_defaults)
{
  GHashTableIter iter;
  gpointer key, value;
  ModulemdSimpleSet *set = NULL;

  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  g_hash_table_remove_all (self->profile_defaults);

  if (profile_defaults)
    {
      g_hash_table_iter_init (&iter, profile_defaults);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          modulemd_simpleset_copy ((ModulemdSimpleSet *)value, &set);
          g_hash_table_replace (
            self->profile_defaults, g_strdup ((const gchar *)key), set);
          set = NULL;
        }
    }

  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_PROFILE_DEFAULTS]);
}


/**
 * modulemd_defaults_dup_profiles_for_stream:
 * @stream: The name of the stream from which to retrieve defaults
 *
 * Returns: (array zero-terminated=1) (transfer full): A zero-terminated array
 * of strings that provides the list of profiles that should be installed by
 * default when this stream is specified.
 *
 * Since: 1.1
 */
gchar **
modulemd_defaults_dup_profiles_for_stream (ModulemdDefaults *self,
                                           const gchar *stream)
{
  ModulemdSimpleSet *set = NULL;
  gchar **profiles = NULL;

  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), NULL);

  set = g_hash_table_lookup (self->profile_defaults, stream);

  profiles = modulemd_simpleset_dup (set);
  g_clear_pointer (&set, g_object_unref);

  return profiles;
}


/**
 * modulemd_defaults_peek_profile_defaults:
 *
 * Retrieves a hash table of the profile defaults.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A GHashTable
 * containing the set of profile defaults for streams of this module. This hash
 * table is maintained by the ModulemdDefaults object and must not be freed or
 * modified. If modification is necessary, use
 * modulemd_defaults_dup_profile_defaults() instead.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_defaults_peek_profile_defaults (ModulemdDefaults *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), NULL);

  return self->profile_defaults;
}


/**
 * modulemd_defaults_dup_profile_defaults:
 *
 * Retrieves a copy of the hash table of profile defaults.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer container): A
 * GHashTable containing the set of profile defaults for streams of this
 * module. This hash table is a copy and must be freed with
 * g_hash_table_unref() when the caller is finished with it. The elements it
 * contains are maintained by the hash table and will be automatically freed
 * when their key is removed or the hash table is freed.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_defaults_dup_profile_defaults (ModulemdDefaults *self)
{
  GHashTableIter iter;
  GHashTable *new;
  gpointer key, value;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), NULL);

  new =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  g_hash_table_iter_init (&iter, self->profile_defaults);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      modulemd_simpleset_copy ((ModulemdSimpleSet *)value, &set);
      g_hash_table_replace (new, g_strdup ((const gchar *)key), set);
      set = NULL;
    }

  return new;
}


/**
 * modulemd_defaults_add_intent:
 * @intent: (transfer none) (not nullable): The #ModulemdIntent to add to the
 * intents table.
 *
 * Adds an intent object to the hash table.
 *
 * Since: 1.5
 */
void
modulemd_defaults_add_intent (ModulemdDefaults *self, ModulemdIntent *intent)
{
  g_autoptr (ModulemdIntent) copy = NULL;

  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));
  g_return_if_fail (MODULEMD_IS_INTENT (intent));

  copy = modulemd_intent_copy (intent);
  g_hash_table_replace (self->intents,
                        g_strdup (modulemd_intent_peek_intent_name (intent)),
                        g_object_ref (copy));

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_INTENTS]);
}


/**
 * modulemd_defaults_set_intents:
 * @intents: (element-type utf8 ModulemdIntent) (nullable) (transfer none):
 * A #GHashTable containing defaults for individual system intents.
 *
 * Since: 1.5
 */
void
modulemd_defaults_set_intents (ModulemdDefaults *self, GHashTable *intents)
{
  GHashTableIter iter;
  gpointer key, value;
  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  g_hash_table_remove_all (self->intents);
  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_INTENTS]);

  if (intents)
    {
      g_hash_table_iter_init (&iter, intents);

      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          modulemd_defaults_add_intent (self, MODULEMD_INTENT (value));
        }
    }
}


/**
 * modulemd_defaults_peek_intents:
 *
 * Get a pointer to the intents hash table. The returned table is managed by the
 * #ModulemdDefaults object and must not be modified or freed.
 *
 * Returns: (element-type utf8 ModulemdIntent) (transfer none): A pointer to the
 * intents hash table.
 *
 * Since: 1.5
 */
GHashTable *
modulemd_defaults_peek_intents (ModulemdDefaults *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), NULL);

  return self->intents;
}


/**
 * modulemd_defaults_dup_intents:
 *
 * Get a copy of the intents hash table. The returned table is managed by the
 * caller and must be freed with g_hash_table_unref()
 *
 * Returns: (element-type utf8 ModulemdIntent) (transfer none): A copy of the
 * intents hash table.
 *
 * Since: 1.5
 */
GHashTable *
modulemd_defaults_dup_intents (ModulemdDefaults *self)
{
  g_autoptr (GHashTable) intents = NULL;
  ModulemdIntent *intent = NULL;
  gpointer key, value;
  GHashTableIter iter;

  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), NULL);

  intents =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  g_hash_table_iter_init (&iter, self->intents);

  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      intent = MODULEMD_INTENT (value);
      g_hash_table_replace (
        intents,
        g_strdup (modulemd_intent_peek_intent_name (intent)),
        modulemd_intent_copy (intent));
    }

  return g_hash_table_ref (intents);
}


/**
 * modulemd_defaults_new_from_file:
 * @yaml_file: A YAML file containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.2
 */
ModulemdDefaults *
modulemd_defaults_new_from_file (const gchar *yaml_file, GError **error)
{
  return modulemd_defaults_new_from_file_ext (yaml_file, NULL, error);
}


/**
 * modulemd_defaults_new_from_file_ext:
 * @yaml_file: A YAML file containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.4
 */
ModulemdDefaults *
modulemd_defaults_new_from_file_ext (const gchar *yaml_file,
                                     GPtrArray **failures,
                                     GError **error)
{
  GObject *object = NULL;
  GPtrArray *data = NULL;
  ModulemdDefaults *defaults = NULL;

  if (!parse_yaml_file (yaml_file, &data, failures, error))
    {
      return NULL;
    }

  for (gsize i = 0; i < data->len; i++)
    {
      object = g_ptr_array_index (data, i);
      if (MODULEMD_IS_DEFAULTS (object))
        {
          defaults = MODULEMD_DEFAULTS (g_object_ref (object));
          break;
        }
    }

  if (!defaults)
    {
      g_set_error (error,
                   MODULEMD_DEFAULTS_ERROR,
                   MODULEMD_DEFAULTS_ERROR_MISSING_CONTENT,
                   "Provided YAML file contained no valid defaults objects");
    }

  g_clear_pointer (&data, g_ptr_array_unref);

  return defaults;
}


/**
 * modulemd_defaults_new_from_string:
 * @yaml_string: A YAML string containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.2
 */
ModulemdDefaults *
modulemd_defaults_new_from_string (const gchar *yaml_string, GError **error)
{
  return modulemd_defaults_new_from_string_ext (yaml_string, NULL, error);
}


/**
 * modulemd_defaults_new_from_string_ext:
 * @yaml_string: A YAML string containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.4
 */
ModulemdDefaults *
modulemd_defaults_new_from_string_ext (const gchar *yaml_string,
                                       GPtrArray **failures,
                                       GError **error)
{
  GObject *object = NULL;
  GPtrArray *data = NULL;
  ModulemdDefaults *defaults = NULL;

  if (!parse_yaml_string (yaml_string, &data, failures, error))
    {
      return NULL;
    }

  for (gsize i = 0; i < data->len; i++)
    {
      object = g_ptr_array_index (data, i);
      if (MODULEMD_IS_DEFAULTS (object))
        {
          defaults = MODULEMD_DEFAULTS (g_object_ref (object));
          break;
        }
    }

  if (!defaults)
    {
      g_set_error (error,
                   MODULEMD_DEFAULTS_ERROR,
                   MODULEMD_DEFAULTS_ERROR_MISSING_CONTENT,
                   "Provided YAML file contained no valid defaults objects");
    }

  g_clear_pointer (&data, g_ptr_array_unref);

  return defaults;
}


/**
 * modulemd_defaults_new_from_stream:
 * @stream: A YAML stream containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.4
 */
ModulemdDefaults *
modulemd_defaults_new_from_stream (FILE *stream, GError **error)
{
  return modulemd_defaults_new_from_stream_ext (stream, NULL, error);
}


/**
 * modulemd_defaults_new_from_stream_ext:
 * @stream: A YAML stream containing the module metadata and other related
 * information such as default streams.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Constructs a new #ModulemdDefaults object from the first valid
 * modulemd-defaults document in the given module stream. This will ignore any
 * documents of other types, malformed documents and defaults that appear later
 * in the stream.
 *
 * Returns: A #ModulemdDefaults object constructed the first valid
 * modulemd-defaults document in the given module stream. This must be freed
 * with g_object_unref() when no longer needed.
 *
 * Since: 1.4
 */
ModulemdDefaults *
modulemd_defaults_new_from_stream_ext (FILE *stream,
                                       GPtrArray **failures,
                                       GError **error)
{
  GObject *object = NULL;
  g_autoptr (GPtrArray) data = NULL;
  ModulemdDefaults *defaults = NULL;

  if (!parse_yaml_stream (stream, &data, failures, error))
    {
      return NULL;
    }

  for (gsize i = 0; i < data->len; i++)
    {
      object = g_ptr_array_index (data, i);
      if (MODULEMD_IS_DEFAULTS (object))
        {
          defaults = MODULEMD_DEFAULTS (g_object_ref (object));
          break;
        }
    }

  if (!defaults)
    {
      g_set_error (error,
                   MODULEMD_DEFAULTS_ERROR,
                   MODULEMD_DEFAULTS_ERROR_MISSING_CONTENT,
                   "Provided YAML stream contained no valid defaults objects");
    }

  return defaults;
}


/**
 * modulemd_defaults_dump:
 * @file_path: File path for exporting the YAML representation of this defaults
 * object
 *
 * Exports the YAML representation of this defaults object to a file.
 *
 * Since: 1.1
 */
void
modulemd_defaults_dump (ModulemdDefaults *self, const gchar *file_path)
{
  GPtrArray *objects = NULL;
  GError *error = NULL;

  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  objects = g_ptr_array_new ();

  g_ptr_array_add (objects, self);

  if (!emit_yaml_file (objects, file_path, &error))
    {
      g_debug ("Failed to export YAML: [%s]", error->message);
    }

  g_ptr_array_unref (objects);
}

/**
 * modulemd_defaults_dumps:
 * @yaml_string: (out): File path for exporting the YAML representation of this defaults
 * object
 *
 * Exports the YAML representation of this defaults object to a string. The
 * caller is responsible for calling g_free() on this string when they are
 * finished with it.
 *
 * Since: 1.1
 */
void
modulemd_defaults_dumps (ModulemdDefaults *self, gchar **yaml_string)
{
  GPtrArray *objects = NULL;
  GError *error = NULL;

  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  objects = g_ptr_array_new ();

  g_ptr_array_add (objects, self);

  if (!emit_yaml_string (objects, yaml_string, &error))
    {
      g_debug ("Failed to export YAML: [%s]", error->message);
    }

  g_ptr_array_unref (objects);
}


static void
modulemd_defaults_get_property (GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
  ModulemdDefaults *self = MODULEMD_DEFAULTS (object);

  switch (prop_id)
    {
    case PROP_VERSION:
      g_value_set_uint64 (value, modulemd_defaults_peek_version (self));
      break;

    case PROP_MODULE_NAME:
      g_value_set_string (value, modulemd_defaults_peek_module_name (self));
      break;

    case PROP_DEFAULT_STREAM:
      g_value_set_string (value, modulemd_defaults_peek_default_stream (self));
      break;

    case PROP_INTENTS:
      g_value_take_boxed (value, modulemd_defaults_dup_intents (self));
      break;

    case PROP_PROFILE_DEFAULTS:
      g_value_take_boxed (value,
                          modulemd_defaults_dup_profile_defaults (self));
      break;


    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}

static void
modulemd_defaults_set_property (GObject *object,
                                guint prop_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
  ModulemdDefaults *self = MODULEMD_DEFAULTS (object);

  switch (prop_id)
    {
    case PROP_VERSION:
      modulemd_defaults_set_version (self, g_value_get_uint64 (value));
      break;

    case PROP_MODULE_NAME:
      modulemd_defaults_set_module_name (self, g_value_get_string (value));
      break;

    case PROP_DEFAULT_STREAM:
      modulemd_defaults_set_default_stream (self, g_value_get_string (value));
      break;

    case PROP_INTENTS:
      modulemd_defaults_set_intents (self, g_value_get_boxed (value));
      break;

    case PROP_PROFILE_DEFAULTS:
      modulemd_defaults_set_profile_defaults (self, g_value_get_boxed (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}

static void
modulemd_defaults_class_init (ModulemdDefaultsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_defaults_finalize;
  object_class->get_property = modulemd_defaults_get_property;
  object_class->set_property = modulemd_defaults_set_property;

  properties[PROP_VERSION] =
    g_param_spec_uint64 ("version",
                         "Module Defaults file format version",
                         "An integer property representing the defaults file "
                         "format used.",
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_MODULE_NAME] = g_param_spec_string (
    "module-name",
    "Module Name",
    "The name of the module to which these defaults apply.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_DEFAULT_STREAM] = g_param_spec_string (
    "default-stream",
    "Default Stream",
    "The name of the stream that will be used by default for this module.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdDefaults:profile-defaults: (type GLib.HashTable(utf8,ModulemdSimpleSet))
   */
  properties[PROP_PROFILE_DEFAULTS] = g_param_spec_boxed (
    "profile-defaults",
    "Profile Defaults",
    "A hash table comprised of the set of profiles that act as the default "
    "for each available stream of the module.",
    G_TYPE_HASH_TABLE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdDefaults:intents: (type GLib.HashTable(utf8,ModulemdIntent))
   */
  properties[PROP_INTENTS] = g_param_spec_boxed (
    "intents",
    "Intents",
    "A hash table describing divergent defaults based on the intent of the "
    "system.",
    G_TYPE_HASH_TABLE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);


  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_defaults_init (ModulemdDefaults *self)
{
  self->intents =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  self->profile_defaults =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}


/**
 * modulemd_defaults_copy:
 * Returns a deep-copy of the defaults object.
 *
 * Returns: (transfer full): A deep-copied #ModulemdDefaults object. This
 * object must be freed with g_object_unref().
 *
 * Since: 1.3
 */
ModulemdDefaults *
modulemd_defaults_copy (ModulemdDefaults *self)
{
  ModulemdDefaults *new_defaults = modulemd_defaults_new ();

  modulemd_defaults_set_version (new_defaults,
                                 modulemd_defaults_peek_version (self));
  modulemd_defaults_set_module_name (
    new_defaults, modulemd_defaults_peek_module_name (self));
  modulemd_defaults_set_default_stream (
    new_defaults, modulemd_defaults_peek_default_stream (self));
  modulemd_defaults_set_profile_defaults (
    new_defaults, modulemd_defaults_peek_profile_defaults (self));

  modulemd_defaults_set_intents (new_defaults,
                                 modulemd_defaults_peek_intents (self));

  return new_defaults;
}

/**
 * modulemd_defaults_merge:
 * @first: A #ModulemdDefaults object providing the base for the merge.
 * @second: A #ModulemdDefaults object being merged onto @first.
 * @override: In the case of a conflict, should @second completely replace the
 * contents of @first.
 *
 * Returns: (transfer full): A merged or replaced #ModulemdDefaults object. In
 * case of unresolvable merge, NULL will be returned and an error will be set.
 * This object must be freed with g_object_unref().
 *
 * Since: 1.3
 */
ModulemdDefaults *
modulemd_defaults_merge (ModulemdDefaults *first,
                         ModulemdDefaults *second,
                         gboolean override,
                         GError **error)
{
  g_autoptr (ModulemdDefaults) defaults = NULL;
  GHashTable *profile_defaults = NULL;
  g_autoptr (GHashTable) intents = NULL;
  ModulemdIntent *base_intent = NULL;
  ModulemdIntent *merge_intent = NULL;
  g_autoptr (ModulemdIntent) new_intent = NULL;
  g_autoptr (GHashTable) base_profiles = NULL;
  GHashTable *merge_profiles = NULL;
  const gchar *intent_name = NULL;
  ModulemdSimpleSet *profile = NULL;
  GHashTableIter iter, profile_iter;
  gpointer key, value, orig_value, prof_key, prof_value;

  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (first), NULL);
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (second), NULL);

  if (override)
    {
      /* If override is set, then returning a copy of second is the
       * shortest path
       */
      return modulemd_defaults_copy (second);
    }

  /* First check for incompatibilities with the streams */
  if (g_strcmp0 (modulemd_defaults_peek_default_stream (first),
                 modulemd_defaults_peek_default_stream (second)))
    {
      /* Default streams don't match and override is not set.
       * Return an error
       */
      g_set_error (
        error,
        MODULEMD_DEFAULTS_ERROR,
        MODULEMD_DEFAULTS_ERROR_CONFLICTING_STREAMS,
        "Conflicting default streams when merging defaults for module %s",
        modulemd_defaults_peek_module_name (first));
      return NULL;
    }

  defaults = modulemd_defaults_copy (first);

  /* Merge the profile defaults */
  profile_defaults = modulemd_defaults_peek_profile_defaults (defaults);

  g_hash_table_iter_init (&iter,
                          modulemd_defaults_peek_profile_defaults (second));
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      orig_value = g_hash_table_lookup (profile_defaults, key);
      if (orig_value)
        {
          /* This key already exists in the first defaults object.
           * Check whether they have identical values
           */
          if (!modulemd_simpleset_is_equal (orig_value, value))
            {
              g_set_error (error,
                           MODULEMD_DEFAULTS_ERROR,
                           MODULEMD_DEFAULTS_ERROR_CONFLICTING_PROFILES,
                           "Conflicting profile defaults when merging "
                           "defaults for module %s",
                           modulemd_defaults_peek_module_name (first));
              return NULL;
            }
        }
      else
        {
          /* This key is new. Add it */
          g_hash_table_replace (profile_defaults,
                                g_strdup (key),
                                g_object_ref (MODULEMD_SIMPLESET (value)));
        }
    }


  /* Merge intents */
  intents = modulemd_defaults_dup_intents (defaults);
  g_hash_table_iter_init (&iter, modulemd_defaults_peek_intents (second));
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      merge_intent = MODULEMD_INTENT (value);
      /* Check if this module name exists in the current table */
      intent_name = modulemd_intent_peek_intent_name (merge_intent);
      base_intent = g_hash_table_lookup (
        intents, modulemd_intent_peek_intent_name (merge_intent));

      if (!base_intent)
        {
          /* This intent doesn't exist yet, so just add it completely. */
          g_hash_table_insert (intents,
                               g_strdup (intent_name),
                               modulemd_intent_copy (merge_intent));
          continue;
        }

      /* Compare the default stream for this intent */
      if (g_strcmp0 (modulemd_intent_peek_default_stream (base_intent),
                     modulemd_intent_peek_default_stream (merge_intent)))
        {
          /* The streams didn't match, so bail out */
          g_set_error (error,
                       MODULEMD_DEFAULTS_ERROR,
                       MODULEMD_DEFAULTS_ERROR_CONFLICTING_INTENT_STREAM,
                       "Conflicting default stream for intent profile [%s]"
                       "when merging defaults for module %s",
                       (const gchar *)intent_name,
                       modulemd_defaults_peek_module_name (first));
          return NULL;
        }

      /* Construct a new Intent with the merged values which will replace
       * the existing one at the end */
      new_intent = modulemd_intent_copy (base_intent);

      /* Merge the profile definitions for this intent */
      base_profiles = modulemd_intent_dup_profile_defaults (new_intent);

      merge_profiles = modulemd_intent_peek_profile_defaults (merge_intent);
      g_hash_table_iter_init (&profile_iter, merge_profiles);
      while (g_hash_table_iter_next (&profile_iter, &prof_key, &prof_value))
        {
          /* Check if this profile exists in this intent */
          profile = g_hash_table_lookup (base_profiles, prof_key);

          if (!profile)
            {
              /* Add this profile to the intent */
              modulemd_simpleset_copy (prof_value, &profile);
              g_hash_table_insert (
                base_profiles, g_strdup ((const gchar *)prof_key), profile);
              continue;
            }

          if (!modulemd_simpleset_is_equal (profile, prof_value))
            {
              /* If we get here, the sets were unequal, so we need to fail */
              g_set_error (error,
                           MODULEMD_DEFAULTS_ERROR,
                           MODULEMD_DEFAULTS_ERROR_CONFLICTING_INTENT_PROFILE,
                           "Conflicting intent profile [%s:%s] when merging "
                           "defaults for module %s",
                           (const gchar *)intent_name,
                           (const gchar *)prof_key,
                           modulemd_defaults_peek_module_name (first));
              return NULL;
            }
        }

      modulemd_intent_set_profile_defaults (new_intent, base_profiles);
      g_clear_pointer (&base_profiles, g_hash_table_unref);
      g_hash_table_replace (
        intents, g_strdup (intent_name), g_object_ref (new_intent));
      g_clear_pointer (&new_intent, g_object_unref);
    }

  modulemd_defaults_set_intents (defaults, intents);

  return g_object_ref (defaults);
}
