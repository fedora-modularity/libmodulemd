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
#include <glib.h>
#include <glib/gstdio.h>
#include <yaml.h>
#include <errno.h>
#include "private/modulemd-yaml.h"
#include "private/modulemd-util.h"

#define _yaml_parser_translation_recurse_down(fn)                             \
  do                                                                          \
    {                                                                         \
      if (!fn (translation, parser, error))                                   \
        return FALSE;                                                         \
    }                                                                         \
  while (0)

static gboolean
_parse_translation_data (ModulemdTranslation *translation,
                         yaml_parser_t *parser,
                         GError **error);

static gboolean
_parse_translation_entries (ModulemdTranslation *translation,
                            yaml_parser_t *parser,
                            GError **error);
static ModulemdTranslationEntry *
_parse_translation_entry (yaml_parser_t *parser,
                          const gchar *locale,
                          GError **error);

gboolean
_parse_translation (yaml_parser_t *parser,
                    GObject **object,
                    guint64 version,
                    GError **error)
{
  g_autoptr (ModulemdTranslation) translation = NULL;
  MODULEMD_INIT_TRACE
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  guint64 mdversion;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Use the pre-processed version */
  if (version && version <= MD_TRANSLATION_VERSION_LATEST)
    {
      // clang-format off
      translation = g_object_new (MODULEMD_TYPE_TRANSLATION,
                                  "mdversion", version,
                                  NULL);
      // clang-format on
    }
  else
    {
      /* No mdversion was discovered during pre-processing */
      MMD_YAML_SET_ERROR (error, "Unknown modulemd translation version");
      return FALSE;
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the main document content */
          in_map = TRUE;
          break;


        case YAML_MAPPING_END_EVENT:
          /* This is the end of the main document content. */
          if (!in_map)
            {
              MMD_YAML_SET_ERROR (error, "Map end received before map start.");
              return FALSE;
            }
          done = TRUE;
          break;


        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              MMD_YAML_SET_ERROR (error, "Scalar received before map start.");
              return FALSE;
            }

          /* Handle "document: modulemd-translations" */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "document"))
            {
              g_debug ("TRACE: root entry [document]");

              YAML_PARSER_PARSE_WITH_EXIT (parser, &value_event, error);

              if (value_event.type != YAML_SCALAR_EVENT ||
                  g_strcmp0 ((const gchar *)value_event.data.scalar.value,
                             "modulemd-translations"))
                {
                  yaml_event_delete (&value_event);
                  MMD_YAML_SET_ERROR (error, "Document type mismatch");
                  return FALSE;
                }
              yaml_event_delete (&value_event);
            }

          /* Record the modulemd version for the parser */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "version"))
            {
              g_debug ("TRACE: root entry [version]");
              YAML_PARSER_PARSE_WITH_EXIT (parser, &value_event, error);
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_SET_ERROR (error, "Unknown modulemd version");
                  return FALSE;
                }

              mdversion = g_ascii_strtoull (
                (const gchar *)value_event.data.scalar.value, NULL, 10);
              yaml_event_delete (&value_event);
              if (!mdversion)
                {
                  MMD_YAML_SET_ERROR (error,
                                      "Unknown modulemd defaults version");
                  return FALSE;
                }

              if (mdversion != version)
                {
                  /* Preprocessing and real parser don't match!
                   * This should be impossible
                   */
                  MMD_YAML_SET_ERROR (error,
                                      "Modulemd translations version doesn't "
                                      "match preprocessing");
                  return FALSE;
                }
            }

          /* Process the data section */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "data"))
            {
              _yaml_parser_translation_recurse_down (_parse_translation_data);
            }

          else
            {
              g_debug ("Unexpected key in root: %s",
                       (const gchar *)event.data.scalar.value);
              if (!skip_unknown_yaml (parser, error))
                return FALSE;
            }
          break;


        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_SET_ERROR (error,
                              "Unexpected YAML event in root: %s",
                              mmd_yaml_get_event_name (event.type));
          return FALSE;
        }
      yaml_event_delete (&event);
    }

  *object = g_object_ref (G_OBJECT (translation));
  return TRUE;
}


