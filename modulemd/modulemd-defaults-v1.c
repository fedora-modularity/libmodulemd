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

#include "modulemd-defaults-v1.h"
#include "modulemd-errors.h"
#include "private/modulemd-defaults-private.h"
#include "private/modulemd-defaults-v1-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include <glib.h>
#include <inttypes.h>

struct _ModulemdDefaultsV1
{
  GObject parent_instance;

  gchar *default_stream;

  /* @key: stream name
   * @value: #GHashTable set of profile names
   */
  GHashTable *profile_defaults;

  /* @key: intent name
   * @value: string stream name
   */
  GHashTable *intent_default_streams;

  /* @key: intent name
   * @value: #GHashTable set of profile names
   */
  GHashTable *intent_default_profiles;
};

G_DEFINE_TYPE (ModulemdDefaultsV1,
               modulemd_defaults_v1,
               MODULEMD_TYPE_DEFAULTS)


static gboolean
modulemd_defaults_v1_equals (ModulemdDefaults *self_1,
                             ModulemdDefaults *self_2)
{
  ModulemdDefaultsV1 *v1_self_1 = NULL;
  ModulemdDefaultsV1 *v1_self_2 = NULL;

  g_return_val_if_fail (MODULEMD_IS_DEFAULTS_V1 (self_1), FALSE);
  v1_self_1 = MODULEMD_DEFAULTS_V1 (self_1);
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS_V1 (self_2), FALSE);
  v1_self_2 = MODULEMD_DEFAULTS_V1 (self_2);

  if (!MODULEMD_DEFAULTS_CLASS (modulemd_defaults_v1_parent_class)
         ->equals (self_1, self_2))
    {
      return FALSE;
    }

  if (g_strcmp0 (v1_self_1->default_stream, v1_self_2->default_stream) != 0)
    {
      return FALSE;
    }

  /*Check profile_defaults: size, keys, values*/
  if (!modulemd_hash_table_equals (v1_self_1->profile_defaults,
                                   v1_self_2->profile_defaults,
                                   modulemd_hash_table_sets_are_equal_wrapper))
    {
      return FALSE;
    }

  /*Check intent_default_streams: size, keys, values*/
  if (!modulemd_hash_table_equals (v1_self_1->intent_default_streams,
                                   v1_self_2->intent_default_streams,
                                   g_str_equal))
    {
      return FALSE;
    }

  /*Check intent_default_profiles: size, keys, values*/
  if (!modulemd_hash_table_equals (v1_self_1->intent_default_profiles,
                                   v1_self_2->intent_default_profiles,
                                   modulemd_hash_table_sets_are_equal_wrapper))
    {
      return FALSE;
    }

  return TRUE;
}


ModulemdDefaultsV1 *
modulemd_defaults_v1_new (const gchar *module_name)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_DEFAULTS_V1,
                       "module-name", module_name,
                       NULL);
  // clang-format on
}


static void
modulemd_defaults_v1_finalize (GObject *object)
{
  ModulemdDefaultsV1 *self = (ModulemdDefaultsV1 *)object;

  g_clear_pointer (&self->default_stream, g_free);
  g_clear_pointer (&self->profile_defaults, g_hash_table_unref);
  g_clear_pointer (&self->intent_default_streams, g_hash_table_unref);
  g_clear_pointer (&self->intent_default_profiles, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_defaults_v1_parent_class)->finalize (object);
}


static ModulemdDefaults *
modulemd_defaults_v1_copy (ModulemdDefaults *self)
{
  ModulemdDefaultsV1 *v1_self = NULL;
  g_autoptr (ModulemdDefaultsV1) copy = NULL;
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS_V1 (self), NULL);
  v1_self = MODULEMD_DEFAULTS_V1 (self);

  copy = MODULEMD_DEFAULTS_V1 (
    MODULEMD_DEFAULTS_CLASS (modulemd_defaults_v1_parent_class)->copy (self));

  /* Copy the default stream */
  modulemd_defaults_v1_set_default_stream (
    copy, modulemd_defaults_v1_get_default_stream (v1_self, NULL), NULL);

  /* Copy the profile defaults table */
  g_clear_pointer (&copy->profile_defaults, g_hash_table_unref);
  copy->profile_defaults =
    modulemd_hash_table_deep_str_set_copy (v1_self->profile_defaults);

  /* Copy intent default_stream table */
  g_clear_pointer (&copy->intent_default_streams, g_hash_table_unref);
  copy->intent_default_streams =
    modulemd_hash_table_deep_str_copy (v1_self->intent_default_streams);

  /* Copy intent profile_defaults table */
  g_clear_pointer (&copy->intent_default_profiles, g_hash_table_unref);
  copy->intent_default_profiles = modulemd_hash_table_deep_str_str_set_copy (
    v1_self->intent_default_profiles);

  return MODULEMD_DEFAULTS (g_steal_pointer (&copy));
}


