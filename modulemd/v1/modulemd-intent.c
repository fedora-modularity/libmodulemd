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
#include "modulemd-intent.h"


struct _ModulemdIntent
{
  GObject parent_instance;

  gchar *intent_name;
  gchar *default_stream;
  GHashTable *profile_defaults;
};

G_DEFINE_TYPE (ModulemdIntent, modulemd_intent, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_INTENT_NAME,
  PROP_DEFAULT_STREAM,
  PROP_PROFILE_DEFAULTS,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdIntent *
modulemd_intent_new (const gchar *name)
{
  return g_object_new (MODULEMD_TYPE_INTENT, "intent-name", name, NULL);
}

static void
modulemd_intent_finalize (GObject *object)
{
  ModulemdIntent *self = (ModulemdIntent *)object;

  g_clear_pointer (&self->intent_name, g_free);
  g_clear_pointer (&self->default_stream, g_free);
  g_clear_pointer (&self->profile_defaults, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_intent_parent_class)->finalize (object);
}

static void
modulemd_intent_get_property (GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
  ModulemdIntent *self = MODULEMD_INTENT (object);

  switch (prop_id)
    {
    case PROP_INTENT_NAME:
      g_value_set_string (value, modulemd_intent_peek_intent_name (self));
      break;

    case PROP_DEFAULT_STREAM:
      g_value_set_string (value, modulemd_intent_peek_default_stream (self));
      break;

    case PROP_PROFILE_DEFAULTS:
      g_value_take_boxed (value, modulemd_intent_dup_profile_defaults (self));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_intent_set_property (GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
  ModulemdIntent *self = MODULEMD_INTENT (object);

  switch (prop_id)
    {
    case PROP_INTENT_NAME:
      modulemd_intent_set_intent_name (self, g_value_get_string (value));
      break;

    case PROP_DEFAULT_STREAM:
      modulemd_intent_set_default_stream (self, g_value_get_string (value));
      break;

    case PROP_PROFILE_DEFAULTS:
      modulemd_intent_set_profile_defaults (self, g_value_get_boxed (value));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_intent_class_init (ModulemdIntentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_intent_finalize;
  object_class->get_property = modulemd_intent_get_property;
  object_class->set_property = modulemd_intent_set_property;

  properties[PROP_INTENT_NAME] = g_param_spec_string (
    "intent-name",
    "Intent Name",
    "The name of the module to which these defaults apply.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  properties[PROP_DEFAULT_STREAM] = g_param_spec_string (
    "default-stream",
    "Default Stream",
    "The name of the stream that will be used by default for this module.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdIntent:profile-defaults: (type GLib.HashTable(utf8,ModulemdSimpleSet))
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
modulemd_intent_init (ModulemdIntent *self)
{
  self->profile_defaults =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}


void
modulemd_intent_set_intent_name (ModulemdIntent *self, const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_INTENT (self));
  g_return_if_fail (name);

  if (g_strcmp0 (self->intent_name, name) != 0)
    {
      g_clear_pointer (&self->intent_name, g_free);
      if (name)
        {
          self->intent_name = g_strdup (name);
        }
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_INTENT_NAME]);
    }
}


const gchar *
modulemd_intent_peek_intent_name (ModulemdIntent *self)
{
  g_return_val_if_fail (self, NULL);

  return self->intent_name;
}


gchar *
modulemd_intent_dup_intent_name (ModulemdIntent *self)
{
  g_return_val_if_fail (self, NULL);

  return g_strdup (self->intent_name);
}


void
modulemd_intent_set_default_stream (ModulemdIntent *self, const gchar *stream)
{
  g_return_if_fail (MODULEMD_IS_INTENT (self));

  if (g_strcmp0 (self->default_stream, stream) != 0)
    {
      g_clear_pointer (&self->default_stream, g_free);
      if (stream)
        {
          self->default_stream = g_strdup (stream);
        }
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties[PROP_DEFAULT_STREAM]);
    }
}


const gchar *
modulemd_intent_peek_default_stream (ModulemdIntent *self)
{
  g_return_val_if_fail (self, NULL);

  return self->default_stream;
}


gchar *
modulemd_intent_dup_default_stream (ModulemdIntent *self)
{
  g_return_val_if_fail (self, NULL);

  return g_strdup (self->default_stream);
}


void
modulemd_intent_set_profiles_for_stream (ModulemdIntent *self,
                                         const gchar *stream,
                                         gchar **profiles)
{
  ModulemdSimpleSet *set = NULL;
  g_return_if_fail (MODULEMD_IS_INTENT (self));

  set = modulemd_simpleset_new ();
  modulemd_simpleset_set (set, profiles);

  g_hash_table_replace (self->profile_defaults, g_strdup (stream), set);
  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_PROFILE_DEFAULTS]);
}


void
modulemd_intent_assign_profiles_for_stream (ModulemdIntent *self,
                                            const gchar *stream,
                                            ModulemdSimpleSet *profiles)
{
  ModulemdSimpleSet *set = NULL;
  g_return_if_fail (MODULEMD_IS_INTENT (self));

  modulemd_simpleset_copy (profiles, &set);

  g_hash_table_replace (self->profile_defaults, g_strdup (stream), set);
  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_PROFILE_DEFAULTS]);
}


void
modulemd_intent_set_profile_defaults (ModulemdIntent *self,
                                      GHashTable *profile_defaults)
{
  GHashTableIter iter;
  gpointer key, value;
  ModulemdSimpleSet *set = NULL;

  g_return_if_fail (MODULEMD_IS_INTENT (self));

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
modulemd_intent_dup_profiles_for_stream (ModulemdIntent *self,
                                         const gchar *stream)
{
  ModulemdSimpleSet *set = NULL;
  gchar **profiles = NULL;

  g_return_val_if_fail (MODULEMD_IS_INTENT (self), NULL);

  set = g_hash_table_lookup (self->profile_defaults, stream);

  profiles = modulemd_simpleset_dup (set);
  g_clear_pointer (&set, g_object_unref);

  return profiles;
}


GHashTable *
modulemd_intent_peek_profile_defaults (ModulemdIntent *self)
{
  g_return_val_if_fail (MODULEMD_IS_INTENT (self), NULL);

  return self->profile_defaults;
}


GHashTable *
modulemd_intent_dup_profile_defaults (ModulemdIntent *self)
{
  GHashTableIter iter;
  GHashTable *new;
  gpointer key, value;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (MODULEMD_IS_INTENT (self), NULL);

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


ModulemdIntent *
modulemd_intent_copy (ModulemdIntent *self)
{
  g_autoptr (ModulemdIntent) new_intent = NULL;

  if (!self)
    return NULL;

  g_return_val_if_fail (MODULEMD_IS_INTENT (self), NULL);

  new_intent = modulemd_intent_new (self->intent_name);

  modulemd_intent_set_default_stream (new_intent, self->default_stream);
  modulemd_intent_set_profile_defaults (
    new_intent, modulemd_intent_peek_profile_defaults (self));

  return g_object_ref (new_intent);
}
