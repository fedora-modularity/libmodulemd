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
#include "private/modulemd-private.h"
#include "private/modulemd-yaml.h"
#include "private/modulemd-util.h"


enum ModulemdReqType
{
  MODULEMD_REQ_REQUIRES,
  MODULEMD_REQ_BUILDREQUIRES
};

#define _yaml_parser_modulemd_recurse_down(fn)                                \
  do                                                                          \
    {                                                                         \
      if (!fn (modulestream, parser, error))                                  \
        {                                                                     \
          goto error;                                                         \
        }                                                                     \
    }                                                                         \
  while (0)

static gboolean
_parse_modulemd_data (ModulemdModuleStream *modulestream,
                      yaml_parser_t *parser,
                      GError **error);
static gboolean
_parse_modulemd_licenses (ModulemdModuleStream *modulestream,
                          yaml_parser_t *parser,
                          GError **error);
static gboolean
_parse_modulemd_xmd (ModulemdModuleStream *modulestream,
                     yaml_parser_t *parser,
                     GError **error);
static gboolean
_parse_modulemd_deps (ModulemdModuleStream *modulestream,
                      yaml_parser_t *parser,
                      GError **error);
static gboolean
_parse_modulemd_refs (ModulemdModuleStream *modulestream,
                      yaml_parser_t *parser,
                      GError **error);
static gboolean
_parse_modulemd_profiles (ModulemdModuleStream *modulestream,
                          yaml_parser_t *parser,
                          GError **error);
static gboolean
_parse_modulemd_profile (yaml_parser_t *parser,
                         const gchar *name,
                         ModulemdProfile **_profile,
                         GError **error);
static gboolean
_parse_modulemd_api (ModulemdModuleStream *modulestream,
                     yaml_parser_t *parser,
                     GError **error);
static gboolean
_parse_modulemd_filters (ModulemdModuleStream *modulestream,
                         yaml_parser_t *parser,
                         GError **error);
static gboolean
_parse_modulemd_buildopts (ModulemdModuleStream *modulestream,
                           yaml_parser_t *parser,
                           GError **error);
static gboolean
_parse_modulemd_rpm_buildopts (ModulemdBuildopts *buildopts,
                               yaml_parser_t *parser,
                               GError **error);
static gboolean
_parse_modulemd_components (ModulemdModuleStream *modulestream,
                            yaml_parser_t *parser,
                            GError **error);
static gboolean
_parse_modulemd_rpm_components (yaml_parser_t *parser,
                                GHashTable **_components,
                                GError **error);
static gboolean
_parse_modulemd_rpm_component (yaml_parser_t *parser,
                               const gchar *name,
                               ModulemdComponentRpm **_components,
                               GError **error);
static gboolean
_parse_modulemd_modulestream_components (yaml_parser_t *parser,
                                         GHashTable **_components,
                                         GError **error);
static gboolean
_parse_modulemd_modulestream_component (yaml_parser_t *parser,
                                        const gchar *name,
                                        ModulemdComponentModule **_components,
                                        GError **error);
static gboolean
_parse_modulemd_artifacts (ModulemdModuleStream *modulestream,
                           yaml_parser_t *parser,
                           GError **error);
static gboolean
_parse_modulemd_servicelevels (ModulemdModuleStream *modulestream,
                               yaml_parser_t *parser,
                               GError **error);
static gboolean
_parse_modulemd_servicelevel (yaml_parser_t *parser,
                              const gchar *name,
                              ModulemdServiceLevel **_servicelevel,
                              GError **error);