static gboolean
modulemd_defaults_v1_validate (ModulemdDefaults *self, GError **error)
{
  gboolean result = FALSE;
  g_autoptr (GError) nested_error = NULL;

  result = MODULEMD_DEFAULTS_CLASS (modulemd_defaults_v1_parent_class)
             ->validate (self, &nested_error);
  if (!result)
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
    }

  return TRUE;
}


static guint64
modulemd_defaults_v1_get_mdversion (ModulemdDefaults *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), 0);

  return MD_DEFAULTS_VERSION_ONE;
}


void
modulemd_defaults_v1_set_default_stream (ModulemdDefaultsV1 *self,
                                         const gchar *default_stream,
                                         const gchar *intent)
{
  g_return_if_fail (MODULEMD_IS_DEFAULTS_V1 (self));

  if (default_stream)
    {
      if (intent)
        {
          /* If this is an intent-specific default stream, add it to the
           * table.
           */
          g_hash_table_replace (self->intent_default_streams,
                                g_strdup (intent),
                                g_strdup (default_stream));
        }
      else
        {
          /* This is the fallback default for non-specific intents */
          g_clear_pointer (&self->default_stream, g_free);
          self->default_stream = g_strdup (default_stream);
        }
    }
  else
    {
      if (intent)
        {
          /* Remove the intent-specific default stream */
          g_hash_table_remove (self->intent_default_streams, intent);
        }
      else
        {
          /* Remove the fallback default stream */
          g_clear_pointer (&self->default_stream, g_free);
        }
    }
}


const gchar *
modulemd_defaults_v1_get_default_stream (ModulemdDefaultsV1 *self,
                                         const gchar *intent)
{
  const gchar *default_stream = NULL;
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS_V1 (self), NULL);

  if (!intent)
    {
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

  default_stream = g_hash_table_lookup (self->intent_default_streams, intent);
  if (default_stream)
    {
      if (default_stream[0] == '\0')
        {
          /* If the intent is zero-length, it means explicitly no default
           * stream, so return NULL here
           */
          return NULL;
        }
      return default_stream;
    }

  /* No intent-specific default. Return the fallback defaults */
  return self->default_stream;
}


GStrv
modulemd_defaults_v1_get_streams_with_default_profiles_as_strv (
  ModulemdDefaultsV1 *self, const gchar *intent)
{
  GHashTable *profile_set = NULL;

  g_return_val_if_fail (MODULEMD_IS_DEFAULTS_V1 (self), NULL);

  if (!intent)
    {
      return modulemd_ordered_str_keys_as_strv (self->profile_defaults);
    }

  profile_set = g_hash_table_lookup (self->intent_default_profiles, intent);
  if (profile_set)
    {
      return modulemd_ordered_str_keys_as_strv (profile_set);
    }

  /* No intent-specific default. Return the fallback defaults */
  return modulemd_ordered_str_keys_as_strv (self->profile_defaults);
}


static GHashTable *
modulemd_defaults_v1_get_or_create_profile_table (ModulemdDefaultsV1 *self,
                                                  const gchar *intent)
{
  GHashTable *profile_table = NULL;
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS_V1 (self), NULL);

  if (intent)
    {
      /* Get or create the profile table for this intent */
      profile_table =
        g_hash_table_lookup (self->intent_default_profiles, intent);
      if (!profile_table)
        {
          /* This table didn't exist yet, so create it */
          profile_table = g_hash_table_new_full (
            g_str_hash, g_str_equal, g_free, modulemd_hash_table_unref);

          /* Add the new table back to the intent table */
          g_hash_table_replace (
            self->intent_default_profiles, g_strdup (intent), profile_table);
        }
    }
  else
    {
      /* These are the fallback defaults */
      profile_table = self->profile_defaults;
    }

  return profile_table;
}


static void
modulemd_defaults_v1_add_or_clear_default_profile_for_stream (
  ModulemdDefaultsV1 *self,
  const gchar *stream_name,
  const gchar *profile_name,
  const gchar *intent)
{
  g_autoptr (GHashTable) profile_table = NULL;
  g_autoptr (GHashTable) profiles = NULL;
  g_return_if_fail (MODULEMD_IS_DEFAULTS_V1 (self));
  g_return_if_fail (stream_name);


  profile_table = g_hash_table_ref (
    modulemd_defaults_v1_get_or_create_profile_table (self, intent));

  /* Get a reference to the profile set within the table */
  profiles = g_hash_table_lookup (profile_table, stream_name);

  if (profiles)
    {
      g_hash_table_ref (profiles);
    }
  else
    {
      /* A profile set for this stream doesn't exist yet. Create it. */
      profiles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

      /* Add the new profile set back to the profile table */
      g_hash_table_replace (
        profile_table, g_strdup (stream_name), g_hash_table_ref (profiles));
    }

  if (profile_name)
    {
      /* Add a new profile name for this stream. Since we're operating on a
       * reference to the internal value, we don't need to explicitly save this
       * back
       */
      g_hash_table_add (profiles, g_strdup (profile_name));
    }
  else
    {
      /* No profile name was provided, so turn this to the empty set */
      g_hash_table_remove_all (profiles);
    }
}


