/* modulemd-yaml-parser-modulemd.c
 *
 * Copyright (C) 2017 Stephen Gallagher
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
#include "modulemd.h"
#include "modulemd-yaml.h"
#include "modulemd-util.h"


enum ModulemdReqType
{
  MODULEMD_REQ_REQUIRES,
  MODULEMD_REQ_BUILDREQUIRES
};

#define _yaml_parser_modulemd_recurse_down(fn)                                \
  do                                                                          \
    {                                                                         \
      if (!fn (module, parser, error))                                        \
        {                                                                     \
          goto error;                                                         \
        }                                                                     \
    }                                                                         \
  while (0)

static gboolean
_parse_modulemd_data (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error);
static gboolean
_parse_modulemd_licenses (ModulemdModule *module,
                          yaml_parser_t *parser,
                          GError **error);
static gboolean
_parse_modulemd_xmd (ModulemdModule *module,
                     yaml_parser_t *parser,
                     GError **error);
static gboolean
_parse_modulemd_deps (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error);
static gboolean
_parse_modulemd_refs (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error);
static gboolean
_parse_modulemd_profiles (ModulemdModule *module,
                          yaml_parser_t *parser,
                          GError **error);
static gboolean
_parse_modulemd_profile (yaml_parser_t *parser,
                         const gchar *name,
                         ModulemdProfile **_profile,
                         GError **error);
static gboolean
_parse_modulemd_api (ModulemdModule *module,
                     yaml_parser_t *parser,
                     GError **error);
static gboolean
_parse_modulemd_filters (ModulemdModule *module,
                         yaml_parser_t *parser,
                         GError **error);
static gboolean
_parse_modulemd_buildopts (ModulemdModule *module,
                           yaml_parser_t *parser,
                           GError **error);
static gboolean
_parse_modulemd_components (ModulemdModule *module,
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
_parse_modulemd_module_components (yaml_parser_t *parser,
                                   GHashTable **_components,
                                   GError **error);
static gboolean
_parse_modulemd_module_component (yaml_parser_t *parser,
                                  const gchar *name,
                                  ModulemdComponentModule **_components,
                                  GError **error);
static gboolean
_parse_modulemd_artifacts (ModulemdModule *module,
                           yaml_parser_t *parser,
                           GError **error);
static gboolean
_parse_modulemd_servicelevels (ModulemdModule *module,
                               yaml_parser_t *parser,
                               GError **error);
static gboolean
_parse_modulemd_servicelevel (yaml_parser_t *parser,
                              const gchar *name,
                              ModulemdServiceLevel **_servicelevel,
                              GError **error);

gboolean
_parse_modulemd (yaml_parser_t *parser,
                 GObject **object,
                 guint64 version,
                 GError **error)
{
  yaml_event_t event;
  yaml_event_t value_event;
  gboolean done = FALSE;
  gboolean result = FALSE;
  guint64 mdversion;
  ModulemdModule *module = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd");

  module = modulemd_module_new ();

  /* Use the pre-processed mdversion */
  if (version && version <= MD_VERSION_LATEST)
    {
      modulemd_module_set_mdversion (module, version);
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
              modulemd_module_set_mdversion (module, mdversion);
            }

          /* Process the data section */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "data"))
            {
              g_debug ("TRACE: root entry [data]");
              _yaml_parser_modulemd_recurse_down (_parse_modulemd_data);
            }

          else
            {
              g_message ("Unexpected key in root: %s",
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
  *object = (GObject *)module;

error:
  yaml_event_delete (&event);
  yaml_event_delete (&value_event);

  g_debug ("TRACE: exiting _parse_modulemd");
  return result;
}

static gboolean
_parse_modulemd_data (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  yaml_event_t value_event;
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

              modulemd_module_set_name (
                module, (const gchar *)value_event.data.scalar.value);
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

              modulemd_module_set_stream (
                module, (const gchar *)value_event.data.scalar.value);
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

              modulemd_module_set_version (module, version);
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

              modulemd_module_set_context (
                module, (const gchar *)value_event.data.scalar.value);
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

              modulemd_module_set_arch (
                module, (const gchar *)value_event.data.scalar.value);

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

              modulemd_module_set_summary (
                module, (const gchar *)value_event.data.scalar.value);

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

              modulemd_module_set_description (
                module, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }

          /* Module EOL (obsolete) */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "eol"))
            {
              if (modulemd_module_peek_mdversion (module) > MD_VERSION_1)
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

              modulemd_module_set_eol (module, eol);
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
              g_message ("Unexpected key in data: %s",
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
  yaml_event_delete (&event);
  yaml_event_delete (&value_event);

  g_debug ("TRACE: exiting _parse_modulemd_data");
  return result;
}

static gboolean
_parse_modulemd_licenses (ModulemdModule *module,
                          yaml_parser_t *parser,
                          GError **error)
{
  yaml_event_t event;
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
              modulemd_module_set_module_licenses (module, set);
            }
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "content"))
            {
              modulemd_module_set_content_licenses (module, set);
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
  yaml_event_delete (&event);
  g_clear_pointer (&set, g_object_unref);

  g_debug ("TRACE: exiting _parse_modulemd_licenses");
  return result;
}

