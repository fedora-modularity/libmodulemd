/* modulemd-module.c
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
#include <glib.h>
#include <yaml.h>

enum
{
    MD_PROP_0,

    //MD_PROP_API,
    //MD_PROP_ARTIFACTS,
    //MD_PROP_BUILDOPTS,
    MD_PROP_BUILDREQUIRES,
    MD_PROP_COMMUNITY,
    //MD_PROP_COMPONENTS,
    //MD_PROP_CONTENT_LIC,
    MD_PROP_DESC,
    MD_PROP_DOCS,
    //MD_PROP_FILTER,
    MD_PROP_MDVERSION,
    //MD_PROP_MODULE_LIC,
    MD_PROP_NAME,
    //MD_PROP_PROFILES,
    MD_PROP_REQUIRES,
    MD_PROP_STREAM,
    MD_PROP_SUMMARY,
    MD_PROP_TRACKER,
    MD_PROP_VERSION,
    MD_PROP_XMD,

    MD_N_PROPERTIES
};

static GParamSpec *md_properties[MD_N_PROPERTIES] = { NULL, };

struct _ModulemdModule
{
    GObject parent_instance;

    /* == Members == */
    // ModulemdAPI *api;
    // ModulemdArtifacts *artifacts;
    // ModulemdBuildopts *buildopts;
    GHashTable *buildrequires;
    gchar *community;
    // ModulemdComponents *components;
    // gchar **content_licenses;
    gchar *description;
    gchar *documentation;
    // ModulemdFilter *filter;
    guint64 mdversion;
    // gchar **module_licenses;
    gchar *name;
    // GHashTable *profiles;
    GHashTable *requires;
    gchar *stream;
    gchar *summary;
    gchar *tracker;
    guint64 version;
    GHashTable *xmd;
};

G_DEFINE_TYPE (ModulemdModule, modulemd_module, G_TYPE_OBJECT)

/**
 * modulemd_module_set_buildrequires:
 * @buildrequires: (element-type utf8 utf8): The requirements to build this
 * module.
 *
 * Sets the 'buildrequires' property.
 */
void
modulemd_module_set_buildrequires (ModulemdModule *self,
                                   GHashTable *buildrequires)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));
    g_return_if_fail (buildrequires);

    if (buildrequires != self->buildrequires) {
        g_hash_table_unref (self->buildrequires);
        self->buildrequires = g_hash_table_ref (buildrequires);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_BUILDREQUIRES]);
    }
}

/**
 * modulemd_module_get_buildrequires:
 *
 * Retrieves the "buildrequires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing the "buildrequires" property.
 */
GHashTable *
modulemd_module_get_buildrequires (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return g_hash_table_ref(self->buildrequires);
}

/**
 * modulemd_module_set_community:
 * @community: the module community.
 *
 * Sets the "community" property.
 */
void
modulemd_module_set_community (ModulemdModule *self,
                               const gchar *community)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));

    if (g_strcmp0(self->community, community) != 0) {
        g_free (self->community);
        self->community = g_strdup (community);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_COMMUNITY]);
    }
}

/**
 * modulemd_module_get_community:
 *
 * Retrieves the "community" for modulemd.
 *
 * Returns: A string containing the "community" property.
 */
const gchar *
modulemd_module_get_community (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return self->community;
}

/**
 * modulemd_module_set_description:
 * @description: the module description.
 *
 * Sets the "description" property.
 */
void
modulemd_module_set_description (ModulemdModule *self,
                                 const gchar *description)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));

    if (g_strcmp0(self->description, description) != 0) {
        g_free (self->description);
        self->description = g_strdup (description);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_DESC]);
    }
}

/**
 * modulemd_module_get_description:
 *
 * Retrieves the "description" for modulemd.
 *
 * Returns: A string containing the "description" property.
 */
const gchar *
modulemd_module_get_description (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return self->description;
}

/**
 * modulemd_module_set_documentation:
 * @documentation: the module documentation.
 *
 * Sets the "documentation" property.
 */
void
modulemd_module_set_documentation (ModulemdModule *self,
                                   const gchar *documentation)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));

    if (g_strcmp0(self->documentation, documentation) != 0) {
        g_free (self->documentation);
        self->documentation = g_strdup (documentation);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_DOCS]);
    }
}

/**
 * modulemd_module_get_documentation:
 *
 * Retrieves the "documentation" for modulemd.
 *
 * Returns: A string containing the "documentation" property.
 */
