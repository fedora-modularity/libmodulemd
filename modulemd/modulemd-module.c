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

  MD_PROP_ARCH,
  MD_PROP_BUILDREQUIRES,
  MD_PROP_COMMUNITY,
  MD_PROP_CONTENT_LIC,
  MD_PROP_CONTEXT,
  MD_PROP_DEPS,
  MD_PROP_DESC,
  MD_PROP_DOCS,
  MD_PROP_EOL,
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
  MD_PROP_SL,
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
  gchar *arch;
  GHashTable *buildrequires;
  gchar *community;
  ModulemdSimpleSet *content_licenses;
  gchar *context;
  gchar *description;
  GPtrArray *dependencies;
  gchar *documentation;
  GDate *eol;
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
  GHashTable *servicelevels;
  gchar *stream;
  gchar *summary;
  gchar *tracker;
  guint64 version;
  GHashTable *xmd;
};

G_DEFINE_TYPE (ModulemdModule, modulemd_module, G_TYPE_OBJECT)

/**
 * modulemd_module_set_arch:
 * @arch: (nullable): the module artifact architecture.
 *
 * Sets the "arch" property.
 */
void
modulemd_module_set_arch (ModulemdModule *self, const gchar *arch)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (g_strcmp0 (self->arch, arch) != 0)
    {
      g_free (self->arch);
      self->arch = g_strdup (arch);
      g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_ARCH]);
    }
}

/**
 * modulemd_module_get_arch:
 *
 * Retrieves the "arch" for modulemd.
 *
 * Returns: A string containing the "arch" property.
 */
const gchar *
modulemd_module_get_arch (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->arch;
}

/**
 * modulemd_module_set_buildrequires:
 * @buildrequires: (nullable) (element-type utf8 utf8): The requirements to build this
 * module.
 *
 * Sets the 'buildrequires' property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 */
void
modulemd_module_set_buildrequires (ModulemdModule *self,
                                   GHashTable *buildrequires)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (modulemd_module_get_mdversion (self) < 2);

  if (buildrequires != self->buildrequires)
    {
      if (self->buildrequires)
        {
          g_hash_table_unref (self->buildrequires);
        }

      if (buildrequires)
        {
          self->buildrequires =
            _modulemd_hash_table_deep_str_copy (buildrequires);
        }
      else
        {
          self->buildrequires = NULL;
        }
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
  g_return_val_if_fail (modulemd_module_get_mdversion (self) < 2, NULL);

  return g_hash_table_ref (self->buildrequires);
}

/**
 * modulemd_module_set_community:
 * @community: (nullable): the module community.
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
 * @licenses: (nullable): A #ModuleSimpleSet: The licenses under which the components of
 * this module are released.
 *
 * Sets the content_licenses property.
 */
void
modulemd_module_set_content_licenses (ModulemdModule *self,
                                      ModulemdSimpleSet *licenses)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!licenses || MODULEMD_IS_SIMPLESET (licenses));

  /* TODO: Test for differences before replacing */
  if (self->content_licenses)
    {
      g_object_unref (self->content_licenses);
    }

  if (licenses)
    {
      self->content_licenses = g_object_ref (licenses);
    }
  else
    {
      self->content_licenses = NULL;
    }

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
 * modulemd_module_set_context:
 * @context: (nullable): the module artifact architecture.
 *
 * Sets the "context" property.
 */
void
modulemd_module_set_context (ModulemdModule *self, const gchar *context)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (g_strcmp0 (self->context, context) != 0)
    {
      g_free (self->context);
      self->context = g_strdup (context);
      g_object_notify_by_pspec (G_OBJECT (self),
                                md_properties[MD_PROP_CONTEXT]);
    }
}

/**
 * modulemd_module_get_context:
 *
 * Retrieves the "context" for modulemd.
 *
 * Returns: A string containing the "context" property.
 */
const gchar *
modulemd_module_get_context (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->context;
}


/**
 * modulemd_module_set_dependencies:
 * @deps: (array zero-terminated=1) (element-type ModulemdDependencies) (transfer container) (nullable):
 * The NULL-terminated list of dependencies.
 *
 * Sets the list of dependency objects for this module.
 */
void
modulemd_module_set_dependencies (ModulemdModule *self, GPtrArray *deps)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (modulemd_module_get_mdversion (self) >= 2);

  if (self->dependencies != deps)
    {
      if (self->dependencies)
        {
          g_clear_pointer (&self->dependencies, g_ptr_array_unref);
        }

      if (deps)
        {
          self->dependencies = g_ptr_array_ref (deps);
        }
      g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_DEPS]);
    }
}