gboolean
_parse_module_stream (yaml_parser_t *parser,
                      GObject **object,
                      guint64 version,
                      GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  gboolean result = FALSE;
  guint64 mdversion;
  g_autoptr (ModulemdModuleStream) modulestream = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_module_stream");

  modulestream = modulemd_modulestream_new ();

  /* Use the pre-processed mdversion */
  if (version && version <= MD_VERSION_LATEST)
    {
      modulemd_modulestream_set_mdversion (modulestream, version);
    }
  else
    {
      /* No mdversion was discovered during pre-processing */
      MMD_YAML_ERROR_RETURN (error, "Unknown modulemd version");
    }

  /* Parse until the end of this document */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the main document content. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* This is the end of the main document content. */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:

          /* Handle "document: modulemd" */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "document"))
            {
              g_debug ("TRACE: root entry [document]");
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT ||
                  g_strcmp0 ((const gchar *)value_event.data.scalar.value,
                             "modulemd"))
                {
                  MMD_YAML_ERROR_RETURN (error, "Unknown document type");
                }
              yaml_event_delete (&value_event);
            }

          /* Record the modulemd version for the parser */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "version"))
            {
              g_debug ("TRACE: root entry [mdversion]");
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Unknown modulemd version");
                }

              mdversion = g_ascii_strtoull (
                (const gchar *)value_event.data.scalar.value, NULL, 10);
              if (!mdversion)
                {
                  MMD_YAML_ERROR_RETURN (error, "Unknown modulemd version");
                }
              yaml_event_delete (&value_event);

              if (mdversion != version)
                {
                  /* Preprocessing and real parser don't match!
                   * This should be impossible
                   */
                  MMD_YAML_ERROR_RETURN (
                    error, "ModuleMD version doesn't match preprocessing");
                }
              modulemd_modulestream_set_mdversion (modulestream, mdversion);
            }

          /* Process the data section */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "data"))
            {
              g_debug ("TRACE: root entry [data]");
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_data);
            }

          else
            {
              g_debug ("Unexpected key in root: %s",
                       (const gchar *)event.data.scalar.value);
              MMD_YAML_ERROR_RETURN (error, "Unexpected key in root");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in root");
          break;
        }
      yaml_event_delete (&event);
    }

  result = TRUE;
  *object = g_object_ref ((GObject *)modulestream);

error:
  g_debug ("TRACE: exiting _parse_module_stream");
  return result;
}

static gboolean
_parse_modulemd_data (ModulemdModuleStream *modulestream,
                      yaml_parser_t *parser,
                      GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  guint64 version;
  GDate *eol = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_modulemd_data");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the data content. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* This is the end of the data content. */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Module name */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "name"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse module name");
                }

              modulemd_modulestream_set_name (
                modulestream, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }

          /* Module stream */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "stream"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module stream");
                }

              modulemd_modulestream_set_stream (
                modulestream, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }

          /* Module version */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "version"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module version");
                }

              version = g_ascii_strtoull (
                (const gchar *)value_event.data.scalar.value, NULL, 10);
              if (!version)
                {
                  MMD_YAML_ERROR_RETURN (error, "Unknown module version");
                }

              modulemd_modulestream_set_version (modulestream, version);
              yaml_event_delete (&value_event);
            }

          /* Module Context */

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "context"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module context");
                }

              modulemd_modulestream_set_context (
                modulestream, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }

          /* Module Artifact Architecture */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "arch"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (
                    error, "Failed to parse module artifact architecture");
                }

              modulemd_modulestream_set_arch (
                modulestream, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          /* Module summary */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "summary"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module summary");
                }

              modulemd_modulestream_set_summary (
                modulestream, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          /* Module description */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "description"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module description");
                }

              modulemd_modulestream_set_description (
                modulestream, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          /* Module EOL (obsolete) */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "eol"))
            {
              if (modulemd_modulestream_get_mdversion (modulestream) >
                  MD_VERSION_1)
                {
                  /* EOL is not supported in v2 or later; use servicelevel */
                  MMD_YAML_ERROR_RETURN (
                    error,
                    "EOL is not supported in v2 or later; use servicelevel");
                }

              /* Get the EOL date */
              if (!_parse_modulemd_date (parser, &eol, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Failed to parse module EOL date");
                }

              modulemd_modulestream_set_eol (modulestream, eol);
              g_date_free (eol);
            }

          /* Service Levels */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "servicelevels"))
            {
              _yaml_parser_modulemd_recurse_down (
                _parse_modulemd_servicelevels);
            }

          /* licenses */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "license"))
            {
              /* Process the module and content licenses */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_licenses);
            }

          /* xmd */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "xmd"))
            {
              /* Process the extensible metadata block */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_xmd);
            }

          /* dependencies */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "dependencies"))
            {
              /* Process the build and runtime dependencies of this module */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_deps);
            }

          /* references */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "references"))
            {
              /* Process the reference links for this module */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_refs);
            }

          /* profiles */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "profiles"))
            {
              /* Process the install profiles for this module */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_profiles);
            }

          /* api */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "api"))
            {
              /* Process the API list */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_api);
            }

          /* filter */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "filter"))
            {
              /* Process the filtered-out output components */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_filters);
            }

          /* buildopts */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "buildopts"))
            {
              /* Process special build options for this module */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_buildopts);
            }

          /* Components */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "components"))
            {
              /* Process the components that comprise this module */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_components);
            }

          /* Artifacts */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "artifacts"))
            {
              /* Process the output artifacts of this module */
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_artifacts);
            }

          else
            {
              g_debug ("Unexpected key in data: %s",
                       (const gchar *)event.data.scalar.value);
              MMD_YAML_ERROR_RETURN (error, "Unexpected key in data");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in data");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;

error:

  g_debug ("TRACE: exiting _parse_modulemd_data");
  return result;
}

