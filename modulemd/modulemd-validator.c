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
#include <popt.h>

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
};

int
main (int argc, const char *argv[])
{
  int opt;
  poptContext pc;
  struct validator_options *options;
  const char *filename;
  GError *error = NULL;
  ModulemdModule **modules = NULL;
  gboolean all_valid = TRUE;
  setlocale (LC_ALL, "");

  options = g_malloc0 (sizeof (struct validator_options));

  struct poptOption long_options[] = {
    { "verbose",
      'v',
      POPT_ARG_VAL,
      &options->verbosity,
      MMD_VERBOSE,
      _ ("Display progress messages."),
      NULL },
    { "debug",
      '\0',
      POPT_ARG_VAL,
      &options->verbosity,
      MMD_DEBUG,
      _ ("Display progress and debug messages."),
      NULL },
    POPT_AUTOHELP{ NULL, 0, 0, NULL, 0 },
    POPT_TABLEEND
  };

  pc = poptGetContext (argv[0], argc, argv, long_options, 0);
  poptSetOtherOptionHelp (pc,
                          "[OPTION...] <modulemd_file> [<modulemd_file> ...]");

  while ((opt = poptGetNextOpt (pc)) != -1)
    {
      switch (opt)
        {
        default:
          fprintf (stderr,
                   "\nInvalid option %s: %s\n\n",
                   poptBadOption (pc, 0),
                   poptStrerror (opt));
          poptPrintUsage (pc, stderr, 0);
          return EXIT_FAILURE;
        }
    }

  if (!poptPeekArg (pc))
    {
      fprintf (stderr,
               "At least one file must be specified on the command-line\n");
      return EXIT_FAILURE;
    }

  while ((filename = poptGetArg (pc)))
    {
      if (options->verbosity >= MMD_VERBOSE)
        {
          fprintf (stdout, "Validating %s\n", filename);
        }

      modules = parse_yaml_file (filename, &error);

      if (error)
        {
          fprintf (stderr, "%s failed to validate\n", filename);
          if (options->verbosity >= MMD_VERBOSE)
            {
              fprintf (stdout, "ERROR: %s\n", error->message);
            }
          all_valid = FALSE;
        }

      if (modules)
        {
          for (gsize i = 0; modules[i]; i++)
            {
              g_object_unref (modules[i]);
            }
          g_clear_pointer (&modules, g_free);
        }

      g_clear_pointer (&error, g_error_free);
    }

  if (all_valid)
    {
      return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;
}