/**
 * modulemd_module_add_dependencies:
 * @dep: A dependency object to add to this module
 *
 * Helper function to populate the dependencies list
 */
void
modulemd_module_add_dependencies (ModulemdModule *self,
                                  ModulemdDependencies *dep)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (modulemd_module_get_mdversion (self) >= 2);
  g_return_if_fail (MODULEMD_IS_DEPENDENCIES (dep));

  if (!self->dependencies)
    {
      self->dependencies = g_ptr_array_new_with_free_func (g_object_unref);
    }

  g_ptr_array_add (self->dependencies, g_object_ref (dep));
  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_DEPS]);
}


/**
 * modulemd_module_get_dependencies
 *
 * Returns: (element-type ModulemdDependencies) (transfer container): The list
 * of dependency objects for this module.
 */
GPtrArray *
modulemd_module_get_dependencies (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);
  g_return_val_if_fail (modulemd_module_get_mdversion (self) >= 2, NULL);

  if (self->dependencies == NULL)
    {
      return NULL;
    }

  return g_ptr_array_ref (self->dependencies);
}


/**
 * modulemd_module_set_description:
 * @description: (nullable): the module description.
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
 * @documentation: (nullable): the module documentation.
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
 * modulemd_module_set_eol:
 * @date: (nullable): The end-of-life date of the module
 *
 * Sets the "eol" property.
 *
 * Note: This property is obsolete. Use "servicelevels" instead.  This will fail
 * on modulemd files using the version 2 or later formats.
 */
void
modulemd_module_set_eol (ModulemdModule *self, const GDate *date)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (modulemd_module_get_mdversion (self) < 2);

  if (!date)
    {
      gboolean previously_valid = g_date_valid (self->eol);

      g_date_clear (self->eol, 1);

      if (previously_valid)
        {
          g_object_notify_by_pspec (G_OBJECT (self),
                                    md_properties[MD_PROP_EOL]);
        }

      return;
    }

  g_return_if_fail (g_date_valid (date));

  if (!g_date_valid (self->eol) || g_date_compare (date, self->eol) != 0)
    {
      /* Date is changing. Update it */
      g_date_set_year (self->eol, g_date_get_year (date));
      g_date_set_month (self->eol, g_date_get_month (date));
      g_date_set_day (self->eol, g_date_get_day (date));

      g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_EOL]);
    }
}

/**
 * modulemd_module_get_eol:
 *
 * Retrieves the "eol" property.
 *
 * Note: This property is obsolete. Use "servicelevels" instead. This will fail
 * on modulemd files using the version 2 or later formats.
 *
 * Returns: A #GDate containing the "EOL" date
 */
const GDate *
modulemd_module_get_eol (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);
  g_return_val_if_fail (modulemd_module_get_mdversion (self) < 2, NULL);

  if (!g_date_valid (self->eol))
    {
      return NULL;
    }

  return self->eol;
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
 * @components: (nullable) (element-type utf8 ModulemdComponentModule): The hash table of
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

  if (components != self->module_components)
    {
      if (self->module_components)
        {
          g_hash_table_unref (self->module_components);
        }

      if (components)
        {
          self->module_components =
            _modulemd_hash_table_deep_obj_copy (components);
        }
      else
        {
          self->module_components = NULL;
        }
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
 * @licenses: (nullable): A #ModuleSimpleSet: The licenses under which the components of
 * this module are released.
 *
 * Sets the module_licenses property.
 */
void
modulemd_module_set_module_licenses (ModulemdModule *self,
                                     ModulemdSimpleSet *licenses)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!licenses || MODULEMD_IS_SIMPLESET (licenses));

  /* TODO: Test for differences before replacing */
  if (self->module_licenses)
    {
      g_object_unref (self->module_licenses);
    }

  if (licenses)
    {
      self->module_licenses = g_object_ref (licenses);
    }
  else
    {
      self->module_licenses = NULL;
    }

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
 * @name: (nullable): the module name.
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
 * @profiles: (nullable) (element-type utf8 ModulemdProfile): The profiles avaiable for
 * this module.
 *
 * Sets the 'profiles' property.
 */

