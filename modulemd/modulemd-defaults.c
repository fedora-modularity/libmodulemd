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

#include "modulemd-defaults.h"
#include "modulemd-simpleset.h"

struct _ModulemdDefaults
{
  GObject parent_instance;

  /* == Members == */
  guint64 version;
  gchar *module_name;
  gchar *default_stream;
  GHashTable *profile_defaults;
};

G_DEFINE_TYPE (ModulemdDefaults, modulemd_defaults, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_VERSION,
  PROP_MODULE_NAME,
  PROP_DEFAULT_STREAM,
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

    case PROP_PROFILE_DEFAULTS:
      g_value_set_boxed (value,
                         modulemd_defaults_peek_profile_defaults (self));
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


  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_defaults_init (ModulemdDefaults *self)
{
  self->profile_defaults =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}