static gboolean
_parse_modulemd_licenses (ModulemdModuleStream *modulestream,
                          yaml_parser_t *parser,
                          GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  gboolean result = FALSE;
  gboolean done = FALSE;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_licenses");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the license content. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the license content */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Each scalar event represents a license type */
          if (!_simpleset_from_sequence (parser, &set, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Invalid sequence");
            }

          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "module"))
            {
              modulemd_modulestream_set_module_licenses (modulestream, set);
            }
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "content"))
            {
              modulemd_modulestream_set_content_licenses (modulestream, set);
            }
          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unknown license type");
            }

          g_clear_pointer (&set, g_object_unref);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in licenses");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;
error:
  g_clear_pointer (&set, g_object_unref);

  g_debug ("TRACE: exiting _parse_modulemd_licenses");
  return result;
}

static gboolean
_parse_modulemd_xmd (ModulemdModuleStream *modulestream,
                     yaml_parser_t *parser,
                     GError **error)
{
  gboolean result = FALSE;
  g_autoptr (GHashTable) xmd = NULL;
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (GVariant) variant = NULL;
  GVariantIter iter;
  gchar *key;
  GVariant *value;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_modulemd_xmd");

  YAML_PARSER_PARSE_WITH_ERROR_RETURN (parser, &event, error, "Parser error");
  if (!(event.type == YAML_MAPPING_START_EVENT))
    {
      MMD_YAML_ERROR_RETURN (error, "Invalid mapping");
    }
  yaml_event_delete (&event);

  if (!parse_raw_yaml_mapping (parser, &variant, error))
    {
      MMD_YAML_ERROR_RETURN (error, "Invalid raw mapping");
    }

  if (!g_variant_is_of_type (variant, G_VARIANT_TYPE_DICTIONARY))
    {
      MMD_YAML_ERROR_RETURN (error, "XMD wasn't a dictionary");
    }

  xmd = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_variant_unref);
  g_variant_iter_init (&iter, variant);
  while (g_variant_iter_next (&iter, "{sv}", &key, &value))
    {
      g_hash_table_insert (xmd, g_strdup (key), value);
      g_free (key);
    }

  /* Save this hash table as the xmd property */
  modulemd_modulestream_set_xmd (modulestream, xmd);

  result = TRUE;

error:
  g_debug ("TRACE: exiting _parse_modulemd_xmd");
  return result;
}


static gboolean
_parse_modulemd_deps_v1 (ModulemdModuleStream *modulestream,
                         yaml_parser_t *parser,
                         GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  GHashTable *reqs = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_deps_v1");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the dependency content. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the dependency content */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!_hashtable_from_mapping (parser, &reqs, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Invalid mapping");
            }

          if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                          "buildrequires"))
            {
              modulemd_modulestream_set_buildrequires (modulestream, reqs);
            }
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "requires"))
            {
              modulemd_modulestream_set_requires (modulestream, reqs);
            }
          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unknown dependency type");
            }

          g_clear_pointer (&reqs, g_hash_table_unref);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in deps");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;
error:
  g_clear_pointer (&reqs, g_hash_table_unref);

  g_debug ("TRACE: exiting _parse_modulemd_deps_v1");
  return result;
}


