/* modulemd-modulemetadata.c
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
#include <yaml.h>

enum
{
    MD_PROP_0,

    MD_PROP_NAME,

    MD_N_PROPERTIES
};

static GParamSpec *md_properties[MD_N_PROPERTIES] = { NULL, };

struct module_dep_ref {
    char *name;
    char *ref; /* the stream, git tag or other commit-ish */
};

struct modulemd_component {
    gchar *rationale;
    gchar *repository;
    gchar *cache;
    gchar *ref;
    gchar **arches;
    gchar **multilib;
    guint buildorder;

};

struct _ModuleMetadata
{
    GObject parent_instance;

    gchar *name;
    gchar *stream;
    gchar *version;
    gchar *summary;
    gchar *description;
    gchar *community;
    gchar *documentation;
    gchar *tracker;
    gchar **module_licenses;
    gchar **content_licenses;
    GHashTable *xmd;
    struct module_dep_ref **build_deps;
    struct module_dep_ref **runtime_deps;
    GHashTable *profiles;
    GHashTable *api;
    GHashTable *filter;
    GHashTable *buildopts;
    GHashTable *components;
};

G_DEFINE_TYPE (ModuleMetadata, modulemd_modulemetadata, G_TYPE_OBJECT)

static void
modulemd_modulemetadata_set_property (GObject *gobject,
                                      guint property_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
    ModuleMetadata *self = MODULEMD_MODULEMETADATA(gobject);

    switch (property_id) {
    case MD_PROP_NAME:
        g_clear_pointer (&self->name, g_free);
        self->name = g_value_dup_string (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
        break;
    }
}

static void
modulemd_modulemetadata_get_property (GObject *gobject,
                                      guint property_id,
                                      GValue *value,
                                      GParamSpec *pspec)
{
}

static void
modulemd_modulemetadata_dispose (GObject *gobject)
{
    G_OBJECT_CLASS (modulemd_modulemetadata_parent_class)->dispose (gobject);
}

static void
modulemd_modulemetadata_finalize (GObject *gobject)
{
    ModuleMetadata *self = MODULEMD_MODULEMETADATA(gobject);
    g_clear_pointer (&self->name, g_free);

    G_OBJECT_CLASS (modulemd_modulemetadata_parent_class)->finalize (gobject);
}

static void
modulemd_modulemetadata_class_init (ModuleMetadataClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->set_property = modulemd_modulemetadata_set_property;
    object_class->get_property = modulemd_modulemetadata_get_property;

    object_class->dispose = modulemd_modulemetadata_dispose;
    object_class->finalize = modulemd_modulemetadata_finalize;

    md_properties[MD_PROP_NAME] =
	    g_param_spec_string ("name",
                             "Module Name",
                             "A string property representing the name of "
                             "the module.",
                             "",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties (
        object_class,
        MD_N_PROPERTIES,
        md_properties);
}

static void
modulemd_modulemetadata_init (ModuleMetadata *self)
{
}

/**
 * modulemd_modulemetadata_new:
 *
 * Allocates a new #ModuleMetadata.
 *
 * Return value: a new #ModuleMetadata.
 */
ModuleMetadata *
modulemd_modulemetadata_new (void)
{
    ModuleMetadata *md;

    md = g_object_new (modulemd_modulemetadata_get_type(), NULL);
    return md;
}
