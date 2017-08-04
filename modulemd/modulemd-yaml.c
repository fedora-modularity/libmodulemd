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
      g_set_error_literal (                                                   \
        error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_PARSE, msg);          \
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
        case YAML_NO_EVENT:
        case YAML_STREAM_START_EVENT:
          /* No event requires no special handling */
          break;

        case YAML_ALIAS_EVENT:
        case YAML_SCALAR_EVENT:
        case YAML_SEQUENCE_START_EVENT:
        case YAML_SEQUENCE_END_EVENT:
        case YAML_MAPPING_START_EVENT:
        case YAML_MAPPING_END_EVENT:
          /* These events are not implemented at this level */
          g_assert_not_reached ();
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
          /* Should never happen; all cases should be handled explicitly */
          g_assert_not_reached ();
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

        case YAML_MAPPING_END_EVENT:
          /* This is the end of the main document content. */
          done = TRUE;
          break;

        default:
          /* We should never see other events at this level */
          g_assert_not_reached ();
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
              /* TODO _yaml_recurse_down (_parse_modulemd_licenses); */
            }

          /* xmd */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "xmd"))
            {
              /* Process the extensible metadata block */
              /* TODO _yaml_recurse_down (_parse_modulemd_xmd); */
            }

          /* dependencies */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "dependencies"))
            {
              /* Process the build and runtime dependencies of this module */
              /* TODO _yaml_recurse_down (_parse_modulemd_deps); */
            }

          /* references */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "references"))
            {
              /* Process the reference links for this module */
              /* TODO _yaml_recurse_down (_parse_modulemd_refs); */
            }

          /* profiles */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "profiles"))
            {
              /* Process the install profiles for this module */
              /* TODO _yaml_recurse_down (_parse_modulemd_profiles); */
            }

          /* api */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "api"))
            {
              /* Process the API list */
              /* TODO _yaml_recurse_down (_parse_modulemd_api); */
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
          /* Any event not handled above is unexpected */
          g_assert_not_reached ();
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
