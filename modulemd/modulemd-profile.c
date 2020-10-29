/*
 * This file is part of libmodulemd
 * Copyright (C) 2018 Red Hat, Inc.
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
#include <yaml.h>

#include "modulemd-module-stream.h"
#include "modulemd-profile.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-profile-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define P_DEFAULT_STRING "__PROFILE_NAME_UNSET__"

struct _ModulemdProfile
{
  GObject parent_instance;

  gchar *name;
  gchar *description;
  gboolean is_default;

  GHashTable *rpms;

  ModulemdModuleStream *owner;
};

G_DEFINE_TYPE (ModulemdProfile, modulemd_profile, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_NAME,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


gboolean
modulemd_profile_equals_wrapper (const void *a, const void *b)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE ((ModulemdProfile *)a), FALSE);
  g_return_val_if_fail (MODULEMD_IS_PROFILE ((ModulemdProfile *)b), FALSE);

  return modulemd_profile_equals ((ModulemdProfile *)a, (ModulemdProfile *)b);
}


gboolean
modulemd_profile_equals (ModulemdProfile *self_1, ModulemdProfile *self_2)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self_1), FALSE);
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self_2), FALSE);

  if (g_strcmp0 (modulemd_profile_get_name (self_1),
                 modulemd_profile_get_name (self_2)) != 0)
    {
      return FALSE;
    }

  if (g_strcmp0 (modulemd_profile_get_description (self_1, NULL),
                 modulemd_profile_get_description (self_2, NULL)))
    {
      return FALSE;
    }

  //Check rpms: size, set values
  if (!modulemd_hash_table_sets_are_equal (self_1->rpms, self_2->rpms))
    {
      return FALSE;
    }

  /* Test the negations of is_default just in case somehow they are different
   * non-zero values
   */
  if (!self_1->is_default != !self_2->is_default)
    {
      return FALSE;
    }

  return TRUE;
}


ModulemdProfile *
modulemd_profile_new (const gchar *name)
{
  return g_object_new (MODULEMD_TYPE_PROFILE, "name", name, NULL);
}


ModulemdProfile *
modulemd_profile_copy (ModulemdProfile *self)
{
  g_autoptr (ModulemdProfile) p = NULL;
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  p = modulemd_profile_new (modulemd_profile_get_name (self));

  modulemd_profile_set_description (
    p, modulemd_profile_get_description (self, NULL));

  g_hash_table_unref (p->rpms);
  p->rpms = g_hash_table_ref (self->rpms);

  return g_steal_pointer (&p);
}


static void
modulemd_profile_finalize (GObject *object)
{
  ModulemdProfile *self = (ModulemdProfile *)object;

  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->description, g_free);
  g_clear_pointer (&self->rpms, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_profile_parent_class)->finalize (object);
}


static void
modulemd_profile_set_name (ModulemdProfile *self, const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));
  g_return_if_fail (name);
  g_return_if_fail (g_strcmp0 (name, P_DEFAULT_STRING));

  g_clear_pointer (&self->name, g_free);
  self->name = g_strdup (name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}


const gchar *
modulemd_profile_get_name (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  return self->name;
}


void
modulemd_profile_set_description (ModulemdProfile *self,
                                  const gchar *description)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));

  g_clear_pointer (&self->description, g_free);
  self->description = g_strdup (description);
}


const gchar *
modulemd_profile_get_description (ModulemdProfile *self, const gchar *locale)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  /* Retrieve translated strings, if available */
  if (self->owner)
    {
      ModulemdTranslationEntry *entry =
        modulemd_module_stream_get_translation_entry (self->owner, locale);

      if (entry != NULL)
        {
          const gchar *translation =
            modulemd_translation_entry_get_profile_description (entry,
                                                                self->name);

          if (translation != NULL)
            {
              return translation;
            }
        }
    }

  return self->description;
}


void
modulemd_profile_set_default (ModulemdProfile *self)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));
  self->is_default = TRUE;
}

void
modulemd_profile_unset_default (ModulemdProfile *self)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));
  self->is_default = FALSE;
}


gboolean
modulemd_profile_is_default (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), FALSE);

  return self->is_default;
}


void
modulemd_profile_add_rpm (ModulemdProfile *self, const gchar *rpm)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));
  g_hash_table_add (self->rpms, g_strdup (rpm));
}


void
modulemd_profile_remove_rpm (ModulemdProfile *self, const gchar *rpm)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));
  g_hash_table_remove (self->rpms, rpm);
}


void
modulemd_profile_clear_rpms (ModulemdProfile *self)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));
  g_hash_table_remove_all (self->rpms);
}