void
modulemd_module_set_profiles (ModulemdModule *self, GHashTable *profiles)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (profiles != self->profiles)
    {
      if (self->profiles)
        {
          g_hash_table_unref (self->profiles);
        }

      if (profiles)
        {
          self->profiles = _modulemd_hash_table_deep_obj_copy (profiles);
        }
      else
        {
          self->profiles = NULL;
        }
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
 * @requires: (nullable) (element-type utf8 utf8): The requirements to run this module
 *
 * Sets the 'requires' property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 */
void
modulemd_module_set_requires (ModulemdModule *self, GHashTable *requires)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (modulemd_module_get_mdversion (self) < 2);

  if (requires != self->requires)
    {
      if (self->requires)
        {
          g_hash_table_unref (self->requires);
        }

      if (requires)
        {
          self->requires = _modulemd_hash_table_deep_str_copy (requires);
        }
      else
        {
          self->requires = NULL;
        }
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
 * containing the "requires" property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 */
GHashTable *
modulemd_module_get_requires (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);
  g_return_val_if_fail (modulemd_module_get_mdversion (self) < 2, NULL);

  return g_hash_table_ref (self->requires);
}

/**
 * modulemd_module_set_rpm_api:
 * @apis: (nullable): A #ModuleSimpleSet: The set of binary RPM packages that form the
 * public API for this module.
 *
 * Sets the rpm_api property.
 */
void
modulemd_module_set_rpm_api (ModulemdModule *self, ModulemdSimpleSet *apis)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!apis || MODULEMD_IS_SIMPLESET (apis));

  /* TODO: Test for differences before replacing */
  if (self->rpm_api)
    {
      g_object_unref (self->rpm_api);
    }

  if (apis)
    {
      self->rpm_api = g_object_ref (apis);
    }
  else
    {
      self->rpm_api = NULL;
    }

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
 * @artifacts: (nullable): A #ModuleSimpleSet: The set of binary RPM packages that are
 * contained in this module. Generally populated by the module build service.
 *
 * Sets the rpm_artifacts property.
 */
void
modulemd_module_set_rpm_artifacts (ModulemdModule *self,
                                   ModulemdSimpleSet *artifacts)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!artifacts || MODULEMD_IS_SIMPLESET (artifacts));

  /* TODO: Test for differences before replacing */
  if (self->rpm_artifacts)
    {
      g_object_unref (self->rpm_artifacts);
    }

  if (artifacts)
    {
      self->rpm_artifacts = g_object_ref (artifacts);
    }
  else
    {
      self->rpm_artifacts = NULL;
    }

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
 * @buildopts: (nullable) (element-type utf8 utf8) (transfer container): A dictionary of
 * build options to pass to rpmbuild. Currently the only recognized key is
 * "macros".
 *
 * Sets the 'rpm-buildopts' property.
 */
void
modulemd_module_set_rpm_buildopts (ModulemdModule *self, GHashTable *buildopts)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (buildopts != self->rpm_buildopts)
    {
      if (self->rpm_buildopts)
        {
          g_hash_table_unref (self->rpm_buildopts);
        }

      if (buildopts)
        {
          self->rpm_buildopts = _modulemd_hash_table_deep_str_copy (buildopts);
        }
      else
        {
          self->rpm_buildopts = NULL;
        }

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
 * @components: (nullable) (element-type utf8 ModulemdComponentRpm): The hash table of
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

  if (components != self->rpm_components)
    {
      if (self->rpm_components)
        {
          g_hash_table_unref (self->rpm_components);
        }

      if (components)
        {
          self->rpm_components =
            _modulemd_hash_table_deep_obj_copy (components);
        }
      else
        {
          self->rpm_components = NULL;
        }
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
 * @filter: (nullable): A #ModuleSimpleSet: The set of binary RPM packages that are
 * explicitly filtered out of this module.
 *
 * Sets the rpm_artifacts property.
 */
void
modulemd_module_set_rpm_filter (ModulemdModule *self,
                                ModulemdSimpleSet *filter)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!filter || MODULEMD_IS_SIMPLESET (filter));

  /* TODO: Test for differences before replacing */
  if (self->rpm_filter)
    {
      g_object_unref (self->rpm_filter);
    }

  if (filter)
    {
      self->rpm_filter = g_object_ref (filter);
    }
  else
    {
      self->rpm_filter = NULL;
    }

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
 * modulemd_module_set_servicelevels:
 * @servicelevels: (nullable) (element-type utf8 ModulemdServiceLevel): A hash table of #ServiceLevel objects
 *
 * Sets the service levels for the module.
 */
void
modulemd_module_set_servicelevels (ModulemdModule *self,
                                   GHashTable *servicelevels)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (servicelevels != self->servicelevels)
    {
      if (self->servicelevels)
        {
          g_hash_table_unref (self->servicelevels);
        }

      if (servicelevels)
        {
          self->servicelevels =
            _modulemd_hash_table_deep_obj_copy (servicelevels);
        }
      else
        {
          self->servicelevels = NULL;
        }
      g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_SL]);
    }
}


