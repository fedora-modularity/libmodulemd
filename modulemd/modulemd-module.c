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
#include "modulemd-yaml.h"
#include "modulemd-util.h"
#include <glib.h>
#include <yaml.h>

enum
{
  MD_PROP_0,

  MD_PROP_BUILDREQUIRES,
  MD_PROP_COMMUNITY,
  MD_PROP_CONTENT_LIC,
  MD_PROP_DESC,
  MD_PROP_DOCS,
  MD_PROP_MDVERSION,
  MD_PROP_MODULE_LIC,
  MD_PROP_MODULE_COMPONENTS,
  MD_PROP_NAME,
  MD_PROP_PROFILES,
  MD_PROP_RPM_API,
  MD_PROP_RPM_ARTIFACTS,
  MD_PROP_RPM_COMPONENTS,
  MD_PROP_RPM_BUILDOPTS,
  MD_PROP_RPM_FILTER,
  MD_PROP_REQUIRES,
  MD_PROP_STREAM,
  MD_PROP_SUMMARY,
  MD_PROP_TRACKER,
  MD_PROP_VERSION,
  MD_PROP_XMD,

  MD_N_PROPERTIES
};

static GParamSpec *md_properties[MD_N_PROPERTIES] = {
  NULL,
};

struct _ModulemdModule
{
  GObject parent_instance;

  /* == Members == */
  GHashTable *buildrequires;
  gchar *community;
  ModulemdSimpleSet *content_licenses;
  gchar *description;
  gchar *documentation;
  guint64 mdversion;
  GHashTable *module_components;
  ModulemdSimpleSet *module_licenses;
  gchar *name;
  GHashTable *profiles;
  GHashTable *requires;
  ModulemdSimpleSet *rpm_api;
  ModulemdSimpleSet *rpm_artifacts;
  GHashTable *rpm_buildopts;
  GHashTable *rpm_components;
  ModulemdSimpleSet *rpm_filter;
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

  if (buildrequires != self->buildrequires)
    {
      g_hash_table_unref (self->buildrequires);
      self->buildrequires = _modulemd_hash_table_deep_str_copy (buildrequires);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_BUILDREQUIRES]);
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

  return g_hash_table_ref (self->buildrequires);
}

/**
 * modulemd_module_set_community:
 * @community: the module community.
 *
 * Sets the "community" property.
 */
void
modulemd_module_set_community (ModulemdModule *self, const gchar *community)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (g_strcmp0 (self->community, community) != 0)
    {
      g_free (self->community);
      self->community = g_strdup (community);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_COMMUNITY]);
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
 * modulemd_module_set_content_licenses:
 * @licenses: A #ModuleSimpleSet: The licenses under which the components of
 * this module are released.
 *
 * Sets the content_licenses property.
 */
void
modulemd_module_set_content_licenses (ModulemdModule *self,
                                      ModulemdSimpleSet *licenses)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (MODULEMD_IS_SIMPLESET (licenses));

  /* TODO: Test for differences before replacing */
  g_object_unref (self->content_licenses);
  self->content_licenses = g_object_ref (licenses);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_CONTENT_LIC]);
}

/**
 * modulemd_module_get_content_licenses:
 *
 * Retrieves the "content_licenses" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of licenses in the
 * "content_licenses" property.
 */
ModulemdSimpleSet *
modulemd_module_get_content_licenses (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_object_ref (self->content_licenses);
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

  if (g_strcmp0 (self->description, description) != 0)
    {
      g_free (self->description);
      self->description = g_strdup (description);
      g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_DESC]);
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

  if (g_strcmp0 (self->documentation, documentation) != 0)
    {
      g_free (self->documentation);
      self->documentation = g_strdup (documentation);
      g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_DOCS]);
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
modulemd_module_set_mdversion (ModulemdModule *self, const guint64 mdversion)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  if (self->mdversion != mdversion)
    {
      self->mdversion = mdversion;
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_MDVERSION]);
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
 * modulemd_module_set_module_components:
 * @components: (element-type utf8 ModulemdComponentModule): The hash table of
 * module components that comprise this module. The keys are the module name,
 * the values are a #ModulemdComponentModule containing information about that
 * module.
 *
 * Sets the module_components property.
 */