static gboolean
_parse_translation_data (ModulemdTranslation *translation,
                         yaml_parser_t *parser,
                         GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  guint64 modified;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the main document content */
          in_map = TRUE;
          break;


        case YAML_MAPPING_END_EVENT:
          /* This is the end of the main document content. */
          if (!in_map)
            {
              MMD_YAML_SET_ERROR (error, "Map end received before map start.");
              return FALSE;
            }
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              MMD_YAML_SET_ERROR (error, "Scalar received before map start.");
              return FALSE;
            }

          /* Module Name */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "module"))
            {
              YAML_PARSER_PARSE_WITH_EXIT (parser, &value_event, error);
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_SET_ERROR (error, "Failed to parse module name");
                  return FALSE;
                }

              modulemd_translation_set_module_name (
                translation, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }

          /* Module stream */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "stream"))
            {
              YAML_PARSER_PARSE_WITH_EXIT (parser, &value_event, error);
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_SET_ERROR (error, "Failed to parse module stream");
                  return FALSE;
                }

              modulemd_translation_set_module_stream (
                translation, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }

          /* Modified */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "modified"))
            {
              YAML_PARSER_PARSE_WITH_EXIT (parser, &value_event, error);
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_SET_ERROR (error, "Failed to parse modified value");
                  return FALSE;
                }

              modified = g_ascii_strtoull (
                (const gchar *)value_event.data.scalar.value, NULL, 10);
              yaml_event_delete (&value_event);

              if (!modified)
                {
                  MMD_YAML_SET_ERROR (error,
                                      "Unknown modulemd defaults version");
                  return FALSE;
                }

              modulemd_translation_set_modified (translation, modified);
            }

          /* Translation Entries */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "translations"))
            {
              _yaml_parser_translation_recurse_down (
                _parse_translation_entries);
            }

          else
            {
              g_debug ("Unexpected key in data: %s",
                       (const gchar *)event.data.scalar.value);
              if (!skip_unknown_yaml (parser, error))
                return FALSE;
            }

          break;


        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_SET_ERROR (error,
                              "Unexpected YAML event in data: %s",
                              mmd_yaml_get_event_name (event.type));
          return FALSE;
        }

      yaml_event_delete (&event);
    }

  return TRUE;
}

static gboolean
_parse_translation_entries (ModulemdTranslation *translation,
                            yaml_parser_t *parser,
                            GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  const gchar *locale = NULL;
  g_autoptr (ModulemdTranslationEntry) entry = NULL;

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the main document content */
          in_map = TRUE;
          break;


        case YAML_MAPPING_END_EVENT:
          /* This is the end of the main document content. */
          if (!in_map)
            {
              MMD_YAML_SET_ERROR (error, "Map end received before map start.");
              return FALSE;
            }
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              MMD_YAML_SET_ERROR (error, "Scalar received before map start.");
              return FALSE;
            }

          locale = (const gchar *)event.data.scalar.value;
          entry = _parse_translation_entry (parser, locale, error);
          if (!entry)
            return FALSE;

          modulemd_translation_add_entry (translation, entry);
          g_clear_pointer (&entry, g_object_unref);

          break;


        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_SET_ERROR (error,
                              "Unexpected YAML event in translations: %s",
                              mmd_yaml_get_event_name (event.type));
          return FALSE;
        }

      yaml_event_delete (&event);
    }

  return TRUE;
}

static ModulemdTranslationEntry *
_parse_translation_entry (yaml_parser_t *parser,
                          const gchar *locale,
                          GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  g_autoptr (GHashTable) profiles = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_autoptr (ModulemdTranslationEntry) entry =
    g_object_new (MODULEMD_TYPE_TRANSLATION_ENTRY, "locale", locale, NULL);


  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the main document content */
          in_map = TRUE;
          break;


        case YAML_MAPPING_END_EVENT:
          /* This is the end of the main document content. */
          if (!in_map)
            {
              MMD_YAML_SET_ERROR (error, "Map end received before map start.");
              return NULL;
            }
          done = TRUE;
          break;


        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              MMD_YAML_SET_ERROR (error, "Scalar received before map start.");
              return NULL;
            }

          /* Summary */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "summary"))
            {
              YAML_PARSER_PARSE_WITH_EXIT (parser, &value_event, error);
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_SET_ERROR (error, "Failed to parse summary");
                  return NULL;
                }

              modulemd_translation_entry_set_summary (
                entry, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "description"))
            {
              YAML_PARSER_PARSE_WITH_EXIT (parser, &value_event, error);
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_SET_ERROR (error, "Failed to parse description");
                  return NULL;
                }

              modulemd_translation_entry_set_description (
                entry, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "profiles"))
            {
              if (!_hashtable_from_mapping (parser, &profiles, error))
                return NULL;

              g_hash_table_iter_init (&iter, profiles);
              while (g_hash_table_iter_next (&iter, &key, &value))
                {
                  modulemd_translation_entry_set_profile_description (
                    entry, (const gchar *)key, (const gchar *)value);
                }
              g_clear_pointer (&profiles, g_hash_table_unref);
            }

          else
            {
              g_debug ("Unexpected key in entries: %s",
                       (const gchar *)event.data.scalar.value);
              if (!skip_unknown_yaml (parser, error))
                return NULL;
            }

          break;


        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_SET_ERROR (
            error,
            "Unexpected YAML event in translation entry %s: %s",
            locale,
            mmd_yaml_get_event_name (event.type));
          return FALSE;
        }
      yaml_event_delete (&event);
    }

  return g_object_ref (entry);
}