/**
 * modulemd_module_add_servicelevel:
 * @sl_name: The name of the service level to add
 * @servicelevel: A #ServiceLevel object to add to the hash table
 *
 * Adds a service levels to the module. If sl_name already exists, it will be
 * replaced by this entry.
 */
void
modulemd_module_add_servicelevels (ModulemdModule *self,
                                   const gchar *sl_name,
                                   ModulemdServiceLevel *servicelevel)
{
  gchar *name;
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (!(servicelevel || sl_name))
    {
      return;
    }

  name = g_strdup (sl_name);
  g_hash_table_replace (self->servicelevels, name, servicelevel);

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_SL]);
}


/**
 * modulemd_module_get_servicelevels:
 *
 * Retrieves the service levels for the module
 *
 * Returns: (element-type utf8 ModulemdServiceLevel) (transfer container): A
 * hash table containing the service levels.
 */
GHashTable *
modulemd_module_get_servicelevels (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_hash_table_ref (self->servicelevels);
}


/**
 * modulemd_module_set_stream:
 * @stream: (nullable): the module stream.
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
 * @summary: (nullable): the module summary.
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
 * @tracker: (nullable): the module tracker.
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
 * @xmd: (nullable) (element-type utf8 GVariant): Extensible metadata block
 *
 * Sets the 'xmd' property.
 */
void
modulemd_module_set_xmd (ModulemdModule *self, GHashTable *xmd)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (xmd != self->xmd)
    {
      if (self->xmd)
        {
          g_hash_table_unref (self->xmd);
        }

      if (xmd)
        {
          self->xmd = _modulemd_hash_table_deep_variant_copy (xmd);
        }
      else
        {
          self->xmd = NULL;
        }
      g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_XMD]);
    }
}