void
modulemd_module_set_module_components (ModulemdModule *self,
                                       GHashTable *components)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (components);

  if (components != self->module_components)
    {
      g_hash_table_unref (self->module_components);
      self->module_components =
        _modulemd_hash_table_deep_obj_copy (components);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_MODULE_COMPONENTS]);
    }
}

/**
 * modulemd_module_get_module_components:
 *
 * Retrieves the "module-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentModule) (transfer container): A hash table
 * containing the "module-components" property.
 */
GHashTable *
modulemd_module_get_module_components (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_hash_table_ref (self->module_components);
}

/**
 * modulemd_module_set_module_licenses:
 * @licenses: A #ModuleSimpleSet: The licenses under which the components of
 * this module are released.
 *
 * Sets the module_licenses property.
 */
void
modulemd_module_set_module_licenses (ModulemdModule *self,
                                     ModulemdSimpleSet *licenses)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (MODULEMD_IS_SIMPLESET (licenses));

  /* TODO: Test for differences before replacing */
  g_object_unref (self->module_licenses);
  self->module_licenses = g_object_ref (licenses);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_MODULE_LIC]);
}

/**
 * modulemd_module_get_module_licenses:
 *
 * Retrieves the "module_licenses" for modulemd
 *
 * Returns: (transfer full): a #ModulemdSimpleSet containing the set of
 * licenses in the "module_licenses" property.
 */
ModulemdSimpleSet *
modulemd_module_get_module_licenses (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_object_ref (self->module_licenses);
}

/**
 * modulemd_module_set_name:
 * @name: the module name.
 *
 * Sets the "name" property.
 */
void
modulemd_module_set_name (ModulemdModule *self, const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (g_strcmp0 (self->name, name) != 0)
    {
      g_free (self->name);
      self->name = g_strdup (name);
      g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_NAME]);
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
 * modulemd_module_set_profiles:
 * @profiles: (element-type utf8 ModulemdProfile): The profiles avaiable for
 * this module.
 *
 * Sets the 'profiles' property.
 */

void
modulemd_module_set_profiles (ModulemdModule *self, GHashTable *profiles)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (profiles);

  if (profiles != self->profiles)
    {
      g_hash_table_unref (self->profiles);
      self->profiles = _modulemd_hash_table_deep_obj_copy (profiles);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_PROFILES]);
    }
}

/**
 * modulemd_module_get_profiles:
 *
 * Retrieves the "profiles" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdProfile) (transfer container): A hash
 * table containing the "profiles" property.
 */
GHashTable *
modulemd_module_get_profiles (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_hash_table_ref (self->profiles);
}

/**
 * modulemd_module_set_requires:
 * @requires: (element-type utf8 utf8): The requirements to run this module
 *
 * Sets the 'requires' property.
 */
void
modulemd_module_set_requires (ModulemdModule *self, GHashTable *requires)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (requires);

  if (requires != self->requires)
    {
      g_hash_table_unref (self->requires);
      self->requires = _modulemd_hash_table_deep_str_copy (requires);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_REQUIRES]);
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

  return g_hash_table_ref (self->requires);
}

/**
 * modulemd_module_set_rpm_api:
 * @apis: A #ModuleSimpleSet: The set of binary RPM packages that form the
 * public API for this module.
 *
 * Sets the rpm_api property.
 */
void
modulemd_module_set_rpm_api (ModulemdModule *self, ModulemdSimpleSet *apis)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (MODULEMD_IS_SIMPLESET (apis));

  /* TODO: Test for differences before replacing */
  g_object_unref (self->rpm_api);
  self->rpm_api = g_object_ref (apis);

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_RPM_API]);
}

/**
 * modulemd_module_get_rpm_api:
 *
 * Retrieves the "rpm_api" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of binary RPM
 * packages in the "rpm_api" property.
 */
ModulemdSimpleSet *
modulemd_module_get_rpm_api (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_object_ref (self->rpm_api);
}

/**
 * modulemd_module_set_rpm_artifacts:
 * @artifacts: A #ModuleSimpleSet: The set of binary RPM packages that are
 * contained in this module. Generally populated by the module build service.
 *
 * Sets the rpm_artifacts property.
 */
void
modulemd_module_set_rpm_artifacts (ModulemdModule *self,
                                   ModulemdSimpleSet *artifacts)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (MODULEMD_IS_SIMPLESET (artifacts));

  /* TODO: Test for differences before replacing */
  g_object_unref (self->rpm_artifacts);
  self->rpm_artifacts = g_object_ref (artifacts);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_RPM_ARTIFACTS]);
}

