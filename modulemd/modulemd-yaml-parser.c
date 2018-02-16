/* modulemd-yaml-parser.c
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

GQuark
modulemd_yaml_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-yaml-error-quark");
}

#define MMD_YAML_NOEVENT_ERROR_RETURN(error, msg)                             \
  do                                                                          \
    {                                                                         \
      g_message (msg);                                                        \
      g_set_error_literal (                                                   \
        error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE, msg);          \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define _yaml_parser_recurse_down(fn)                                         \
  do                                                                          \
    {                                                                         \
      if (!fn (module, parser, error))                                        \
        {                                                                     \
          goto error;                                                         \
        }                                                                     \
    }                                                                         \
  while (0)

enum ModulemdReqType
{
  MODULEMD_REQ_REQUIRES,
  MODULEMD_REQ_BUILDREQUIRES
};

static ModulemdModule **
_parse_yaml (yaml_parser_t *parser, GError **error);

static gboolean
_parse_modulemd_root (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error);
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
static gboolean
_parse_modulemd_date (yaml_parser_t *parser, GDate **_date, GError **error);
static gboolean
_simpleset_from_sequence (yaml_parser_t *parser,
                          ModulemdSimpleSet **_set,
                          GError **error);
static gboolean
_hashtable_from_mapping (yaml_parser_t *parser,
                         GHashTable **_htable,
                         GError **error);

ModulemdModule **
parse_yaml_file (const gchar *path, GError **error)
{
  FILE *yaml_file = NULL;
  ModulemdModule **modules = NULL;
  yaml_parser_t parser;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (path, NULL);

  g_debug ("TRACE: entering parse_yaml_file");

  yaml_parser_initialize (&parser);

  errno = 0;
  yaml_file = g_fopen (path, "rb");
  if (!yaml_file)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_OPEN,
                   "Failed to open file: %s",
                   g_strerror (errno));
      goto error;
    }

  yaml_parser_set_input_file (&parser, yaml_file);

  modules = _parse_yaml (&parser, error);

error:
  yaml_parser_delete (&parser);
  if (yaml_file)
    {
      fclose (yaml_file);
    }
  g_debug ("TRACE: exiting parse_yaml_file");
  return modules;
}

ModulemdModule **
parse_yaml_string (const gchar *yaml, GError **error)
{
  ModulemdModule **modules = NULL;
  yaml_parser_t parser;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (yaml, NULL);

  g_debug ("TRACE: entering parse_yaml_string");

  yaml_parser_initialize (&parser);

  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml, strlen (yaml));

  modules = _parse_yaml (&parser, error);
  yaml_parser_delete (&parser);

  g_debug ("TRACE: exiting parse_yaml_string");
  return modules;
}

static ModulemdModule **
_parse_yaml (yaml_parser_t *parser, GError **error)
{
  gsize count = 0;
  ModulemdModule **modules = NULL;
  yaml_event_t event;
  gboolean done = FALSE;

  g_debug ("TRACE: entering _parse_yaml");
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_STREAM_START_EVENT:
          /* The start of the stream requires no action */
          break;

        case YAML_STREAM_END_EVENT:
          /* Processing of the YAML is complete */
          done = TRUE;
          break;

        case YAML_DOCUMENT_START_EVENT:
          count++;
          modules =
            g_realloc_n (modules, count + 1, sizeof (ModulemdModule *));
          modules[count] = NULL;

          /* New document; create a new ModulemdModule object */
          modules[count - 1] = modulemd_module_new ();

          if (!_parse_modulemd_root (modules[count - 1], parser, error))
            {
              /* This document was invalid, so we'll skip it. The parser
               * should now be at the YAML_DOCUMENT_END_EVENT, so we'll
               * delete the module in-progress, decrement the count and
               * continue the loop in case there are other documents to be
               * processed.
               */
              g_message ("Invalid document [%s]. Skipping it.", (*error)->message);
              g_clear_pointer (&modules[count - 1], g_object_unref);
              g_clear_error (error);
              count --;
              continue;
            }

          break;

        case YAML_DOCUMENT_END_EVENT:
          /* This document is complete. */
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event at toplevel");
          break;
        }

      yaml_event_delete (&event);
    }