const gchar *
modulemd_module_get_documentation (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return self->documentation;
}

/**
 * modulemd_module_set_mdversion
 * @mdversion: the metadata version
 *
 * Sets the "mdversion" property.
 */
void
modulemd_module_set_mdversion (ModulemdModule *self,
                               const guint64 mdversion)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));
    if (self->mdversion != mdversion) {
        self->mdversion = mdversion;
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_MDVERSION]);
    }
}

/**
 * modulemd_module_get_mdversion:
 *
 * Retrieves the "mdversion" for modulemd.
 *
 * Returns: A 64-bit unsigned integer containing the "mdversion" property.
 */
const guint64
modulemd_module_get_mdversion (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), 0);

    return self->mdversion;
}

/**
 * modulemd_module_set_name:
 * @name: the module name.
 *
 * Sets the "name" property.
 */
void
modulemd_module_set_name (ModulemdModule *self,
                          const gchar *name)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));

    if (g_strcmp0(self->name, name) != 0) {
        g_free (self->name);
        self->name = g_strdup (name);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_NAME]);
    }
}

/**
 * modulemd_module_get_name:
 *
 * Retrieves the "name" for modulemd.
 *
 * Returns: A string containing the "name" property.
 */
const gchar *
modulemd_module_get_name (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return self->name;
}

/**
 * modulemd_module_set_requires:
 * @requires: (element-type utf8 utf8): The requirements to run this module
 *
 * Sets the 'requires' property.
 */
void
modulemd_module_set_requires (ModulemdModule *self,
                              GHashTable *requires)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));
    g_return_if_fail (requires);

    if (requires != self->requires) {
        g_hash_table_unref (self->requires);
        self->requires = g_hash_table_ref (requires);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_REQUIRES]);
    }
}

/**
 * modulemd_module_get_requires:
 *
 * Retrieves the "requires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing the "requires" property.
 */
GHashTable *
modulemd_module_get_requires (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return self->requires;
}

/**
 * modulemd_module_set_stream:
 * @stream: the module stream.
 *
 * Sets the "stream" property.
 */
void
modulemd_module_set_stream (ModulemdModule *self,
                            const gchar *stream)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));

    if (g_strcmp0(self->stream, stream) != 0) {
        g_free (self->stream);
        self->stream = g_strdup (stream);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_STREAM]);
    }
}

/**
 * modulemd_module_get_stream:
 *
 * Retrieves the "stream" for modulemd.
 *
 * Returns: A string containing the "stream" property.
 */
const gchar *
modulemd_module_get_stream (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return self->stream;
}

/**
 * modulemd_module_set_summary:
 * @summary: the module summary.
 *
 * Sets the "summary" property.
 */
void
modulemd_module_set_summary (ModulemdModule *self,
                             const gchar *summary)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));

    if (g_strcmp0(self->summary, summary) != 0) {
        g_free (self->summary);
        self->summary = g_strdup (summary);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_SUMMARY]);
    }
}

/**
 * modulemd_module_get_summary:
 *
 * Retrieves the "summary" for modulemd.
 *
 * Returns: A string containing the "summary" property.
 */
const gchar *
modulemd_module_get_summary (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return self->summary;
}

/**
 * modulemd_module_set_tracker:
 * @tracker: the module tracker.
 *
 * Sets the "tracker" property.
 */
void
modulemd_module_set_tracker (ModulemdModule *self,
                             const gchar *tracker)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));

    if (g_strcmp0(self->tracker, tracker) != 0) {
        g_free (self->tracker);
        self->tracker = g_strdup (tracker);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_TRACKER]);
    }
}

/**
 * modulemd_module_get_tracker:
 *
 * Retrieves the "tracker" for modulemd.
 *
 * Returns: A string containing the "tracker" property.
 */
const gchar *
modulemd_module_get_tracker (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return self->tracker;
}

/**
 * modulemd_module_set_version
 * @version: the module version
 *
 * Sets the "version" property.
 */
void
modulemd_module_set_version (ModulemdModule *self,
                             const guint64 version)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));
    if (self->version != version) {
        self->version = version;
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_VERSION]);
    }
}

/**
 * modulemd_module_get_version:
 *
 * Retrieves the "version" for modulemd.
 *
 * Returns: A 64-bit unsigned integer containing the "version" property.
 */
const guint64
modulemd_module_get_version (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), 0);

    return self->version;
}

