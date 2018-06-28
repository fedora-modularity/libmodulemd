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
#include "modulemd-translation-entry.h"
#include "private/modulemd-util.h"

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
  PROP_DESC,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdTranslationEntry *
modulemd_translation_entry_new (const gchar *locale)
{
  return g_object_new (
    MODULEMD_TYPE_TRANSLATION_ENTRY, "locale", locale, NULL);
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


ModulemdTranslationEntry *
modulemd_translation_entry_copy (ModulemdTranslationEntry *self)
{
  ModulemdTranslationEntry *copy = NULL;

  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  // clang-format off
  copy = g_object_new (
    MODULEMD_TYPE_TRANSLATION_ENTRY,
    "locale", modulemd_translation_entry_peek_locale (self),
    "summary", modulemd_translation_entry_peek_summary (self),
    "description", modulemd_translation_entry_peek_description (self),
    NULL);
  // clang-format on

  g_hash_table_unref (copy->profile_descriptions);
  copy->profile_descriptions =
    _modulemd_hash_table_deep_str_copy (self->profile_descriptions);

  return copy;
}


void
modulemd_translation_entry_set_locale (ModulemdTranslationEntry *self,
                                       const gchar *locale)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self));

  g_clear_pointer (&self->locale, g_free);
  self->locale = g_strdup (locale);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LOCALE]);
}


gchar *
modulemd_translation_entry_get_locale (ModulemdTranslationEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  return g_strdup (self->locale);
}


const gchar *
modulemd_translation_entry_peek_locale (ModulemdTranslationEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  return self->locale;
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


gchar *
modulemd_translation_entry_get_summary (ModulemdTranslationEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  return g_strdup (self->summary);
}


const gchar *
modulemd_translation_entry_peek_summary (ModulemdTranslationEntry *self)
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

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DESC]);
}


gchar *
modulemd_translation_entry_get_description (ModulemdTranslationEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  return g_strdup (self->description);
}


const gchar *
modulemd_translation_entry_peek_description (ModulemdTranslationEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  return self->description;
}


void
modulemd_translation_entry_set_profile_description (
  ModulemdTranslationEntry *self,
  const gchar *profile_name,
  const gchar *profile_description)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self));
  g_return_if_fail (profile_name);

  if (profile_description)
    {
      g_hash_table_replace (self->profile_descriptions,
                            g_strdup (profile_name),
                            g_strdup (profile_description));
    }
  else
    {
      /* Remove this profile from the list */
      g_hash_table_remove (self->profile_descriptions, profile_name);
    }
}


gchar *
modulemd_translation_entry_get_profile_description (
  ModulemdTranslationEntry *self, const gchar *profile_name)
{
  gchar *profile_description = NULL;
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);
  g_return_val_if_fail (profile_name, NULL);

  profile_description =
    g_hash_table_lookup (self->profile_descriptions, profile_name);
  if (!profile_description)
    return NULL;

  return g_strdup (profile_description);
}


const gchar *
modulemd_translation_entry_peek_profile_description (
  ModulemdTranslationEntry *self, const gchar *profile_name)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);
  g_return_val_if_fail (profile_name, NULL);

  return g_hash_table_lookup (self->profile_descriptions, profile_name);
}


GHashTable *
modulemd_translation_entry_get_all_profile_descriptions (
  ModulemdTranslationEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (self), NULL);

  return _modulemd_hash_table_deep_str_copy (self->profile_descriptions);
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
      g_value_take_string (value,
                           modulemd_translation_entry_get_locale (self));
      break;

    case PROP_SUMMARY:
      g_value_take_string (value,
                           modulemd_translation_entry_get_summary (self));
      break;

    case PROP_DESC:
      g_value_take_string (value,
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

    case PROP_DESC:
      modulemd_translation_entry_set_description (self,
                                                  g_value_get_string (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_translation_entry_defaults (GObject *object)
{
  ModulemdTranslationEntry *self = MODULEMD_TRANSLATION_ENTRY (object);

  g_return_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (object));

  /* Set a default of C.UTF-8 for the locale */
  if (!self->locale)
    {
      self->locale = g_strdup ("C.UTF-8");
    }


  G_OBJECT_CLASS (modulemd_translation_entry_parent_class)
    ->constructed (object);
}


static void
modulemd_translation_entry_class_init (ModulemdTranslationEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_translation_entry_finalize;
  object_class->get_property = modulemd_translation_entry_get_property;
  object_class->set_property = modulemd_translation_entry_set_property;
  object_class->constructed = modulemd_translation_entry_defaults;

  properties[PROP_LOCALE] = g_param_spec_string (
    "locale",
    "Translation Locale",
    "Specifies the language and dialect of this translation. Must be be "
    "specified as described by "
    "https://www.gnu.org/software/libc/manual/html_node/Locale-Names.html",
    NULL,
    G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_SUMMARY] =
    g_param_spec_string ("summary",
                         "Stream Summary",
                         "A short description of the module stream, "
                         "translated into the specified language.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_DESC] =
    g_param_spec_string ("description",
                         "Stream Description",
                         "A detailed description of the module stream, "
                         "translated into the specified language.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_translation_entry_init (ModulemdTranslationEntry *self)
{
  self->profile_descriptions =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}