void
modulemd_defaults_v1_add_default_profile_for_stream (ModulemdDefaultsV1 *self,
                                                     const gchar *stream_name,
                                                     const gchar *profile_name,
                                                     const gchar *intent)
{
  g_return_if_fail (MODULEMD_IS_DEFAULTS_V1 (self));
  g_return_if_fail (stream_name);
  g_return_if_fail (profile_name);

  return modulemd_defaults_v1_add_or_clear_default_profile_for_stream (
    self, stream_name, profile_name, intent);
}


void
modulemd_defaults_v1_set_empty_default_profiles_for_stream (
  ModulemdDefaultsV1 *self, const gchar *stream_name, const gchar *intent)
{
  g_return_if_fail (MODULEMD_IS_DEFAULTS_V1 (self));
  g_return_if_fail (stream_name);

  return modulemd_defaults_v1_add_or_clear_default_profile_for_stream (
    self, stream_name, NULL, intent);
}


void
modulemd_defaults_v1_remove_default_profiles_for_stream (
  ModulemdDefaultsV1 *self, const gchar *stream_name, const gchar *intent)
{
  g_autoptr (GHashTable) profile_table = NULL;

  g_return_if_fail (MODULEMD_IS_DEFAULTS_V1 (self));
  g_return_if_fail (stream_name);

  profile_table = g_hash_table_ref (
    modulemd_defaults_v1_get_or_create_profile_table (self, intent));

  g_hash_table_remove (profile_table, stream_name);
}


GStrv
modulemd_defaults_v1_get_default_profiles_for_stream_as_strv (
  ModulemdDefaultsV1 *self, const gchar *stream_name, const gchar *intent)
{
  GHashTable *profile_table = NULL;
  GHashTable *profiles = NULL;
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS_V1 (self), NULL);
  g_return_val_if_fail (stream_name, NULL);


  if (intent)
    {
      /* Get or create the profile table for this intent */
      profile_table =
        g_hash_table_lookup (self->intent_default_profiles, intent);
      if (profile_table)
        {
          /* See if this stream name appears in the profile defaults
           * for this intent
           */
          profiles = g_hash_table_lookup (profile_table, stream_name);
        }

      if (profiles)
        {
          return modulemd_ordered_str_keys_as_strv (profiles);
        }

      /* We didn't find this profile in the intents, try the fallback
       * defaults
       */
    }


  /* These are the fallback defaults */
  profiles = g_hash_table_lookup (self->profile_defaults, stream_name);
  if (!profiles)
    {
      return NULL;
    }

  return modulemd_ordered_str_keys_as_strv (profiles);
}


static void
modulemd_defaults_v1_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *UNUSED (value),
                                   GParamSpec *pspec)
{
  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_defaults_v1_set_property (GObject *object,
                                   guint prop_id,
                                   const GValue *UNUSED (value),
                                   GParamSpec *pspec)
{
  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_defaults_v1_class_init (ModulemdDefaultsV1Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdDefaultsClass *defaults_class =
    MODULEMD_DEFAULTS_CLASS (object_class);

  object_class->finalize = modulemd_defaults_v1_finalize;
  object_class->get_property = modulemd_defaults_v1_get_property;
  object_class->set_property = modulemd_defaults_v1_set_property;

  defaults_class->copy = modulemd_defaults_v1_copy;
  defaults_class->get_mdversion = modulemd_defaults_v1_get_mdversion;
  defaults_class->validate = modulemd_defaults_v1_validate;
  defaults_class->equals = modulemd_defaults_v1_equals;
}


static void
modulemd_defaults_v1_init (ModulemdDefaultsV1 *self)
{
  self->profile_defaults = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_hash_table_unref);

  self->intent_default_streams =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  self->intent_default_profiles = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_hash_table_unref);
}


static gboolean
modulemd_defaults_v1_parse_yaml_profiles (yaml_parser_t *parser,
                                          GHashTable *profile_defaults,
                                          GError **error);

static gboolean
modulemd_defaults_v1_parse_intents (yaml_parser_t *parser,
                                    ModulemdDefaultsV1 *defaults,
                                    gboolean strict,
                                    GError **error);

static gboolean
modulemd_defaults_v1_parse_intent (yaml_parser_t *parser,
                                   gboolean strict,
                                   gchar **_default_stream,
                                   GHashTable **_profile_defaults,
                                   GError **error);

