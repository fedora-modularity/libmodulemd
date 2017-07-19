/* test-modulemd-metadata.c
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

#include "modulemd-modulemetadata.h"

#include <glib.h>
#include <locale.h>

typedef struct _ModuleMetadataFixture {
    ModulemdModuleMetadata *md;
} ModuleMetadataFixture;

static void
modulemd_modulemetadata_set_up(ModuleMetadataFixture *fixture,
                               gconstpointer user_data)
{
    fixture->md = modulemd_modulemetadata_new();
}

static void
modulemd_modulemetadata_tear_down(ModuleMetadataFixture *fixture,
                                  gconstpointer user_data)
{
    modulemd_modulemetadata_free(fixture->md);
}

static void
modulemd_modulemetadata_test_community(ModuleMetadataFixture *fixture,
                                       gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_community(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_community(md, "MyCommunity");
    g_assert_cmpstr(modulemd_modulemetadata_get_community(md), ==, "MyCommunity");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_community(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_community(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_description(ModuleMetadataFixture *fixture,
                                         gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_description(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_description(md, "ModuleDesc");
    g_assert_cmpstr(modulemd_modulemetadata_get_description(md), ==, "ModuleDesc");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_description(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_description(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_documentation(ModuleMetadataFixture *fixture,
                                           gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_documentation(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_documentation(md, "ModuleDocs");
    g_assert_cmpstr(modulemd_modulemetadata_get_documentation(md), ==, "ModuleDocs");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_documentation(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_documentation(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_name(ModuleMetadataFixture *fixture,
                                  gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_name(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_name(md, "ModuleName");
    g_assert_cmpstr(modulemd_modulemetadata_get_name(md), ==, "ModuleName");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_name(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_name(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_stream(ModuleMetadataFixture *fixture,
                                    gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_stream(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_stream(md, "ModuleStream");
    g_assert_cmpstr(modulemd_modulemetadata_get_stream(md), ==, "ModuleStream");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_stream(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_stream(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_summary(ModuleMetadataFixture *fixture,
                                     gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_summary(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_summary(md, "ModuleSummary");
    g_assert_cmpstr(modulemd_modulemetadata_get_summary(md), ==, "ModuleSummary");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_summary(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_summary(md), ==, NULL);
}

static void
modulemd_modulemetadata_test_tracker(ModuleMetadataFixture *fixture,
                                     gconstpointer user_data)
{
    ModulemdModuleMetadata *md = fixture->md;

    /* Should be initialized to NULL */
    g_assert_cmpstr(modulemd_modulemetadata_get_tracker(md), ==, NULL);

    /* Assign a valid string */
    modulemd_modulemetadata_set_tracker(md, "ModuleTracker");
    g_assert_cmpstr(modulemd_modulemetadata_get_tracker(md), ==, "ModuleTracker");

    /* Reassign it to NULL */
    modulemd_modulemetadata_set_tracker(md, NULL);
    g_assert_cmpstr(modulemd_modulemetadata_get_tracker(md), ==, NULL);
}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);
    g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

    // Define the tests.
    g_test_add ("/modulemd/modulemetadata/test_prop_community",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_community,
                modulemd_modulemetadata_tear_down);
    g_test_add ("/modulemd/modulemetadata/test_prop_description",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_description,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_prop_documentation",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_documentation,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_prop_name",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_name,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_prop_stream",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_stream,
                modulemd_modulemetadata_tear_down);

    g_test_add ("/modulemd/modulemetadata/test_prop_summary",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_summary,
                modulemd_modulemetadata_tear_down);
    g_test_add ("/modulemd/modulemetadata/test_prop_tracker",
                ModuleMetadataFixture, NULL,
                modulemd_modulemetadata_set_up,
                modulemd_modulemetadata_test_tracker,
                modulemd_modulemetadata_tear_down);

    return g_test_run ();
}