/**
 * modulemd_module_get_xmd:
 *
 * Retrieves the "xmd" for modulemd.
 *
 * Returns: (element-type utf8 GVariant) (transfer container): A hash table
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
    case MD_PROP_ARCH:
      modulemd_module_set_arch (self, g_value_get_string (value));
      break;

    case MD_PROP_BUILDREQUIRES:
      modulemd_module_set_buildrequires (self, g_value_get_boxed (value));
      break;

    case MD_PROP_COMMUNITY:
      modulemd_module_set_community (self, g_value_get_string (value));
      break;

    case MD_PROP_CONTENT_LIC:
      modulemd_module_set_content_licenses (self, g_value_get_object (value));
      break;

    case MD_PROP_CONTEXT:
      modulemd_module_set_context (self, g_value_get_string (value));
      break;

    case MD_PROP_DEPS:
      modulemd_module_set_dependencies (self, g_value_get_boxed (value));
      break;

    case MD_PROP_DESC:
      modulemd_module_set_description (self, g_value_get_string (value));
      break;

    case MD_PROP_DOCS:
      modulemd_module_set_documentation (self, g_value_get_string (value));
      break;

    case MD_PROP_EOL:
      modulemd_module_set_eol (self, g_value_get_boxed (value));
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

    case MD_PROP_SL:
      modulemd_module_set_servicelevels (self, g_value_get_boxed (value));
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
    case MD_PROP_ARCH:
      g_value_set_string (value, modulemd_module_get_arch (self));
      break;

    case MD_PROP_BUILDREQUIRES:
      g_value_set_boxed (value, modulemd_module_get_buildrequires (self));
      break;

    case MD_PROP_COMMUNITY:
      g_value_set_string (value, modulemd_module_get_community (self));
      break;

    case MD_PROP_CONTENT_LIC:
      g_value_set_object (value, modulemd_module_get_content_licenses (self));
      break;

    case MD_PROP_CONTEXT:
      g_value_set_string (value, modulemd_module_get_context (self));
      break;

    case MD_PROP_DEPS:
      g_value_set_boxed (value, modulemd_module_get_description (self));
      break;

    case MD_PROP_DESC:
      g_value_set_string (value, modulemd_module_get_description (self));
      break;

    case MD_PROP_DOCS:
      g_value_set_string (value, modulemd_module_get_documentation (self));
      break;

    case MD_PROP_EOL:
      g_value_set_boxed (value, modulemd_module_get_eol (self));
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

    case MD_PROP_SL:
      g_value_set_boxed (value, modulemd_module_get_servicelevels (self));
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

  g_clear_pointer (&self->arch, g_free);
  g_clear_pointer (&self->buildrequires, g_hash_table_unref);
  g_clear_pointer (&self->community, g_free);
  g_clear_pointer (&self->content_licenses, g_object_unref);
  g_clear_pointer (&self->context, g_free);
  g_clear_pointer (&self->dependencies, g_ptr_array_unref);
  g_clear_pointer (&self->description, g_free);
  g_clear_pointer (&self->documentation, g_free);
  g_clear_pointer (&self->eol, g_date_free);
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
  g_clear_pointer (&self->servicelevels, g_hash_table_unref);
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

  md_properties[MD_PROP_ARCH] =
    g_param_spec_string ("arch",
                         "Module Artifact Architecture",
                         "Contains a string describing the module's "
                         "artifacts' main hardware architecture "
                         "compatibility, distinguishing the module artifact, "
                         "e.g. a repository, from others with the same name, "
                         "stream, version and context. This is not a generic "
                         "hardware family (i.e. basearch). Examples: i386, "
                         "i486, armv7hl, x86_64. Filled in by the buildsystem "
                         "during the compose stage.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

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

  md_properties[MD_PROP_CONTEXT] =
    g_param_spec_string ("context",
                         "Module Context",
                         "The context flag serves to distinguish module "
                         "builds with the same name, stream and version and "
                         "plays an important role in future automatic module "
                         "stream name expansion. Filled in by the "
                         "buildsystem. A short hash of the module's name, "
                         "stream, version and its expanded runtime "
                         "dependencies.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModule:dependencies: (type GLib.PtrArray(ModulemdDependencies)) (transfer container)
   */
  md_properties[MD_PROP_DEPS] =
    g_param_spec_boxed ("dependencies",
                        "Module Dependencies",
                        "A list of build and runtime requirements needed by "
                        "this module.",
                        G_TYPE_PTR_ARRAY,
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

  /**
     * ModulemdModule:eol: (type GLib.Date)
     */
  md_properties[MD_PROP_EOL] =
    g_param_spec_boxed ("eol",
                        "End of Life",
                        "An ISO-8601 compatible YYYY-MM-DD value representing "
                        "the end-of-life date of this module. This field is "
                        "obsolete; use 'servicelevels' instead.",
                        G_TYPE_DATE,
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

  /**
     * ModulemdModule:servicelevels: (type GLib.HashTable(utf8,ModulemdServiceLevel)) (transfer container)
     */
  md_properties[MD_PROP_SL] =
    g_param_spec_boxed ("servicelevels",
                        "Service Levels",
                        "A dictionary of service levels that apply to this "
                        "module.",
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
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
     * ModulemdModule:xmd: (type GLib.HashTable(utf8,GVariant)) (transfer container)
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

  self->eol = g_date_new ();

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

  self->servicelevels =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->xmd = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_variant_unref);
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

/**
 * modulemd_module_dump:
 * @yaml_file: A string containing the path to the output file
 *
 * Writes this module out to a YAML document on disk.
 */
void
modulemd_module_dump (ModulemdModule *self, const gchar *yaml_file)
{
  GError *error = NULL;
  ModulemdModule **modules = g_malloc0_n (2, sizeof (ModulemdModule *));

  modules[0] = self;
  modules[1] = NULL;

  if (!emit_yaml_file (modules, yaml_file, &error))
    {
      g_message ("Error emitting YAML file: %s", error->message);
      g_error_free (error);
    }

  g_free (modules);
}

/**
 * modulemd_module_dumps:
 *
 * Writes this module out to a YAML document string.
 *
 * Return value: A string containing a YAML representation of this module.
 */
gchar *
modulemd_module_dumps (ModulemdModule *self)
{
  GError *error = NULL;
  gchar *yaml = NULL;
  ModulemdModule **modules = g_malloc0_n (2, sizeof (ModulemdModule *));

  modules[0] = self;
  modules[1] = NULL;

  if (!emit_yaml_string (modules, &yaml, &error))
    {
      g_message ("Error emitting YAML string: %s", error->message);
      g_error_free (error);
      yaml = NULL;
    }

  g_free (modules);
  return yaml;
}

/**
 * modulemd_module_dump_all:
 * @module_array: (array zero-terminated=1) (element-type ModulemdModule) (transfer container):
 * A zero-terminated array of modules to be output
 *
 * This function writes out a file containing one or more YAML documents
 * generated from the supplied modules.
 */
void
modulemd_module_dump_all (GPtrArray *module_array, const gchar *yaml_file)
{
  GError *error = NULL;
  ModulemdModule **modules =
    g_malloc0_n (module_array->len + 1, sizeof (ModulemdModule *));

  for (gsize i = 0; i < module_array->len; i++)
    {
      modules[i] = g_ptr_array_index (module_array, i);
    }
  modules[module_array->len] = NULL;

  if (!emit_yaml_file (modules, yaml_file, &error))
    {
      g_message ("Error emitting YAML file: %s", error->message);
      g_error_free (error);
    }

  g_free (modules);
}

/**
 * modulemd_module_dumps_all:
 * @module_array: (array zero-terminated=1) (element-type ModulemdModule) (transfer container):
 * A zero-terminated array of modules to be output
 *
 * This function returns an allocated string containing one or more YAML
 * documents generated from the supplied modules.
 *
 * Return value: A string containing a YAML representation of all provided modules.
 */
gchar *
modulemd_module_dumps_all (GPtrArray *module_array)
{
  GError *error = NULL;
  gchar *yaml = NULL;
  ModulemdModule **modules =
    g_malloc0_n (module_array->len + 1, sizeof (ModulemdModule *));

  for (gsize i = 0; i < module_array->len; i++)
    {
      modules[i] = g_ptr_array_index (module_array, i);
    }
  modules[module_array->len] = NULL;

  if (!emit_yaml_string (modules, &yaml, &error))
    {
      g_message ("Error emitting YAML string: %s", error->message);
      g_error_free (error);
      yaml = NULL;
    }

  g_free (modules);
  return yaml;
}

static gboolean
_modulemd_upgrade_v1_to_v2 (ModulemdModule *self)
{
  const GDate *eol = NULL;
  ModulemdServiceLevel *sl = NULL;
  GHashTable *buildrequires = NULL;
  GHashTable *requires = NULL;
  ModulemdDependencies *v2_dep = NULL;
  GHashTableIter iter;
  gpointer key, value;
  GPtrArray *deps = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self), FALSE);

  /* Upgrade the EOL field to a "rawhide" servicelevel*/
  eol = modulemd_module_get_eol (self);
  if (g_date_valid (eol))
    {
      sl = modulemd_servicelevel_new ();
      modulemd_servicelevel_set_eol (sl, eol);

      modulemd_module_add_servicelevels (self, "rawhide", sl);
    }

  /* Upgrade the build and runtime requirements */
  v2_dep = modulemd_dependencies_new ();


  /* First do BuildRequires */
  buildrequires = modulemd_module_get_buildrequires (self);
  g_hash_table_iter_init (&iter, buildrequires);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      modulemd_dependencies_add_buildrequires_single (
        v2_dep, g_strdup (key), g_strdup (value));
    }

  /* Now add runtime Requires */
  requires = modulemd_module_get_requires (self);
  g_hash_table_iter_init (&iter, requires);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      modulemd_dependencies_add_requires_single (
        v2_dep, g_strdup (key), g_strdup (value));
    }

  deps = g_ptr_array_new ();
  g_ptr_array_add (deps, v2_dep);

  modulemd_module_set_mdversion (self, 2);
  modulemd_module_set_dependencies (self, deps);

  return TRUE;
}

gboolean
modulemd_module_upgrade (ModulemdModule *self)
{
  gboolean result = FALSE;
  guint64 mdversion;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self), FALSE);

  mdversion = modulemd_module_get_mdversion (self);

  /* Upgrade from v1 to v2 */
  if (mdversion < 2)
    {
      result = _modulemd_upgrade_v1_to_v2 (self);
      if (!result)
        goto done;
    }

  /* Future upgrades go here */

  result = TRUE;
done:
  return result;
}