ModulemdDefaultsV1 *
modulemd_defaults_v1_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                 gboolean strict,
                                 GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (GError) nested_error = FALSE;
  ModulemdDefaultsV1 *defaults = NULL;
  gboolean done = FALSE;
  g_autofree gchar *scalar = NULL;
  guint64 modified;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!modulemd_subdocument_info_get_data_parser (
        subdoc, &parser, strict, error))
    {
      g_debug ("get_data_parser() failed: %s", (*error)->message);
      return NULL;
    }

  /* Create a module with a placeholder name. We'll verify that this has been
   * changed before we return it. This is because we can't guarantee that we
   * will get the module name from the YAML before reading any of the other
   * data, but it's easier to process the rest of the contents with the
   * constructed object.
   */
  defaults = modulemd_defaults_v1_new (DEFAULT_PLACEHOLDER);

  YAML_PARSER_PARSE_WITH_EXIT (&parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (
        error, event, "Missing mapping in defaults data entry");
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (&parser, &event, error);
      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          if (g_str_equal (event.data.scalar.value, "module"))
            {
              if (!g_str_equal (modulemd_defaults_get_module_name (
                                  MODULEMD_DEFAULTS (defaults)),
                                DEFAULT_PLACEHOLDER))
                {
                  /* The module name was set earlier call, which means it is
                   * not expected here
                   */
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error, event, "Module name encountered twice.");
                }

              scalar = modulemd_yaml_parse_string (&parser, &nested_error);
              if (!scalar)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse module name in default data: %s",
                    nested_error->message);
                }

              /* Use a private internal function to set the module_name.
               * External consumers should never be allowed to change this
               * value, but we need to be able to modify the placeholder.
               */
              modulemd_defaults_set_module_name (MODULEMD_DEFAULTS (defaults),
                                                 scalar);
              g_clear_pointer (&scalar, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "modified"))
            {
              modified = modulemd_yaml_parse_uint64 (&parser, &nested_error);
              if (nested_error)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse modified in defaults data: %s",
                    nested_error->message);
                }

              modulemd_defaults_set_modified (MODULEMD_DEFAULTS (defaults),
                                              modified);
            }

          else if (g_str_equal (event.data.scalar.value, "stream"))
            {
              if (modulemd_defaults_v1_get_default_stream (defaults, NULL))
                {
                  /* We already have a default stream. It should not appear
                   * twice in the same document.
                   */
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error, event, "Default stream encountered twice.");
                }

              scalar = modulemd_yaml_parse_string (&parser, &nested_error);
              if (!scalar)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse default stream in default data: %s",
                    nested_error->message);
                }
              modulemd_defaults_v1_set_default_stream (defaults, scalar, NULL);
              g_clear_pointer (&scalar, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "profiles"))
            {
              if (!modulemd_defaults_v1_parse_yaml_profiles (
                    &parser, defaults->profile_defaults, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }
          else if (g_str_equal (event.data.scalar.value, "intents"))
            {
              if (!modulemd_defaults_v1_parse_intents (
                    &parser, defaults, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }
          else
            {
              SKIP_UNKNOWN (&parser,
                            NULL,
                            "Unexpected key in defaults data: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event %s in defaults data",
            mmd_yaml_get_event_name (event.type));
          break;
        }

      yaml_event_delete (&event);
    }

  if (!modulemd_defaults_validate (MODULEMD_DEFAULTS (defaults),
                                   &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

  return g_steal_pointer (&defaults);
}


static gboolean
modulemd_defaults_v1_parse_yaml_profiles (yaml_parser_t *parser,
                                          GHashTable *profile_defaults,
                                          GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (GError) nested_error = NULL;
  g_autofree gchar *stream_name = NULL;
  g_autoptr (GHashTable) profile_set = NULL;
  gboolean done = FALSE;

  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Missing mapping in defaults data entry");
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:

          stream_name = g_strdup ((const gchar *)event.data.scalar.value);
          if (!stream_name)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error,
                event,
                "Failed to parse stream name in profile defaults");
            }

          /* Check to see if we've encountered this stream name previously */
          if (g_hash_table_contains (profile_defaults, stream_name))
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error,
                event,
                "Encountered stream name %s more than once in profile "
                "defaults",
                stream_name);
            }

          profile_set = modulemd_yaml_parse_string_set (parser, &nested_error);
          if (!profile_set)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error,
                event,
                "Failed to parse profile_set in profile defaults for %s: %s",
                stream_name,
                nested_error->message);
            }

          g_hash_table_replace (profile_defaults,
                                g_strdup (stream_name),
                                g_steal_pointer (&profile_set));
          g_clear_pointer (&stream_name, g_free);
          break;


        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event %s in profile defaults.",
            mmd_yaml_get_event_name (event.type));
          break;
        }

      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_defaults_v1_parse_intents (yaml_parser_t *parser,
                                    ModulemdDefaultsV1 *defaults,
                                    gboolean strict,
                                    GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (GError) nested_error = NULL;
  g_autofree gchar *intent_name = NULL;
  g_autofree gchar *default_stream = NULL;
  g_autoptr (GHashTable) profile_set = NULL;
  gboolean done = FALSE;

  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Missing mapping in intents");
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          intent_name = g_strdup ((const gchar *)event.data.scalar.value);
          if (!intent_name)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error, event, "Failed to parse intent name in defaults");
            }

          /* Check to see if we've encountered this intent name previously */
          if (g_hash_table_contains (defaults->intent_default_streams,
                                     intent_name) ||
              g_hash_table_contains (defaults->intent_default_profiles,
                                     intent_name))
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error,
                event,
                "Encountered intent name %s more than once in defaults",
                intent_name);
            }

          if (!modulemd_defaults_v1_parse_intent (
                parser, strict, &default_stream, &profile_set, &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }

          g_hash_table_replace (defaults->intent_default_streams,
                                g_strdup (intent_name),
                                g_strdup (default_stream));
          g_hash_table_replace (defaults->intent_default_profiles,
                                g_strdup (intent_name),
                                g_hash_table_ref (profile_set));
          g_clear_pointer (&default_stream, g_free);
          g_clear_pointer (&profile_set, g_hash_table_unref);
          g_clear_pointer (&intent_name, g_free);
          break;


        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in intents.");
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_defaults_v1_parse_intent (yaml_parser_t *parser,
                                   gboolean strict,
                                   gchar **_default_stream,
                                   GHashTable **_profile_defaults,
                                   GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  g_autofree gchar *default_stream = NULL;
  g_autoptr (GHashTable) profile_defaults = NULL;
  g_autoptr (GError) nested_error = FALSE;
  gboolean done = FALSE;
  gboolean in_map = FALSE;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  profile_defaults = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_hash_table_unref);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT: in_map = TRUE; break;

        case YAML_MAPPING_END_EVENT:
          in_map = FALSE;
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error, event, "Missing mapping in intent data");
            }
          if (g_str_equal (event.data.scalar.value, "stream"))
            {
              if (default_stream)
                {
                  /* We already have a default stream. It should not appear
                   * twice in the same document.
                   */
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error, event, "Default stream encountered twice.");
                }

              default_stream =
                modulemd_yaml_parse_string (parser, &nested_error);
              if (!default_stream)
                {
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error,
                    event,
                    "Failed to parse default stream in intent data: %s",
                    nested_error->message);
                }
            }
          else if (g_str_equal (event.data.scalar.value, "profiles"))
            {
              if (!modulemd_defaults_v1_parse_yaml_profiles (
                    parser, profile_defaults, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }
          else
            {
              SKIP_UNKNOWN (parser,
                            FALSE,
                            "Unexpected key in intent data: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in intent data");
          break;
        }

      yaml_event_delete (&event);
    }

  *_default_stream = g_steal_pointer (&default_stream);
  *_profile_defaults = g_steal_pointer (&profile_defaults);

  return TRUE;
}