/**
 * modulemd_module_get_rpm_artifacts:
 *
 * Retrieves the "rpm_artifacts" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of binary RPMs
 * contained in this module.
 */
ModulemdSimpleSet *
modulemd_module_get_rpm_artifacts (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_object_ref (self->rpm_artifacts);
}


/**
 * modulemd_module_set_rpm_buildopts:
 * @buildopts: (element-type utf8 utf8) (transfer container): A dictionary of
 * build options to pass to rpmbuild. Currently the only recognized key is
 * "macros".
 *
 * Sets the 'rpm-buildopts' property.
 */
void
modulemd_module_set_rpm_buildopts (ModulemdModule *self, GHashTable *buildopts)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (buildopts);

  if (buildopts != self->rpm_buildopts)
    {
      g_hash_table_unref (self->rpm_buildopts);
      self->rpm_buildopts = _modulemd_hash_table_deep_str_copy (buildopts);

      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_RPM_BUILDOPTS]);
    }
}

/**
 * modulemd_module_get_rpm_buildopts:
 *
 * Retrieves the "rpm-buildopts" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing the "rpm-buildopts" property.
 */
GHashTable *
modulemd_module_get_rpm_buildopts (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_hash_table_ref (self->rpm_buildopts);
}

/**
 * modulemd_module_set_rpm_components:
 * @components: (element-type utf8 ModulemdComponentRpm): The hash table of
 * module components that comprise this module. The keys are the module name,
 * the values are a #ModulemdComponentRpm containing information about that
 * module.
 *
 * Sets the rpm_components property.
 */
void
modulemd_module_set_rpm_components (ModulemdModule *self,
                                    GHashTable *components)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (components);

  if (components != self->rpm_components)
    {
      g_hash_table_unref (self->rpm_components);
      self->rpm_components = _modulemd_hash_table_deep_obj_copy (components);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_RPM_COMPONENTS]);
    }
}

/**
 * modulemd_module_get_rpm_components:
 *
 * Retrieves the "rpm-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentRpm) (transfer container): A hash table
 * containing the "rpm-components" property.
 */
GHashTable *
modulemd_module_get_rpm_components (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_hash_table_ref (self->rpm_components);
}

/**
 * modulemd_module_set_rpm_filter:
 * @filter: A #ModuleSimpleSet: The set of binary RPM packages that are
 * explicitly filtered out of this module.
 *
 * Sets the rpm_artifacts property.
 */
void
modulemd_module_set_rpm_filter (ModulemdModule *self,
                                ModulemdSimpleSet *filter)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (MODULEMD_IS_SIMPLESET (filter));

  /* TODO: Test for differences before replacing */
  g_object_unref (self->rpm_filter);
  self->rpm_filter = g_object_ref (filter);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_RPM_FILTER]);
}

/**
 * modulemd_module_get_rpm_filter:
 *
 * Retrieves the "rpm_filter" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of binary RPMs
 * filtered out of this module.
 */
ModulemdSimpleSet *
modulemd_module_get_rpm_filter (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_object_ref (self->rpm_filter);
}

/**
 * modulemd_module_set_stream:
 * @stream: the module stream.
 *
 * Sets the "stream" property.
 */
void
modulemd_module_set_stream (ModulemdModule *self, const gchar *stream)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (g_strcmp0 (self->stream, stream) != 0)
    {
      g_free (self->stream);
      self->stream = g_strdup (stream);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_STREAM]);
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
modulemd_module_set_summary (ModulemdModule *self, const gchar *summary)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (g_strcmp0 (self->summary, summary) != 0)
    {
      g_free (self->summary);
      self->summary = g_strdup (summary);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_SUMMARY]);
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
modulemd_module_set_tracker (ModulemdModule *self, const gchar *tracker)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (g_strcmp0 (self->tracker, tracker) != 0)
    {
      g_free (self->tracker);
      self->tracker = g_strdup (tracker);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_TRACKER]);
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
modulemd_module_set_version (ModulemdModule *self, const guint64 version)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  if (self->version != version)
    {
      self->version = version;
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_VERSION]);
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
modulemd_module_set_xmd (ModulemdModule *self, GHashTable *xmd)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (xmd);

  if (xmd != self->xmd)
    {
      g_hash_table_unref (self->xmd);
      self->xmd = _modulemd_hash_table_deep_str_copy (xmd);
      g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_XMD]);
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

  return g_hash_table_ref (self->xmd);
}

