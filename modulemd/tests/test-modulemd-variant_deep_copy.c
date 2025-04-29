/*
 * This file is part of libmodulemd
 * Copyright (C) 2025 Red Hat, Inc.
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
#include <locale.h>
#include <stdlib.h>

#include "private/modulemd-util.h"

/*
 * modulemd_variant_deep_copy() triggered a GLib critical warning
 * from glib >= 2.84.1 when parsing a /data/xmd modulemd-stream-v2 element
 * with {} value (an empty flow mapping). This test exhibits that code path
 * and relies on G_DEBUG=fatal-criticals environment variable to crash the
 * test.
 * <https://github.com/fedora-modularity/libmodulemd/issues/623>.
 */
static void
test_empty_a_sv (void)
{
  g_autoptr (GVariantDict) dictionary =
    NULL; /* g_variant_dict_end() does not free */
  g_autoptr (GVariant) input = NULL;
  g_autoptr (GVariant) output = NULL;

  /* Build a GVariant with an empty dictionary, results to an "a{sv}" of
   * zero size. */
  dictionary = g_variant_dict_new (NULL);
  input = g_variant_dict_end (dictionary);

  /* Exhibit the library. */
  output = modulemd_variant_deep_copy (input);
  g_assert_true (output != NULL);

  /* Compare the content. */
  g_assert_true (g_variant_get_type (output) == g_variant_get_type (input));
  g_assert_true (g_variant_get_size (output) == g_variant_get_size (input));
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  g_test_init (&argc, &argv, NULL);
  g_test_set_nonfatal_assertions ();

  if (!g_setenv ("G_DEBUG", "fatal-criticals", TRUE))
    {
      g_fprintf (stderr, "Failed to set G_DEBUG environment variable.\n");
      exit (EXIT_FAILURE);
    }

  g_test_add_func ("/modulemd/util/variant_deep_copy/empty_a{sv}",
                   test_empty_a_sv);

  return g_test_run ();
}
