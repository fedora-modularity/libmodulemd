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

#define _POSIX_SOURCE  /* for fdopen() */

#include "modulemd.h"
#include "private/modulemd-yaml.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <stdio.h>

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

static gchar *
run_fedmod_lint (ModulemdSubdocument *doc,
                 const gchar *fedmod)
{
  gint mmd_fd;
  FILE *mmd_file = NULL;
  g_autofree gchar *mmd_filename = NULL;
  gint retval;
  gchar *fedmod_lint_output = NULL;
  const gchar *argv[] = {fedmod, "fedmod", "lint", NULL};

  g_return_val_if_fail (doc != NULL, NULL);
  g_return_val_if_fail (fedmod != NULL, NULL);

  mmd_fd = g_file_open_tmp ("fedmod-lint-XXXXXX",
                            &mmd_filename,
                            NULL);

  if (mmd_fd < 0)
    {
      g_warning ("Couldn't open temporary file for linting.");
      goto exit;
    }

  mmd_file = fdopen (mmd_fd, "w");

  if (! mmd_file)
    {
      g_warning ("Couldn't associate temp file stream for linting.");
      goto exit;
    }

  retval = fprintf (mmd_file, "%s", modulemd_subdocument_get_yaml (doc));

  if (retval < 0)
    {
      g_warning ("Couldn't write MMD to temp file for linting.");
      goto exit;
    }

  argv[3] = mmd_filename;

  g_spawn_sync (NULL,
                (gchar **) argv,
                NULL,
                G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_FILE_AND_ARGV_ZERO,
                NULL,
                NULL,
                &fedmod_lint_output,
                NULL,
                NULL,
                NULL);

exit:
  if (mmd_file)
    {
      fclose (mmd_file);
    }
  else if (mmd_fd >= 0)
    {
      g_close (mmd_fd, NULL);
    }

  if (mmd_filename)
    {
      g_unlink (mmd_filename);
    }

  return fedmod_lint_output;
}

static void
summarize_failures (ModulemdSubdocument *doc,
                    const gchar *fedmod)
{
  g_autofree gchar *details = NULL;
  g_return_if_fail (doc != NULL);

  if (fedmod)
    {
      details = run_fedmod_lint (doc, fedmod);
    }

  if (! details)
    {
      /* Either 'fedmod' isn't available or something went wrong running
       * 'fedmod lint', fall back to displaying the failed YAML document. */
      details = g_strdup (modulemd_subdocument_get_yaml (doc));
    }

  fprintf (stdout,
           "\nFailed subdocument (%s): \n%s\n",
           modulemd_subdocument_get_gerror (doc)->message,
           details);
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
  g_autofree gchar *fedmod = NULL;
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

  fedmod = g_find_program_in_path ("fedmod");

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
                  summarize_failures (doc, fedmod);
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