error:
  /* error handling */
  if (*error)
    {
      if (modules)
        {
          /* Free any modules allocated up to this point */
          for (gsize i = 0; modules[i]; i++)
            {
              g_object_unref (modules[i]);
            }
          g_clear_pointer (&modules, g_free);
        }
    }

  g_debug ("TRACE: exiting _parse_yaml");
  return modules;
}

static gboolean
_parse_modulemd_root (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error)
{
  yaml_event_t event;
  gboolean done = FALSE;
  guint64 version;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_root");

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
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT ||
                  g_strcmp0 ((const gchar *)event.data.scalar.value,
                             "modulemd"))
                {
                  MMD_YAML_ERROR_RETURN (error, "Unknown document type");
                }
            }

          /* Record the modulemd version for the parser */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "version"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Unknown modulemd version");
                }

              version = g_ascii_strtoull (
                (const gchar *)event.data.scalar.value, NULL, 10);
              if (!version)
                {
                  MMD_YAML_ERROR_RETURN (error, "Unknown modulemd version");
                }

              modulemd_module_set_mdversion (module, version);
            }

          /* Process the data section */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "data"))
            {
              _yaml_parser_recurse_down (_parse_modulemd_data);
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
    }

error:
  if (*error)
    {
      /* Move the parser to the end of the mapping */
      do
        {
          YAML_PARSER_PARSE_WITH_ERROR_RETURN (
            parser, &event, error, "Parser error");
        } while (event.type != YAML_DOCUMENT_END_EVENT);
      return FALSE;
    }

  g_debug ("TRACE: exiting _parse_modulemd_root");
  return TRUE;
}

static gboolean
_parse_modulemd_data (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error)
{
  yaml_event_t event;
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
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse module name");
                }

              modulemd_module_set_name (
                module, (const gchar *)event.data.scalar.value);
            }

          /* Module stream */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "stream"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module stream");
                }

              modulemd_module_set_stream (
                module, (const gchar *)event.data.scalar.value);
            }

          /* Module version */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "version"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module version");
                }

              version = g_ascii_strtoull (
                (const gchar *)event.data.scalar.value, NULL, 10);
              if (!version)
                {
                  MMD_YAML_ERROR_RETURN (error, "Unknown module version");
                }

              modulemd_module_set_version (module, version);
            }

          /* Module Context */

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "context"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module context");
                }

              modulemd_module_set_context (
                module, (const gchar *)event.data.scalar.value);
            }

          /* Module Artifact Architecture */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "arch"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (
                    error, "Failed to parse module artifact architecture");
                }

              modulemd_module_set_arch (
                module, (const gchar *)event.data.scalar.value);
            }

          /* Module summary */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "summary"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module summary");
                }

              modulemd_module_set_summary (
                module, (const gchar *)event.data.scalar.value);
            }

          /* Module description */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "description"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module description");
                }

              modulemd_module_set_description (
                module, (const gchar *)event.data.scalar.value);
            }

          /* Module EOL (obsolete) */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "eol"))
            {
              if (modulemd_module_get_mdversion (module) > 1)
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
              _yaml_parser_recurse_down (_parse_modulemd_servicelevels);
            }

          /* licenses */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "license"))
            {
              /* Process the module and content licenses */
              _yaml_parser_recurse_down (_parse_modulemd_licenses);
            }

          /* xmd */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "xmd"))
            {
              /* Process the extensible metadata block */
              _yaml_parser_recurse_down (_parse_modulemd_xmd);
            }

          /* dependencies */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "dependencies"))
            {
              /* Process the build and runtime dependencies of this module */
              _yaml_parser_recurse_down (_parse_modulemd_deps);
            }

          /* references */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "references"))
            {
              /* Process the reference links for this module */
              _yaml_parser_recurse_down (_parse_modulemd_refs);
            }

          /* profiles */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "profiles"))
            {
              /* Process the install profiles for this module */
              _yaml_parser_recurse_down (_parse_modulemd_profiles);
            }

          /* api */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "api"))
            {
              /* Process the API list */
              _yaml_parser_recurse_down (_parse_modulemd_api);
            }

          /* filter */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "filter"))
            {
              /* Process the filtered-out output components */
              _yaml_parser_recurse_down (_parse_modulemd_filters);
            }

          /* buildopts */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "buildopts"))
            {
              /* Process special build options for this module */
              _yaml_parser_recurse_down (_parse_modulemd_buildopts);
            }

          /* Components */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "components"))
            {
              /* Process the components that comprise this module */
              _yaml_parser_recurse_down (_parse_modulemd_components);
            }

          /* Artifacts */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "artifacts"))
            {
              /* Process the output artifacts of this module */
              _yaml_parser_recurse_down (_parse_modulemd_artifacts);
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
    }