static gboolean
modulemd_defaults_v1_emit_profiles (GHashTable *profile_table,
                                    yaml_emitter_t *emitter,
                                    GError **error);

static gboolean
modulemd_defaults_v1_emit_intents (ModulemdDefaultsV1 *self,
                                   yaml_emitter_t *emitter,
                                   GError **error);


gboolean
modulemd_defaults_v1_emit_yaml (ModulemdDefaultsV1 *self,
                                yaml_emitter_t *emitter,
                                GError **error)
{
  MODULEMD_INIT_TRACE ();
  g_autoptr (GError) nested_error = NULL;
  guint64 modified;
  g_autofree gchar *modified_string = NULL;

  if (!modulemd_defaults_validate (MODULEMD_DEFAULTS (self), &nested_error))
    {
      /* Validation failed */
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Defaults object failed validation: ");
      return FALSE;
    }

  /* First emit the standard document headers */
  if (!modulemd_yaml_emit_document_headers (
        emitter,
        MODULEMD_YAML_DOC_DEFAULTS,
        modulemd_defaults_get_mdversion (MODULEMD_DEFAULTS (self)),
        error))
    {
      return FALSE;
    }

  /* Start the data: section mapping */
  if (!mmd_emitter_start_mapping (emitter, YAML_BLOCK_MAPPING_STYLE, error))
    {
      return FALSE;
    }

  /* Fill in the default data */

  /* The module name is mandatory */
  if (!mmd_emitter_scalar (emitter, "module", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (
        emitter,
        modulemd_defaults_get_module_name (MODULEMD_DEFAULTS (self)),
        YAML_PLAIN_SCALAR_STYLE,
        error))
    {
      return FALSE;
    }

  /* The modified field is optional */
  modified = modulemd_defaults_get_modified (MODULEMD_DEFAULTS (self));
  if (modified)
    {
      modified_string = g_strdup_printf ("%" PRIu64, modified);
      EMIT_KEY_VALUE (emitter, error, "modified", modified_string);
    }

  /* The default stream is optional
   * Always emit the stream quoted, since a purely numeric-looking stream such
   * as 5.30 might otherwise be interpreted by parsers like pyyaml as a number
   * and result in being read (and written) as '5.3'.
   */

  if (modulemd_defaults_v1_get_default_stream (self, NULL) != NULL)
    {
      EMIT_KEY_VALUE_FULL (
        emitter,
        error,
        "stream",
        modulemd_defaults_v1_get_default_stream (self, NULL),
        YAML_DOUBLE_QUOTED_SCALAR_STYLE);
    }

  /* Profiles are optional */
  if (g_hash_table_size (self->profile_defaults) > 0)
    {
      if (!modulemd_defaults_v1_emit_profiles (
            self->profile_defaults, emitter, error))
        {
          return FALSE;
        }
    }

  /* Intents are optional */
  if (g_hash_table_size (self->intent_default_streams) > 0 ||
      g_hash_table_size (self->intent_default_profiles) > 0)
    {
      modulemd_defaults_v1_emit_intents (self, emitter, &nested_error);
    }

  /* Close the data: section mapping */
  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }

  /* Close the top-level section mapping */
  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }

  /* End the document */
  if (!mmd_emitter_end_document (emitter, error))
    {
      return FALSE;
    }

  return TRUE;
}

