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
_emit_modulemd_root (yaml_emitter_t *emitter,
                     ModulemdModuleStream *modulestream,
                     GError **error);
static gboolean
_emit_modulemd_data (yaml_emitter_t *emitter,
                     ModulemdModuleStream *modulestream,
                     GError **error);
static gboolean
_emit_modulemd_servicelevels (yaml_emitter_t *emitter,
                              ModulemdModuleStream *modulestream,
                              GError **error);
static gboolean
_emit_modulemd_servicelevel_entry (yaml_emitter_t *emitter,
                                   ModulemdModuleStream *modulestream,
                                   gchar *key,
                                   ModulemdServiceLevel *sl,
                                   GError **error);
static gboolean
_emit_modulemd_licenses (yaml_emitter_t *emitter,
                         ModulemdModuleStream *modulestream,
                         GError **error);
static gboolean
_emit_modulemd_xmd (yaml_emitter_t *emitter,
                    ModulemdModuleStream *modulestream,
                    GError **error);
static gboolean
_emit_modulemd_deps_v1 (yaml_emitter_t *emitter,
                        ModulemdModuleStream *modulestream,
                        GError **error);
static gboolean
_emit_modulemd_deps_v2 (yaml_emitter_t *emitter,
                        ModulemdModuleStream *modulestream,
                        GError **error);

static gboolean
_emit_modulemd_refs (yaml_emitter_t *emitter,
                     ModulemdModuleStream *modulestream,
                     GError **error);
static gboolean
_emit_modulemd_profiles (yaml_emitter_t *emitter,
                         ModulemdModuleStream *modulestream,
                         GError **error);
static gboolean
_emit_modulemd_api (yaml_emitter_t *emitter,
                    ModulemdModuleStream *modulestream,
                    GError **error);
static gboolean
_emit_modulemd_filters (yaml_emitter_t *emitter,
                        ModulemdModuleStream *modulestream,
                        GError **error);
static gboolean
_emit_modulemd_buildopts (yaml_emitter_t *emitter,
                          ModulemdModuleStream *modulestream,
                          GError **error);

static gboolean
_emit_modulemd_rpm_buildopts (yaml_emitter_t *emitter,
                              ModulemdBuildopts *buildopts,
                              GError **error);

static gboolean
_emit_modulemd_components (yaml_emitter_t *emitter,
                           ModulemdModuleStream *modulestream,
                           GError **error);
static gboolean
_emit_modulemd_artifacts (yaml_emitter_t *emitter,
                          ModulemdModuleStream *modulestream,
                          GError **error);