error:
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_data");
  return TRUE;
}

static gboolean
_parse_modulemd_licenses (ModulemdModule *module,
                          yaml_parser_t *parser,
                          GError **error)
{
  yaml_event_t event;
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
    }

error:
  g_clear_pointer (&set, g_object_unref);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_licenses");
  return TRUE;
}

static gboolean
_parse_modulemd_xmd (ModulemdModule *module,
                     yaml_parser_t *parser,
                     GError **error)
{
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

error:
  g_hash_table_unref (xmd);
  if (*error)
    {
      return FALSE;
    }

  g_debug ("TRACE: exiting _parse_modulemd_xmd");
  return TRUE;
}


static gboolean
_parse_modulemd_deps_v1 (ModulemdModule *module,
                         yaml_parser_t *parser,
                         GError **error)
{
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
    }

error:
  g_clear_pointer (&reqs, g_hash_table_unref);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_deps_v1");
  return TRUE;
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
    }

error:
  g_clear_pointer (&reqs, g_hash_table_unref);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_deps_v2");
  return TRUE;
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
  gboolean ret = FALSE;
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
    }

  modulemd_module_add_dependencies (module, dep);
  g_object_unref (dep);


  ret = TRUE;
error:
  g_debug ("TRACE: exiting _parse_modulemd_v2_dep");
  return ret;
}


static gboolean
_parse_modulemd_v2_dep_map (ModulemdModule *module,
                            yaml_parser_t *parser,
                            enum ModulemdReqType reqtype,
                            ModulemdDependencies *dep,
                            GError **error)
{
  gboolean ret = FALSE;
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  yaml_event_t event;
  gchar *module_name = NULL;
  ModulemdSimpleSet *set = NULL;

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

          switch (reqtype)
            {
            case MODULEMD_REQ_BUILDREQUIRES:
              modulemd_dependencies_add_buildrequires (
                dep,
                module_name,
                (const gchar **)modulemd_simpleset_get (set));
              break;
            case MODULEMD_REQ_REQUIRES:
              modulemd_dependencies_add_requires (
                dep,
                module_name,
                (const gchar **)modulemd_simpleset_get (set));
              break;
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in v2_dep_map");
          break;
        }
    }

  ret = TRUE;
error:
  g_debug ("TRACE: exiting _parse_modulemd_v2_dep_map");
  return ret;
}


static gboolean
_parse_modulemd_deps (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error)
{
  gboolean result = FALSE;
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_deps");

  switch (modulemd_module_get_mdversion (module))
    {
    case 1: result = _parse_modulemd_deps_v1 (module, parser, error); break;

    case 2: result = _parse_modulemd_deps_v2 (module, parser, error); break;

    default:
      MMD_YAML_NOEVENT_ERROR_RETURN (error, "Unknown modulemd version");
      break;
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
  yaml_event_t event;
  GHashTable *refs = NULL;
  gpointer value;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_refs");

  /* This is a hack so we can use the MMD_YAML_ERROR_RETURN macro below */
  event.type = YAML_SCALAR_EVENT;

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

error:
  g_clear_pointer (&refs, g_hash_table_unref);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_refs");
  return TRUE;
}

static gboolean
_parse_modulemd_profiles (ModulemdModule *module,
                          yaml_parser_t *parser,
                          GError **error)
{
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
    }
  modulemd_module_set_profiles (module, profiles);

error:
  g_hash_table_unref (profiles);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_profiles");
  return TRUE;
}

static gboolean
_parse_modulemd_profile (yaml_parser_t *parser,
                         const gchar *name,
                         ModulemdProfile **_profile,
                         GError **error)
{
  yaml_event_t event;
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
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "No value for description");
                }

              modulemd_profile_set_description (
                profile, (const gchar *)event.data.scalar.value);
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
    }

  *_profile = g_object_ref (profile);