static gboolean
_parse_modulemd_v2_dep (ModulemdModuleStream *modulestream,
                        yaml_parser_t *parser,
                        GError **error);

static gboolean
_parse_modulemd_deps_v2 (ModulemdModuleStream *modulestream,
                         yaml_parser_t *parser,
                         GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_deps_v2");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_SEQUENCE_START_EVENT:
          /* This is the start of the dependency content. */
          break;

        case YAML_SEQUENCE_END_EVENT:
          /* We're done processing the dependency content */
          done = TRUE;
          break;

        case YAML_MAPPING_START_EVENT:
          if (!_parse_modulemd_v2_dep (modulestream, parser, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (
                error, "Failed to parse requires/buildrequires");
              break;
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in deps");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;

error:

  g_debug ("TRACE: exiting _parse_modulemd_deps_v2");
  return result;
}


static gboolean
_parse_modulemd_v2_dep_map (ModulemdModuleStream *modulestream,
                            yaml_parser_t *parser,
                            enum ModulemdReqType reqtype,
                            ModulemdDependencies *dep,
                            GError **error);

static gboolean
_parse_modulemd_v2_dep (ModulemdModuleStream *modulestream,
                        yaml_parser_t *parser,
                        GError **error)
{
  gboolean result = FALSE;
  gboolean done = FALSE;
  MMD_INIT_YAML_EVENT (event);
  enum ModulemdReqType reqtype;
  g_autoptr (ModulemdDependencies) dep = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_v2_dep");

  dep = modulemd_dependencies_new ();
  if (dep == NULL)
    {
      event.type = YAML_NO_EVENT;
      MMD_YAML_ERROR_RETURN (
        error, "Could not allocate new Modulemd.Dependencies object");
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT:
          /* We've processed the whole map */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (g_strcmp0 ((const gchar *)event.data.scalar.value,
                         "buildrequires") == 0)
            {
              reqtype = MODULEMD_REQ_BUILDREQUIRES;
            }
          else if (g_strcmp0 ((const gchar *)event.data.scalar.value,
                              "requires") == 0)
            {
              reqtype = MODULEMD_REQ_REQUIRES;
            }
          else
            {
              MMD_YAML_ERROR_RETURN (error,
                                     "Dependency map had key other than "
                                     "'requires' or 'buildrequires'");
            }

          if (!_parse_modulemd_v2_dep_map (
                modulestream, parser, reqtype, dep, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (
                error, "Error processing dependency map.");
            }

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in v2_dep");
          break;
        }

      yaml_event_delete (&event);
    }

  modulemd_modulestream_add_dependencies (modulestream, dep);


  result = TRUE;
error:
  g_debug ("TRACE: exiting _parse_modulemd_v2_dep");
  return result;
}


static gboolean
_parse_modulemd_v2_dep_map (ModulemdModuleStream *modulestream,
                            yaml_parser_t *parser,
                            enum ModulemdReqType reqtype,
                            ModulemdDependencies *dep,
                            GError **error)
{
  gboolean result = FALSE;
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gchar *module_name = NULL;
  ModulemdSimpleSet *set = NULL;
  const gchar **dep_set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_v2_dep_map");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");
      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* Start processing the available modules and streams */
          in_map = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          /* We've received them all */
          done = TRUE;
          in_map = FALSE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              MMD_YAML_ERROR_RETURN (error,
                                     "Unexpected YAML event in v2_dep_map");
            }

          module_name = g_strdup ((const gchar *)event.data.scalar.value);

          if (!_simpleset_from_sequence (parser, &set, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error,
                                             "Could not parse set of streams");
            }
          dep_set = (const gchar **)modulemd_simpleset_dup (set);

          switch (reqtype)
            {
            case MODULEMD_REQ_BUILDREQUIRES:
              modulemd_dependencies_add_buildrequires (
                dep, module_name, dep_set);
              break;
            case MODULEMD_REQ_REQUIRES:
              modulemd_dependencies_add_requires (dep, module_name, dep_set);
              break;
            }

          g_clear_pointer (&module_name, g_free);

          for (gsize i = 0; i < modulemd_simpleset_size (set); i++)
            {
              g_clear_pointer (&dep_set[i], g_free);
            }
          g_clear_pointer (&dep_set, g_free);
          g_clear_pointer (&set, g_object_unref);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in v2_dep_map");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;
error:
  g_clear_pointer (&module_name, g_free);
  g_debug ("TRACE: exiting _parse_modulemd_v2_dep_map");
  return result;
}


