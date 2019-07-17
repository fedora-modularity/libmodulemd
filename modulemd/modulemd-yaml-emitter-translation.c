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
#include <inttypes.h>
#include "private/modulemd-yaml.h"
#include "private/modulemd-util.h"


static gboolean
_emit_translation_root (yaml_emitter_t *emitter,
                        ModulemdTranslation *translation,
                        GError **error);

static gboolean
_emit_translation_data (yaml_emitter_t *emitter,
                        ModulemdTranslation *translation,
                        GError **error);

static gboolean
_emit_translation_entries (yaml_emitter_t *emitter,
                           ModulemdTranslation *translation,
                           GError **error);

static gboolean
_emit_translation_entry (yaml_emitter_t *emitter,
                         ModulemdTranslationEntry *entry,
                         GError **error);


gboolean
_emit_translation (yaml_emitter_t *emitter,
                   ModulemdTranslation *translation,
                   GError **error)
{
  MODULEMD_INIT_TRACE
  MMD_INIT_YAML_EVENT (event);

  yaml_document_start_event_initialize (&event, NULL, NULL, NULL, 0);
  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Error starting document");

  if (!_emit_translation_root (emitter, translation, error))
    return FALSE;

  yaml_document_end_event_initialize (&event, 0);
  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Error ending document");

  return TRUE;
}


static gboolean
_emit_translation_root (yaml_emitter_t *emitter,
                        ModulemdTranslation *translation,
                        GError **error)
{
  MODULEMD_INIT_TRACE
  MMD_INIT_YAML_EVENT (event);
  guint64 mdversion = modulemd_translation_get_mdversion (translation);
  const gchar *module_name =
    modulemd_translation_peek_module_name (translation);
  const gchar *module_stream =
    modulemd_translation_peek_module_stream (translation);
  guint64 modified = modulemd_translation_get_modified (translation);
  g_autofree gchar *name = NULL;
  g_autofree gchar *value = NULL;

  if (mdversion < 1)
    {
      /* The mdversion is required and has not been specified.
       * This translatino is invalid
       */
      MMD_EMITTER_SET_ERROR (
        error, "Metadata version unspecified. Translation is invalid.");
      return FALSE;
    }

  if (!module_name || !(module_name[0]))
    {
      /* The module name is required and is missing */
      MMD_EMITTER_SET_ERROR (error, "Module name is missing");
      return FALSE;
    }

  if (!module_stream || !(module_stream[0]))
    {
      /* The module stream is required and is missing */
      MMD_EMITTER_SET_ERROR (error, "Module stream is missing");
      return FALSE;
    }

  if (modified < 1)
    {
      /* The modified time is required and has not been specified.
       * This translation is invalid
       */
      MMD_EMITTER_SET_ERROR (
        error, "Modified value unspecified. Translation is invalid.");
      return FALSE;
    }

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Error starting root mapping");


  /* Emit document: modulemd-translations */
  name = g_strdup ("document");
  value = g_strdup ("modulemd-translations");
  MMD_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  /* Emit the metadata version */
  name = g_strdup ("version");
  value = g_strdup_printf ("%" PRIu64, mdversion);
  MMD_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  /* Emit the data section */
  name = g_strdup ("data");
  MMD_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_translation_data (emitter, translation, error))
    return FALSE;


  yaml_mapping_end_event_initialize (&event);
  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Error ending root mapping");

  return TRUE;
}


static gboolean
_emit_translation_data (yaml_emitter_t *emitter,
                        ModulemdTranslation *translation,
                        GError **error)
{
  MODULEMD_INIT_TRACE
  MMD_INIT_YAML_EVENT (event);
  g_autofree gchar *name = NULL;
  g_autofree gchar *value = NULL;

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Error starting data mapping");

  /* Module Name */
  name = g_strdup ("module");
  value = modulemd_translation_get_module_name (translation);
  MMD_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  /* Module Stream */
  name = g_strdup ("stream");
  value = modulemd_translation_get_module_stream (translation);
  MMD_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  /* Modified */
  name = g_strdup ("modified");
  value = g_strdup_printf ("%" PRIu64,
                           modulemd_translation_get_modified (translation));
  MMD_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* Translations */
  name = g_strdup ("translations");
  MMD_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_translation_entries (emitter, translation, error))
    return FALSE;

  yaml_mapping_end_event_initialize (&event);
  MMD_EMIT_WITH_EXIT (emitter, &event, error, "Error ending data mapping");

  return TRUE;
}


static gboolean
_emit_translation_entries (yaml_emitter_t *emitter,
                           ModulemdTranslation *translation,
                           GError **error)
{
  MODULEMD_INIT_TRACE
  MMD_INIT_YAML_EVENT (event);
  g_autofree gchar *name = NULL;
  g_autoptr (GPtrArray) keys = NULL;
  g_autoptr (ModulemdTranslationEntry) entry = NULL;

  keys = modulemd_translation_get_locales (translation);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  MMD_EMIT_WITH_EXIT (
    emitter, &event, error, "Error starting translations mapping");

  for (gsize i = 0; i < keys->len; i++)
    {
      entry = modulemd_translation_get_entry_by_locale (
        translation, g_ptr_array_index (keys, i));

      /* Add the locale */
      name = modulemd_translation_entry_get_locale (entry);
      MMD_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_translation_entry (emitter, entry, error))
        return FALSE;

      g_clear_pointer (&entry, g_object_unref);
    }

  yaml_mapping_end_event_initialize (&event);
  MMD_EMIT_WITH_EXIT (
    emitter, &event, error, "Error ending translations mapping");
  return TRUE;
}

static gboolean
_emit_translation_entry (yaml_emitter_t *emitter,
                         ModulemdTranslationEntry *entry,
                         GError **error)
{
  MODULEMD_INIT_TRACE
  MMD_INIT_YAML_EVENT (event);
  g_autofree gchar *name = NULL;
  g_autofree gchar *value = NULL;
  g_autoptr (GHashTable) profile_descriptions = NULL;

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  MMD_EMIT_WITH_EXIT (
    emitter, &event, error, "Error starting translation entry mapping");

  name = g_strdup ("summary");
  value = modulemd_translation_entry_get_summary (entry);
  if (!value)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "Translation entry missing summary field.");
      return FALSE;
    }
  MMD_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  name = g_strdup ("description");
  value = modulemd_translation_entry_get_description (entry);
  if (!value)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "Translation entry missing description field.");
      return FALSE;
    }
  MMD_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  /* Profile Descriptions */
  profile_descriptions =
    modulemd_translation_entry_get_all_profile_descriptions (entry);
  if (profile_descriptions)
    {
      name = g_strdup ("profiles");
      MMD_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_hashtable (
            emitter, profile_descriptions, YAML_PLAIN_SCALAR_STYLE, error))
        {
          return FALSE;
        }
    }

  yaml_mapping_end_event_initialize (&event);
  MMD_EMIT_WITH_EXIT (
    emitter, &event, error, "Error ending translation entry mapping");

  return TRUE;
}
