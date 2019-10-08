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
#include "modulemd-defaults.h"
#include "modulemd-simpleset.h"
#include "private/modulemd-private.h"
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
  guint64 modified;
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


guint64
modulemd_defaults_peek_version (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, MD_DEFAULTS_VERSION_UNSET);

  return self->version;
}


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


const gchar *
modulemd_defaults_peek_module_name (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, NULL);

  return self->module_name;
}


gchar *
modulemd_defaults_dup_module_name (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, NULL);

  return g_strdup (self->module_name);
}


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


const gchar *
modulemd_defaults_peek_default_stream (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, NULL);

  if (self->default_stream &&
      g_str_equal (self->default_stream, DEFAULT_MERGE_CONFLICT))
    {
      /* During an index merge, we determined that this was in conflict
       * with another set of ModulemdDefaults for the same module. If we
       * see this, treat it as no default stream when querying for it.
       */
      return NULL;
    }
  return self->default_stream;
}


gchar *
modulemd_defaults_dup_default_stream (ModulemdDefaults *self)
{
  g_return_val_if_fail (self, NULL);

  if (self->default_stream &&
      g_str_equal (self->default_stream, DEFAULT_MERGE_CONFLICT))
    {
      /* During an index merge, we determined that this was in conflict
       * with another set of ModulemdDefaults for the same module. If we
       * see this, treat it as no default stream when querying for it.
       */
      return NULL;
    }

  return g_strdup (self->default_stream);
}


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


GHashTable *
modulemd_defaults_peek_profile_defaults (ModulemdDefaults *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), NULL);

  return self->profile_defaults;
}


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


GHashTable *
modulemd_defaults_peek_intents (ModulemdDefaults *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), NULL);

  return self->intents;
}


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


void
modulemd_defaults_set_modified (ModulemdDefaults *self, guint64 modified)
{
  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));
  self->modified = modified;
}


guint64
modulemd_defaults_get_modified (ModulemdDefaults *self)
{
  return self->modified;
}


ModulemdDefaults *
modulemd_defaults_new_from_file (const gchar *yaml_file, GError **error)
{
  return modulemd_defaults_new_from_file_ext (yaml_file, NULL, error);
}


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


ModulemdDefaults *
modulemd_defaults_new_from_string (const gchar *yaml_string, GError **error)
{
  return modulemd_defaults_new_from_string_ext (yaml_string, NULL, error);
}


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


ModulemdDefaults *
modulemd_defaults_new_from_stream (FILE *stream, GError **error)
{
  return modulemd_defaults_new_from_stream_ext (stream, NULL, error);
}


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


ModulemdDefaults *
modulemd_defaults_copy (ModulemdDefaults *self)
{
  if (!self)
    return NULL;

  ModulemdDefaults *new_defaults = modulemd_defaults_new ();

  modulemd_defaults_set_version (new_defaults,
                                 modulemd_defaults_peek_version (self));
  modulemd_defaults_set_module_name (
    new_defaults, modulemd_defaults_peek_module_name (self));
  modulemd_defaults_set_modified (new_defaults,
                                  modulemd_defaults_get_modified (self));
  modulemd_defaults_set_default_stream (
    new_defaults, modulemd_defaults_peek_default_stream (self));
  modulemd_defaults_set_profile_defaults (
    new_defaults, modulemd_defaults_peek_profile_defaults (self));

  modulemd_defaults_set_intents (new_defaults,
                                 modulemd_defaults_peek_intents (self));

  return new_defaults;
}


static gboolean
modulemd_defaults_merge_default_profiles (GHashTable *from_profile_defaults,
                                          GHashTable *merged_profile_defaults,
                                          guint64 from_modified,
                                          guint64 into_modified,
                                          GError **error);

static void
modulemd_defaults_merge_intent_default_streams (ModulemdIntent *from_intent,
                                                ModulemdIntent *into_intent,
                                                const gchar *intent_name,
                                                guint64 from_modified,
                                                guint64 into_modified);