static gboolean
_parse_modulemd_xmd (ModulemdModule *module,
                     yaml_parser_t *parser,
                     GError **error)
{
  gboolean result = FALSE;
  GHashTable *xmd = NULL;
  yaml_event_t event;
  GVariant *variant;
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
      g_hash_table_insert (xmd, g_strdup (key), g_variant_ref (value));

      g_variant_unref (value);
      g_free (key);
    }

  /* Save this hash table as the xmd property */
  modulemd_module_set_xmd (module, xmd);

  result = TRUE;
error:
  yaml_event_delete (&event);
  g_hash_table_unref (xmd);

  g_debug ("TRACE: exiting _parse_modulemd_xmd");
  return result;
}


static gboolean
_parse_modulemd_deps_v1 (ModulemdModule *module,
                         yaml_parser_t *parser,
                         GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
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
              modulemd_module_set_buildrequires (module, reqs);
            }
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "requires"))
            {
              modulemd_module_set_requires (module, reqs);
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
  yaml_event_delete (&event);
  g_clear_pointer (&reqs, g_hash_table_unref);

  g_debug ("TRACE: exiting _parse_modulemd_deps_v1");
  return result;
}


static gboolean
_parse_modulemd_v2_dep (ModulemdModule *module,
                        yaml_parser_t *parser,
                        GError **error);

static gboolean
_parse_modulemd_deps_v2 (ModulemdModule *module,
                         yaml_parser_t *parser,
                         GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gboolean done = FALSE;
  GHashTable *reqs = NULL;

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
          if (!_parse_modulemd_v2_dep (module, parser, error))
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
  yaml_event_delete (&event);
  g_clear_pointer (&reqs, g_hash_table_unref);

  g_debug ("TRACE: exiting _parse_modulemd_deps_v2");
  return result;
}


static gboolean
_parse_modulemd_v2_dep_map (ModulemdModule *module,
                            yaml_parser_t *parser,
                            enum ModulemdReqType reqtype,
                            ModulemdDependencies *dep,
                            GError **error);

static gboolean
_parse_modulemd_v2_dep (ModulemdModule *module,
                        yaml_parser_t *parser,
                        GError **error)
{
  gboolean result = FALSE;
  gboolean done = FALSE;
  yaml_event_t event;
  enum ModulemdReqType reqtype;
  ModulemdDependencies *dep = NULL;

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
                module, parser, reqtype, dep, error))
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

  modulemd_module_add_dependencies (module, dep);


  result = TRUE;
error:
  g_clear_pointer (&dep, g_object_unref);
  yaml_event_delete (&event);
  g_debug ("TRACE: exiting _parse_modulemd_v2_dep");
  return result;
}


static gboolean
_parse_modulemd_v2_dep_map (ModulemdModule *module,
                            yaml_parser_t *parser,
                            enum ModulemdReqType reqtype,
                            ModulemdDependencies *dep,
                            GError **error)
{
  gboolean result = FALSE;
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  yaml_event_t event;
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
  yaml_event_delete (&event);
  g_debug ("TRACE: exiting _parse_modulemd_v2_dep_map");
  return result;
}


