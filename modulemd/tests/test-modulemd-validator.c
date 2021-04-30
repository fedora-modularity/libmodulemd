/*
 * This file is part of libmodulemd
 * Copyright (C) 2021 Red Hat, Inc.
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#include <glib.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <locale.h>

gint test_number = 0;
gint failed = 0;

gchar **validator_argv = NULL;
gchar *validator_stdout = NULL;
gchar *validator_stderr = NULL;
gint validator_exit_status = 0;
gint expected_exit_code = 0;
gchar *expected_stdout = NULL;
gchar *expected_stderr = NULL;

static void
ok (gboolean value, const gchar *name)
{
  test_number++;
  if (value)
    {
      g_fprintf (stdout, "ok %d - %s\n", test_number, name);
    }
  else
    {
      failed++;
      g_fprintf (stdout, "not ok %d - %s\n", test_number, name);
    }
}

static void
skip_n (gint tests, const gchar *reason)
{
  while (tests-- > 0)
    {
      test_number++;
      if (!reason)
        reason = "";
      g_fprintf (stdout, "ok %d # SKIP %s\n", test_number, reason);
    }
}

static void
skip (const gchar *reason)
{
  test_number++;
  if (!reason)
    reason = "";
  g_fprintf (stdout, "ok %d # SKIP %s\n", test_number, reason);
}

static gboolean
test_execute (void)
{
  gboolean executed;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *command = g_strjoinv (" ", validator_argv);
  g_fprintf (stdout, "# Executing: %s\n", command);
  executed = g_spawn_sync (NULL,
                           validator_argv,
                           NULL,
                           G_SPAWN_SEARCH_PATH,
                           NULL,
                           NULL,
                           &validator_stdout,
                           &validator_stderr,
                           &validator_exit_status,
                           &error);
  ok (executed, "command executed");
  if (error)
    {
      g_fprintf (stdout, "# Exec failed with: %s\n", error->message);
    }
  return executed;
}

static void
test_stdout (void)
{
  if (expected_stdout)
    {
      const gchar *found =
        g_strstr_len (validator_stdout, -1, expected_stdout);
      ok (NULL != found, "standard output conforms");
      if (!found)
        g_fprintf (stdout,
                   "# expected: %s\n# got: %s\n",
                   expected_stdout,
                   validator_stdout);
    }
  else
    {
      skip ("no check for standard output specified");
    }
}

static void
test_stderr (void)
{
  if (expected_stderr)
    {
      const gchar *found =
        g_strstr_len (validator_stderr, -1, expected_stderr);
      ok (NULL != found, "error output conforms");
      if (!found)
        g_fprintf (stderr,
                   "# expected: %s\n# got: %s\n",
                   expected_stderr,
                   validator_stderr);
    }
  else
    {
      skip ("no check for error output specified");
    }
}

static void
test_exit_code (void)
{
  g_autoptr (GError) error = NULL;
  g_autofree gchar *message = NULL;
  g_spawn_check_exit_status (validator_exit_status, &error);
  message = g_strdup_printf ("exit code was %d", expected_exit_code);
  if (0 == expected_exit_code)
    ok (!error, message);
  else
    ok (error && error->domain == G_SPAWN_EXIT_ERROR &&
          error->code == expected_exit_code,
        message);
}


int
main (int argc, char *argv[])
{
  GOptionEntry entries[] = {
    { "code",
      '\0',
      G_OPTION_FLAG_NONE,
      G_OPTION_ARG_INT,
      &expected_exit_code,
      "Expected exit code (default is 0)",
      NULL },
    { "stdout",
      '\0',
      G_OPTION_FLAG_NONE,
      G_OPTION_ARG_STRING,
      &expected_stdout,
      "Check standard output for a substring (default is no check)",
      NULL },
    { "stderr",
      '\0',
      G_OPTION_FLAG_NONE,
      G_OPTION_ARG_STRING,
      &expected_stderr,
      "Check error output for a substring (default is no check)",
      NULL },
    { NULL }
  };
  GOptionContext *context;
  g_autoptr (GError) error = NULL;

  setlocale (LC_ALL, "");

  /* "-- --help" is not handled as a non-option if g_test_init() is used.
   * Therefore this program does not Glib test frame work. */
  context = g_option_context_new (
    "MODULEMD_VALIDATOR_EXECUTABLE [MODULEMD_VALIDATOR_ARGUMENT...] - "
    "test modulemd-validator behavior");
  g_option_context_add_main_entries (context, entries, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_fprintf (stderr, "Could not parse arguments: %s\n", error->message);
      exit (EXIT_FAILURE);
    }
  g_option_context_free (context);
  validator_argv = argv + 1;
  if (!g_strcmp0 (validator_argv[0], "--"))
    {
      validator_argv++;
    }
  if (!validator_argv[0])
    {
      g_fprintf (stderr, "No positional arguments.\n");
      exit (EXIT_FAILURE);
    }

  g_fprintf (stdout, "1..4\n");
  if (test_execute ())
    {
      test_exit_code ();
      test_stdout ();
      test_stderr ();
    }
  else
    {
      skip_n (3, "program failed to execute");
    }
  g_free (expected_stdout);
  g_free (expected_stderr);
  g_free (validator_stdout);
  g_free (validator_stderr);
  exit (failed ? EXIT_FAILURE : EXIT_SUCCESS);
}