GStrv
modulemd_profile_get_rpms_as_strv (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpms);
}


void
modulemd_profile_set_owner (ModulemdProfile *self, ModulemdModuleStream *owner)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM (owner));

  self->owner = owner;
}


static void
modulemd_profile_get_property (GObject *object,
                               guint prop_id,
                               GValue *value,
                               GParamSpec *pspec)
{
  ModulemdProfile *self = MODULEMD_PROFILE (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, modulemd_profile_get_name (self));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_profile_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
  ModulemdProfile *self = MODULEMD_PROFILE (object);

  switch (prop_id)
    {
    case PROP_NAME:
      modulemd_profile_set_name (self, g_value_get_string (value));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_profile_class_init (ModulemdProfileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_profile_finalize;
  object_class->get_property = modulemd_profile_get_property;
  object_class->set_property = modulemd_profile_set_property;

  properties[PROP_NAME] = g_param_spec_string (
    "name",
    "Name",
    "The name of this profile.",
    P_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_profile_init (ModulemdProfile *self)
{
  self->rpms = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}


/* === YAML Functions === */

ModulemdProfile *
modulemd_profile_parse_yaml (yaml_parser_t *parser,
                             const gchar *name,
                             gboolean strict,
                             GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean is_default = FALSE;
  g_autofree gchar *value = NULL;
  g_autoptr (ModulemdProfile) p = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (
        error, event, "No map in profile: %s", nested_error->message);
    }

  p = modulemd_profile_new (name);

  /* Read in additional attributes */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          if (g_str_equal (event.data.scalar.value, "rpms"))
            {
              g_hash_table_unref (p->rpms);
              p->rpms = modulemd_yaml_parse_string_set (parser, &nested_error);
              if (p->rpms == NULL)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse rpm list in profile: %s",
                    nested_error->message);
                }
            }

          else if (g_str_equal (event.data.scalar.value, "description"))
            {
              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse description in profile: %s",
                    nested_error->message);
                }
              modulemd_profile_set_description (p, value);
              g_clear_pointer (&value, g_free);
            }

          else if (g_str_equal (event.data.scalar.value, "default"))
            {
              is_default = modulemd_yaml_parse_bool (parser, &nested_error);
              if (nested_error)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }

              if (is_default)
                {
                  modulemd_profile_set_default (p);
                }
              else
                {
                  modulemd_profile_unset_default (p);
                }
            }

          else
            {
              SKIP_UNKNOWN (parser,
                            FALSE,
                            "Unexpected key in profile body: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error, event, "Unexpected YAML event in profile");
          break;
        }

      yaml_event_delete (&event);
    }
  return g_steal_pointer (&p);
}


gboolean
modulemd_profile_emit_yaml (ModulemdProfile *self,
                            yaml_emitter_t *emitter,
                            GError **error)
{
  MODULEMD_INIT_TRACE ();
  int ret;
  g_auto (GStrv) rpms = NULL;
  g_autoptr (GError) nested_error = NULL;
  MMD_INIT_YAML_EVENT (event);

  ret = mmd_emitter_scalar (emitter,
                            modulemd_profile_get_name (self),
                            YAML_PLAIN_SCALAR_STYLE,
                            &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Failed to emit profile name: ");
      return FALSE;
    }

  ret = mmd_emitter_start_mapping (
    emitter, YAML_BLOCK_MAPPING_STYLE, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Failed to start profile mapping: ");
      return FALSE;
    }

  if (modulemd_profile_get_description (self, NULL) != NULL)
    {
      ret = mmd_emitter_scalar (
        emitter, "description", YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Failed to emit profile description key: ");
          return FALSE;
        }

      ret = mmd_emitter_scalar (emitter,
                                modulemd_profile_get_description (self, NULL),
                                YAML_PLAIN_SCALAR_STYLE,
                                &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Failed to emit profile description value: ");
          return FALSE;
        }
    }

  if (g_hash_table_size (self->rpms) != 0)
    {
      ret = mmd_emitter_scalar (
        emitter, "rpms", YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (error,
                                      g_steal_pointer (&nested_error),
                                      "Failed to emit profile rpms key: ");
          return FALSE;
        }

      rpms = modulemd_profile_get_rpms_as_strv (self);

      ret = mmd_emitter_strv (
        emitter, YAML_BLOCK_SEQUENCE_STYLE, rpms, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (error,
                                      g_steal_pointer (&nested_error),
                                      "Failed to emit profile rpms: ");
          return FALSE;
        }
    }

  ret = mmd_emitter_end_mapping (emitter, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Failed to end profile mapping");
      return FALSE;
    }
  return TRUE;
}