static gboolean
modulemd_defaults_v1_emit_profiles (GHashTable *profile_table,
                                    yaml_emitter_t *emitter,
                                    GError **error)
{
  g_autoptr (GPtrArray) stream_names = NULL;
  g_auto (GStrv) streams = NULL;
  gchar *stream_name = NULL;
  GHashTable *profile_set = NULL;

  /* Start the "profiles:" section */
  if (!mmd_emitter_scalar (
        emitter, "profiles", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  /* Start the mapping for "profiles:" */
  if (!mmd_emitter_start_mapping (emitter, YAML_BLOCK_MAPPING_STYLE, error))
    {
      return FALSE;
    }


  stream_names =
    modulemd_ordered_str_keys (profile_table, modulemd_strcmp_sort);
  for (guint i = 0; i < stream_names->len; i++)
    {
      stream_name = g_ptr_array_index (stream_names, i);
      profile_set = g_hash_table_lookup (profile_table, stream_name);
      if (!profile_set)
        {
          /* This should be impossible. Warn and continue */
          g_warning ("Encountered NULL profile set for stream %s",
                     stream_name);
          continue;
        }

      if (!mmd_emitter_scalar (
            emitter, stream_name, YAML_PLAIN_SCALAR_STYLE, error))
        {
          return FALSE;
        }

      streams = modulemd_ordered_str_keys_as_strv (profile_set);
      if (!mmd_emitter_strv (
            emitter, YAML_FLOW_SEQUENCE_STYLE, streams, error))
        {
          return FALSE;
        }
      g_clear_pointer (&streams, g_strfreev);
    }

  /* End the mapping for "profiles:" */
  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }

  return TRUE;
}


static gboolean
modulemd_defaults_v1_emit_intents (ModulemdDefaultsV1 *self,
                                   yaml_emitter_t *emitter,
                                   GError **error)
{
  g_autoptr (GHashTable) intent_names = NULL;
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  g_autoptr (GPtrArray) intents = NULL;
  gchar *intent = NULL;
  gchar *intent_default_stream = NULL;
  GHashTable *intent_default_profiles = NULL;


  /* Emit the section name */
  if (!mmd_emitter_scalar (emitter, "intents", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  /* Start the mapping for "intents:" */
  if (!mmd_emitter_start_mapping (emitter, YAML_BLOCK_MAPPING_STYLE, error))
    {
      return FALSE;
    }

  /* Get the union of the keys from intents_stream_defaults and
   * intents_profile_defaults
   */
  intent_names = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  g_hash_table_iter_init (&iter, self->intent_default_streams);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      /* Don't bother copying the key; we're only using this as an index */
      g_hash_table_add (intent_names, key);
    }
  g_hash_table_iter_init (&iter, self->intent_default_profiles);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      /* Don't bother copying the key; we're only using this as an index */
      g_hash_table_add (intent_names, key);
    }

  intents = modulemd_ordered_str_keys (intent_names, modulemd_strcmp_sort);
  g_clear_pointer (&intent_names, g_hash_table_unref);

  for (guint i = 0; i < intents->len; i++)
    {
      intent = g_ptr_array_index (intents, i);

      /* Emit the intent name */
      if (!mmd_emitter_scalar (
            emitter, intent, YAML_PLAIN_SCALAR_STYLE, error))
        {
          return FALSE;
        }

      /* Start the mapping for this intent */
      if (!mmd_emitter_start_mapping (
            emitter, YAML_BLOCK_MAPPING_STYLE, error))
        {
          return FALSE;
        }

      /* The default stream is optional
       * Always emit the stream quoted, since a purely numeric-looking stream
       * such as 5.30 might otherwise be interpreted by parsers like pyyaml as
       * a number and result in being read (and written) as '5.3'.
       */
      intent_default_stream =
        g_hash_table_lookup (self->intent_default_streams, intent);

      if (intent_default_stream)
        {
          EMIT_KEY_VALUE_FULL (emitter,
                               error,
                               "stream",
                               intent_default_stream,
                               YAML_DOUBLE_QUOTED_SCALAR_STYLE);

          intent_default_profiles =
            g_hash_table_lookup (self->intent_default_profiles, intent);
        }
      if (intent_default_profiles)
        {
          if (!modulemd_defaults_v1_emit_profiles (
                intent_default_profiles, emitter, error))
            {
              return FALSE;
            }
        }

      /* End the mapping for this intent */
      if (!mmd_emitter_end_mapping (emitter, error))
        {
          return FALSE;
        }
    }

  /* End the mapping for "intents:" */
  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }

  return TRUE;
}

