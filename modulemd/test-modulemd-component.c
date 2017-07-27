/* test-modulemd-module.c
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

#include "modulemd-component.h"

#include <glib.h>
#include <locale.h>

typedef struct _ComponentFixture
{
    ModulemdComponent *component;
} ComponentFixture;

static void
modulemd_component_set_up (ComponentFixture *fixture,
                           gconstpointer    user_data)
{
    fixture->component = modulemd_component_new ();
}

static void
modulemd_component_tear_down (ComponentFixture *fixture,
                              gconstpointer    user_data)
{
    g_object_unref (fixture->component);
}

static void
modulemd_component_test_create (ComponentFixture *fixture,
                                gconstpointer    user_data)
{
    g_assert_true (MODULEMD_IS_COMPONENT(fixture->component));
}

int
main (int argc, char *argv[])
{

    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);
    g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

    // Define the tests.
    g_test_add ("/modulemd/component/test_create",
                ComponentFixture, NULL,
                modulemd_component_set_up,
                modulemd_component_test_create,
                modulemd_component_tear_down);

    return g_test_run ();
}

