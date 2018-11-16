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

#include "modulemd-translation-entry.h"
#include "private/glib-extensions.h"
#include "private/modulemd-translation-entry-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define TE_DEFAULT_STRING "__LOCALE_UNSET__"

struct _ModulemdTranslationEntry
{
  GObject parent_instance;

  gchar *locale;
  gchar *summary;
  gchar *description;

  GHashTable *profile_descriptions;
};

G_DEFINE_TYPE (ModulemdTranslationEntry,
               modulemd_translation_entry,
               G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_LOCALE,
  PROP_SUMMARY,
  PROP_DESCRIPTION,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdTranslationEntry *
modulemd_translation_entry_new (const gchar *locale)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY,
                       "locale", locale,
                       NULL);
  // clang-format on
}


ModulemdTranslationEntry *
modulemd_translation_entry_copy (ModulemdTranslationEntry *self)
{
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  te = modulemd_translation_entry_new (
    modulemd_translation_entry_get_locale (self));

  modulemd_translation_entry_set_summary (
    te, modulemd_translation_entry_get_summary (self));
  modulemd_translation_entry_set_description (
    te, modulemd_translation_entry_get_description (self));

  g_hash_table_unref (te->profile_descriptions);
  te->profile_descriptions =
    modulemd_hash_table_deep_str_copy (self->profile_descriptions);

  return g_object_ref (te);
}


static void
modulemd_translation_entry_finalize (GObject *object)
{
  ModulemdTranslationEntry *self = (ModulemdTranslationEntry *)object;

  g_clear_pointer (&self->locale, g_free);
  g_clear_pointer (&self->summary, g_free);
  g_clear_pointer (&self->description, g_free);
  g_clear_pointer (&self->profile_descriptions, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_translation_entry_parent_class)->finalize (object);
}


void
modulemd_translation_entry_set_summary (ModulemdTranslationEntry *self,
                                        const gchar *summary)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self));

  g_clear_pointer (&self->summary, g_free);
  self->summary = g_strdup (summary);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SUMMARY]);
}


const gchar *
modulemd_translation_entry_get_summary (ModulemdTranslationEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  return self->summary;
}


void
modulemd_translation_entry_set_description (ModulemdTranslationEntry *self,
                                            const gchar *description)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self));

  g_clear_pointer (&self->description, g_free);
  self->description = g_strdup (description);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DESCRIPTION]);
}


const gchar *
modulemd_translation_entry_get_description (ModulemdTranslationEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  return self->description;
}


static void
modulemd_translation_entry_set_locale (ModulemdTranslationEntry *self,
                                       const gchar *locale)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self));

  /* It is a coding error if we ever get a NULL name here */
  g_return_if_fail (locale);

  /* It is a coding error if we ever get the default string here */
  g_return_if_fail (g_strcmp0 (locale, TE_DEFAULT_STRING));

  g_clear_pointer (&self->locale, g_free);
  self->locale = g_strdup (locale);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LOCALE]);
}


const gchar *
modulemd_translation_entry_get_locale (ModulemdTranslationEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  return self->locale;
}


gchar **
modulemd_translation_entry_get_profiles_as_strv (
  ModulemdTranslationEntry *self)
{
  return modulemd_ordered_str_keys_as_strv (self->profile_descriptions);
}


void
modulemd_translation_entry_set_profile_description (
  ModulemdTranslationEntry *self,
  const gchar *profile_name,
  const gchar *profile_description)
{
  g_hash_table_replace (self->profile_descriptions,
                        g_strdup (profile_name),
                        g_strdup (profile_description));
}


const gchar *
modulemd_translation_entry_get_profile_description (
  ModulemdTranslationEntry *self, const gchar *profile_name)
{
  return (const gchar *)g_hash_table_lookup (self->profile_descriptions,
                                             profile_name);
}