static gboolean
modulemd_defaults_v1_merge_default_profiles (
  GHashTable *from_profile_defaults,
  GHashTable *merged_profile_defaults,
  guint64 from_modified,
  guint64 into_modified,
  GError **error);
static GHashTable *
modulemd_defaults_v1_copy_intent_profiles (GHashTable *intent_profiles);


ModulemdDefaults *
modulemd_defaults_v1_merge (ModulemdDefaultsV1 *from,
                            ModulemdDefaultsV1 *into,
                            gboolean strict_default_streams,
                            GError **error)
{
  g_autoptr (ModulemdDefaultsV1) merged = NULL;
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  GHashTable *intent_profiles = NULL;
  GHashTable *merged_intent_profiles = NULL;
  guint64 from_modified;
  guint64 into_modified;
  g_autoptr (GHashTable) intent_profile_defaults = NULL;
  gchar *intent_name = NULL;
  gchar *intent_default_stream = NULL;
  gchar *merged_default_stream = NULL;
  g_autoptr (GError) nested_error = NULL;
  const gchar *module_name =
    modulemd_defaults_get_module_name (MODULEMD_DEFAULTS (into));

  from_modified = modulemd_defaults_get_modified (MODULEMD_DEFAULTS (from));
  into_modified = modulemd_defaults_get_modified (MODULEMD_DEFAULTS (into));

  /* Start from a copy of "into" */
  merged =
    MODULEMD_DEFAULTS_V1 (modulemd_defaults_copy (MODULEMD_DEFAULTS (into)));

  /* Merge the default streams */
  if (from->default_stream && !merged->default_stream)
    {
      modulemd_defaults_v1_set_default_stream (
        merged, from->default_stream, NULL);
    }
  else if (merged->default_stream && from->default_stream)
    {
      if (g_str_equal (merged->default_stream, DEFAULT_MERGE_CONFLICT))
        {
          /* A previous pass over this same module encountered a merge
           * conflict, so keep it.
           */
        }
      else if (!g_str_equal (merged->default_stream, from->default_stream))
        {
          if (from_modified > into_modified)
            {
              modulemd_defaults_v1_set_default_stream (
                merged, from->default_stream, NULL);
            }
          else if (from_modified == into_modified)
            {
              /* They have conflicting default streams */
              g_info ("Module stream mismatch in merge: %s != %s",
                      into->default_stream,
                      from->default_stream);
              if (strict_default_streams)
                {
                  g_set_error (
                    error,
                    MODULEMD_ERROR,
                    MMD_ERROR_VALIDATE,
                    "Default stream mismatch in module %s: %s != %s",
                    module_name,
                    into->default_stream,
                    from->default_stream);
                  return NULL;
                }
              modulemd_defaults_v1_set_default_stream (
                merged, DEFAULT_MERGE_CONFLICT, NULL);
            }
        }
      else
        {
          /* They're the same, so change nothing */
        }
    }
  else
    {
      /* The 'from' default stream was NULL. Make no changes. */
    }


  /* == Merge profile defaults == */

  /* Iterate through 'from' and see if there are additions or conflicts */
  if (!modulemd_defaults_v1_merge_default_profiles (from->profile_defaults,
                                                    merged->profile_defaults,
                                                    from_modified,
                                                    into_modified,
                                                    &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

  /* == Merge intent defaults == */

  /* Merge intent default stream values */

  /* Iterate through 'from', adding any new values and checking the existing
   * ones for equivalence.
   */
  g_hash_table_iter_init (&iter, from->intent_default_streams);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      intent_name = (gchar *)key;
      intent_default_stream = (gchar *)value;
      merged_default_stream =
        g_hash_table_lookup (merged->intent_default_streams, intent_name);

      /* If there is no new default stream, just jump to the next item */
      if (!intent_default_stream)
        {
          continue;
        }

      if (!merged_default_stream)
        {
          /* New entry, just add it */
          g_hash_table_insert (merged->intent_default_streams,
                               g_strdup (intent_name),
                               g_strdup (intent_default_stream));
        }

      else if (!g_str_equal (intent_default_stream, merged_default_stream))
        {
          if (from_modified > into_modified)
            {
              g_hash_table_replace (merged->intent_default_streams,
                                    g_strdup (intent_name),
                                    g_strdup (intent_default_stream));
            }
          else if (into_modified == from_modified)
            {
              g_set_error (
                error,
                MODULEMD_ERROR,
                MMD_ERROR_VALIDATE,
                "Profile default stream mismatch in intents: %s != %s",
                intent_default_stream,
                merged_default_stream);
              return NULL;
            }
        }
    }


  /* Merge intent default profile values */

  /* Now copy 'from' into merged, checking for conflicts */
  g_hash_table_iter_init (&iter, from->intent_default_profiles);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      intent_name = (gchar *)key;
      intent_profiles = (GHashTable *)value;
      merged_intent_profiles =
        g_hash_table_lookup (merged->intent_default_profiles, intent_name);

      if (!merged_intent_profiles)
        {
          intent_profile_defaults =
            modulemd_defaults_v1_copy_intent_profiles (intent_profiles);

          /* This wasn't in 'merged', so just add it */
          g_hash_table_insert (merged->intent_default_profiles,
                               g_strdup (intent_name),
                               g_steal_pointer (&intent_profile_defaults));
          continue;
        }

      /* Go through each of the profile defaults and see if they're additive or
       * conflicting
       */
      if (!modulemd_defaults_v1_merge_default_profiles (intent_profiles,
                                                        merged_intent_profiles,
                                                        from_modified,
                                                        into_modified,
                                                        &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return NULL;
        }
    }

  /* Set the modified value to the higher of the two provided */
  if (from_modified > into_modified)
    {
      modulemd_defaults_set_modified (MODULEMD_DEFAULTS (merged),
                                      from_modified);
    }

  return MODULEMD_DEFAULTS (g_steal_pointer (&merged));
}


static GHashTable *
modulemd_defaults_v1_copy_intent_profiles (GHashTable *intent_profiles)
{
  g_autoptr (GHashTable) intent_profile_defaults = NULL;
  gchar *stream_name = NULL;
  GHashTable *profile_defaults = NULL;
  GHashTableIter iter;
  gpointer key;
  gpointer value;


  intent_profile_defaults = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_hash_table_unref);

  g_hash_table_iter_init (&iter, intent_profiles);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      stream_name = (gchar *)key;
      profile_defaults = (GHashTable *)value;

      /* Copy the profile defaults */
      g_hash_table_insert (
        intent_profile_defaults,
        g_strdup (stream_name),
        modulemd_hash_table_deep_set_copy (profile_defaults));
    }

  return g_steal_pointer (&intent_profile_defaults);
}