static void
modulemd_module_set_property (GObject *gobject,
                              guint property_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
  ModulemdModule *self = MODULEMD_MODULE (gobject);

  switch (property_id)
    {
    case MD_PROP_BUILDREQUIRES:
      modulemd_module_set_buildrequires (self, g_value_get_boxed (value));
      break;

    case MD_PROP_COMMUNITY:
      modulemd_module_set_community (self, g_value_get_string (value));
      break;

    case MD_PROP_CONTENT_LIC:
      modulemd_module_set_content_licenses (self, g_value_get_object (value));
      break;

    case MD_PROP_DESC:
      modulemd_module_set_description (self, g_value_get_string (value));
      break;

    case MD_PROP_DOCS:
      modulemd_module_set_documentation (self, g_value_get_string (value));
      break;

    case MD_PROP_MDVERSION:
      modulemd_module_set_mdversion (self, g_value_get_uint64 (value));
      break;

    case MD_PROP_MODULE_COMPONENTS:
      modulemd_module_set_module_components (self, g_value_get_boxed (value));
      break;

    case MD_PROP_MODULE_LIC:
      modulemd_module_set_module_licenses (self, g_value_get_object (value));
      break;

    case MD_PROP_NAME:
      modulemd_module_set_name (self, g_value_get_string (value));
      break;

    case MD_PROP_PROFILES:
      modulemd_module_set_profiles (self, g_value_get_boxed (value));
      break;

    case MD_PROP_REQUIRES:
      modulemd_module_set_requires (self, g_value_get_boxed (value));
      break;

    case MD_PROP_RPM_API:
      modulemd_module_set_rpm_api (self, g_value_get_object (value));
      break;

    case MD_PROP_RPM_ARTIFACTS:
      modulemd_module_set_rpm_artifacts (self, g_value_get_object (value));
      break;

    case MD_PROP_RPM_BUILDOPTS:
      modulemd_module_set_rpm_buildopts (self, g_value_get_boxed (value));
      break;

    case MD_PROP_RPM_COMPONENTS:
      modulemd_module_set_rpm_components (self, g_value_get_boxed (value));
      break;

    case MD_PROP_RPM_FILTER:
      modulemd_module_set_rpm_filter (self, g_value_get_object (value));
      break;

    case MD_PROP_STREAM:
      modulemd_module_set_stream (self, g_value_get_string (value));
      break;

    case MD_PROP_SUMMARY:
      modulemd_module_set_summary (self, g_value_get_string (value));
      break;

    case MD_PROP_TRACKER:
      modulemd_module_set_tracker (self, g_value_get_string (value));
      break;

    case MD_PROP_VERSION:
      modulemd_module_set_version (self, g_value_get_uint64 (value));
      break;

    case MD_PROP_XMD:
      modulemd_module_set_xmd (self, g_value_get_boxed (value));
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
  ModulemdModule *self = MODULEMD_MODULE (gobject);

  switch (property_id)
    {
    case MD_PROP_BUILDREQUIRES:
      g_value_set_boxed (value, modulemd_module_get_buildrequires (self));
      break;

    case MD_PROP_COMMUNITY:
      g_value_set_string (value, modulemd_module_get_community (self));
      break;

    case MD_PROP_CONTENT_LIC:
      g_value_set_object (value, modulemd_module_get_content_licenses (self));
      break;

    case MD_PROP_DESC:
      g_value_set_string (value, modulemd_module_get_description (self));
      break;

    case MD_PROP_DOCS:
      g_value_set_string (value, modulemd_module_get_documentation (self));
      break;
    case MD_PROP_MDVERSION:
      g_value_set_uint64 (value, modulemd_module_get_mdversion (self));
      break;

    case MD_PROP_MODULE_COMPONENTS:
      g_value_set_boxed (value, modulemd_module_get_module_components (self));
      break;

    case MD_PROP_MODULE_LIC:
      g_value_set_object (value, modulemd_module_get_module_licenses (self));
      break;

    case MD_PROP_NAME:
      g_value_set_string (value, modulemd_module_get_name (self));
      break;

    case MD_PROP_PROFILES:
      g_value_set_boxed (value, modulemd_module_get_profiles (self));
      break;

    case MD_PROP_REQUIRES:
      g_value_set_boxed (value, modulemd_module_get_requires (self));
      break;

    case MD_PROP_RPM_API:
      g_value_set_object (value, modulemd_module_get_rpm_api (self));
      break;

    case MD_PROP_RPM_ARTIFACTS:
      g_value_set_object (value, modulemd_module_get_rpm_artifacts (self));
      break;

    case MD_PROP_RPM_BUILDOPTS:
      g_value_set_boxed (value, modulemd_module_get_rpm_buildopts (self));
      break;

    case MD_PROP_RPM_COMPONENTS:
      g_value_set_boxed (value, modulemd_module_get_rpm_components (self));
      break;

    case MD_PROP_RPM_FILTER:
      g_value_set_object (value, modulemd_module_get_rpm_filter (self));
      break;

    case MD_PROP_STREAM:
      g_value_set_string (value, modulemd_module_get_stream (self));
      break;

    case MD_PROP_SUMMARY:
      g_value_set_string (value, modulemd_module_get_summary (self));
      break;

    case MD_PROP_TRACKER:
      g_value_set_string (value, modulemd_module_get_tracker (self));
      break;

    case MD_PROP_VERSION:
      g_value_set_uint64 (value, modulemd_module_get_version (self));
      break;

    case MD_PROP_XMD:
      g_value_set_boxed (value, modulemd_module_get_xmd (self));
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

  g_clear_pointer (&self->buildrequires, g_hash_table_unref);
  g_clear_pointer (&self->community, g_free);
  g_clear_pointer (&self->content_licenses, g_object_unref);
  g_clear_pointer (&self->description, g_free);
  g_clear_pointer (&self->documentation, g_free);
  g_clear_pointer (&self->module_components, g_hash_table_unref);
  g_clear_pointer (&self->module_licenses, g_object_unref);
  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->profiles, g_hash_table_unref);
  g_clear_pointer (&self->requires, g_hash_table_unref);
  g_clear_pointer (&self->rpm_api, g_object_unref);
  g_clear_pointer (&self->rpm_artifacts, g_object_unref);
  g_clear_pointer (&self->rpm_buildopts, g_hash_table_unref);
  g_clear_pointer (&self->rpm_components, g_hash_table_unref);
  g_clear_pointer (&self->rpm_filter, g_object_unref);
  g_clear_pointer (&self->stream, g_free);
  g_clear_pointer (&self->summary, g_free);
  g_clear_pointer (&self->tracker, g_free);
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

  md_properties[MD_PROP_CONTENT_LIC] =
    g_param_spec_object ("content-licenses",
                         "Module Content Licenses",
                         "The set of licenses under which the contents "
                         "of this module are released.",
                         MODULEMD_TYPE_SIMPLESET,
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
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
     * ModulemdModule:components-module: (type GLib.HashTable(utf8,ModulemdComponentModule)) (transfer container)
     */
  md_properties[MD_PROP_MODULE_COMPONENTS] =
    g_param_spec_boxed ("components-module",
                        "Module Components",
                        "The module components that define this module.",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  md_properties[MD_PROP_MODULE_LIC] =
    g_param_spec_object ("module-licenses",
                         "Module Licenses",
                         "The set of licenses under which this module is "
                         "released.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  md_properties[MD_PROP_NAME] =
    g_param_spec_string ("name",
                         "Module Name",
                         "A string property representing the name of "
                         "the module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
     * ModulemdModule:profiles: (type GLib.HashTable(utf8,ModulemdProfile)) (transfer container)
     */
  md_properties[MD_PROP_PROFILES] =
    g_param_spec_boxed ("profiles",
                        "Module Profiles",
                        "A dictionary property representing the module "
                        "profiles.",
                        G_TYPE_HASH_TABLE,
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

  md_properties[MD_PROP_RPM_API] =
    g_param_spec_object ("rpm-api",
                         "Module API - RPMs",
                         "The RPMs that make up the public API of this "
                         "module.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  md_properties[MD_PROP_RPM_ARTIFACTS] =
    g_param_spec_object ("rpm-artifacts",
                         "Module artifacts - RPMs",
                         "The RPMs that make up the output artifacts for "
                         "this module.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
     * ModulemdModule:rpm-buildopts: (type GLib.HashTable(utf8,utf8)) (transfer container)
     */
  md_properties[MD_PROP_RPM_BUILDOPTS] =
    g_param_spec_boxed ("rpm-buildopts",
                        "RPM build options",
                        "A dictionary of options to pass to RPM build. "
                        "Currently the only supported key is \"macros\" "
                        "which is used for specifying custom RPM build "
                        "macros.",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
     * ModulemdModule:components-rpm: (type GLib.HashTable(utf8,ModulemdComponentRpm)) (transfer container)
     */
  md_properties[MD_PROP_RPM_COMPONENTS] =
    g_param_spec_boxed ("components-rpm",
                        "RPM Components",
                        "The RPM components that define this module.",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  md_properties[MD_PROP_RPM_FILTER] =
    g_param_spec_object ("rpm-filter",
                         "Module filter - RPMs",
                         "The RPMs that are explicitly filtered out of "
                         "this module.",
                         MODULEMD_TYPE_SIMPLESET,
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
                         0,
                         G_MAXUINT64,
                         0,
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
    object_class, MD_N_PROPERTIES, md_properties);
}

static void
modulemd_module_init (ModulemdModule *self)
{
  /* Allocate the members */
  self->buildrequires =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  self->module_components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  self->rpm_components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->content_licenses = modulemd_simpleset_new ();
  self->module_licenses = modulemd_simpleset_new ();

  self->profiles =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->requires =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  self->rpm_api = modulemd_simpleset_new ();
  self->rpm_artifacts = modulemd_simpleset_new ();
  self->rpm_buildopts =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  self->rpm_filter = modulemd_simpleset_new ();

  self->xmd = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
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


/**
 * modulemd_module_new_from_file:
 * @yaml_file: A YAML file containing the module metadata. If this file
 * contains more than one module, only the first will be loaded.
 *
 * Allocates a new #ModulemdModule from a file.
 *
 * Return value: a new #ModulemdModule.
 */
ModulemdModule *
modulemd_module_new_from_file (const gchar *yaml_file)
{
  GError *error = NULL;
  ModulemdModule **modules = NULL;

  modules = parse_yaml_file (yaml_file, &error);
  if (!modules)
    {
      g_message ("Error parsing YAML: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  for (gsize i = 1; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }

  return modules[0];
}

/**
 * modulemd_module_new_all_from_file:
 * @yaml_file: A YAML file containing the module metadata.
 * @_modules: (out) (array zero-terminated=1) (element-type ModulemdModule) (transfer container):
 * A zero-terminated array of modules contained in this document.
 *
 * Allocates a list of new #ModulemdModule from a file.
 */
void
modulemd_module_new_all_from_file (const gchar *yaml_file,
                                   ModulemdModule ***_modules)
{
  GError *error = NULL;
  ModulemdModule **modules = NULL;

  modules = parse_yaml_file (yaml_file, &error);
  if (!modules)
    {
      g_message ("Error parsing YAML: %s", error->message);
      g_error_free (error);
      return;
    }

  *_modules = modules;
}

/**
 * modulemd_module_new_from_string:
 * @yaml_string: A YAML string containing the module metadata. If this string
 * contains more than one module, only the first will be loaded.
 *
 * Allocates a new #ModulemdModule from a string.
 *
 * Return value: a new #ModulemdModule.
 */
ModulemdModule *
modulemd_module_new_from_string (const gchar *yaml_string)
{
  GError *error = NULL;
  ModulemdModule **modules = NULL;

  modules = parse_yaml_string (yaml_string, &error);
  if (!modules)
    {
      g_message ("Error parsing YAML: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  for (gsize i = 1; modules[i]; i++)
    {
      g_object_unref (modules[i]);
    }

  return modules[0];
}

/**
 * modulemd_module_new_all_from_string:
 * @yaml_string: A YAML string containing the module metadata.
 * @_modules: (out) (array zero-terminated=1) (element-type ModulemdModule) (transfer container):
 * A zero-terminated array of modules contained in this document.
 *
 * Allocates a list of new #ModulemdModule from a string.
 */
void
modulemd_module_new_all_from_string (const gchar *yaml_string,
                                     ModulemdModule ***_modules)
{
  GError *error = NULL;
  ModulemdModule **modules = NULL;

  modules = parse_yaml_string (yaml_string, &error);
  if (!modules)
    {
      g_message ("Error parsing YAML: %s", error->message);
      g_error_free (error);
      return;
    }

  *_modules = modules;
}