ModulemdDefaults *
modulemd_defaults_merge (ModulemdDefaults *first,
                         ModulemdDefaults *second,
                         gboolean override,
                         GError **error)
{
  g_autoptr (ModulemdDefaults) merged = NULL;
  ModulemdIntent *from_intent = NULL;
  ModulemdIntent *merged_intent = NULL;
  const gchar *intent_name = NULL;
  GHashTableIter iter;
  gpointer key, value;


  g_return_val_if_fail (first && MODULEMD_IS_DEFAULTS (first), NULL);
  g_return_val_if_fail (second && MODULEMD_IS_DEFAULTS (second), NULL);

  if (override)
    {
      /* If override is set, then returning a copy of second is the
       * shortest path
       */
      return modulemd_defaults_copy (second);
    }

  /* Start from a copy of the base */
  merged = modulemd_defaults_copy (first);

  /* == Merge default streams == */
  if (second->default_stream && !merged->default_stream)
    {
      /* Only the second Defaults had a default stream, so set that */
      modulemd_defaults_set_default_stream (merged, second->default_stream);
    }
  else if (merged->default_stream && second->default_stream)
    {
      /* Both of them had a defaults set */

      /* Shortcut past if we already know there are conflicts in this
       * default stream.
       */
      if (!g_str_equal (merged->default_stream, DEFAULT_MERGE_CONFLICT))
        {
          /* If second has a higher modified value, use its value.
           * If first has a higher modified value, it's already saved in
           * merged from the copy()
           */
          if (second->modified > first->modified)
            {
              modulemd_defaults_set_default_stream (merged,
                                                    second->default_stream);
            }
          else if (first->modified == second->modified)
            {
              if (!g_str_equal (first->default_stream, second->default_stream))
                {
                  /* They have conflicting default streams */
                  g_info ("Module stream mismatch in merge: %s != %s",
                          first->default_stream,
                          second->default_stream);

                  /* Set the special conflicting value */
                  modulemd_defaults_set_default_stream (
                    merged, DEFAULT_MERGE_CONFLICT);
                }
              /* Otherwise, they are the same and merged will have the correct
               * value from the copy()
               */
            }
        }
    }
  /* If neither of the above matched, both first and second had NULL for the
   * default stream, so nothing to do
   */


  /* == Merge profile defaults == */
  if (!modulemd_defaults_merge_default_profiles (second->profile_defaults,
                                                 merged->profile_defaults,
                                                 second->modified,
                                                 first->modified,
                                                 error))
    {
      return NULL;
    }


  /* == Merge intent defaults == */

  /* --- Merge intent default stream values --- */

  /* Iterate through 'second', adding any new values and checking the existing
   * ones for equivalence.
   */
  g_hash_table_iter_init (&iter, modulemd_defaults_peek_intents (second));
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_return_val_if_fail (value && MODULEMD_IS_INTENT (value), FALSE);

      intent_name = (gchar *)key;
      from_intent = MODULEMD_INTENT (value);

      merged_intent = g_hash_table_lookup (merged->intents, intent_name);
      if (!merged_intent)
        {
          /* This intent doesn't exist yet, so just add it completely. */
          g_hash_table_insert (merged->intents,
                               g_strdup (intent_name),
                               modulemd_intent_copy (from_intent));
          continue;
        }

      /* Merge the intent default streams */
      modulemd_defaults_merge_intent_default_streams (from_intent,
                                                      merged_intent,
                                                      intent_name,
                                                      second->modified,
                                                      first->modified);

      /* Merge the intent default profiles */
      if (!modulemd_defaults_merge_default_profiles (
            modulemd_intent_peek_profile_defaults (from_intent),
            modulemd_intent_peek_profile_defaults (merged_intent),
            second->modified,
            first->modified,
            error))
        {
          return NULL;
        }
    }

  /* Set the modified value to the higher of the two provided */
  if (second->modified > first->modified)
    modulemd_defaults_set_modified (merged, second->modified);

  return g_steal_pointer (&merged);
}


