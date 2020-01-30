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
#include <inttypes.h>
#include <yaml.h>

#include "modulemd-errors.h"
#include "modulemd-translation-entry.h"
#include "modulemd-translation.h"
#include "private/glib-extensions.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-translation-entry-private.h"
#include "private/modulemd-translation-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define T_DEFAULT_STRING "__TRANSLATION_VALUE_UNSET__"
#define T_PLACEHOLDER_STRING "__TRANSLATION_VALUE_NOT_YET_SET__"

struct _ModulemdTranslation
{
  GObject parent_instance;

  guint64 version;
  gchar *module_name;
  gchar *module_stream;
  guint64 modified;

  GHashTable *translation_entries;
};

G_DEFINE_TYPE (ModulemdTranslation, modulemd_translation, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_VERSION,
  PROP_MODULE_NAME,
  PROP_MODULE_STREAM,
  PROP_MODIFIED,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdTranslation *
modulemd_translation_new (guint64 version,
                          const gchar *module_name,
                          const gchar *module_stream,
                          guint64 modified)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_TRANSLATION,
                       "version", version,
                       "module_name", module_name,
                       "module_stream", module_stream,
                       "modified", modified,
                       NULL);
  // clang-format on
}


ModulemdTranslation *
modulemd_translation_copy (ModulemdTranslation *self)
{
  g_autoptr (ModulemdTranslation) t = NULL;
  gpointer key;
  gpointer value;
  GHashTableIter iter;

  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), NULL);


  t = modulemd_translation_new (modulemd_translation_get_version (self),
                                modulemd_translation_get_module_name (self),
                                modulemd_translation_get_module_stream (self),
                                modulemd_translation_get_modified (self));

  g_hash_table_iter_init (&iter, self->translation_entries);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      modulemd_translation_set_translation_entry (t, value);
    }

  return g_steal_pointer (&t);
}


gboolean
modulemd_translation_validate (ModulemdTranslation *self, GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), FALSE);

  if (g_str_equal (modulemd_translation_get_module_name (self),
                   T_PLACEHOLDER_STRING))
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Translation module name is unset.");
      return FALSE;
    }
  if (strlen (modulemd_translation_get_module_name (self)) == 0)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Translation module name is empty.");
      return FALSE;
    }
  if (g_str_equal (modulemd_translation_get_module_stream (self),
                   T_PLACEHOLDER_STRING))
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Translation module stream is unset.");
      return FALSE;
    }
  if (strlen (modulemd_translation_get_module_stream (self)) == 0)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Translation module stream is unset.");
      return FALSE;
    }
  if (modulemd_translation_get_modified (self) == 0)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Translation module modified is empty.");
      return FALSE;
    }

  return TRUE;
}


static void
modulemd_translation_finalize (GObject *object)
{
  ModulemdTranslation *self = (ModulemdTranslation *)object;

  g_clear_pointer (&self->module_name, g_free);
  g_clear_pointer (&self->module_stream, g_free);
  g_clear_pointer (&self->translation_entries, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_translation_parent_class)->finalize (object);
}


guint64
modulemd_translation_get_version (ModulemdTranslation *self)
{
  return self->version;
}


const gchar *
modulemd_translation_get_module_name (ModulemdTranslation *self)
{
  return self->module_name;
}


const gchar *
modulemd_translation_get_module_stream (ModulemdTranslation *self)
{
  return self->module_stream;
}


guint64
modulemd_translation_get_modified (ModulemdTranslation *self)
{
  return self->modified;
}


static void
modulemd_translation_set_version (ModulemdTranslation *self, guint64 version)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));
  g_return_if_fail (version != 0);

  self->version = version;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VERSION]);
}


static void
modulemd_translation_set_module_name (ModulemdTranslation *self,
                                      const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));
  g_return_if_fail (module_name);
  g_return_if_fail (g_strcmp0 (module_name, T_DEFAULT_STRING));

  g_clear_pointer (&self->module_name, g_free);
  self->module_name = g_strdup (module_name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_NAME]);
}


static void
modulemd_translation_set_module_stream (ModulemdTranslation *self,
                                        const gchar *module_stream)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));
  g_return_if_fail (module_stream);
  g_return_if_fail (g_strcmp0 (module_stream, T_DEFAULT_STRING));

  g_clear_pointer (&self->module_stream, g_free);
  self->module_stream = g_strdup (module_stream);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_STREAM]);
}


void
modulemd_translation_set_modified (ModulemdTranslation *self, guint64 modified)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));

  self->modified = modified;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODIFIED]);
}


GStrv
modulemd_translation_get_locales_as_strv (ModulemdTranslation *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->translation_entries);
}


