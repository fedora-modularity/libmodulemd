/* modulemd-yaml.c
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

#define MODULEMD_YAML_ERROR modulemd_yaml_error_quark ()

static GQuark
modulemd_yaml_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-yaml-error-quark");
}

enum ModulemdYamlError
{
  MODULEMD_YAML_ERROR_OPEN,
  MODULEMD_YAML_ERROR_PARSE
};

#define YAML_PARSER_PARSE_WITH_ERROR_RETURN(parser, event, error, msg)        \
  do                                                                          \
    {                                                                         \
      if (!yaml_parser_parse (parser, event))                                 \
        {                                                                     \
          g_set_error_literal (                                               \
            error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE, msg);      \
          goto error;                                                         \
        }                                                                     \
      g_debug ("Parser event: %u\n", (event)->type);                          \
    }                                                                         \
  while (0)

#define MMD_YAML_ERROR_RETURN(error, msg)                                     \
  do                                                                          \
    {                                                                         \
      g_message (msg);                                                        \
      g_set_error_literal (                                                   \
        error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE, msg);          \
      g_debug ("Error occurred while parsing event %u", event.type);          \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define MMD_YAML_ERROR_RETURN_RETHROW(error, msg)                             \
  do                                                                          \
    {                                                                         \
      g_message (msg);                                                        \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define _yaml_recurse_down(fn)                                                \
  do                                                                          \
    {                                                                         \
      if (!fn (module, parser, error))                                        \
        {                                                                     \
          goto error;                                                         \
        }                                                                     \
    }                                                                         \
  while (0)

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
                         ModulemdProfile **_profile,
                         GError **error);
static gboolean
_parse_modulemd_api (ModulemdModule *module,
                     yaml_parser_t *parser,
                     GError **error);

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
  FILE *yaml_file;
  gsize count = 0;
  ModulemdModule **modules = NULL;
  yaml_parser_t parser;
  yaml_event_t event;
  gboolean done = FALSE;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (path, NULL);

  g_debug ("TRACE: entering parse_yaml_file\n");

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

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        &parser, &event, error, "Parser error");

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

          if (!_parse_modulemd_root (modules[count - 1], &parser, error))
            {
              goto error;
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

  g_debug ("TRACE: exiting parse_yaml_file\n");
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

  g_debug ("TRACE: entering _parse_modulemd_root\n");

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
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse document type");
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
              _yaml_recurse_down (_parse_modulemd_data);
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
      return FALSE;
    }

  g_debug ("TRACE: exiting _parse_modulemd_root\n");
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

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_modulemd_data\n");

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

          /* licenses */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "license"))
            {
              /* Process the module and content licenses */
              _yaml_recurse_down (_parse_modulemd_licenses);
            }

          /* xmd */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "xmd"))
            {
              /* Process the extensible metadata block */
              _yaml_recurse_down (_parse_modulemd_xmd);
            }

          /* dependencies */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "dependencies"))
            {
              /* Process the build and runtime dependencies of this module */
              _yaml_recurse_down (_parse_modulemd_deps);
            }

          /* references */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "references"))
            {
              /* Process the reference links for this module */
              _yaml_recurse_down (_parse_modulemd_refs);
            }

          /* profiles */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "profiles"))
            {
              /* Process the install profiles for this module */
              _yaml_recurse_down (_parse_modulemd_profiles);
            }

          /* api */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "api"))
            {
              /* Process the API list */
              _yaml_recurse_down (_parse_modulemd_api);
            }

          /* filter */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "filter"))
            {
              /* Process the filtered-out output components */
              /* TODO _yaml_recurse_down (_parse_modulemd_filters); */
            }

          /* buildopts */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "buildopts"))
            {
              /* Process special build options for this module */
              /* TODO _yaml_recurse_down (_parse_modulemd_buildopts); */
            }

          /* Components */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "components"))
            {
              /* Process the components that comprise this module */
              /* TODO _yaml_recurse_down (_parse_modulemd_components); */
            }

          /* Artifacts */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "artifacts"))
            {
              /* Process the output artifacts of this module */
              /* TODO _yaml_recurse_down (_parse_modulemd_artifacts); */
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
  g_debug ("TRACE: exiting _parse_modulemd_data\n");
  return TRUE;
}

static gboolean
_parse_modulemd_licenses (ModulemdModule *module,
                          yaml_parser_t *parser,
                          GError **error)
{
  yaml_event_t event;
  gboolean done = FALSE;
  ModulemdSimpleSet *set;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_licenses\n");

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
  g_debug ("TRACE: exiting _parse_modulemd_licenses\n");
  return TRUE;
}

static gboolean
_parse_modulemd_xmd (ModulemdModule *module,
                     yaml_parser_t *parser,
                     GError **error)
{
  GHashTable *xmd = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_modulemd_xmd\n");

  if (!_hashtable_from_mapping (parser, &xmd, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Invalid mapping");
    }

  /* Save this hash table as the xmd property */
  modulemd_module_set_xmd (module, xmd);

error:
  g_hash_table_unref (xmd);
  if (*error)
    {
      return FALSE;
    }

  g_debug ("TRACE: exiting _parse_modulemd_xmd\n");
  return TRUE;
}

static gboolean
_parse_modulemd_deps (ModulemdModule *module,
                      yaml_parser_t *parser,
                      GError **error)
{
  yaml_event_t event;
  gboolean done = FALSE;
  GHashTable *reqs = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_deps\n");

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
          /* Each scalar event represents a license type */
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
  g_debug ("TRACE: exiting _parse_modulemd_deps\n");
  return TRUE;
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

  g_debug ("TRACE: entering _parse_modulemd_refs\n");

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
  g_debug ("TRACE: exiting _parse_modulemd_refs\n");
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

  g_debug ("TRACE: entering _parse_modulemd_profiles\n");

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
          if (!_parse_modulemd_profile (parser, &profile, error))
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
  g_debug ("TRACE: exiting _parse_modulemd_profiles\n");
  return TRUE;
}

static gboolean
_parse_modulemd_profile (yaml_parser_t *parser,
                         ModulemdProfile **_profile,
                         GError **error)
{
  yaml_event_t event;
  gboolean done = FALSE;
  ModulemdSimpleSet *set = NULL;
  ModulemdProfile *profile = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_modulemd_profile\n");

  profile = modulemd_profile_new ();

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
  g_debug ("TRACE: exiting _parse_modulemd_profile\n");
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

  g_debug ("TRACE: entering _parse_modulemd_api\n");

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
  g_debug ("TRACE: exiting _parse_modulemd_api\n");
  return TRUE;
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

  g_debug ("TRACE: entering _simpleset_from_sequence\n");

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
  g_debug ("TRACE: exiting _simpleset_from_sequence\n");
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
  g_debug ("TRACE: entering _hashtable_from_mapping\n");

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

  g_debug ("TRACE: exiting _hashtable_from_mapping\n");
  return TRUE;
}
