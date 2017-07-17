/* modulemd-metadata.c
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

    //MD_PROP_API,
    //MD_PROP_ARTIFACTS,
    //MD_PROP_BUILDOPTS,
    //MD_PROP_BUILDREQUIRES,
    MD_PROP_COMMUNITY,
    //MD_PROP_COMPONENTS,
    //MD_PROP_CONTENT_LIC,
    MD_PROP_DESC,
    MD_PROP_DOCS,
    //MD_PROP_FILTER,
    //MD_PROP_MDVERSION,
    //MD_PROP_MODULE_LIC,
    MD_PROP_NAME,
    //MD_PROP_PROFILES,
    //MD_PROP_REQUIRES,
    MD_PROP_STREAM,
    MD_PROP_SUMMARY,
    MD_PROP_TRACKER,
    //MD_PROP_VERSION,
    //MD_PROP_XMD,

    MD_N_PROPERTIES
};

static GParamSpec *md_properties[MD_N_PROPERTIES] = { NULL, };


struct _ModulemdModuleMetadata
{
    GObject parent_instance;

    /* == Members == */
    // ModulemdModuleAPI *api;
    // ModulemdModuleArtifacts *artifacts;
    // ModulemdModuleBuildopts *buildopts;
    // GHashTable *buildrequires;
    gchar *community;
    // ModulemdModuleComponents *components;
    // gchar **content_licenses;
    gchar *description;
    gchar *documentation;
    // ModulemdModuleFilter *filter;
    // gint mdversion;
    // gchar **module_licenses;
    gchar *name;
    // GHashTable *profiles;
    // GHashTable *requires;
    gchar *stream;
    gchar *summary;
    gchar *tracker;
    // gint version;
    // GHashTable *xmd;
};

G_DEFINE_TYPE (ModulemdModuleMetadata, modulemd_modulemetadata, G_TYPE_OBJECT)

static void
modulemd_modulemetadata_set_property (GObject *gobject,
                                      guint property_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
    ModulemdModuleMetadata *self = MODULEMD_MODULEMETADATA(gobject);

    switch (property_id) {
    /* Simple string properties */
    case MD_PROP_COMMUNITY:
        g_clear_pointer (&self->community, g_free);
        self->community = g_value_dup_string (value);
        break;
    case MD_PROP_DESC:
        g_clear_pointer (&self->description, g_free);
        self->description = g_value_dup_string (value);
        break;
    case MD_PROP_DOCS:
        g_clear_pointer (&self->documentation, g_free);
        self->documentation = g_value_dup_string (value);
        break;
    case MD_PROP_NAME:
        g_clear_pointer (&self->name, g_free);
        self->name = g_value_dup_string (value);
        break;
    case MD_PROP_STREAM:
        g_clear_pointer (&self->stream, g_free);
        self->stream = g_value_dup_string (value);
        break;
    case MD_PROP_SUMMARY:
        g_clear_pointer (&self->summary, g_free);
        self->summary = g_value_dup_string (value);
        break;
    case MD_PROP_TRACKER:
        g_clear_pointer (&self->tracker, g_free);
        self->tracker = g_value_dup_string (value);
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
    ModulemdModuleMetadata *self = MODULEMD_MODULEMETADATA(gobject);

    switch (property_id) {
    /* Simple string properties */
    case MD_PROP_COMMUNITY:
        g_value_set_string (value, self->community);
        break;

    case MD_PROP_DESC:
        g_value_set_string (value, self->description);
        break;

    case MD_PROP_DOCS:
        g_value_set_string (value, self->documentation);
        break;

    case MD_PROP_NAME:
        g_value_set_string (value, self->name);
        break;

    case MD_PROP_STREAM:
        g_value_set_string (value, self->stream);
        break;

    case MD_PROP_SUMMARY:
        g_value_set_string (value, self->summary);
        break;

    case MD_PROP_TRACKER:
        g_value_set_string (value, self->tracker);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
        break;
    }
}

static void
modulemd_modulemetadata_dispose (GObject *gobject)
{
    G_OBJECT_CLASS (modulemd_modulemetadata_parent_class)->dispose (gobject);
}

static void
modulemd_modulemetadata_finalize (GObject *gobject)
{
    ModulemdModuleMetadata *self = MODULEMD_MODULEMETADATA(gobject);
    g_clear_pointer (&self->name, g_free);

    G_OBJECT_CLASS (modulemd_modulemetadata_parent_class)->finalize (gobject);
}

static void
modulemd_modulemetadata_class_init (ModulemdModuleMetadataClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->set_property = modulemd_modulemetadata_set_property;
    object_class->get_property = modulemd_modulemetadata_get_property;

    object_class->dispose = modulemd_modulemetadata_dispose;
    object_class->finalize = modulemd_modulemetadata_finalize;

    md_properties[MD_PROP_COMMUNITY] =
	    g_param_spec_string ("community",
                             "Module Community",
                             "A string property representing a link to the "
                             "upstream community for this module.",
                             "",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    md_properties[MD_PROP_DESC] =
	    g_param_spec_string ("description",
                             "Module Description",
                             "A string property representing a detailed "
                             "description of the module.",
                             "",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    md_properties[MD_PROP_DOCS] =
	    g_param_spec_string ("documentation",
                             "Module Documentation",
                             "A string property representing a link to the "
                             "upstream documentation for this module.",
                             "",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    md_properties[MD_PROP_NAME] =
        g_param_spec_string ("name",
                             "Module Name",
                             "A string property representing the name of "
                             "the module.",
                             "",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    md_properties[MD_PROP_STREAM] =
	    g_param_spec_string ("stream",
                             "Module Stream",
                             "A string property representing the stream name "
                             "of the module.",
                             "",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    md_properties[MD_PROP_SUMMARY] =
	    g_param_spec_string ("summary",
                             "Module Short Description",
                             "A string property representing a short summary "
                             "of the module.",
                             "",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    md_properties[MD_PROP_TRACKER] =
	    g_param_spec_string ("tracker",
                             "Module Bug Tracker",
                             "A string property representing a link to the "
                             "upstream bug tracker for this module.",
                             "",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties (
        object_class,
        MD_N_PROPERTIES,
        md_properties);
}

static void
modulemd_modulemetadata_init (ModulemdModuleMetadata *self)
{
}

/**
 * modulemd_modulemetadata_new:
 *
 * Allocates a new #ModulemdModuleMetadata.
 *
 * Return value: a new #ModulemdModuleMetadata.
 */
ModulemdModuleMetadata *
modulemd_modulemetadata_new (void)
{
    ModulemdModuleMetadata *md;

    md = g_object_new (modulemd_modulemetadata_get_type(), NULL);
    return md;
}