static void
modulemd_translation_entry_get_property (GObject *object,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
  ModulemdTranslationEntry *self = MODULEMD_TRANSLATION_ENTRY (object);

  switch (prop_id)
    {
    case PROP_LOCALE:
      g_value_set_string (value, modulemd_translation_entry_get_locale (self));
      break;

    case PROP_SUMMARY:
      g_value_set_string (value,
                          modulemd_translation_entry_get_summary (self));
      break;

    case PROP_DESCRIPTION:
      g_value_set_string (value,
                          modulemd_translation_entry_get_description (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_translation_entry_set_property (GObject *object,
                                         guint prop_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
  ModulemdTranslationEntry *self = MODULEMD_TRANSLATION_ENTRY (object);

  switch (prop_id)
    {
    case PROP_LOCALE:
      modulemd_translation_entry_set_locale (self, g_value_get_string (value));
      break;

    case PROP_SUMMARY:
      modulemd_translation_entry_set_summary (self,
                                              g_value_get_string (value));
      break;

    case PROP_DESCRIPTION:
      modulemd_translation_entry_set_description (self,
                                                  g_value_get_string (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_translation_entry_class_init (ModulemdTranslationEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_translation_entry_finalize;
  object_class->get_property = modulemd_translation_entry_get_property;
  object_class->set_property = modulemd_translation_entry_set_property;

  properties[PROP_LOCALE] = g_param_spec_string (
    "locale",
    "Locale",
    "The locale for this translation entry. It must correspond to the format "
    "specified by libc locale names. This field may only be set on object "
    "construction and is immutable afterwards.",
    TE_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_SUMMARY] =
    g_param_spec_string ("summary",
                         "Summary",
                         "The summary of this module stream translated into "
                         "the language specified by locale.",
                         TE_DEFAULT_STRING,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_DESCRIPTION] =
    g_param_spec_string ("description",
                         "Description",
                         "The description of this module stream translated "
                         "into the language specified by locale.",
                         TE_DEFAULT_STRING,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_translation_entry_init (ModulemdTranslationEntry *self)
{
  self->profile_descriptions =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}


/* === YAML Functions === */

static GHashTable *
modulemd_translation_entry_parse_yaml_profiles (yaml_parser_t *parser,
                                                GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  g_autofree gchar *profile_name = NULL;
  g_autofree gchar *value = NULL;
  g_autoptr (GHashTable) profiles = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  profiles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  /* Parse the profiles */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

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
              MMD_YAML_ERROR_EVENT_EXIT (
                error, event, "Missing mapping in translation entry profiles");
              break;
            }

          profile_name = g_strdup ((const gchar *)event.data.scalar.value);
          value = modulemd_yaml_parse_string (parser, &nested_error);
          if (!value)
            {
              MMD_YAML_ERROR_EVENT_EXIT (error,
                                         event,
                                         "Error parsing profile value: %s",
                                         nested_error->message);
            }
          g_hash_table_replace (profiles,
                                g_steal_pointer (&profile_name),
                                g_steal_pointer (&value));

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event in translation entry profile");
          break;
        }
      yaml_event_delete (&event);
    }
  return g_steal_pointer (&profiles);
}

ModulemdTranslationEntry *
modulemd_translation_entry_parse_yaml (yaml_parser_t *parser, GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  g_autofree gchar *value;
  GHashTable *profiles = NULL;
  g_autoptr (ModulemdTranslationEntry) te = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Read in the locale of the translation entry */
  YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
  if (event.type != YAML_SCALAR_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (
        error, event, "Missing translation entry locale");
    }
  te = modulemd_translation_entry_new ((const gchar *)event.data.scalar.value);
  yaml_event_delete (&event);

  /* Read in any supplementary attributes of the translation entry
   */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

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
              MMD_YAML_ERROR_EVENT_EXIT (
                error, event, "Missing mapping in translation entry");
              break;
            }
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "summary"))
            {
              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse summary in translation entry: %s",
                    nested_error->message);
                }
              modulemd_translation_entry_set_summary (te, value);
              g_clear_pointer (&value, g_free);
            }
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "description"))
            {
              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse description in translation entry: %s",
                    nested_error->message);
                }
              modulemd_translation_entry_set_description (te, value);
              g_clear_pointer (&value, g_free);
            }
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "profiles"))
            {
              profiles = modulemd_translation_entry_parse_yaml_profiles (
                parser, &nested_error);
              if (profiles == NULL)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (error,
                                             event,
                                             "Failed to parse profiles: %s",
                                             nested_error->message);
                }
              g_hash_table_unref (te->profile_descriptions);
              te->profile_descriptions = profiles;
            }
          else
            {
              MMD_YAML_ERROR_EVENT_EXIT (
                error, event, "Unknown key in translation entry body");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_EVENT_EXIT (
            error, event, "Unexpected YAML event in translation entry");
          break;
        }
      yaml_event_delete (&event);
    }

  return g_steal_pointer (&te);
}