static gboolean
_parse_modulemd_deps (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error)
{
  gboolean result = FALSE;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_deps");

  if (modulemd_module_peek_mdversion (module) == MD_VERSION_1)
    {
      result = _parse_modulemd_deps_v1 (module, parser, error);
      goto error;
    }
  else if (modulemd_module_peek_mdversion (module) >= MD_VERSION_2)
    {
      result = _parse_modulemd_deps_v2 (module, parser, error);
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
_parse_modulemd_refs (ModulemdModule *module,
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
      modulemd_module_set_community (module, (const gchar *)value);
      g_hash_table_remove (refs, "community");
    }

  if ((value = g_hash_table_lookup (refs, "documentation")))
    {
      modulemd_module_set_documentation (module, (const gchar *)value);
      g_hash_table_remove (refs, "documentation");
    }

  if ((value = g_hash_table_lookup (refs, "tracker")))
    {
      modulemd_module_set_tracker (module, (const gchar *)value);
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
_parse_modulemd_profiles (ModulemdModule *module,
                          yaml_parser_t *parser,
                          GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
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
  modulemd_module_set_profiles (module, profiles);

  result = TRUE;

error:
  yaml_event_delete (&event);
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
  yaml_event_t event;
  yaml_event_t value_event;
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
  yaml_event_delete (&event);
  yaml_event_delete (&value_event);
  g_object_unref (profile);

  g_debug ("TRACE: exiting _parse_modulemd_profile");
  return result;
}

static gboolean
_parse_modulemd_api (ModulemdModule *module,
                     yaml_parser_t *parser,
                     GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
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
              modulemd_module_set_rpm_api (module, set);
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
  yaml_event_delete (&event);
  g_object_unref (set);

  g_debug ("TRACE: exiting _parse_modulemd_api");
  return result;
}

static gboolean
_parse_modulemd_filters (ModulemdModule *module,
                         yaml_parser_t *parser,
                         GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
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
              modulemd_module_set_rpm_filter (module, set);
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
  yaml_event_delete (&event);
  g_clear_pointer (&set, g_object_unref);

  g_debug ("TRACE: exiting _parse_modulemd_filters");
  return result;
}

static gboolean
_parse_modulemd_buildopts (ModulemdModule *module,
                           yaml_parser_t *parser,
                           GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gboolean done = FALSE;
  GHashTable *opts = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_buildopts");

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
              if (!_hashtable_from_mapping (parser, &opts, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (error,
                                                 "Parse error in buildopts");
                }
              modulemd_module_set_rpm_buildopts (module, opts);
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

  result = TRUE;

error:
  yaml_event_delete (&event);
  g_hash_table_unref (opts);

  g_debug ("TRACE: exiting _parse_modulemd_buildopts");
  return result;
}

static gboolean
_parse_modulemd_components (ModulemdModule *module,
                            yaml_parser_t *parser,
                            GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
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
              modulemd_module_set_rpm_components (module, components);
              g_hash_table_unref (components);
            }
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "modules"))
            {
              if (!_parse_modulemd_module_components (
                    parser, &components, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Could not parse module components");
                }
              modulemd_module_set_module_components (module, components);
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
  yaml_event_delete (&event);

  g_debug ("TRACE: exiting _parse_modulemd_components");
  return result;
}

static gboolean
_parse_modulemd_rpm_components (yaml_parser_t *parser,
                                GHashTable **_components,
                                GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
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
  yaml_event_delete (&event);
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
  yaml_event_t event;
  yaml_event_t value_event;
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

      yaml_event_delete (&event);
    }
  *_component = g_object_ref (component);

  result = TRUE;

error:
  yaml_event_delete (&event);
  yaml_event_delete (&value_event);
  g_object_unref (component);

  g_debug ("TRACE: exiting _parse_modulemd_module_components");
  return result;
}

static gboolean
_parse_modulemd_module_components (yaml_parser_t *parser,
                                   GHashTable **_components,
                                   GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  gboolean done = FALSE;
  GHashTable *components = NULL;
  gchar *name = NULL;
  ModulemdComponentModule *component = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_modulemd_module_components");

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
          if (!_parse_modulemd_module_component (
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
  yaml_event_delete (&event);
  g_hash_table_unref (components);

  g_debug ("TRACE: exiting _parse_modulemd_module_components");
  return result;
}

static gboolean
_parse_modulemd_module_component (yaml_parser_t *parser,
                                  const gchar *name,
                                  ModulemdComponentModule **_component,
                                  GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
  yaml_event_t value_event;
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
  yaml_event_delete (&event);
  yaml_event_delete (&value_event);
  g_object_unref (component);

  g_debug ("TRACE: exiting _parse_modulemd_module_component");
  return result;
}

static gboolean
_parse_modulemd_artifacts (ModulemdModule *module,
                           yaml_parser_t *parser,
                           GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
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
              modulemd_module_set_rpm_artifacts (module, set);
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
  yaml_event_delete (&event);
  g_object_unref (set);

  g_debug ("TRACE: exiting _parse_modulemd_artifacts");
  return result;
}

static gboolean
_parse_modulemd_servicelevels (ModulemdModule *module,
                               yaml_parser_t *parser,
                               GError **error)
{
  gboolean result = FALSE;
  yaml_event_t event;
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
  modulemd_module_set_servicelevels (module, servicelevels);

  result = TRUE;

error:
  yaml_event_delete (&event);
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
  yaml_event_t event;
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
  yaml_event_delete (&event);
  g_object_unref (sl);

  g_debug ("TRACE: exiting _parse_modulemd_servicelevel");
  return result;
}