static gboolean
modulemd_defaults_v1_merge_default_profiles (
  GHashTable *from_profile_defaults,
  GHashTable *merged_profile_defaults,
  guint64 from_modified,
  guint64 into_modified,
  GError **error)
{
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  gchar *stream_name = NULL;
  GHashTable *from_profiles = NULL;
  GHashTable *merged_profiles = NULL;

  g_hash_table_iter_init (&iter, from_profile_defaults);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      stream_name = (gchar *)key;
      from_profiles = (GHashTable *)value;
      merged_profiles =
        g_hash_table_lookup (merged_profile_defaults, stream_name);

      if (!merged_profiles)
        {
          /* Didn't appear in the profiles list, so just add it to merged */
          g_hash_table_insert (
            merged_profile_defaults,
            g_strdup (stream_name),
            modulemd_hash_table_deep_set_copy (from_profiles));
          continue;
        }

      /* Check to see if they match */
      if (!modulemd_hash_table_sets_are_equal (from_profiles, merged_profiles))
        {
          if (from_modified > into_modified)
            {
              g_hash_table_replace (
                merged_profile_defaults,
                g_strdup (stream_name),
                modulemd_hash_table_deep_set_copy (from_profiles));
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
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Profile default mismatch in stream: %s",
                           stream_name);
              return FALSE;
            }
        }

      /* They were a complete match, so no need to add it a second time */
    }

  return TRUE;
}
