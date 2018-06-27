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
_emit_defaults_root (yaml_emitter_t *emitter,
                     ModulemdDefaults *defaults,
                     GError **error);

static gboolean
_emit_defaults_data (yaml_emitter_t *emitter,
                     ModulemdDefaults *defaults,
                     GError **error);

static gboolean
_emit_defaults_profiles (yaml_emitter_t *emitter,
                         ModulemdDefaults *defaults,
                         GError **error);

static gboolean
_emit_defaults_intents (yaml_emitter_t *emitter,
                        ModulemdDefaults *defaults,
                        GError **error);

static gboolean
_emit_intent (yaml_emitter_t *emitter, ModulemdIntent *intent, GError **error);

static gboolean
_emit_intent_profiles (yaml_emitter_t *emitter,
                       ModulemdIntent *intent,
                       GError **error);

gboolean
_emit_defaults (yaml_emitter_t *emitter,
                ModulemdDefaults *defaults,
                GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;

  g_debug ("TRACE: entering _emit_defaults");
  yaml_document_start_event_initialize (&event, NULL, NULL, NULL, 0);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting document");

  if (!_emit_defaults_root (emitter, defaults, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to process root");
    }

  yaml_document_end_event_initialize (&event, 0);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending document");

  result = TRUE;

error:

  g_debug ("TRACE: exiting _emit_defaults");
  return result;
}

static gboolean
_emit_defaults_root (yaml_emitter_t *emitter,
                     ModulemdDefaults *defaults,
                     GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  guint64 mdversion = modulemd_defaults_peek_version (defaults);
  const gchar *module_name = modulemd_defaults_peek_module_name (defaults);
  gchar *name = NULL;
  gchar *value = NULL;

  g_debug ("TRACE: entering _emit_defaults_root");
  if (mdversion < 1)
    {
      /* The mdversion is required and has not been specified.
       * This module is invalid
       */
      MMD_YAML_EMITTER_ERROR_RETURN (
        error, "Module Metadata version unspecified. Module is invalid.");
    }

  if (!module_name || !(module_name[0]))
    {
      /* The module name is required and is missing */
      MMD_YAML_EMITTER_ERROR_RETURN (error, "Module name is missing");
    }

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting root mapping");


  /* document: modulemd */
  name = g_strdup ("document");
  value = g_strdup ("modulemd-defaults");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* The modulemd version */
  name = g_strdup ("version");
  value = g_strdup_printf ("%" PRIu64, mdversion);
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* The data */
  name = g_strdup ("data");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_defaults_data (emitter, defaults, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit data");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending root mapping");

  result = TRUE;

error:
  g_clear_pointer (&name, g_free);
  g_clear_pointer (&value, g_free);

  g_debug ("TRACE: exiting _emit_defaults_root");
  return result;
}

static gboolean
_emit_defaults_data (yaml_emitter_t *emitter,
                     ModulemdDefaults *defaults,
                     GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  gchar *value = NULL;

  g_debug ("TRACE: entering _emit_defaults_data");

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting data mapping");


  /* Module name */
  name = g_strdup ("module");
  value = modulemd_defaults_dup_module_name (defaults);
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* Module default stream */
  value = modulemd_defaults_dup_default_stream (defaults);
  if (value)
    {
      name = g_strdup ("stream");

      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }


  /* Profile Defaults */
  if (!_emit_defaults_profiles (emitter, defaults, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error,
                                     "Could not write out profile defaults");
    }

  /* Intents */

  if (!_emit_defaults_intents (emitter, defaults, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not write out intents");
    }


  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending data mapping");

  result = TRUE;
error:
  g_clear_pointer (&name, g_free);
  g_clear_pointer (&value, g_free);

  g_debug ("TRACE: exiting _emit_defaults_data");
  return result;
}

static gboolean
_emit_defaults_profiles (yaml_emitter_t *emitter,
                         ModulemdDefaults *defaults,
                         GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  GHashTable *profile_defaults = NULL;
  GPtrArray *keys = NULL;
  ModulemdSimpleSet *set = NULL;

  g_debug ("TRACE: entering _emit_defaults_profiles");

  name = g_strdup ("profiles");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  /* Start the map */
  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting profile default mapping");

  profile_defaults = modulemd_defaults_peek_profile_defaults (defaults);
  keys = _modulemd_ordered_str_keys (profile_defaults, _modulemd_strcmp_sort);
  for (gsize i = 0; i < keys->len; i++)
    {
      name = g_strdup (g_ptr_array_index (keys, i));
      set = g_hash_table_lookup (profile_defaults, name);

      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);
      _emit_modulemd_simpleset (emitter, set, YAML_FLOW_SEQUENCE_STYLE, error);
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending profile default mapping");


  result = TRUE;

error:
  g_clear_pointer (&name, g_free);
  g_clear_pointer (&keys, g_ptr_array_unref);

  g_debug ("TRACE: exiting _emit_defaults_profiles");
  return result;
}


static gboolean
_emit_defaults_intents (yaml_emitter_t *emitter,
                        ModulemdDefaults *defaults,
                        GError **error)
{
  gboolean result = FALSE;
  g_autofree gchar *name = NULL;
  yaml_event_t event;
  GHashTable *intents = NULL;
  g_autoptr (GPtrArray) keys = NULL;
  ModulemdIntent *intent = NULL;

  g_debug ("TRACE: entering _emit_defaults_intents");

  name = g_strdup ("intents");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  /* Start the map */
  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting intents mapping");

  intents = modulemd_defaults_peek_intents (defaults);
  keys = _modulemd_ordered_str_keys (intents, _modulemd_strcmp_sort);

  for (gsize i = 0; i < keys->len; i++)
    {
      name = g_strdup (g_ptr_array_index (keys, i));
      intent = g_hash_table_lookup (intents, name);
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_intent (emitter, intent, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not write out intents");
        }
    }


  /* End the map */
  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending intents mapping");

  result = TRUE;
error:
  yaml_event_delete (&event);

  g_debug ("TRACE: exiting _emit_defaults_intents");
  return result;
}


static gboolean
_emit_intent (yaml_emitter_t *emitter, ModulemdIntent *intent, GError **error)
{
  gboolean result = FALSE;
  g_autofree gchar *name = NULL;
  g_autofree gchar *value = NULL;
  yaml_event_t event;

  g_debug ("TRACE: entering _emit_intent");

  /* Start the map */
  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting intent mapping");

  /* Emit the default stream, if any */
  value = modulemd_intent_dup_default_stream (intent);
  if (value)
    {
      name = g_strdup ("stream");

      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  if (!_emit_intent_profiles (emitter, intent, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (
        error, "Could not write out intent profile defaults");
    }


  /* End the map */
  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending intents mapping");

  result = TRUE;
error:
  yaml_event_delete (&event);

  g_debug ("TRACE: exiting _emit_intent");
  return result;
}

static gboolean
_emit_intent_profiles (yaml_emitter_t *emitter,
                       ModulemdIntent *intent,
                       GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  GHashTable *profile_defaults = NULL;
  GPtrArray *keys = NULL;
  ModulemdSimpleSet *set = NULL;

  g_debug ("TRACE: entering _emit_intent_profiles");

  name = g_strdup ("profiles");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  /* Start the map */
  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting profile default mapping");

  profile_defaults = modulemd_intent_peek_profile_defaults (intent);
  keys = _modulemd_ordered_str_keys (profile_defaults, _modulemd_strcmp_sort);
  for (gsize i = 0; i < keys->len; i++)
    {
      name = g_strdup (g_ptr_array_index (keys, i));
      set = g_hash_table_lookup (profile_defaults, name);

      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);
      _emit_modulemd_simpleset (emitter, set, YAML_FLOW_SEQUENCE_STYLE, error);
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending intent profile default mapping");


  result = TRUE;

error:
  g_clear_pointer (&name, g_free);
  g_clear_pointer (&keys, g_ptr_array_unref);

  g_debug ("TRACE: exiting _emit_intent_profiles");
  return result;
}