static gboolean
_parse_modulemd_deps (ModulemdModuleStream *modulestream,
                      yaml_parser_t *parser,
                      GError **error)
{
  gboolean result = FALSE;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_deps");

  if (modulemd_modulestream_get_mdversion (modulestream) == MD_VERSION_1)
    {
      result = _parse_modulemd_deps_v1 (modulestream, parser, error);
      goto error;
    }
  else if (modulemd_modulestream_get_mdversion (modulestream) >= MD_VERSION_2)
    {
      result = _parse_modulemd_deps_v2 (modulestream, parser, error);
      goto error;
    }
  else
    {
      MMD_YAML_NOEVENT_ERROR_RETURN (error, "Incompatible modulemd version");
    }

error:
  g_debug ("TRACE: exiting _parse_modulemd_deps");
  return result;
}


static gboolean
_parse_modulemd_refs (ModulemdModuleStream *modulestream,
                      yaml_parser_t *parser,
                      GError **error)
{
  gboolean result = FALSE;
  GHashTable *refs = NULL;
  gpointer value;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_refs");

  if (!_hashtable_from_mapping (parser, &refs, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Invalid mapping");
    }

  if ((value = g_hash_table_lookup (refs, "community")))
    {
      modulemd_modulestream_set_community (modulestream, (const gchar *)value);
      g_hash_table_remove (refs, "community");
    }

  if ((value = g_hash_table_lookup (refs, "documentation")))
    {
      modulemd_modulestream_set_documentation (modulestream,
                                               (const gchar *)value);
      g_hash_table_remove (refs, "documentation");
    }

  if ((value = g_hash_table_lookup (refs, "tracker")))
    {
      modulemd_modulestream_set_tracker (modulestream, (const gchar *)value);
      g_hash_table_remove (refs, "tracker");
    }

  /* Make sure there were no other entries */
  if (g_hash_table_size (refs) > 0)
    {
      MMD_YAML_ERROR_RETURN (error, "Unexpected key found in references.");
    }

  result = TRUE;

error:
  g_clear_pointer (&refs, g_hash_table_unref);
  g_debug ("TRACE: exiting _parse_modulemd_refs");
  return result;
}

static gboolean
_parse_modulemd_profiles (ModulemdModuleStream *modulestream,
                          yaml_parser_t *parser,
                          GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  GHashTable *profiles = NULL;
  gchar *name = NULL;
  ModulemdProfile *profile = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_profiles");

  profiles =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the profiles. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the profiles */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Each entry is the key for a dictionary of ModulemdProfile
           * objects
           */
          name = g_strdup ((const gchar *)event.data.scalar.value);
          if (!_parse_modulemd_profile (parser, name, &profile, error))
            {
              g_free (name);
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Invalid profile");
            }
          g_hash_table_insert (profiles, name, profile);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in profiles");
          break;
        }

      yaml_event_delete (&event);
    }
  modulemd_modulestream_set_profiles (modulestream, profiles);

  result = TRUE;

error:
  g_hash_table_unref (profiles);

  g_debug ("TRACE: exiting _parse_modulemd_profiles");
  return result;
}

static gboolean
_parse_modulemd_profile (yaml_parser_t *parser,
                         const gchar *name,
                         ModulemdProfile **_profile,
                         GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  ModulemdSimpleSet *set = NULL;
  ModulemdProfile *profile = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_profile");

  profile = modulemd_profile_new ();
  modulemd_profile_set_name (profile, name);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the profile content. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the profile content */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Each entry must be one of "rpms" or "description" */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "rpms"))
            {
              /* Get the set of RPMs */
              if (!_simpleset_from_sequence (parser, &set, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Could not parse profile RPMs");
                }
              modulemd_profile_set_rpms (profile, set);
              g_object_unref (set);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "description"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "No value for description");
                }

              modulemd_profile_set_description (
                profile, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }
          else
            {
              /* Unknown field in profile */
              MMD_YAML_ERROR_RETURN (error, "Unknown key in profile body");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in profiles");
          break;
        }

      yaml_event_delete (&event);
    }

  *_profile = g_object_ref (profile);

  result = TRUE;