static gboolean
modulemd_defaults_merge_default_profiles (GHashTable *from_profile_defaults,
                                          GHashTable *merged_profile_defaults,
                                          guint64 from_modified,
                                          guint64 into_modified,
                                          GError **error)
{
  GHashTableIter iter;
  gpointer key, value;
  gchar *stream_name = NULL;
  ModulemdSimpleSet *from_profiles = NULL;
  ModulemdSimpleSet *merged_profiles = NULL;
  ModulemdSimpleSet *copied_profiles = NULL;

  g_hash_table_iter_init (&iter, from_profile_defaults);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      stream_name = (gchar *)key;
      from_profiles = (ModulemdSimpleSet *)value;
      merged_profiles =
        g_hash_table_lookup (merged_profile_defaults, stream_name);

      if (!merged_profiles)
        {
          /* Didn't appear in the profiles list, so just add it to merged */
          modulemd_simpleset_copy (from_profiles, &copied_profiles);
          g_hash_table_insert (
            merged_profile_defaults, g_strdup (stream_name), copied_profiles);
          copied_profiles = NULL;
          continue;
        }

      /* Check to see if they match */
      if (!modulemd_simpleset_is_equal (from_profiles, merged_profiles))
        {
          if (from_modified > into_modified)
            {
              modulemd_simpleset_copy (from_profiles, &copied_profiles);
              g_hash_table_insert (merged_profile_defaults,
                                   g_strdup (stream_name),
                                   copied_profiles);
              copied_profiles = NULL;
            }
          else if (into_modified > from_modified)
            {
              /* Already there, so just continue */
              continue;
            }
          else
            {
              /* The profile sets differed. This is an unresolvable merge
               * conflict
               */
              g_set_error (error,
                           MODULEMD_DEFAULTS_ERROR,
                           MODULEMD_DEFAULTS_ERROR_CONFLICTING_PROFILES,
                           "Profile default mismatch in stream: %s",
                           stream_name);
              return FALSE;
            }
        }

      /* They were a complete match, so no need to add it a second time */
    }

  return TRUE;
}

static void
modulemd_defaults_merge_intent_default_streams (ModulemdIntent *from_intent,
                                                ModulemdIntent *into_intent,
                                                const gchar *intent_name,
                                                guint64 from_modified,
                                                guint64 into_modified)
{
  const gchar *from_default_stream = NULL;
  const gchar *into_default_stream = NULL;

  g_return_if_fail (from_intent && MODULEMD_IS_INTENT (from_intent));
  g_return_if_fail (into_intent && MODULEMD_IS_INTENT (into_intent));
  g_return_if_fail (intent_name);

  from_default_stream = modulemd_intent_peek_default_stream (from_intent);

  /* If there is no new default stream, just jump to the next item */
  if (!from_default_stream)
    return;

  into_default_stream = modulemd_intent_peek_default_stream (into_intent);


  /* If a previous merge has already marked this as conflicting, just bail
   * out here and move on to the next intent
   */
  if (g_str_equal (into_default_stream, DEFAULT_MERGE_CONFLICT))
    return;


  if (into_default_stream)
    {
      /* Both default stream names are present.
       * If they are equal, there's nothing to do.
       */

      if (!g_str_equal (into_default_stream, from_default_stream))
        {
          if (from_modified > into_modified)
            {
              /* Set the default stream of from as the merged value */
              modulemd_intent_set_default_stream (into_intent,
                                                  from_default_stream);
              return;
            }
          else if (into_modified == from_modified)
            {
              g_info (
                "Module stream mismatch in merge: %s != %s for intent %s",
                into_default_stream,
                from_default_stream,
                intent_name);
              modulemd_intent_set_default_stream (into_intent,
                                                  DEFAULT_MERGE_CONFLICT);
              return;
            }
          /* Otherwise into is already set, so do nothing */
        }
    }
  else /* !into_default_stream */
    {
      /* There was no default stream set yet, so just add the new one */
      modulemd_intent_set_default_stream (into_intent, from_default_stream);
    }
}
