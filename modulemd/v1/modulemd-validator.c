/* test-modulemd-validator.c
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
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "modulemd.h"
#include "private/modulemd-yaml.h"

#include <glib.h>
#include <locale.h>

/* TODO: implement internationalization */

#ifndef _
#ifdef HAVE_GETTEXT
#define _(STRING) gettext (STRING)
#else
#define _(STRING) STRING
#endif /* HAVE_GETTEXT */
#endif /* _ */

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

static GOptionEntry entries[] = { { "quiet",
                                    'q',
                                    G_OPTION_FLAG_NO_ARG,
                                    G_OPTION_ARG_CALLBACK,
                                    set_verbosity,
                                    "Print no output",
                                    NULL },
                                  { "verbose",
                                    'v',
                                    G_OPTION_FLAG_NO_ARG,
                                    G_OPTION_ARG_CALLBACK,
                                    set_verbosity,
                                    "Be verbose",
                                    NULL },
                                  { "debug",
                                    0,
                                    G_OPTION_FLAG_NO_ARG,
                                    G_OPTION_ARG_CALLBACK,
                                    set_verbosity,
                                    "Output debugging messages",
                                    NULL },
                                  { G_OPTION_REMAINING,
                                    0,
                                    0,
                                    G_OPTION_ARG_FILENAME_ARRAY,
                                    &options.filenames,
                                    "Files to be validated",
                                    NULL },
                                  { NULL } };

int
main (int argc, char *argv[])
{
  const char *filename;
  GOptionContext *context;
  g_autoptr (GError) error = NULL;
  gboolean all_valid = TRUE;
  g_autoptr (GPtrArray) objects = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  ModulemdSubdocument *doc = NULL;
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
      fprintf (stderr,
               "At least one file must be specified on the command-line\n");
      return EXIT_FAILURE;
    }

  for (gsize i = 0; options.filenames[i]; i++)
    {
      filename = options.filenames[i];
      if (options.verbosity >= MMD_VERBOSE)
        {
          fprintf (stdout, "Validating %s\n", filename);
        }

      objects = modulemd_objects_from_file_ext (filename, &failures, &error);
      if (!objects)
        {
          if (options.verbosity >= MMD_DEFAULT)
            {
              fprintf (stderr, "%s was not valid YAML\n", filename);
              fprintf (stdout, "ERROR: %s\n", error->message);
            }
          all_valid = FALSE;
        }

      else if (failures->len > 0)
        {
          if (options.verbosity >= MMD_DEFAULT)
            {
              fprintf (stderr, "%s failed to validate\n", filename);
              for (gsize i = 0; i < failures->len; i++)
                {
                  doc = (ModulemdSubdocument *)g_ptr_array_index (failures, i);
                  fprintf (stdout,
                           "\nFailed subdocument (%s): \n%s\n",
                           modulemd_subdocument_get_gerror (doc)->message,
                           modulemd_subdocument_get_yaml (doc));
                }
            }
          all_valid = FALSE;
        }

      g_clear_pointer (&objects, g_ptr_array_unref);
      g_clear_pointer (&error, g_error_free);
    }

  if (all_valid)
    {
      return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;
}