error:
  g_object_unref (profile);

  g_debug ("TRACE: exiting _parse_modulemd_profile");
  return result;
}

static gboolean
_parse_modulemd_api (ModulemdModuleStream *modulestream,
                     yaml_parser_t *parser,
                     GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_api");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the API. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the API */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Currently, we only support "rpms" here */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "rpms"))
            {
              if (!_simpleset_from_sequence (parser, &set, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (error, "Parse error in API");
                }
              modulemd_modulestream_set_rpm_api (modulestream, set);
            }
          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unknown API type");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in api");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;

error:
  g_object_unref (set);

  g_debug ("TRACE: exiting _parse_modulemd_api");
  return result;
}

static gboolean
_parse_modulemd_filters (ModulemdModuleStream *modulestream,
                         yaml_parser_t *parser,
                         GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_filters");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the filters. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the filters */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Currently, we only support "rpms" here */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "rpms"))
            {
              if (!_simpleset_from_sequence (parser, &set, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (error,
                                                 "Parse error in filters");
                }
              modulemd_modulestream_set_rpm_filter (modulestream, set);
            }
          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unknown filter type");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in filters");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;

error:
  g_clear_pointer (&set, g_object_unref);

  g_debug ("TRACE: exiting _parse_modulemd_filters");
  return result;
}

static gboolean
_parse_modulemd_buildopts (ModulemdModuleStream *modulestream,
                           yaml_parser_t *parser,
                           GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (ModulemdBuildopts) buildopts = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_buildopts");

  buildopts = modulemd_buildopts_new ();

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the buildopts. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the buildopts */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Currently, we only support "rpms" here */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "rpms"))
            {
              if (!_parse_modulemd_rpm_buildopts (buildopts, parser, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Parse error in RPM buildopts");
                }
            }
          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unknown buildopt type");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in buildopts");
          break;
        }

      yaml_event_delete (&event);
    }

  modulemd_modulestream_set_buildopts (modulestream, buildopts);

  result = TRUE;

error:

  g_debug ("TRACE: exiting _parse_modulemd_buildopts");
  return result;
}

static gboolean
_parse_modulemd_rpm_buildopts (ModulemdBuildopts *buildopts,
                               yaml_parser_t *parser,
                               GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  gboolean in_mapping = FALSE;
  g_autoptr (ModulemdSimpleSet) set = NULL;


  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_rpm_buildopts");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the RPM buildopts. */
          in_mapping = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the RPM buildopts */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_mapping)
            {
              MMD_YAML_ERROR_RETURN (
                error, "Received a scalar when a map was expected.");
              break;
            }

          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "macros"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse RPM macros");
                }
              modulemd_buildopts_set_rpm_macros (
                buildopts, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "whitelist"))
            {
              if (!_simpleset_from_sequence (parser, &set, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Parse error in RPM whitelist");
                }
              modulemd_buildopts_set_rpm_whitelist_simpleset (buildopts, set);
            }
          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unknown RPM buildopt key");
            }

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error,
                                 "Unexpected YAML event in RPM buildopts");
          break;
        }
      yaml_event_delete (&event);
    }

  result = TRUE;
error:

  g_debug ("TRACE: exiting _parse_modulemd_rpm_buildopts");
  return result;
}

static gboolean
_parse_modulemd_components (ModulemdModuleStream *modulestream,
                            yaml_parser_t *parser,
                            GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  GHashTable *components = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_components");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the component content. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the component content */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Each key is a type of component */
          g_debug ("Component type: %s",
                   (const gchar *)event.data.scalar.value);
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "rpms"))
            {
              if (!_parse_modulemd_rpm_components (parser, &components, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Could not parse RPM components");
                }
              modulemd_modulestream_set_rpm_components (modulestream,
                                                        components);
              g_hash_table_unref (components);
            }
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "modules"))
            {
              if (!_parse_modulemd_modulestream_components (
                    parser, &components, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Could not parse module components");
                }
              modulemd_modulestream_set_module_components (modulestream,
                                                           components);
              g_hash_table_unref (components);
            }
          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unknown component type");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in components");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;

