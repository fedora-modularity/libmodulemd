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

#include <glib.h>
#include <locale.h>

#include "modulemd.h"
#include "private/modulemd-subdocument-private.h"

typedef struct _SubdocumentFixture
{
} SubdocumentFixture;


static void
modulemd_subdocument_basic (SubdocumentFixture *fixture,
                            gconstpointer user_data)
{
  const gchar *yaml = "document: modulemd\nversion: 1";
  g_autoptr (ModulemdSubdocument) document = NULL;

  document = modulemd_subdocument_new ();

  modulemd_subdocument_set_doctype (document, MODULEMD_TYPE_MODULE);
  g_assert_cmpuint (
    modulemd_subdocument_get_doctype (document), ==, MODULEMD_TYPE_MODULE);

  modulemd_subdocument_set_version (document, 1);
  g_assert_cmpuint (modulemd_subdocument_get_version (document), ==, 1);

  modulemd_subdocument_set_yaml (document, yaml);
  g_assert_cmpstr (modulemd_subdocument_get_yaml (document), ==, yaml);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  g_test_add ("/modulemd/regressions/issue14_v1",
              SubdocumentFixture,
              NULL,
              NULL,
              modulemd_subdocument_basic,
              NULL);

  return g_test_run ();
}