void
modulemd_translation_set_translation_entry (
  ModulemdTranslation *self, ModulemdTranslationEntry *translation_entry)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));

  g_hash_table_insert (
    self->translation_entries,
    g_strdup (modulemd_translation_entry_get_locale (translation_entry)),
    modulemd_translation_entry_copy (translation_entry));
}


ModulemdTranslationEntry *
modulemd_translation_get_translation_entry (ModulemdTranslation *self,
                                            const gchar *locale)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), NULL);

  return g_hash_table_lookup (self->translation_entries, locale);
}


static void
modulemd_translation_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  ModulemdTranslation *self = MODULEMD_TRANSLATION (object);

  switch (prop_id)
    {
    case PROP_VERSION:
      g_value_set_uint64 (value, modulemd_translation_get_version (self));
      break;
    case PROP_MODULE_NAME:
      g_value_set_string (value, modulemd_translation_get_module_name (self));
      break;
    case PROP_MODULE_STREAM:
      g_value_set_string (value,
                          modulemd_translation_get_module_stream (self));
      break;
    case PROP_MODIFIED:
      g_value_set_uint64 (value, modulemd_translation_get_modified (self));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_translation_set_property (GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  ModulemdTranslation *self = MODULEMD_TRANSLATION (object);

  switch (prop_id)
    {
    case PROP_VERSION:
      modulemd_translation_set_version (self, g_value_get_uint64 (value));
      break;
    case PROP_MODULE_NAME:
      modulemd_translation_set_module_name (self, g_value_get_string (value));
      break;
    case PROP_MODULE_STREAM:
      modulemd_translation_set_module_stream (self,
                                              g_value_get_string (value));
      break;
    case PROP_MODIFIED:
      modulemd_translation_set_modified (self, g_value_get_uint64 (value));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_translation_class_init (ModulemdTranslationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_translation_finalize;
  object_class->get_property = modulemd_translation_get_property;
  object_class->set_property = modulemd_translation_set_property;

  properties[PROP_VERSION] =
    g_param_spec_uint64 ("version",
                         "Version",
                         "The metadata version of this Translation object.",
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_MODULE_NAME] = g_param_spec_string (
    "module-name",
    "Module name",
    "The name of the module to which these translations apply.",
    T_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_MODULE_STREAM] = g_param_spec_string (
    "module-stream",
    "Module stream",
    "The name of the module stream to which these translations apply.",
    T_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_MODIFIED] = g_param_spec_uint64 (
    "modified",
    "Modified",
    "The last modified time represented as a 64-bit integer "
    "(such as 201807011200)",
    0,
    G_MAXUINT64,
    0,
    G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_translation_init (ModulemdTranslation *self)
{
  self->translation_entries =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}


/* === YAML Functions === */

static GHashTable *
modulemd_translation_parse_yaml_entries (yaml_parser_t *parser,
                                         gboolean strict,
                                         GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (GError) nested_error = NULL;
  gboolean in_map = FALSE;
  gboolean done = FALSE;
  gchar *locale;
  g_autoptr (GHashTable) translation_entries = NULL;
  g_autoptr (ModulemdTranslationEntry) te = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  translation_entries =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

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
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error, event, "Missing mapping in translation data entry");
            }

          te = modulemd_translation_entry_parse_yaml (
            parser,
            (const gchar *)event.data.scalar.value,
            strict,
            &nested_error);
          if (te == NULL)
            {
              MMD_YAML_ERROR_EVENT_EXIT (
                error,
                event,
                "Failed to parse translation entry: %s",
                nested_error->message);
            }


          locale = g_strdup (modulemd_translation_entry_get_locale (te));
          g_hash_table_insert (
            translation_entries, locale, g_steal_pointer (&te));
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event %s in translation entries data",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  /* Work around false-positive in clang static analysis which thinks it's
   * possible for this function to return NULL and not set error.
   */
  if (G_UNLIKELY (translation_entries == NULL))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_EMIT,
                   "Somehow got a NULL hash table here.");
    }

  return g_steal_pointer (&translation_entries);
}


ModulemdTranslation *
modulemd_translation_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                 gboolean strict,
                                 GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (ModulemdTranslation) t = NULL;
  g_autoptr (GError) nested_error = NULL;
  g_autofree gchar *value = NULL;
  guint64 modified;
  g_autoptr (GHashTable) entries = NULL;
  guint64 version = modulemd_subdocument_info_get_mdversion (subdoc);

  if (!modulemd_subdocument_info_get_data_parser (
        subdoc, &parser, strict, error))
    {
      return NULL;
    }

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Create a translation with placeholder module info. */
  t = modulemd_translation_new (
    version, T_PLACEHOLDER_STRING, T_PLACEHOLDER_STRING, 0);

  YAML_PARSER_PARSE_WITH_EXIT (&parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Missing mapping in translation data entry");
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
              if (!g_str_equal (modulemd_translation_get_module_name (t),
                                T_PLACEHOLDER_STRING))
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error, event, "Module name encountered twice");
                }
              value = modulemd_yaml_parse_string (&parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse module name in translation data: %s",
                    nested_error->message);
                }

              modulemd_translation_set_module_name (t, value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "stream"))
            {
              if (!g_str_equal (modulemd_translation_get_module_stream (t),
                                T_PLACEHOLDER_STRING))
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error, event, "Module stream encountered twice");
                }
              value = modulemd_yaml_parse_string (&parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse module stream in translation data: %s",
                    nested_error->message);
                }

              modulemd_translation_set_module_stream (t, value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "modified"))
            {
              modified = modulemd_yaml_parse_uint64 (&parser, &nested_error);
              if (nested_error)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse modified in translation data: %s",
                    nested_error->message);
                }

              modulemd_translation_set_modified (t, modified);
            }
          else if (g_str_equal (event.data.scalar.value, "translations"))
            {
              entries = modulemd_translation_parse_yaml_entries (
                &parser, strict, &nested_error);
              if (!entries)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse translations in translation data: %s",
                    nested_error->message);
                }

              g_hash_table_unref (t->translation_entries);
              t->translation_entries = g_steal_pointer (&entries);
            }
          else
            {
              SKIP_UNKNOWN (&parser,
                            NULL,
                            "Unexpected key in translation data: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event %s in translation data",
            mmd_yaml_get_event_name (event.type));
          break;
        }

      yaml_event_delete (&event);
    }

  if (!modulemd_translation_validate (t, &nested_error))
    {
      MMD_YAML_ERROR_EVENT_EXIT (error,
                                 event,
                                 "Unable to validate translation object: %s",
                                 nested_error->message);
    }

  return g_steal_pointer (&t);
}