error:

  g_debug ("TRACE: exiting _parse_modulemd_components");
  return result;
}

static gboolean
_parse_modulemd_rpm_components (yaml_parser_t *parser,
                                GHashTable **_components,
                                GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  GHashTable *components = NULL;
  gchar *name = NULL;
  ModulemdComponentRpm *component = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_modulemd_rpm_components");

  components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* The dictionary has begun */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We've processed the whole dictionary */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          name = g_strdup ((const gchar *)event.data.scalar.value);
          if (!_parse_modulemd_rpm_component (parser, name, &component, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error,
                                             "Parse error in RPM component");
            }

          /* Set this key and value to the hash table */
          g_hash_table_insert (components, name, component);

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in sequence");
          break;
        }

      yaml_event_delete (&event);
    }
  *_components = g_hash_table_ref (components);

  result = TRUE;

error:
  g_hash_table_unref (components);

  g_debug ("TRACE: exiting _parse_modulemd_rpm_components");
  return result;
}

static gboolean
_parse_modulemd_rpm_component (yaml_parser_t *parser,
                               const gchar *name,
                               ModulemdComponentRpm **_component,
                               GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  ModulemdComponentRpm *component = NULL;
  ModulemdSimpleSet *set = NULL;
  guint64 buildorder = 0;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_modulemd_rpm_component");

  component = modulemd_component_rpm_new ();
  modulemd_component_set_name (MODULEMD_COMPONENT (component), name);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* The dictionary has begun */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We've processed the whole dictionary */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                          "buildorder"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse buildorder value");
                }

              buildorder = g_ascii_strtoull (
                (const gchar *)value_event.data.scalar.value, NULL, 10);
              modulemd_component_set_buildorder (
                MODULEMD_COMPONENT (component), buildorder);

              yaml_event_delete (&value_event);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "rationale"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse rationale value");
                }

              modulemd_component_set_rationale (
                MODULEMD_COMPONENT (component),
                (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "arches"))
            {
              if (!_simpleset_from_sequence (parser, &set, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Error parsing component arches");
                }
              modulemd_component_rpm_set_arches (component, set);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "cache"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse cache value");
                }

              modulemd_component_rpm_set_cache (
                component, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "multilib"))
            {
              if (!_simpleset_from_sequence (parser, &set, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Error parsing multilib arches");
                }
              modulemd_component_rpm_set_multilib (component, set);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "ref"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse ref value");
                }

              modulemd_component_rpm_set_ref (
                component, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "repository"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse repository value");
                }

              modulemd_component_rpm_set_repository (
                component, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unexpected key in component");
            }

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in component");
          break;
        }

      g_clear_pointer (&set, g_object_unref);
      yaml_event_delete (&event);
    }
  *_component = g_object_ref (component);

  result = TRUE;

error:
  g_object_unref (component);

  g_debug ("TRACE: exiting _parse_modulemd_modulestream_components");
  return result;
}

static gboolean
_parse_modulemd_modulestream_components (yaml_parser_t *parser,
                                         GHashTable **_components,
                                         GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  GHashTable *components = NULL;
  gchar *name = NULL;
  ModulemdComponentModule *component = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_modulemd_modulestream_components");

  components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* The dictionary has begun */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We've processed the whole dictionary */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          name = g_strdup ((const gchar *)event.data.scalar.value);
          if (!_parse_modulemd_modulestream_component (
                parser, name, &component, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (
                error, "Parse error in module component");
            }

          /* Set this key and value to the hash table */
          g_hash_table_insert (components, name, component);

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in sequence");
          break;
        }

      yaml_event_delete (&event);
    }
  *_components = g_hash_table_ref (components);

  result = TRUE;

error:
  g_hash_table_unref (components);

  g_debug ("TRACE: exiting _parse_modulemd_modulestream_components");
  return result;
}

