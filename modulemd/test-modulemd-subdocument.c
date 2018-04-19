/* test-modulemd-subdocument.c
 *
 * Copyright (C) 2018 Stephen Gallagher
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
#include <locale.h>

#include "modulemd.h"
#include "modulemd-subdocument-private.h"

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