error:
  g_object_unref (profile);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_profile");
  return TRUE;
}

static gboolean
_parse_modulemd_api (ModulemdModule *module,
                     yaml_parser_t *parser,
                     GError **error)
{
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
    }

error:
  g_object_unref (set);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_api");
  return TRUE;
}

static gboolean
_parse_modulemd_filters (ModulemdModule *module,
                         yaml_parser_t *parser,
                         GError **error)
{
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
    }

error:
  g_object_unref (set);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_filters");
  return TRUE;
}

static gboolean
_parse_modulemd_buildopts (ModulemdModule *module,
                           yaml_parser_t *parser,
                           GError **error)
{
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
              MMD_YAML_ERROR_RETURN (error, "Unknown filter type");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in buildopts");
          break;
        }
    }

error:
  g_hash_table_unref (opts);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_buildopts");
  return TRUE;
}

static gboolean
_parse_modulemd_components (ModulemdModule *module,
                            yaml_parser_t *parser,
                            GError **error)
{
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
    }

error:
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_components");
  return TRUE;
}

static gboolean
_parse_modulemd_rpm_components (yaml_parser_t *parser,
                                GHashTable **_components,
                                GError **error)
{
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
    }
  *_components = g_hash_table_ref (components);

error:
  g_hash_table_unref (components);
  if (*error)
    {
      return FALSE;
    }

  g_debug ("TRACE: exiting _parse_modulemd_rpm_components");
  return TRUE;
}

static gboolean
_parse_modulemd_rpm_component (yaml_parser_t *parser,
                               const gchar *name,
                               ModulemdComponentRpm **_component,
                               GError **error)
{
  yaml_event_t event;
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
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse buildorder value");
                }

              buildorder = g_ascii_strtoull (
                (const gchar *)event.data.scalar.value, NULL, 10);
              modulemd_component_set_buildorder (
                MODULEMD_COMPONENT (component), buildorder);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "rationale"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse rationale value");
                }

              modulemd_component_set_rationale (
                MODULEMD_COMPONENT (component),
                (const gchar *)event.data.scalar.value);
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
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse cache value");
                }

              modulemd_component_rpm_set_cache (
                component, (const gchar *)event.data.scalar.value);
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
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse ref value");
                }

              modulemd_component_rpm_set_ref (
                component, (const gchar *)event.data.scalar.value);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "repository"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse repository value");
                }

              modulemd_component_rpm_set_repository (
                component, (const gchar *)event.data.scalar.value);
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
    }
  *_component = g_object_ref (component);

error:
  g_object_unref (component);
  if (*error)
    {
      return FALSE;
    }

  g_debug ("TRACE: exiting _parse_modulemd_module_components");
  return TRUE;
}

static gboolean
_parse_modulemd_module_components (yaml_parser_t *parser,
                                   GHashTable **_components,
                                   GError **error)
{
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
    }
  *_components = g_hash_table_ref (components);

error:
  g_hash_table_unref (components);
  if (*error)
    {
      return FALSE;
    }

  g_debug ("TRACE: exiting _parse_modulemd_module_components");
  return TRUE;
}

static gboolean
_parse_modulemd_module_component (yaml_parser_t *parser,
                                  const gchar *name,
                                  ModulemdComponentModule **_component,
                                  GError **error)
{
  yaml_event_t event;
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
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse buildorder value");
                }

              buildorder = g_ascii_strtoull (
                (const gchar *)event.data.scalar.value, NULL, 10);
              modulemd_component_set_buildorder (
                MODULEMD_COMPONENT (component), buildorder);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "rationale"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse rationale value");
                }

              modulemd_component_set_rationale (
                MODULEMD_COMPONENT (component),
                (const gchar *)event.data.scalar.value);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "ref"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse ref value");
                }

              modulemd_component_module_set_ref (
                component, (const gchar *)event.data.scalar.value);
            }

          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "repository"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &event, error, "Parser error");
              if (event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse repository value");
                }

              modulemd_component_module_set_repository (
                component, (const gchar *)event.data.scalar.value);
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
    }
  *_component = g_object_ref (component);