static gboolean
modulemd_translation_emit_yaml_entries (ModulemdTranslation *self,
                                        yaml_emitter_t *emitter,
                                        GError **error)
{
  GHashTableIter iter;
  g_autoptr (GError) nested_error = NULL;
  gpointer key;
  gpointer value;

  if (!mmd_emitter_start_mapping (emitter, YAML_BLOCK_MAPPING_STYLE, error))
    {
      return FALSE;
    }

  g_hash_table_iter_init (&iter, self->translation_entries);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      if (!modulemd_translation_entry_emit_yaml (
            (ModulemdTranslationEntry *)value, emitter, &nested_error))
        {
          g_propagate_prefixed_error (error,
                                      g_steal_pointer (&nested_error),
                                      "Error emitting translation entry: ");
          return FALSE;
        }
    }

  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }

  return TRUE;
}

gboolean
modulemd_translation_emit_yaml (ModulemdTranslation *self,
                                yaml_emitter_t *emitter,
                                GError **error)
{
  MODULEMD_INIT_TRACE ();
  g_autoptr (GError) nested_error = NULL;
  g_autofree gchar *modified_string = NULL;

  if (!modulemd_translation_validate (self, &nested_error))
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Translation object failed validation: ");
      return FALSE;
    }

  modified_string =
    g_strdup_printf ("%" PRIu64, modulemd_translation_get_modified (self));

  /* Emit document headers */
  if (!modulemd_yaml_emit_document_headers (
        emitter,
        MODULEMD_YAML_DOC_TRANSLATIONS,
        modulemd_translation_get_version (self),
        error))
    {
      return FALSE;
    }

  /* Start data: */
  if (!mmd_emitter_start_mapping (emitter, YAML_BLOCK_MAPPING_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (emitter, "module", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (emitter,
                           modulemd_translation_get_module_name (self),
                           YAML_PLAIN_SCALAR_STYLE,
                           error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (emitter, "stream", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (emitter,
                           modulemd_translation_get_module_stream (self),
                           YAML_PLAIN_SCALAR_STYLE,
                           error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (
        emitter, "modified", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (
        emitter, modified_string, YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (g_hash_table_size (self->translation_entries) != 0)
    {
      if (!mmd_emitter_scalar (
            emitter, "translations", YAML_PLAIN_SCALAR_STYLE, error))
        {
          return FALSE;
        }

      if (!modulemd_translation_emit_yaml_entries (self, emitter, error))
        {
          return FALSE;
        }
    }

  /* Close the data: mapping */
  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }

  /* Close top-level mapping */
  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }

  /* Close document */
  if (!mmd_emitter_end_document (emitter, error))
    {
      return FALSE;
    }

  return TRUE;
}