gboolean
_emit_modulestream (yaml_emitter_t *emitter,
                    ModulemdModuleStream *modulestream,
                    GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;

  g_debug ("TRACE: entering _emit_modulemd");
  yaml_document_start_event_initialize (&event, NULL, NULL, NULL, 0);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting document");

  if (!_emit_modulemd_root (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to process root");
    }

  yaml_document_end_event_initialize (&event, 0);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending document");

  result = TRUE;

error:

  g_debug ("TRACE: exiting _emit_modulemd");
  return result;
}

static gboolean
_emit_modulemd_root (yaml_emitter_t *emitter,
                     ModulemdModuleStream *modulestream,
                     GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  guint64 mdversion = modulemd_modulestream_get_mdversion (modulestream);
  gchar *name = NULL;
  gchar *value = NULL;

  g_debug ("TRACE: entering _emit_modulemd_root");
  if (mdversion < 1)
    {
      /* The mdversion is required and has not been specified.
       * This module is invalid
       */
      MMD_YAML_EMITTER_ERROR_RETURN (
        error, "Module Metadata version unspecified. Module is invalid.");
    }

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting root mapping");


  /* document: modulemd */
  name = g_strdup ("document");
  value = g_strdup ("modulemd");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* The modulemd version */
  name = g_strdup ("version");
  value = g_strdup_printf ("%" PRIu64, mdversion);
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* The data */
  name = g_strdup ("data");

  yaml_scalar_event_initialize (&event,
                                NULL,
                                NULL,
                                (yaml_char_t *)name,
                                (int)strlen (name),
                                1,
                                1,
                                YAML_PLAIN_SCALAR_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error writing data");
  g_clear_pointer (&name, g_free);

  if (!_emit_modulemd_data (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit data");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending root mapping");

  result = TRUE;

error:
  g_free (name);
  g_free (value);

  g_debug ("TRACE: exiting _emit_modulemd_root");
  return result;
}

static gboolean
_emit_modulemd_data (yaml_emitter_t *emitter,
                     ModulemdModuleStream *modulestream,
                     GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  gchar *value = NULL;
  guint64 version = 0;
  const GDate *eol;

  g_debug ("TRACE: entering _emit_modulemd_data");

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting data mapping");


  /* Module name */
  value = modulemd_modulestream_get_name (modulestream);
  if (value)
    {
      name = g_strdup ("name");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }


  /* Module stream */
  value = modulemd_modulestream_get_stream (modulestream);
  if (value)
    {
      name = g_strdup ("stream");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }


  /* Module version */
  version = modulemd_modulestream_get_version (modulestream);
  if (version)
    {
      name = g_strdup ("version");
      value = g_strdup_printf ("%" PRIu64, version);
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Module Context */
  value = modulemd_modulestream_get_context (modulestream);
  if (value)
    {
      name = g_strdup ("context");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }


  /* Module Artifact Architecture */
  value = modulemd_modulestream_get_arch (modulestream);
  if (value)
    {
      name = g_strdup ("arch");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }


  /* Module summary */
  value = modulemd_modulestream_get_summary (modulestream);
  if (!value)
    {
      /* Summary is mandatory */
      MMD_YAML_EMITTER_ERROR_RETURN (error,
                                     "Missing required option data.summary");
    }
  name = g_strdup ("summary");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* Module description */
  value = modulemd_modulestream_get_description (modulestream);
  if (!value)
    {
      /* Description is mandatory */
      MMD_YAML_EMITTER_ERROR_RETURN (
        error, "Missing required option data.description");
    }
  name = g_strdup ("description");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_FOLDED_SCALAR_STYLE);

  /* Module EOL (obsolete */

  if (modulemd_modulestream_get_mdversion (modulestream) == 1)
    {
      /* EOL is removed from mdversion 2+ */
      eol = modulemd_modulestream_peek_eol (modulestream);
      if (eol)
        {
          name = g_strdup ("eol");
          value = g_strdup_printf ("%.2u-%.2u-%.2u",
                                   g_date_get_year (eol),
                                   g_date_get_month (eol),
                                   g_date_get_day (eol));
          MMD_YAML_EMIT_STR_STR_DICT (
            &event, name, value, YAML_PLAIN_SCALAR_STYLE);
        }
    }

  /* Module Service Levels */
  if (!_emit_modulemd_servicelevels (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit service levels");
    }

  /* Module Licenses */
  if (!_emit_modulemd_licenses (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit licenses");
    }


  /* Extensible Metadata Block */
  if (!_emit_modulemd_xmd (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit xmd");
    }


  /* Dependencies */
  if (modulemd_modulestream_get_mdversion (modulestream) == 1)
    {
      if (!_emit_modulemd_deps_v1 (emitter, modulestream, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit dependencies");
        }
    }
  else
    {
      if (!_emit_modulemd_deps_v2 (emitter, modulestream, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit dependencies");
        }
    }

  /* References */
  if (!_emit_modulemd_refs (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit references");
    }


  /* Profiles */
  if (!_emit_modulemd_profiles (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit references");
    }


  /* API */
  if (!_emit_modulemd_api (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit API");
    }


  /* Filters */
  if (!_emit_modulemd_filters (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit filters");
    }


  /* Build options */
  if (!_emit_modulemd_buildopts (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit buildopts");
    }


  /* Components */
  if (!_emit_modulemd_components (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit buildopts");
    }


  /* Artifacts */
  if (!_emit_modulemd_artifacts (emitter, modulestream, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit buildopts");
    }


  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending data mapping");

  result = TRUE;
error:
  g_free (name);
  g_free (value);

  g_debug ("TRACE: exiting _emit_modulemd_data");
  return result;
}

static gboolean
_emit_modulemd_servicelevels (yaml_emitter_t *emitter,
                              ModulemdModuleStream *modulestream,
                              GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  gchar *key;
  g_autoptr (GHashTable) servicelevels = NULL;
  ModulemdServiceLevel *sl = NULL;
  GPtrArray *keys = NULL;

  g_debug ("TRACE: entering _emit_modulemd_servicelevels");

  servicelevels = modulemd_modulestream_get_servicelevels (modulestream);

  if (!servicelevels || g_hash_table_size (servicelevels) < 1)
    {
      /* No profiles for this module. */
      result = TRUE;
      goto error;
    }

  name = g_strdup ("servicelevels");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting servicelevel mapping");

  keys = _modulemd_ordered_str_keys (servicelevels, _modulemd_strcmp_sort);
  for (gsize i = 0; i < keys->len; i++)
    {
      key = g_ptr_array_index (keys, i);
      sl = g_hash_table_lookup (servicelevels, key);
      if (!_emit_modulemd_servicelevel_entry (
            emitter, modulestream, key, sl, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Could not process service levels");
        }
    }
  g_clear_pointer (&keys, g_ptr_array_unref);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending reference mapping");

  result = TRUE;
error:
  g_free (name);
  if (keys)
    {
      g_ptr_array_unref (keys);
    }

  g_debug ("TRACE: exiting _emit_modulemd_servicelevels");
  return result;
}


static gboolean
_emit_modulemd_servicelevel_entry (yaml_emitter_t *emitter,
                                   ModulemdModuleStream *modulestream,
                                   gchar *key,
                                   ModulemdServiceLevel *sl,
                                   GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;

  gchar *name = NULL;
  gchar *value = NULL;
  const GDate *eol = modulemd_servicelevel_peek_eol (sl);

  if (!eol || !g_date_valid (eol))
    {
      event.type = YAML_NO_EVENT;
      MMD_YAML_ERROR_RETURN (error, "Invalid EOL date");
    }

  name = g_strdup (key);
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting servicelevel inner mapping");

  /* EOL */
  name = g_strdup ("eol");
  value = g_strdup_printf ("%.2u-%.2u-%.2u",
                           g_date_get_year (eol),
                           g_date_get_month (eol),
                           g_date_get_day (eol));
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending servicelevel inner mapping");

  result = TRUE;
error:
  g_free (name);
  g_free (value);

  return result;
}

static gboolean
_emit_modulemd_licenses (yaml_emitter_t *emitter,
                         ModulemdModuleStream *modulestream,
                         GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  g_autoptr (ModulemdSimpleSet) set = NULL;

  g_debug ("TRACE: entering _emit_modulemd_licenses");

  name = g_strdup ("license");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting license mapping");


  /* Module Licenses */
  set = modulemd_modulestream_get_module_licenses (modulestream);
  if (!set || modulemd_simpleset_size (set) < 1)
    {
      /* Module license is mandatory */
      MMD_YAML_EMITTER_ERROR_RETURN (
        error, "Missing required option data.license.module");
    }
  name = g_strdup ("module");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_modulemd_simpleset (
        emitter, set, YAML_BLOCK_SEQUENCE_STYLE, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing module licenses");
    }
  g_clear_pointer (&set, g_object_unref);


  /* Content licenses */
  set = modulemd_modulestream_get_content_licenses (modulestream);
  if (set && modulemd_simpleset_size (set) > 0)
    {
      /* Content licenses are optional */
      name = g_strdup ("content");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_simpleset (
            emitter, set, YAML_BLOCK_SEQUENCE_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error writing module licenses");
        }
    }
  g_clear_pointer (&set, g_object_unref);


  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending license mapping");

  result = TRUE;
error:
  g_free (name);

  g_debug ("TRACE: exiting _emit_modulemd_licenses");
  return result;
}

static gboolean
_emit_modulemd_xmd (yaml_emitter_t *emitter,
                    ModulemdModuleStream *modulestream,
                    GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  g_autoptr (GHashTable) htable = NULL;

  g_debug ("TRACE: entering _emit_modulemd_xmd");

  htable = modulemd_modulestream_get_xmd (modulestream);
  if (htable && g_hash_table_size (htable) > 0)
    {
      name = g_strdup ("xmd");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      /* Start the YAML mapping */
      if (!_emit_modulemd_variant_hashtable (emitter, htable, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error emitting variant hashtable");
        }
    }
  result = TRUE;
error:
  g_free (name);

  g_debug ("TRACE: exiting _emit_modulemd_xmd");
  return result;
}

static gboolean
_emit_modulemd_deps_v1 (yaml_emitter_t *emitter,
                        ModulemdModuleStream *modulestream,
                        GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  g_autoptr (GHashTable) buildrequires = NULL;
  g_autoptr (GHashTable) requires = NULL;

  g_debug ("TRACE: entering _emit_modulemd_deps_v1");

  buildrequires = modulemd_modulestream_get_buildrequires (modulestream);
  requires = modulemd_modulestream_get_requires (modulestream);
  if (!(buildrequires && g_hash_table_size (buildrequires) > 0) &&
      !(requires && g_hash_table_size (requires) > 0))
    {
      /* No dependencies for this module.
       * Unlikely, but not impossible
       */
      result = TRUE;
      goto error;
    }

  name = g_strdup ("dependencies");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting dependency mapping");

  if (buildrequires)
    {
      name = g_strdup ("buildrequires");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_hashtable (
            emitter, buildrequires, YAML_PLAIN_SCALAR_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error writing module build deps");
        }
    }

  if (requires)
    {
      name = g_strdup ("requires");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_hashtable (
            emitter, requires, YAML_PLAIN_SCALAR_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error writing module runtime deps");
        }
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending dependency mapping");

  result = TRUE;
error:
  g_free (name);

  g_debug ("TRACE: exiting _emit_modulemd_deps_v1");
  return result;
}

static gboolean
_modulemd_emit_dep_stream_mapping (yaml_emitter_t *emitter,
                                   GHashTable *reqs,
                                   GError **error);
static gboolean
_emit_modulemd_deps_v2 (yaml_emitter_t *emitter,
                        ModulemdModuleStream *modulestream,
                        GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  g_autoptr (GPtrArray) dependencies = NULL;
  ModulemdDependencies *dep = NULL;
  g_autoptr (GHashTable) reqs = NULL;

  g_debug ("TRACE: entering _emit_modulemd_deps_v2");

  dependencies = modulemd_modulestream_get_dependencies (modulestream);
  if (!(dependencies && dependencies->len > 0))
    {
      /* Unlikely, but not impossible */
      result = TRUE;
      goto error;
    }

  name = g_strdup ("dependencies");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_sequence_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_SEQUENCE_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting dependency sequence");


  for (gsize i = 0; i < dependencies->len; i++)
    {
      yaml_mapping_start_event_initialize (
        &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error starting internal dependency mapping");

      dep = MODULEMD_DEPENDENCIES (g_ptr_array_index (dependencies, i));

      /* Write out the BuildRequires first */
      reqs = modulemd_dependencies_dup_buildrequires (dep);
      if (reqs && g_hash_table_size (reqs) > 0)
        {
          name = g_strdup ("buildrequires");
          MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);
          if (!_modulemd_emit_dep_stream_mapping (emitter, reqs, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error,
                                             "Could not parse stream mapping");
            }
        }
      g_clear_pointer (&reqs, g_hash_table_unref);

      /* Then write out the Requires */
      reqs = modulemd_dependencies_dup_requires (dep);
      if (reqs && g_hash_table_size (reqs) > 0)
        {
          name = g_strdup ("requires");
          MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

          if (!_modulemd_emit_dep_stream_mapping (emitter, reqs, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error,
                                             "Could not parse stream mapping");
            }
        }
      g_clear_pointer (&reqs, g_hash_table_unref);

      yaml_mapping_end_event_initialize (&event);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error ending internal dependency mapping");
    }

  yaml_sequence_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending dependency sequence");

  result = TRUE;
error:
  g_free (name);

  g_debug ("TRACE: exiting _emit_modulemd_deps_v2");
  return result;
}

static gboolean
_modulemd_emit_dep_stream_mapping (yaml_emitter_t *emitter,
                                   GHashTable *reqs,
                                   GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  GPtrArray *keys;
  gchar *name;
  ModulemdSimpleSet *val;

  g_debug ("TRACE: entering _modulemd_emit_dep_stream_mapping");

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting dep stream mapping");

  keys = _modulemd_ordered_str_keys (reqs, _modulemd_strcmp_sort);

  for (gsize i = 0; i < keys->len; i++)
    {
      name = g_strdup (g_ptr_array_index (keys, i));
      val = g_object_ref (g_hash_table_lookup (reqs, name));

      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_simpleset (
            emitter, val, YAML_FLOW_SEQUENCE_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Could not parse stream simpleset");
        }
      g_object_unref (val);
    }
  g_ptr_array_unref (keys);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending dep stream mapping");

  result = TRUE;
error:

  g_debug ("TRACE: exiting _modulemd_emit_dep_stream_mapping");
  return result;
}


static gboolean
_emit_modulemd_refs (yaml_emitter_t *emitter,
                     ModulemdModuleStream *modulestream,
                     GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  g_autofree gchar *name = NULL;
  g_autofree gchar *community = NULL;
  g_autofree gchar *documentation = NULL;
  g_autofree gchar *tracker = NULL;

  g_debug ("TRACE: entering _emit_modulemd_refs");

  community = modulemd_modulestream_get_community (modulestream);
  documentation = modulemd_modulestream_get_documentation (modulestream);
  tracker = modulemd_modulestream_get_tracker (modulestream);

  if (!(community || documentation || tracker))
    {
      /* No references for this module. */
      result = TRUE;
      goto error;
    }

  name = g_strdup ("references");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting reference mapping");

  if (community)
    {
      name = g_strdup ("community");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, community, YAML_PLAIN_SCALAR_STYLE);
    }

  if (documentation)
    {
      name = g_strdup ("documentation");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, documentation, YAML_PLAIN_SCALAR_STYLE);
    }

  if (tracker)
    {
      name = g_strdup ("tracker");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, tracker, YAML_PLAIN_SCALAR_STYLE);
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending reference mapping");

  result = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_refs");
  return result;
}

static gboolean
_emit_modulemd_profile_entry (yaml_emitter_t *emitter,
                              ModulemdModuleStream *modulestream,
                              gchar *key,
                              ModulemdProfile *profile,
                              GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;

  g_autofree gchar *name = NULL;
  g_autofree gchar *description = NULL;
  g_autoptr (ModulemdSimpleSet) rpms = NULL;

  name = g_strdup (key);
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting profile inner mapping");

  /* Description */
  description = modulemd_profile_dup_description (profile);
  if (description)
    {
      name = g_strdup ("description");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, description, YAML_PLAIN_SCALAR_STYLE);
    }

  /* RPMs */
  rpms = modulemd_profile_dup_rpms (profile);
  if (rpms && modulemd_simpleset_size (rpms) > 0)
    {
      name = g_strdup ("rpms");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_simpleset (
            emitter, rpms, YAML_BLOCK_SEQUENCE_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing profile rpms");
        }
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending profile inner mapping");

  result = TRUE;
error:
  g_free (name);
  g_free (description);
  return result;
}

static gboolean
_emit_modulemd_profiles (yaml_emitter_t *emitter,
                         ModulemdModuleStream *modulestream,
                         GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  gchar *key;
  g_autoptr (GHashTable) profiles = NULL;
  ModulemdProfile *profile = NULL;
  GPtrArray *keys = NULL;

  g_debug ("TRACE: entering _emit_modulemd_profiles");

  profiles = modulemd_modulestream_get_profiles (modulestream);

  if (!(profiles && g_hash_table_size (profiles) > 0))
    {
      /* No profiles for this module. */
      result = TRUE;
      goto error;
    }

  name = g_strdup ("profiles");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting profile mapping");

  keys = _modulemd_ordered_str_keys (profiles, _modulemd_strcmp_sort);
  for (gsize i = 0; i < keys->len; i++)
    {
      key = g_ptr_array_index (keys, i);
      profile = g_hash_table_lookup (profiles, key);
      if (!_emit_modulemd_profile_entry (
            emitter, modulestream, key, profile, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not process profiles");
        }
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending reference mapping");

  result = TRUE;
error:
  g_clear_pointer (&keys, g_ptr_array_unref);
  g_clear_pointer (&name, g_free);

  g_debug ("TRACE: exiting _emit_modulemd_profiles");
  return result;
}

static gboolean
_emit_modulemd_api (yaml_emitter_t *emitter,
                    ModulemdModuleStream *modulestream,
                    GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  g_autofree gchar *name = NULL;
  g_autoptr (ModulemdSimpleSet) api = NULL;

  g_debug ("TRACE: entering _emit_modulemd_api");
  api = modulemd_modulestream_get_rpm_api (modulestream);

  if (!(api && modulemd_simpleset_size (api) > 0))
    {
      /* No API for this module */
      result = TRUE;
      goto error;
    }

  name = g_strdup ("api");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting API mapping");

  name = g_strdup ("rpms");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_modulemd_simpleset (
        emitter, api, YAML_BLOCK_SEQUENCE_STYLE, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing API rpms");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending API mapping");

  result = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_api");
  return result;
}

static gboolean
_emit_modulemd_filters (yaml_emitter_t *emitter,
                        ModulemdModuleStream *modulestream,
                        GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  g_autofree gchar *name = NULL;
  g_autoptr (ModulemdSimpleSet) filters = NULL;

  g_debug ("TRACE: entering _emit_modulemd_filters");
  filters = modulemd_modulestream_get_rpm_filter (modulestream);

  if (!(filters && modulemd_simpleset_size (filters) > 0))
    {
      /* No filters for this module */
      result = TRUE;
      goto error;
    }

  name = g_strdup ("filter");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting filter mapping");

  name = g_strdup ("rpms");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_modulemd_simpleset (
        emitter, filters, YAML_BLOCK_SEQUENCE_STYLE, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing filter rpms");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending filter mapping");

  result = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_filters");
  return result;
}

static gboolean
_emit_modulemd_buildopts (yaml_emitter_t *emitter,
                          ModulemdModuleStream *modulestream,
                          GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  g_autofree gchar *name = NULL;
  g_autoptr (ModulemdBuildopts) buildopts = NULL;

  g_debug ("TRACE: entering _emit_modulemd_buildopts");
  buildopts = modulemd_modulestream_get_buildopts (modulestream);
  if (!buildopts)
    {
      result = TRUE;
      goto error;
    }

  name = g_strdup ("buildopts");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting buildopt mapping");

  if (!_emit_modulemd_rpm_buildopts (emitter, buildopts, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing buildopts");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending buildopt mapping");

  result = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_buildopts");
  return result;
}

static gboolean
_emit_modulemd_rpm_buildopts (yaml_emitter_t *emitter,
                              ModulemdBuildopts *buildopts,
                              GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  g_autofree gchar *name = NULL;
  g_autofree gchar *value = NULL;
  g_autoptr (ModulemdSimpleSet) set = NULL;

  g_debug ("TRACE: entering _emit_modulemd_buildopts");

  name = g_strdup ("rpms");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting RPM buildopt mapping");

  value = modulemd_buildopts_get_rpm_macros (buildopts);
  if (value)
    {
      name = g_strdup ("macros");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_LITERAL_SCALAR_STYLE);
    }

  set = modulemd_buildopts_get_rpm_whitelist_simpleset (buildopts);

  if (set)
    {
      name = g_strdup ("whitelist");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);
      if (!_emit_modulemd_simpleset (
            emitter, set, YAML_BLOCK_SEQUENCE_STYLE, error))
        {
          MMD_YAML_EMITTER_ERROR_RETURN (
            error, "Could not emit RPM buildopt whitelist");
        }
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending RPM buildopt mapping");


  result = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_buildopts");
  return result;
}

static gboolean
_emit_modulemd_rpm_components (yaml_emitter_t *emitter,
                               ModulemdModuleStream *modulestream,
                               gchar *key,
                               ModulemdComponentRpm *rpm_component,
                               GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  g_autofree gchar *value = NULL;
  guint64 buildorder;
  ModulemdSimpleSet *set = NULL;

  gchar *name = g_strdup (key);

  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting RPM component inner mapping");

  /* Rationale */
  value =
    modulemd_component_dup_rationale ((MODULEMD_COMPONENT (rpm_component)));
  if (!value)
    {
      MMD_YAML_EMITTER_ERROR_RETURN (error,
                                     "Missing required option: rationale");
    }
  name = g_strdup ("rationale");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  /* Repository */
  value = modulemd_component_rpm_dup_repository (rpm_component);
  if (value)
    {
      name = g_strdup ("repository");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Cache */
  value = modulemd_component_rpm_dup_cache (rpm_component);
  if (value)
    {
      name = g_strdup ("cache");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Ref */
  value = modulemd_component_rpm_dup_ref (rpm_component);
  if (value)
    {
      name = g_strdup ("ref");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Buildorder */
  buildorder =
    modulemd_component_get_buildorder (MODULEMD_COMPONENT (rpm_component));
  if (buildorder)
    {
      name = g_strdup ("buildorder");
      value = g_strdup_printf ("%" PRIu64, buildorder);
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Arches */
  set = modulemd_component_rpm_get_arches (rpm_component);
  if (set && modulemd_simpleset_size (set) > 0)
    {
      name = g_strdup ("arches");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_simpleset (
            emitter, set, YAML_FLOW_SEQUENCE_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error writing RPM component arches");
        }
    }


  /* Multilib */
  set = modulemd_component_rpm_get_multilib (rpm_component);
  if (set && modulemd_simpleset_size (set) > 0)
    {
      name = g_strdup ("multilib");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_simpleset (
            emitter, set, YAML_FLOW_SEQUENCE_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (
            error, "Error writing RPM component multilib");
        }
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending RPM component inner mapping");

  result = TRUE;
error:
  return result;
}

static gboolean
_emit_modulemd_module_components (yaml_emitter_t *emitter,
                                  ModulemdModuleStream *modulestream,
                                  gchar *key,
                                  ModulemdComponentModule *module_component,
                                  GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  g_autofree gchar *name = NULL;
  g_autofree gchar *value = NULL;
  guint64 buildorder;

  name = g_strdup (key);

  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting module component inner mapping");

  /* Rationale */
  value =
    modulemd_component_dup_rationale ((MODULEMD_COMPONENT (module_component)));
  if (!value)
    {
      MMD_YAML_EMITTER_ERROR_RETURN (error,
                                     "Missing required option: rationale");
    }
  name = g_strdup ("rationale");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  /* Repository */
  value = modulemd_component_module_dup_repository (module_component);
  if (value)
    {
      name = g_strdup ("repository");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Ref */
  value = modulemd_component_module_dup_repository (module_component);
  if (value)
    {
      name = g_strdup ("ref");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Buildorder */
  buildorder =
    modulemd_component_get_buildorder (MODULEMD_COMPONENT (module_component));
  if (buildorder)
    {
      name = g_strdup ("buildorder");
      value = g_strdup_printf ("%" PRIu64, buildorder);
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending module component inner mapping");

  result = TRUE;
error:

  return result;
}

static gboolean
_emit_modulemd_components (yaml_emitter_t *emitter,
                           ModulemdModuleStream *modulestream,
                           GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  g_autofree gchar *name = NULL;
  gchar *key;
  ModulemdComponentRpm *rpm_component;
  ModulemdComponentModule *module_component;
  g_autoptr (GHashTable) rpm_components;
  g_autoptr (GHashTable) module_components;
  g_autoptr (GPtrArray) rpm_keys = NULL;
  g_autoptr (GPtrArray) module_keys = NULL;
  gsize i;

  g_debug ("TRACE: entering _emit_modulemd_components");

  rpm_components = modulemd_modulestream_get_rpm_components (modulestream);
  if (rpm_components && g_hash_table_size (rpm_components) < 1)
    {
      g_clear_pointer (&rpm_components, g_hash_table_unref);
    }

  module_components =
    modulemd_modulestream_get_module_components (modulestream);
  if (module_components && g_hash_table_size (module_components) < 1)
    {
      g_clear_pointer (&module_components, g_hash_table_unref);
    }

  if (!(rpm_components || module_components))
    {
      /* Omit this section */
      result = TRUE;
      goto error;
    }

  name = g_strdup ("components");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting component mapping");

  if (rpm_components)
    {
      name = g_strdup ("rpms");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      yaml_mapping_start_event_initialize (
        &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error starting RPM component mapping");

      rpm_keys =
        _modulemd_ordered_str_keys (rpm_components, _modulemd_strcmp_sort);
      for (i = 0; i < rpm_keys->len; i++)
        {
          key = g_ptr_array_index (rpm_keys, i);
          rpm_component = g_hash_table_lookup (rpm_components, key);
          if (!_emit_modulemd_rpm_components (
                emitter, modulestream, key, rpm_component, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (
                error, "Could not process RPM components");
            }
        }

      yaml_mapping_end_event_initialize (&event);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error ending RPM component mapping");
    }

  if (module_components)
    {
      name = g_strdup ("modules");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      yaml_mapping_start_event_initialize (
        &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error starting module component mapping");

      module_keys =
        _modulemd_ordered_str_keys (module_components, _modulemd_strcmp_sort);
      for (i = 0; i < module_keys->len; i++)
        {
          key = g_ptr_array_index (module_keys, i);
          module_component = g_hash_table_lookup (module_components, key);
          if (!_emit_modulemd_module_components (
                emitter, modulestream, key, module_component, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (
                error, "Could not process module components");
            }
        }

      yaml_mapping_end_event_initialize (&event);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error ending module component mapping");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending component mapping");

  result = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_components");
  return result;
}

static gboolean
_emit_modulemd_artifacts (yaml_emitter_t *emitter,
                          ModulemdModuleStream *modulestream,
                          GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  g_autofree gchar *name = NULL;
  g_autoptr (ModulemdSimpleSet) artifacts = NULL;

  g_debug ("TRACE: entering _emit_modulemd_artifacts");

  artifacts = modulemd_modulestream_get_rpm_artifacts (modulestream);
  if (!(artifacts && modulemd_simpleset_size (artifacts) > 0))
    {
      /* No artifacts for this module */
      result = TRUE;
      goto error;
    }

  name = g_strdup ("artifacts");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting artifact mapping");

  name = g_strdup ("rpms");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_modulemd_simpleset (
        emitter, artifacts, YAML_BLOCK_SEQUENCE_STYLE, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing artifact rpms");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending artifact mapping");

  result = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_artifacts");
  return result;
}