error:
  g_object_unref (component);
  if (*error)
    {
      return FALSE;
    }

  g_debug ("TRACE: exiting _parse_modulemd_module_component");
  return TRUE;
}

static gboolean
_parse_modulemd_artifacts (ModulemdModule *module,
                           yaml_parser_t *parser,
                           GError **error)
{
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
    }

error:
  g_object_unref (set);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_artifacts");
  return TRUE;
}

static gboolean
_parse_modulemd_servicelevels (ModulemdModule *module,
                               yaml_parser_t *parser,
                               GError **error)
{
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
    }
  modulemd_module_set_servicelevels (module, servicelevels);

error:
  g_hash_table_unref (servicelevels);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_servicelevels");
  return TRUE;
}

static gboolean
_parse_modulemd_servicelevel (yaml_parser_t *parser,
                              const gchar *name,
                              ModulemdServiceLevel **_servicelevel,
                              GError **error)
{
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
    }

  *_servicelevel = g_object_ref (sl);

error:
  g_object_unref (sl);
  if (*error)
    {
      return FALSE;
    }
  g_debug ("TRACE: exiting _parse_modulemd_servicelevel");
  return TRUE;
}

static gboolean
_parse_modulemd_date (yaml_parser_t *parser, GDate **_date, GError **error)
{
  gboolean ret = FALSE;
  gchar **strv = NULL;
  yaml_event_t event;

  YAML_PARSER_PARSE_WITH_ERROR_RETURN (parser, &event, error, "Parser error");
  if (event.type != YAML_SCALAR_EVENT)
    {
      MMD_YAML_ERROR_RETURN (error, "Failed to parse date");
    }

  strv = g_strsplit ((const gchar *)event.data.scalar.value, "-", 4);

  if (!strv[0] || !strv[1] || !strv[2])
    {
      MMD_YAML_ERROR_RETURN (error, "Date not in the form YYYY-MM-DD");
    }

  *_date = g_date_new_dmy (g_ascii_strtoull (strv[2], NULL, 10), /* Day */
                           g_ascii_strtoull (strv[1], NULL, 10), /* Month */
                           g_ascii_strtoull (strv[0], NULL, 10)); /* Year */

  ret = TRUE;

error:
  return ret;
}

static gboolean
_simpleset_from_sequence (yaml_parser_t *parser,
                          ModulemdSimpleSet **_set,
                          GError **error)
{
  yaml_event_t event;
  gboolean done = FALSE;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _simpleset_from_sequence");

  set = modulemd_simpleset_new ();


  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_SEQUENCE_START_EVENT:
          /* Sequence has begun */
          break;

        case YAML_SEQUENCE_END_EVENT:
          /* Sequence has concluded. Return */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          modulemd_simpleset_add (set, (const gchar *)event.data.scalar.value);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in sequence");
          break;
        }
    }

error:
  if (*error)
    {
      g_object_unref (set);
      return FALSE;
    }
  *_set = set;
  g_debug ("TRACE: exiting _simpleset_from_sequence");
  return TRUE;
}

static gboolean
_hashtable_from_mapping (yaml_parser_t *parser,
                         GHashTable **_htable,
                         GError **error)
{
  yaml_event_t event;
  gboolean done = FALSE;
  GHashTable *htable = NULL;
  gchar *name = NULL;
  gchar *value = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _hashtable_from_mapping");

  htable = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

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
          YAML_PARSER_PARSE_WITH_ERROR_RETURN (
            parser, &event, error, "Parser error");
          if (event.type != YAML_SCALAR_EVENT)
            {
              g_free (name);
              MMD_YAML_ERROR_RETURN (error,
                                     "Non-scalar value for dictionary.");
            }
          value = g_strdup ((const gchar *)event.data.scalar.value);

          /* Set this key and value to the hash table */
          g_hash_table_insert (htable, name, value);

          break;


        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in sequence");
          break;
        }
    }
  *_htable = g_hash_table_ref (htable);

error:
  g_hash_table_unref (htable);
  if (*error)
    {
      return FALSE;
    }

  g_debug ("TRACE: exiting _hashtable_from_mapping");
  return TRUE;
}
