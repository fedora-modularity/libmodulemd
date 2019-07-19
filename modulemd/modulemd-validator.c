/*
 * This file is part of libmodulemd
 * Copyright 2018 Stephen Gallagher
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
#include "private/modulemd-module-index-private.h"
#include "private/modulemd-yaml.h"

#include <glib.h>
#include <locale.h>
#include <errno.h>

enum mmd_verbosity
{
  MMD_QUIET = -1,
  MMD_DEFAULT,
  MMD_VERBOSE,
  MMD_DEBUG
};

struct validator_options
{
  enum mmd_verbosity verbosity;
  gchar **filenames;
};

struct validator_options options = { 0, NULL };

static gboolean
set_verbosity (const gchar *option_name,
               const gchar *value,
               gpointer data,
               GError **error)
{
  g_autofree gchar *debugging_env = NULL;
  if (g_strcmp0 ("-v", option_name) == 0 ||
      g_strcmp0 ("--verbose", option_name) == 0)
    {
      if (options.verbosity < MMD_VERBOSE)
        {
          options.verbosity = MMD_VERBOSE;
        }
    }
  else if (g_strcmp0 ("--debug", option_name) == 0)
    {
      if (options.verbosity < MMD_DEBUG)
        {
          options.verbosity = MMD_DEBUG;
          const gchar *old_debug = g_getenv ("G_MESSAGES_DEBUG");
          if (old_debug != NULL)
            {
              debugging_env =
                g_strdup_printf ("%s,%s", old_debug, G_LOG_DOMAIN);
            }
          else
            {
              debugging_env = g_strdup (G_LOG_DOMAIN);
            }
          g_setenv ("G_MESSAGES_DEBUG", debugging_env, TRUE);
        }
    }
  else if (g_strcmp0 ("-q", option_name) == 0 ||
           g_strcmp0 ("--quiet", option_name) == 0)
    {
      options.verbosity = MMD_QUIET;
    }
  else
    {
      /* We shouldn't be called under any other circumstance */
      g_set_error (error,
                   G_OPTION_ERROR,
                   G_OPTION_ERROR_FAILED,
                   "Called for unknown option \"%s\"",
                   option_name);
      return FALSE;
    }
  return TRUE;
}

// clang-format off
static GOptionEntry entries[] = {
  { "quiet", 'q', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, set_verbosity, "Print no output", NULL },
  { "verbose", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, set_verbosity, "Be verbose", NULL },
  { "debug", 0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, set_verbosity, "Output debugging messages", NULL },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &options.filenames, "Files to be validated", NULL },
  { NULL } };
// clang-format on


static gboolean
parse_file (const gchar *filename, GPtrArray **failures, GError **error)
{
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (FILE) yaml_stream = NULL;
  int saved_errno;
  g_autoptr (ModulemdModuleIndex) index = NULL;

  if (options.verbosity >= MMD_VERBOSE)
    {
      g_fprintf (stdout, "Validating %s\n", filename);
    }

  /* Parse documents */
  yaml_stream = g_fopen (filename, "rb");
  saved_errno = errno;

  if (yaml_stream == NULL)
    {
      if (options.verbosity >= MMD_DEFAULT)
        {
          g_fprintf (stdout,
                     "Failed to open file %s: %s\n",
                     filename,
                     g_strerror (saved_errno));
        }
      return FALSE;
    }


  yaml_parser_set_input_file (&parser, yaml_stream);

  index = modulemd_module_index_new ();
  return modulemd_module_index_update_from_parser (
    index, &parser, TRUE, TRUE, failures, error);
}


int
main (int argc, char *argv[])
{
  const char *filename;
  g_autoptr (GOptionContext) context = NULL;
  g_autoptr (GError) error = NULL;
  gsize num_invalid = 0;
  gboolean ret;
  g_autoptr (GPtrArray) failures = NULL;
  ModulemdSubdocumentInfo *doc = NULL;

  setlocale (LC_ALL, "");

  context = g_option_context_new ("FILES - Simple modulemd YAML validator");
  g_option_context_add_main_entries (context, entries, "modulemd-validator");
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print ("option parsing failed: %s\n", error->message);
      exit (1);
    }

  if (!(options.filenames && options.filenames[0]))
    {
      g_fprintf (stderr,
                 "At least one file must be specified on the command-line\n");
      return EXIT_FAILURE;
    }

  for (gsize i = 0; options.filenames[i]; i++)
    {
      filename = options.filenames[i];

      ret = parse_file (filename, &failures, &error);
      if (!ret)
        {
          num_invalid++;
          if (options.verbosity >= MMD_DEFAULT)
            {
              g_fprintf (stderr, "%s failed to validate\n", filename);

              if (error != NULL)
                {
                  /* Unparseable content */
                  g_fprintf (stderr,
                             "%s could not be read in its entirety: %s\n",
                             filename,
                             error->message);
                }
              if (failures)
                {
                  for (gsize j = 0; j < failures->len; j++)
                    {
                      doc = MODULEMD_SUBDOCUMENT_INFO (
                        g_ptr_array_index (failures, i));
                      g_printf (
                        "\nFailed subdocument (%s): \n%s\n",
                        modulemd_subdocument_info_get_gerror (doc)->message,
                        modulemd_subdocument_info_get_yaml (doc));
                    }
                }
            }
        }
      else
        {
          if (options.verbosity >= MMD_DEFAULT)
            {
              g_printf ("%s validated successfully\n", filename);
            }
        }
      g_clear_error (&error);
      g_clear_pointer (&failures, g_ptr_array_unref);
    }

  return num_invalid;
}
