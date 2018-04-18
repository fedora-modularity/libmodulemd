/* modulemd-yaml-emitter-modulemd.c
 *
 * Copyright (C) 2018 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <yaml.h>
#include <errno.h>
#include <inttypes.h>
#include "modulemd.h"
#include "modulemd-yaml.h"
#include "modulemd-util.h"

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