/**
 * modulemd_module_set_xmd:
 * @xmd: (element-type utf8 utf8): Extensible metadata block
 *
 * Sets the 'xmd' property.
 */
void
modulemd_module_set_xmd (ModulemdModule *self,
                         GHashTable *xmd)
{
    g_return_if_fail (MODULEMD_IS_MODULE (self));
    g_return_if_fail (xmd);

    if (xmd != self->xmd) {
        g_hash_table_unref (self->xmd);
        self->xmd = g_hash_table_ref (xmd);
        g_object_notify_by_pspec (G_OBJECT(self),
                                  md_properties [MD_PROP_XMD]);
    }
}

/**
 * modulemd_module_get_xmd:
 *
 * Retrieves the "xmd" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing the "xmd" property.
 */
GHashTable *
modulemd_module_get_xmd (ModulemdModule *self)
{
    g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

    return g_hash_table_ref(self->xmd);
}

static void
modulemd_module_set_property (GObject *gobject,
                              guint property_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
    ModulemdModule *self = MODULEMD_MODULE(gobject);

    switch (property_id) {
    case MD_PROP_BUILDREQUIRES:
        modulemd_module_set_buildrequires(self, g_value_get_boxed(value));
        break;

    case MD_PROP_COMMUNITY:
        modulemd_module_set_community(self, g_value_get_string(value));
        break;

    case MD_PROP_DESC:
        modulemd_module_set_description(self, g_value_get_string(value));
        break;

    case MD_PROP_DOCS:
        modulemd_module_set_documentation(self, g_value_get_string(value));
        break;

    case MD_PROP_MDVERSION:
        modulemd_module_set_mdversion(self, g_value_get_uint64(value));
        break;

    case MD_PROP_NAME:
        modulemd_module_set_name(self, g_value_get_string(value));
        break;

    case MD_PROP_REQUIRES:
        modulemd_module_set_requires(self, g_value_get_boxed(value));
        break;

    case MD_PROP_STREAM:
        modulemd_module_set_stream(self, g_value_get_string(value));
        break;

    case MD_PROP_SUMMARY:
        modulemd_module_set_summary(self, g_value_get_string(value));
        break;

    case MD_PROP_TRACKER:
        modulemd_module_set_tracker(self, g_value_get_string(value));
        break;

    case MD_PROP_VERSION:
        modulemd_module_set_version(self, g_value_get_uint64(value));
        break;

    case MD_PROP_XMD:
        modulemd_module_set_xmd(self, g_value_get_boxed(value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
        break;
    }
}

static void
modulemd_module_get_property (GObject *gobject,
                              guint property_id,
                              GValue *value,
                              GParamSpec *pspec)
{
    ModulemdModule *self = MODULEMD_MODULE(gobject);

    switch (property_id) {
    case MD_PROP_BUILDREQUIRES:
        g_value_set_boxed (value,
                           modulemd_module_get_buildrequires(self));
        break;

    case MD_PROP_COMMUNITY:
        g_value_set_string (value,
                            modulemd_module_get_community(self));
        break;

    case MD_PROP_DESC:
        g_value_set_string (value,
                            modulemd_module_get_description(self));
        break;

    case MD_PROP_DOCS:
        g_value_set_string (value,
                            modulemd_module_get_documentation(self));
        break;
    case MD_PROP_MDVERSION:
        g_value_set_uint64 (value,
                            modulemd_module_get_mdversion(self));
        break;

    case MD_PROP_NAME:
        g_value_set_string (value,
                            modulemd_module_get_name(self));
        break;

    case MD_PROP_REQUIRES:
        g_value_set_boxed (value,
                           modulemd_module_get_requires(self));
        break;

    case MD_PROP_STREAM:
        g_value_set_string (value,
                            modulemd_module_get_stream(self));
        break;

    case MD_PROP_SUMMARY:
        g_value_set_string (value,
                            modulemd_module_get_summary(self));
        break;

    case MD_PROP_TRACKER:
        g_value_set_string (value,
                            modulemd_module_get_tracker(self));
        break;

    case MD_PROP_VERSION:
        g_value_set_uint64 (value,
                            modulemd_module_get_version(self));
        break;

    case MD_PROP_XMD:
        g_value_set_boxed (value,
                           modulemd_module_get_xmd(self));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
        break;
    }
}

static void
modulemd_module_finalize (GObject *gobject)
{
    ModulemdModule *self = (ModulemdModule *)gobject;
    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->stream, g_free);
    g_clear_pointer (&self->summary, g_free);
    g_clear_pointer (&self->description, g_free);
    g_clear_pointer (&self->community, g_free);
    g_clear_pointer (&self->documentation, g_free);
    g_clear_pointer (&self->tracker, g_free);
    g_clear_pointer (&self->buildrequires, g_hash_table_unref);
    g_clear_pointer (&self->requires, g_hash_table_unref);
    g_clear_pointer (&self->xmd, g_hash_table_unref);

    G_OBJECT_CLASS (modulemd_module_parent_class)->finalize (gobject);
}

static void
modulemd_module_class_init (ModulemdModuleClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->set_property = modulemd_module_set_property;
    object_class->get_property = modulemd_module_get_property;

    object_class->finalize = modulemd_module_finalize;

    /**
     * ModulemdModule:buildrequires: (type GLib.HashTable(utf8,utf8)) (transfer container)
     */
    md_properties[MD_PROP_BUILDREQUIRES] =
        g_param_spec_boxed ("buildrequires",
                            "Module BuildRequires",
                            "A dictionary property representing the required "
                            "build dependencies of the module. Keys are the "
                            "required module names (strings), values are their "
                            "required stream names (also strings).",
                            G_TYPE_HASH_TABLE,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    md_properties[MD_PROP_COMMUNITY] =
	    g_param_spec_string ("community",
                             "Module Community",
                             "A string property representing a link to the "
                             "upstream community for this module.",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    md_properties[MD_PROP_DESC] =
	    g_param_spec_string ("description",
                             "Module Description",
                             "A string property representing a detailed "
                             "description of the module.",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    md_properties[MD_PROP_DOCS] =
	    g_param_spec_string ("documentation",
                             "Module Documentation",
                             "A string property representing a link to the "
                             "upstream documentation for this module.",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    md_properties[MD_PROP_MDVERSION] =
	    g_param_spec_uint64 ("mdversion",
                             "Module Metadata Version",
                             "An int property representing the metadata "
                             "format version used.",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    md_properties[MD_PROP_NAME] =
        g_param_spec_string ("name",
                             "Module Name",
                             "A string property representing the name of "
                             "the module.",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * ModulemdModule:requires: (type GLib.HashTable(utf8,utf8)) (transfer container)
     */
    md_properties[MD_PROP_REQUIRES] =
        g_param_spec_boxed ("requires",
                            "Module Requires",
                            "A dictionary property representing the required "
                            "dependencies of the module. Keys are the "
                            "required module names (strings), values are their "
                            "required stream names (also strings).",
                            G_TYPE_HASH_TABLE,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    md_properties[MD_PROP_STREAM] =
	    g_param_spec_string ("stream",
                             "Module Stream",
                             "A string property representing the stream name "
                             "of the module.",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    md_properties[MD_PROP_SUMMARY] =
	    g_param_spec_string ("summary",
                             "Module Short Description",
                             "A string property representing a short summary "
                             "of the module.",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    md_properties[MD_PROP_TRACKER] =
	    g_param_spec_string ("tracker",
                             "Module Bug Tracker",
                             "A string property representing a link to the "
                             "upstream bug tracker for this module.",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    md_properties[MD_PROP_VERSION] =
	    g_param_spec_uint64 ("version",
                             "Module Version",
                             "An integer property representing the version of "
                             "the module.",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * ModulemdModule:xmd: (type GLib.HashTable(utf8,utf8)) (transfer container)
     */
    md_properties[MD_PROP_XMD] =
        g_param_spec_boxed ("xmd",
                            "Extensible Metadata Block",
                            "A dictionary of user-defined keys and values. "
                            "Optional.  Defaults to an empty dictionary. ",
                            G_TYPE_HASH_TABLE,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (
        object_class,
        MD_N_PROPERTIES,
        md_properties);
}

static void
modulemd_module_init (ModulemdModule *self)
{
    /* Allocate the hash table members */
    self->buildrequires = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                g_free, g_free);

    self->requires = g_hash_table_new_full (g_str_hash, g_str_equal,
                                            g_free, g_free);

    self->xmd = g_hash_table_new_full (g_str_hash, g_str_equal,
                                       g_free, g_free);
}

/**
 * modulemd_module_new:
 *
 * Allocates a new #ModulemdModule.
 *
 * Return value: a new #ModulemdModule.
 */
ModulemdModule *
modulemd_module_new (void)
{
    return g_object_new (MODULEMD_TYPE_MODULE, NULL);
}