static gboolean
_parse_modulemd_modulestream_component (yaml_parser_t *parser,
                                        const gchar *name,
                                        ModulemdComponentModule **_component,
                                        GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  ModulemdComponentModule *component = NULL;
  guint64 buildorder = 0;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_modulemd_rpm_component");

  component = modulemd_component_module_new ();
  modulemd_component_set_name (MODULEMD_COMPONENT (component), name);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* The dictionary has begun */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We've processed the whole dictionary */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                          "buildorder"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse buildorder value");
                }

              buildorder = g_ascii_strtoull (
                (const gchar *)value_event.data.scalar.value, NULL, 10);
              modulemd_component_set_buildorder (
                MODULEMD_COMPONENT (component), buildorder);

              yaml_event_delete (&value_event);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "rationale"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse rationale value");
                }

              modulemd_component_set_rationale (
                MODULEMD_COMPONENT (component),
                (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "ref"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse ref value");
                }

              modulemd_component_module_set_ref (
                component, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "repository"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse repository value");
                }

              modulemd_component_module_set_repository (
                component, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unexpected key in component");
            }

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in component");
          break;
        }

      yaml_event_delete (&event);
    }
  *_component = g_object_ref (component);

  result = TRUE;

error:
  g_object_unref (component);

  g_debug ("TRACE: exiting _parse_modulemd_modulestream_component");
  return result;
}

static gboolean
_parse_modulemd_artifacts (ModulemdModuleStream *modulestream,
                           yaml_parser_t *parser,
                           GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_artifacts");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the artifacts. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the artifacts */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Currently, we only support "rpms" here */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "rpms"))
            {
              if (!_simpleset_from_sequence (parser, &set, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (error,
                                                 "Parse error in artifacts");
                }

              if (!modulemd_simpleset_validate_contents (
                    set, modulemd_validate_nevra, NULL))
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "RPM artifacts not in NEVRA format");
                }

              modulemd_modulestream_set_rpm_artifacts (modulestream, set);
            }
          else
            {
              MMD_YAML_ERROR_RETURN (error, "Unknown artifact type");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in artifacts");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;

error:
  g_object_unref (set);

  g_debug ("TRACE: exiting _parse_modulemd_artifacts");
  return result;
}

static gboolean
_parse_modulemd_servicelevels (ModulemdModuleStream *modulestream,
                               yaml_parser_t *parser,
                               GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  GHashTable *servicelevels = NULL;
  gchar *name = NULL;
  ModulemdServiceLevel *sl = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_servicelevels");

  servicelevels =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the service levels. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the service levels. */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Each entry is the key for a dictionary of ModulemdServiceLevel
           * objects
           */
          name = g_strdup ((const gchar *)event.data.scalar.value);
          if (!_parse_modulemd_servicelevel (parser, name, &sl, error))
            {
              g_free (name);
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Invalid service level");
            }
          g_hash_table_insert (servicelevels, name, sl);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error,
                                 "Unexpected YAML event in service levels");
          break;
        }

      yaml_event_delete (&event);
    }
  modulemd_modulestream_set_servicelevels (modulestream, servicelevels);

  result = TRUE;

error:
  g_hash_table_unref (servicelevels);

  g_debug ("TRACE: exiting _parse_modulemd_servicelevels");
  return result;
}

static gboolean
_parse_modulemd_servicelevel (yaml_parser_t *parser,
                              const gchar *name,
                              ModulemdServiceLevel **_servicelevel,
                              GError **error)
{
  gboolean result = FALSE;
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  ModulemdServiceLevel *sl = NULL;
  GDate *eol = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_servicelevel");

  sl = modulemd_servicelevel_new ();
  modulemd_servicelevel_set_name (sl, name);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the service level content. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the service level content */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Only "eol" is supported right now */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "eol"))
            {
              /* Get the EOL date */
              if (!_parse_modulemd_date (parser, &eol, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Failed to parse EOL date in service level");
                }

              modulemd_servicelevel_set_eol (sl, eol);
              g_date_free (eol);
            }

          else
            {
              /* Unknown field in service level */
              MMD_YAML_ERROR_RETURN (error,
                                     "Unknown key in service level body");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error,
                                 "Unexpected YAML event in service level");
          break;
        }

      yaml_event_delete (&event);
    }

  *_servicelevel = g_object_ref (sl);

  result = TRUE;

error:
  g_object_unref (sl);

  g_debug ("TRACE: exiting _parse_modulemd_servicelevel");
  return result;
}
