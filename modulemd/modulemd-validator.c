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

#include "modulemd.h"
#include "modulemd-yaml.h"

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
        }
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

static GOptionEntry entries[] = { { "verbose",
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
  GError *error = NULL;
  gboolean all_valid = TRUE;
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

  for (gsize i = 1; options.filenames[i]; i++)
    {
      filename = options.filenames[i];
      if (options.verbosity >= MMD_VERBOSE)
        {
          fprintf (stdout, "Validating %s\n", filename);
        }

      if (!parse_yaml_file (filename, NULL, &error))
        {
          fprintf (stderr, "%s failed to validate\n", filename);
          if (options.verbosity >= MMD_VERBOSE)
            {
              fprintf (stdout, "ERROR: %s\n", error->message);
            }
          all_valid = FALSE;
        }

      g_clear_pointer (&error, g_error_free);
    }

  if (all_valid)
    {
      return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;
}