static gboolean
modulemd_translation_entry_emit_yaml_profiles (ModulemdTranslationEntry *self,
                                               yaml_emitter_t *emitter,
                                               GError **error)
{
  MODULEMD_INIT_TRACE ();
  int ret;
  g_autoptr (GError) nested_error = NULL;
  GHashTableIter iter;
  gpointer key, value;
  MMD_INIT_YAML_EVENT (event);

  ret = mmd_emitter_scalar (
    emitter, "profiles", YAML_PLAIN_SCALAR_STYLE, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to emit profiles key: ");
      return FALSE;
    }
  ret = mmd_emitter_start_mapping (
    emitter, YAML_BLOCK_MAPPING_STYLE, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to emit profiles start: ");
      return FALSE;
    }

  g_hash_table_iter_init (&iter, self->profile_descriptions);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      ret = mmd_emitter_scalar (
        emitter, (const gchar *)key, YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error, nested_error, "Failed to emit profile key: ");
          return FALSE;
        }

      ret = mmd_emitter_scalar (
        emitter, (const gchar *)value, YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error, nested_error, "Failed to emit profile value: ");
          return FALSE;
        }
    }

  ret = mmd_emitter_end_mapping (emitter, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to emit profiles end: ");
      return FALSE;
    }
  return TRUE;
}

gboolean
modulemd_translation_entry_emit_yaml (ModulemdTranslationEntry *self,
                                      yaml_emitter_t *emitter,
                                      GError **error)
{
  MODULEMD_INIT_TRACE ();
  int ret;
  g_autoptr (GError) nested_error = NULL;
  MMD_INIT_YAML_EVENT (event);
  /* Emit the Translation Entry Locale */
  ret = mmd_emitter_scalar (emitter,
                            modulemd_translation_entry_get_locale (self),
                            YAML_PLAIN_SCALAR_STYLE,
                            &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to emit translation entry locale: ");
      return FALSE;
    }
  /* Start the mapping for additional attributes of this translation entry */
  ret = mmd_emitter_start_mapping (
    emitter, YAML_BLOCK_MAPPING_STYLE, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to start translation entry mapping: ");
      return FALSE;
    }

  /* Add translation entry attributes if available */
  if (modulemd_translation_entry_get_summary (self) != NULL)
    {
      ret = mmd_emitter_scalar (
        emitter, "summary", YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error, nested_error, "Failed to emit summary key: ");
          return FALSE;
        }

      ret = mmd_emitter_scalar (emitter,
                                modulemd_translation_entry_get_summary (self),
                                YAML_PLAIN_SCALAR_STYLE,
                                &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error, nested_error, "Failed to emit translation entry summary: ");
          return FALSE;
        }
    }

  if (modulemd_translation_entry_get_description (self) != NULL)
    {
      ret = mmd_emitter_scalar (
        emitter, "description", YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error, nested_error, "Failed to emit description key: ");
          return FALSE;
        }

      ret =
        mmd_emitter_scalar (emitter,
                            modulemd_translation_entry_get_description (self),
                            YAML_PLAIN_SCALAR_STYLE,
                            &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error,
            nested_error,
            "Failed to emit translation entry description: ");
          return FALSE;
        }
    }

  if (g_hash_table_size (self->profile_descriptions) != 0)
    {
      ret = modulemd_translation_entry_emit_yaml_profiles (
        self, emitter, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error, nested_error, "Failed to emit profiles: ");
          return FALSE;
        }
    }

  /* End the mapping */
  ret = mmd_emitter_end_mapping (emitter, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to end translation entry mapping: ");
      return FALSE;
    }

  return TRUE;
}
