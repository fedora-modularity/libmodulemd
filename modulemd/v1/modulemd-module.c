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
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "modulemd.h"
#include "modulemd-private.h"
#include "modulemd-yaml.h"
#include "modulemd-util.h"
#include <glib.h>
#include <glib/gprintf.h>
#include <yaml.h>
#include <inttypes.h>

GQuark
modulemd_module_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-module-error-quark");
}

enum
{
  MD_PROP_0,

  MD_PROP_ARCH,
  MD_PROP_BUILDOPTS,
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
  ModulemdBuildopts *buildopts;
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
  GHashTable *rpm_buildopts;
  ModulemdSimpleSet *rpm_artifacts;
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


static gboolean
modulemd_module_upgrade_full (ModulemdModule *self, guint64 version);


/**
 * modulemd_module_set_arch:
 * @arch: (nullable): the module artifact architecture.
 *
 * Sets the "arch" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_arch() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_arch)
const gchar *
modulemd_module_get_arch (ModulemdModule *self)
{
  return modulemd_module_peek_arch (self);
}

/**
 * modulemd_module_peek_arch:
 *
 * Retrieves the "arch" for modulemd.
 *
 * Returns: A string containing the "arch" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_module_peek_arch (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->arch;
}


/**
 * modulemd_module_dup_arch:
 *
 * Retrieves a copy of the "arch" for modulemd.
 *
 * Returns: A copy of the string containing the "arch" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_module_dup_arch (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_strdup (self->arch);
}


/**
 * modulemd_module_set_buildopts:
 * @buildopts: (nullable) (transfer none): A #ModulemdBuildopts object
 *
 * Copies a #ModulemdBuildopts object into the module. This object contains
 * additional instructions to the build system required to build this module.
 *
 * Since: 1.5
 */
void
modulemd_module_set_buildopts (ModulemdModule *self,
                               ModulemdBuildopts *buildopts)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!buildopts || MODULEMD_IS_BUILDOPTS (buildopts));

  g_clear_pointer (&self->buildopts, g_object_unref);
  if (buildopts)
    {
      self->buildopts = modulemd_buildopts_copy (buildopts);
    }

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_BUILDOPTS]);
}


/**
 * modulemd_module_get_buildopts:
 *
 * Get a copy of the #ModulemdBuildopts object
 *
 * Returns: (transfer full): a copy of the #ModulemdBuildopts object. This
 * object must be freed with g_object_unref() when the caller is finished with
 * it. This function will return NULL if no buildopts have been set.
 *
 * Since: 1.5
 */
ModulemdBuildopts *
modulemd_module_get_buildopts (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return modulemd_buildopts_copy (self->buildopts);
}


/**
 * modulemd_module_peek_buildopts: (skip)
 *
 * Get a copy of the #ModulemdBuildopts object
 *
 * Returns: (transfer none): The #ModulemdBuildopts object. This
 * object must not be modified or freed.
 * This function will return NULL if no buildopts have been set.
 *
 * Since: 1.6
 */
ModulemdBuildopts *
modulemd_module_peek_buildopts (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->buildopts;
}


/**
 * modulemd_module_set_buildrequires:
 * @buildrequires: (nullable) (element-type utf8 utf8): The requirements to build this
 * module.
 *
 * Sets the 'buildrequires' property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 *
 * Since: 1.0
 */
void
modulemd_module_set_buildrequires (ModulemdModule *self,
                                   GHashTable *buildrequires)
{
  GHashTableIter iter;
  gpointer module_name, stream_name;
  guint64 version;

  version = modulemd_module_peek_mdversion (self);

  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (self->buildrequires != buildrequires);

  if (version > MD_VERSION_1)
    {
      g_debug ("Incompatible modulemd version");
      return;
    }

  g_hash_table_remove_all (self->buildrequires);

  if (buildrequires)
    {
      g_hash_table_iter_init (&iter, buildrequires);
      while (g_hash_table_iter_next (&iter, &module_name, &stream_name))
        {
          g_hash_table_replace (self->buildrequires,
                                g_strdup ((const gchar *)module_name),
                                g_strdup ((const gchar *)stream_name));
        }
    }

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_BUILDREQUIRES]);
}

/**
 * modulemd_module_get_buildrequires:
 *
 * Retrieves the "buildrequires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer none): A hash table
 * containing the "buildrequires" property.
 *
 * Deprecated: 1.1
 * Use peek_buildrequires() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_buildrequires)
GHashTable *
modulemd_module_get_buildrequires (ModulemdModule *self)
{
  return modulemd_module_peek_buildrequires (self);
}

/**
 * modulemd_module_peek_buildrequires:
 *
 * Retrieves the "buildrequires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer none): A hash table
 * containing the "buildrequires" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_peek_buildrequires (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->buildrequires;
}


/**
 * modulemd_module_dup_buildrequires:
 *
 * Retrieves the "buildrequires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing the "buildrequires" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_dup_buildrequires (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return _modulemd_hash_table_deep_str_copy (self->buildrequires);
}


/**
 * modulemd_module_set_community:
 * @community: (nullable): the module community.
 *
 * Sets the "community" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_community() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_community)
const gchar *
modulemd_module_get_community (ModulemdModule *self)
{
  return modulemd_module_peek_community (self);
}


/**
 * modulemd_module_peek_community:
 *
 * Retrieves the "community" for modulemd.
 *
 * Returns: A string containing the "community" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_module_peek_community (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->community;
}


/**
 * modulemd_module_dup_community:
 *
 * Retrieves a copy of the "community" for modulemd.
 *
 * Returns: A copy of string containing the "community" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_module_dup_community (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_strdup (self->community);
}


/**
 * modulemd_module_set_content_licenses:
 * @licenses: (nullable): A #ModuleSimpleSet: The licenses under which the components of
 * this module are released.
 *
 * Sets the content_licenses property.
 *
 * Since: 1.0
 */
void
modulemd_module_set_content_licenses (ModulemdModule *self,
                                      ModulemdSimpleSet *licenses)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!licenses || MODULEMD_IS_SIMPLESET (licenses));

  modulemd_simpleset_copy (licenses, &self->content_licenses);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_CONTENT_LIC]);
}

/**
 * modulemd_module_get_content_licenses:
 *
 * Retrieves the "content_licenses" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of licenses in the
 * "content_licenses" property.
 *
 * Deprecated: 1.1
 * Use peek_content_licenses() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_content_licenses)
ModulemdSimpleSet *
modulemd_module_get_content_licenses (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->content_licenses;
}


/**
 * modulemd_module_peek_content_licenses:
 *
 * Retrieves the "content_licenses" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of licenses in the
 * "content_licenses" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_peek_content_licenses (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->content_licenses;
}


/**
 * modulemd_module_dup_content_licenses:
 *
 * Retrieves the "content_licenses" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of licenses in the
 * "content_licenses" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_dup_content_licenses (ModulemdModule *self)
{
  ModulemdSimpleSet *set = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  modulemd_simpleset_copy (self->content_licenses, &set);

  return set;
}


/**
 * modulemd_module_set_context:
 * @context: (nullable): the module artifact architecture.
 *
 * Sets the "context" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_context() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_context)
const gchar *
modulemd_module_get_context (ModulemdModule *self)
{
  return modulemd_module_peek_context (self);
}


/**
 * modulemd_module_peek_context:
 *
 * Retrieves the "context" for modulemd.
 *
 * Returns: A string containing the "context" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_module_peek_context (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->context;
}


/**
 * modulemd_module_dup_context:
 *
 * Retrieves a copy of the "context" for modulemd.
 *
 * Returns: A copy of the string containing the "context" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_module_dup_context (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_strdup (self->context);
}


/**
 * modulemd_module_set_dependencies:
 * @deps: (array zero-terminated=1) (element-type ModulemdDependencies) (transfer none) (nullable):
 * The NULL-terminated list of dependencies.
 *
 * Sets the list of dependency objects for this module.
 *
 * Since: 1.0
 */
void
modulemd_module_set_dependencies (ModulemdModule *self, GPtrArray *deps)
{
  gsize i = 0;
  ModulemdDependencies *copy = NULL;
  guint64 mdversion;

  mdversion = modulemd_module_peek_mdversion (self);

  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (mdversion && mdversion < MD_VERSION_2)
    {
      g_debug ("Incompatible modulemd version");
      return;
    }

  g_ptr_array_set_size (self->dependencies, 0);

  if (deps)
    {
      for (i = 0; i < deps->len; i++)
        {
          modulemd_dependencies_copy (g_ptr_array_index (deps, i), &copy);
          g_ptr_array_add (self->dependencies, g_object_ref (copy));
          g_clear_pointer (&copy, g_object_unref);
        }
    }

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_DEPS]);
}


/**
 * modulemd_module_add_dependencies:
 * @dep: A dependency object to add to this module
 *
 * Helper function to populate the dependencies list
 *
 * Since: 1.0
 */
void
modulemd_module_add_dependencies (ModulemdModule *self,
                                  ModulemdDependencies *dep)
{
  ModulemdDependencies *copy = NULL;

  guint64 mdversion;

  mdversion = modulemd_module_peek_mdversion (self);

  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (mdversion && mdversion < MD_VERSION_2)
    {
      g_debug ("Incompatible modulemd version");
      return;
    }

  modulemd_dependencies_copy (dep, &copy);
  g_ptr_array_add (self->dependencies, g_object_ref (copy));
  g_clear_pointer (&copy, g_object_unref);

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_DEPS]);
}


/**
 * modulemd_module_get_dependencies
 *
 * Returns: (element-type ModulemdDependencies) (transfer none): The list
 * of dependency objects for this module.
 *
 * Deprecated: 1.1
 * Use peek_dependencies() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_dependencies)
GPtrArray *
modulemd_module_get_dependencies (ModulemdModule *self)
{
  return modulemd_module_peek_dependencies (self);
}


/**
 * modulemd_module_peek_dependencies
 *
 * Returns: (element-type ModulemdDependencies) (transfer none): The list
 * of dependency objects for this module.
 *
 * Since: 1.1
 */
GPtrArray *
modulemd_module_peek_dependencies (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->dependencies;
}


/**
 * modulemd_module_dup_dependencies
 *
 * Returns: (element-type ModulemdDependencies) (transfer container): The list
 * of dependency objects for this module.
 *
 * Since: 1.1
 */
GPtrArray *
modulemd_module_dup_dependencies (ModulemdModule *self)
{
  GPtrArray *dependencies = NULL;
  ModulemdDependencies *copy = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  dependencies = g_ptr_array_new_with_free_func (g_object_unref);

  for (gsize i = 0; i < self->dependencies->len; i++)
    {
      copy = NULL;
      modulemd_dependencies_copy (g_ptr_array_index (self->dependencies, i),
                                  &copy);
      g_ptr_array_add (dependencies, copy);
    }

  return dependencies;
}


/**
 * modulemd_module_set_description:
 * @description: (nullable): the module description.
 *
 * Sets the "description" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_description() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_description)
const gchar *
modulemd_module_get_description (ModulemdModule *self)
{
  return modulemd_module_peek_description (self);
}


/**
 * modulemd_module_peek_description:
 *
 * Retrieves the "description" for modulemd.
 *
 * Returns: A string containing the "description" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_module_peek_description (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->description;
}


/**
 * modulemd_module_dup_description:
 *
 * Retrieves a copy of the "description" for modulemd.
 *
 * Returns: A copy of the string containing the "description" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_module_dup_description (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_strdup (self->description);
}


/**
 * modulemd_module_set_documentation:
 * @documentation: (nullable): the module documentation.
 *
 * Sets the "documentation" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_documentation() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_documentation)
const gchar *
modulemd_module_get_documentation (ModulemdModule *self)
{
  return modulemd_module_peek_documentation (self);
}


/**
 * modulemd_module_peek_documentation:
 *
 * Retrieves the "documentation" for modulemd.
 *
 * Returns: A string containing the "documentation" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_module_peek_documentation (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->documentation;
}


/**
 * modulemd_module_dup_documentation:
 *
 * Retrieves a copy of the "documentation" for modulemd.
 *
 * Returns: A copy of the string containing the "documentation" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_module_dup_documentation (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_strdup (self->documentation);
}


/**
 * modulemd_module_set_eol:
 * @date: (nullable): The end-of-life date of the module
 *
 * Sets the "eol" property.
 *
 * Note: This property is obsolete. Use "servicelevels" instead.  This will fail
 * on modulemd files using the version 2 or later formats.
 *
 * Since: 1.0
 */
void
modulemd_module_set_eol (ModulemdModule *self, const GDate *date)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (modulemd_module_peek_mdversion (self) < 2);

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
 *
 * Deprecated: 1.1
 * Use peek_eol() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_eol)
const GDate *
modulemd_module_get_eol (ModulemdModule *self)
{
  return modulemd_module_peek_eol (self);
}


/**
 * modulemd_module_peek_eol:
 *
 * Retrieves the "eol" property.
 *
 * Note: This property is obsolete. Use "servicelevels" instead. This will fail
 * on modulemd files using the version 2 or later formats.
 *
 * Returns: A #GDate containing the "EOL" date
 *
 * Since: 1.1
 */
const GDate *
modulemd_module_peek_eol (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  if (!g_date_valid (self->eol))
    {
      return NULL;
    }

  return self->eol;
}


/**
 * modulemd_module_dup_eol:
 *
 * Retrieves a copy of the "eol" property.
 *
 * Note: This property is obsolete. Use "servicelevels" instead. This will fail
 * on modulemd files using the version 2 or later formats.
 *
 * Returns: A #GDate containing a copy of the "EOL" date
 *
 * Since: 1.1
 */
GDate *
modulemd_module_dup_eol (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  if (!g_date_valid (self->eol))
    {
      return NULL;
    }

  return g_date_new_dmy (g_date_get_day (self->eol),
                         g_date_get_month (self->eol),
                         g_date_get_year (self->eol));
}


/**
 * modulemd_module_set_mdversion
 * @mdversion: the metadata version
 *
 * Sets the "mdversion" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_mdversion() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_mdversion)
const guint64
modulemd_module_get_mdversion (ModulemdModule *self)
{
  return modulemd_module_peek_mdversion (self);
}


/**
 * modulemd_module_peek_mdversion:
 *
 * Retrieves the "mdversion" for modulemd.
 *
 * Returns: A 64-bit unsigned integer containing the "mdversion" property.
 */
guint64
modulemd_module_peek_mdversion (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), 0);

  return self->mdversion;
}


/**
 * modulemd_module_add_module_component:
 * @component: A #ModulemdComponentModule
 *
 * Adds a #ModulemdComponentModule to the "module_components" hash table.
 *
 * Since: 1.0
 */
void
modulemd_module_add_module_component (ModulemdModule *self,
                                      ModulemdComponentModule *component)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (MODULEMD_IS_COMPONENT_MODULE (component));

  g_hash_table_replace (
    self->module_components,
    g_strdup (modulemd_component_get_name ((ModulemdComponent *)component)),
    g_object_ref (component));

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_MODULE_COMPONENTS]);
}


/**
 * modulemd_module_clear_module_components:
 *
 * Remove all entries from the "module_components" hash table.
 *
 * Since: 1.0
 */
void
modulemd_module_clear_module_components (ModulemdModule *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  g_hash_table_remove_all (self->module_components);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_MODULE_COMPONENTS]);
}


/**
 * modulemd_module_set_module_components:
 * @components: (nullable) (element-type utf8 ModulemdComponentModule): The hash table of
 * module components that comprise this module. The keys are the module name,
 * the values are a #ModulemdComponentModule containing information about that
 * module.
 *
 * Sets the module_components property.
 *
 * Since: 1.0
 */
void
modulemd_module_set_module_components (ModulemdModule *self,
                                       GHashTable *components)
{
  GHashTableIter iter;
  gpointer key, value;
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if ((!components || g_hash_table_size (components) == 0) &&
      g_hash_table_size (self->module_components) == 0)
    {
      /* Nothing to do; don't send notification */
      return;
    }

  /* For any other case, we'll assume a full replacement */
  modulemd_module_clear_module_components (self);

  if (components)
    {
      g_hash_table_iter_init (&iter, components);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          /* Throw an error if the name of the key and the name the component
           * has internally are different.
           */
          g_hash_table_replace (
            self->module_components,
            g_strdup (
              modulemd_component_get_name ((ModulemdComponent *)value)),
            g_object_ref ((ModulemdComponentModule *)value));
        }
    }
  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_MODULE_COMPONENTS]);
}

/**
 * modulemd_module_get_module_components:
 *
 * Retrieves the "module-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentModule) (transfer none): A hash table
 * containing the "module-components" property.
 *
 * Deprecated: 1.1
 * Use peek_module_components() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_module_components)
GHashTable *
modulemd_module_get_module_components (ModulemdModule *self)
{
  return modulemd_module_peek_module_components (self);
}


/**
 * modulemd_module_peek_module_components:
 *
 * Retrieves the "module-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentModule) (transfer none): A hash table
 * containing the "module-components" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_peek_module_components (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->module_components;
}


/**
 * modulemd_module_dup_module_components:
 *
 * Retrieves the "module-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentModule) (transfer container):
 * A copy of the hash table containing the "module-components" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_dup_module_components (ModulemdModule *self)
{
  GHashTable *components = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  g_hash_table_iter_init (&iter, self->module_components);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_replace (
        components,
        g_strdup ((gchar *)key),
        modulemd_component_copy (MODULEMD_COMPONENT (value)));
    }

  return components;
}


/**
 * modulemd_module_set_module_licenses:
 * @licenses: (nullable): A #ModuleSimpleSet: The licenses under which the components of
 * this module are released.
 *
 * Sets the module_licenses property.
 *
 * Since: 1.0
 */
void
modulemd_module_set_module_licenses (ModulemdModule *self,
                                     ModulemdSimpleSet *licenses)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!licenses || MODULEMD_IS_SIMPLESET (licenses));

  modulemd_simpleset_copy (licenses, &self->module_licenses);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_MODULE_LIC]);
}

/**
 * modulemd_module_get_module_licenses:
 *
 * Retrieves the "module_licenses" for modulemd
 *
 * Returns: (transfer none): a #ModulemdSimpleSet containing the set of
 * licenses in the "module_licenses" property.
 *
 * Deprecated: 1.1
 * Use peek_module_licenses() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_module_licenses)
ModulemdSimpleSet *
modulemd_module_get_module_licenses (ModulemdModule *self)
{
  return modulemd_module_peek_module_licenses (self);
}


/**
 * modulemd_module_peek_module_licenses:
 *
 * Retrieves the "module_licenses" for modulemd
 *
 * Returns: (transfer none): a #ModulemdSimpleSet containing the set of
 * licenses in the "module_licenses" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_peek_module_licenses (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->module_licenses;
}


/**
 * modulemd_module_dup_module_licenses:
 *
 * Retrieves a copy of the "module_licenses" for modulemd
 *
 * Returns: (transfer full): a #ModulemdSimpleSet containing the set of
 * licenses in the "module_licenses" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_dup_module_licenses (ModulemdModule *self)
{
  ModulemdSimpleSet *licenses = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  modulemd_simpleset_copy (self->module_licenses, &licenses);

  return licenses;
}


/**
 * modulemd_module_set_name:
 * @name: (nullable): the module name.
 *
 * Sets the "name" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_name() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_name)
const gchar *
modulemd_module_get_name (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return modulemd_module_peek_name (self);
}


/**
 * modulemd_module_peek_name:
 *
 * Retrieves the "name" for modulemd.
 *
 * Returns: A string containing the "name" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_module_peek_name (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->name;
}


/**
 * modulemd_module_dup_name:
 *
 * Retrieves a copy of the "name" for modulemd.
 *
 * Returns: A copy of the string containing the "name" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_module_dup_name (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_strdup (self->name);
}


/**
 * modulemd_module_add_profile:
 * @profile: A #ModulemdProfile
 *
 * Adds a #ModulemdProfile definition to this module.
 *
 * Since: 1.0
 */
void
modulemd_module_add_profile (ModulemdModule *self, ModulemdProfile *profile)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (MODULEMD_IS_PROFILE (profile));

  g_hash_table_replace (
    self->profiles,
    g_strdup (modulemd_profile_get_name ((ModulemdProfile *)profile)),
    g_object_ref (profile));

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_PROFILES]);
}


/**
 * modulemd_module_clear_profiles:
 *
 * Remove all entries from the "profiles" hash table.
 *
 * Since: 1.0
 */
void
modulemd_module_clear_profiles (ModulemdModule *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  g_hash_table_remove_all (self->profiles);

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_PROFILES]);
}


/**
 * modulemd_module_set_profiles:
 * @profiles: (nullable) (element-type utf8 ModulemdProfile): The profiles avaiable for
 * this module.
 *
 * Sets the 'profiles' property.
 *
 * Since: 1.0
 */

void
modulemd_module_set_profiles (ModulemdModule *self, GHashTable *profiles)
{
  GHashTableIter iter;
  gpointer key, value;

  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if ((!profiles || g_hash_table_size (profiles) == 0) &&
      g_hash_table_size (self->profiles) == 0)
    {
      /* Nothing to do; don't send notification */
      return;
    }

  /* For any other case, we'll assume a full replacement */
  modulemd_module_clear_profiles (self);

  if (profiles)
    {
      g_hash_table_iter_init (&iter, profiles);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          g_hash_table_replace (
            self->profiles,
            g_strdup (modulemd_profile_get_name ((ModulemdProfile *)value)),
            g_object_ref ((ModulemdProfile *)value));
        }
    }

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_PROFILES]);
}


/**
 * modulemd_module_get_profiles:
 *
 * Retrieves the "profiles" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdProfile) (transfer none): A hash
 * table containing the "profiles" property.
 *
 * Deprecated: 1.1
 * Use peek_profiles() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_profiles)
GHashTable *
modulemd_module_get_profiles (ModulemdModule *self)
{
  return modulemd_module_peek_profiles (self);
}


/**
 * modulemd_module_peek_profiles:
 *
 * Retrieves the "profiles" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdProfile) (transfer none): A hash
 * table containing the "profiles" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_peek_profiles (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->profiles;
}


/**
 * modulemd_module_dup_profiles:
 *
 * Retrieves a copy of the "profiles" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdProfile) (transfer container): A hash
 * table containing a copy of the "profiles" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_dup_profiles (ModulemdModule *self)
{
  GHashTable *profiles = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  profiles =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  g_hash_table_iter_init (&iter, self->profiles);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_replace (profiles,
                            g_strdup ((gchar *)key),
                            modulemd_profile_copy (MODULEMD_PROFILE (value)));
    }

  return profiles;
}


/**
 * modulemd_module_set_requires:
 * @requires: (nullable) (element-type utf8 utf8): The requirements to run this module
 *
 * Sets the 'requires' property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 *
 * Since: 1.0
 */
void
modulemd_module_set_requires (ModulemdModule *self, GHashTable *requires)
{
  GHashTableIter iter;
  gpointer module_name, stream_name;
  guint64 version;

  version = modulemd_module_peek_mdversion (self);

  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (self->requires != requires);

  if (version > MD_VERSION_1)
    {
      g_debug ("Incompatible modulemd version");
      return;
    }

  g_hash_table_remove_all (self->requires);

  if (requires)
    {
      g_hash_table_iter_init (&iter, requires);
      while (g_hash_table_iter_next (&iter, &module_name, &stream_name))
        {
          g_hash_table_replace (self->requires,
                                g_strdup ((const gchar *)module_name),
                                g_strdup ((const gchar *)stream_name));
        }
    }
  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_REQUIRES]);
}


/**
 * modulemd_module_get_requires:
 *
 * Retrieves the "requires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer none): A hash table
 * containing the "requires" property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 *
 * Deprecated: 1.1
 * Use peek_requires() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_requires)
GHashTable *
modulemd_module_get_requires (ModulemdModule *self)
{
  return modulemd_module_peek_requires (self);
}


/**
 * modulemd_module_peek_requires:
 *
 * Retrieves the "requires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer none): A hash table
 * containing the "requires" property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_peek_requires (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->requires;
}


/**
 * modulemd_module_dup_requires:
 *
 * Retrieves a copy of  the "requires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing a copy of the "buildrequires" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_dup_requires (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return _modulemd_hash_table_deep_str_copy (self->requires);
}


/**
 * modulemd_module_set_rpm_api:
 * @apis: (nullable): A #ModuleSimpleSet: The set of binary RPM packages that form the
 * public API for this module.
 *
 * Sets the rpm_api property.
 *
 * Since: 1.0
 */
void
modulemd_module_set_rpm_api (ModulemdModule *self, ModulemdSimpleSet *apis)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!apis || MODULEMD_IS_SIMPLESET (apis));

  modulemd_simpleset_copy (apis, &self->rpm_api);

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_RPM_API]);
}


/**
 * modulemd_module_get_rpm_api:
 *
 * Retrieves the "rpm_api" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of binary RPM
 * packages in the "rpm_api" property.
 *
 * Deprecated: 1.1
 * Use peek_rpm_api() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_rpm_api)
ModulemdSimpleSet *
modulemd_module_get_rpm_api (ModulemdModule *self)
{
  return modulemd_module_peek_rpm_api (self);
}


/**
 * modulemd_module_peek_rpm_api:
 *
 * Retrieves the "rpm_api" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of binary RPM
 * packages in the "rpm_api" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_peek_rpm_api (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->rpm_api;
}


/**
 * modulemd_module_dup_rpm_api:
 *
 * Retrieves a copy of the "rpm_api" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of binary RPM
 * packages in the "rpm_api" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_dup_rpm_api (ModulemdModule *self)
{
  ModulemdSimpleSet *api = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  modulemd_simpleset_copy (self->rpm_api, &api);
  return api;
}


/**
 * modulemd_module_set_rpm_artifacts:
 * @artifacts: (nullable): A #ModuleSimpleSet: The set of binary RPM packages that are
 * contained in this module. Generally populated by the module build service.
 *
 * Sets the rpm_artifacts property.
 *
 * Since: 1.0
 */
void
modulemd_module_set_rpm_artifacts (ModulemdModule *self,
                                   ModulemdSimpleSet *artifacts)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!artifacts || MODULEMD_IS_SIMPLESET (artifacts));

  modulemd_simpleset_copy (artifacts, &self->rpm_artifacts);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_RPM_ARTIFACTS]);
}


/**
 * modulemd_module_get_rpm_artifacts:
 *
 * Retrieves the "rpm_artifacts" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of binary RPMs
 * contained in this module.
 *
 * Deprecated: 1.1
 * Use peek_rpm_artifacts() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_rpm_artifacts)
ModulemdSimpleSet *
modulemd_module_get_rpm_artifacts (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return modulemd_module_peek_rpm_artifacts (self);
}


/**
 * modulemd_module_peek_rpm_artifacts:
 *
 * Retrieves the "rpm_artifacts" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of binary RPMs
 * contained in this module.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_peek_rpm_artifacts (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->rpm_artifacts;
}


/**
 * modulemd_module_dup_rpm_artifacts:
 *
 * Retrieves a copy of the "rpm_artifacts" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of binary RPMs
 * contained in this module.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_dup_rpm_artifacts (ModulemdModule *self)
{
  ModulemdSimpleSet *artifacts = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  modulemd_simpleset_copy (self->rpm_artifacts, &artifacts);

  return artifacts;
}


/**
 * modulemd_module_set_rpm_buildopts:
 * @buildopts: (nullable) (element-type utf8 utf8): A dictionary of
 * build options to pass to rpmbuild. Currently the only recognized key is
 * "macros".
 *
 * Sets the 'rpm-buildopts' property.
 *
 * Since: 1.0
 *
 * Deprecated: 1.5
 * Use #ModulemdBuildopts via Modulemd.get_buildopts() instead.
 */
G_DEPRECATED_FOR (modulemd_module_set_buildopts)
void
modulemd_module_set_rpm_buildopts (ModulemdModule *self, GHashTable *buildopts)
{
  const gchar *rpm_macros = NULL;
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  /* First, make sure the Buildopts object exists */
  if (!self->buildopts)
    {
      self->buildopts = modulemd_buildopts_new ();
    }

  rpm_macros = g_hash_table_lookup (buildopts, "macros");
  modulemd_buildopts_set_rpm_macros (self->buildopts, rpm_macros);
}

/**
 * modulemd_module_get_rpm_buildopts:
 *
 * Retrieves the "rpm-buildopts" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer none): A hash table
 * containing the "rpm-buildopts" property.
 *
 * Deprecated: 1.1
 * Use peek_rpm_buildopts() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_rpm_buildopts)
GHashTable *
modulemd_module_get_rpm_buildopts (ModulemdModule *self)
{
  return modulemd_module_peek_rpm_buildopts (self);
}


/**
 * modulemd_module_peek_rpm_buildopts:
 *
 * Retrieves the "rpm-buildopts" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer none): A hash table
 * containing the "rpm-buildopts" property.
 *
 * Since: 1.1
 *
 * Deprecated: 1.5
 * Use #ModulemdBuildopts via Modulemd.get_buildopts() instead.
 */
G_DEPRECATED_FOR (modulemd_module_get_buildopts)
GHashTable *
modulemd_module_peek_rpm_buildopts (ModulemdModule *self)
{
  g_autofree gchar *rpm_macros = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  if (self->buildopts)
    {
      rpm_macros = modulemd_buildopts_get_rpm_macros (self->buildopts);
    }

  if (rpm_macros)
    {
      /* Update the hash table for backwards compatibility */
      g_hash_table_replace (
        self->rpm_buildopts, g_strdup ("macros"), g_strdup (rpm_macros));
    }
  else
    {
      g_hash_table_remove_all (self->rpm_buildopts);
    }

  return self->rpm_buildopts;
}


/**
 * modulemd_module_dup_rpm_buildopts:
 *
 * Retrieves a copy of the "rpm-buildopts" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing the "rpm-buildopts" property.
 *
 * Since: 1.1
 *
 * Deprecated: 1.5
 * Use #ModulemdBuildopts via Modulemd.get_buildopts() instead.
 */
G_DEPRECATED_FOR (modulemd_module_get_buildopts)
GHashTable *
modulemd_module_dup_rpm_buildopts (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  return _modulemd_hash_table_deep_str_copy (
    modulemd_module_peek_rpm_buildopts (self));
  G_GNUC_END_IGNORE_DEPRECATIONS
}


/**
 * modulemd_module_add_rpm_component:
 * @component: A #ModulemdComponentRpm
 *
 * Adds a #ModulemdComponentRpm to the "rpm_components" hash table.
 *
 * Since: 1.0
 */
void
modulemd_module_add_rpm_component (ModulemdModule *self,
                                   ModulemdComponentRpm *component)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (component));

  g_hash_table_replace (
    self->rpm_components,
    g_strdup (modulemd_component_get_name ((ModulemdComponent *)component)),
    g_object_ref (component));

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_RPM_COMPONENTS]);
}


/**
 * modulemd_module_clear_rpm_components:
 *
 * Remove all entries from the "rpm_components" hash table.
 *
 * Since: 1.0
 */
void
modulemd_module_clear_rpm_components (ModulemdModule *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  g_hash_table_remove_all (self->rpm_components);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_RPM_COMPONENTS]);
}


/**
 * modulemd_module_set_rpm_components:
 * @components: (nullable) (element-type utf8 ModulemdComponentRpm): The hash table of
 * module components that comprise this module. The keys are the module name,
 * the values are a #ModulemdComponentRpm containing information about that
 * module.
 *
 * Sets the rpm_components property.
 *
 * Since: 1.0
 */
void
modulemd_module_set_rpm_components (ModulemdModule *self,
                                    GHashTable *components)
{
  GHashTableIter iter;
  gpointer key, value;
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if ((!components || g_hash_table_size (components) == 0) &&
      g_hash_table_size (self->rpm_components) == 0)
    {
      /* Nothing to do; don't send notification */
      return;
    }

  /* For any other case, we'll assume a full replacement */
  modulemd_module_clear_rpm_components (self);

  if (components)
    {
      g_hash_table_iter_init (&iter, components);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          /* Throw an error if the name of the key and the name the component
           * has internally are different.
           */
          g_hash_table_replace (self->rpm_components,
                                g_strdup (modulemd_component_get_name (
                                  (ModulemdComponent *)value)),
                                g_object_ref ((ModulemdComponentRpm *)value));
        }
    }
  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_RPM_COMPONENTS]);
}


/**
 * modulemd_module_get_rpm_components:
 *
 * Retrieves the "rpm-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentRpm) (transfer none): A hash table
 * containing the "rpm-components" property.
 *
 * Deprecated: 1.1
 * Use peek_rpm_components() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_rpm_components)
GHashTable *
modulemd_module_get_rpm_components (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return modulemd_module_peek_rpm_components (self);
}


/**
 * modulemd_module_peek_rpm_components:
 *
 * Retrieves the "rpm-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentRpm) (transfer none): A hash table
 * containing the "rpm-components" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_peek_rpm_components (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->rpm_components;
}


/**
 * modulemd_module_dup_rpm_components:
 *
 * Retrieves the "rpm-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentRpm) (transfer container):
 * A hash table containing the "rpm-components" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_dup_rpm_components (ModulemdModule *self)
{
  GHashTable *components = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  g_hash_table_iter_init (&iter, self->rpm_components);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_replace (
        components,
        g_strdup ((gchar *)key),
        modulemd_component_copy (MODULEMD_COMPONENT (value)));
    }

  return components;
}


/**
 * modulemd_module_set_rpm_filter:
 * @filter: (nullable): A #ModuleSimpleSet: The set of binary RPM packages that are
 * explicitly filtered out of this module.
 *
 * Sets the rpm_artifacts property.
 *
 * Since: 1.0
 */
void
modulemd_module_set_rpm_filter (ModulemdModule *self,
                                ModulemdSimpleSet *filter)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (!filter || MODULEMD_IS_SIMPLESET (filter));

  modulemd_simpleset_copy (filter, &self->rpm_filter);

  g_object_notify_by_pspec (G_OBJECT (self),
                            md_properties[MD_PROP_RPM_FILTER]);
}


/**
 * modulemd_module_get_rpm_filter:
 *
 * Retrieves the "rpm_filter" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of binary RPMs
 * filtered out of this module.
 *
 * Deprecated: 1.1
 * Use peek_rpm_filter() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_rpm_filter)
ModulemdSimpleSet *
modulemd_module_get_rpm_filter (ModulemdModule *self)
{
  return modulemd_module_peek_rpm_filter (self);
}


/**
 * modulemd_module_peek_rpm_filter:
 *
 * Retrieves the "rpm_filter" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of binary RPMs
 * filtered out of this module.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_peek_rpm_filter (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->rpm_filter;
}


/**
 * modulemd_module_dup_rpm_filter:
 *
 * Retrieves a copy of the "rpm_filter" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of binary RPMs
 * filtered out of this module.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_module_dup_rpm_filter (ModulemdModule *self)
{
  ModulemdSimpleSet *filters = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  modulemd_simpleset_copy (self->rpm_filter, &filters);

  return filters;
}


/**
 * modulemd_module_clear_servicelevels:
 *
 * Remove all entries from the "servicelevels" hash table
 *
 * Since: 1.0
 */
void
modulemd_module_clear_servicelevels (ModulemdModule *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  g_hash_table_remove_all (self->servicelevels);

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_SL]);
}


/**
 * modulemd_module_set_servicelevels:
 * @servicelevels: (nullable) (element-type utf8 ModulemdServiceLevel): A hash table of #ServiceLevel objects
 *
 * Sets the service levels for the module.
 *
 * Since: 1.0
 */
void
modulemd_module_set_servicelevels (ModulemdModule *self,
                                   GHashTable *servicelevels)
{
  GHashTableIter iter;
  gpointer key, value;
  const gchar *name = NULL;
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if ((!servicelevels || g_hash_table_size (servicelevels) == 0) &&
      g_hash_table_size (self->servicelevels))
    {
      /* Nothing to do; don't send notification */
      return;
    }

  /* For any other case, we'll assume a full replacement */
  modulemd_module_clear_servicelevels (self);

  if (servicelevels)
    {
      g_hash_table_iter_init (&iter, servicelevels);

      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          /* Always use the servicelevel object's name property for the key.
           * This will protect against coding mistakes where the hash table and
           * its entries have different views of the name.
           */
          name =
            modulemd_servicelevel_get_name ((ModulemdServiceLevel *)value);
          if (!name)
            {
              /* Uh oh; this servicelevel is missing its name.
               * We will have to skip it
               */
              g_warning (
                "Attempted to add a servicelevel with a NULL name. "
                "The hashtable had key '%s'\n",
                (const gchar *)key);
              continue;
            }

          g_hash_table_replace (self->servicelevels,
                                g_strdup (name),
                                g_object_ref ((ModulemdServiceLevel *)value));
        }
    }
  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_SL]);
}


/**
 * modulemd_module_add_servicelevel:
 * @servicelevel: A #ServiceLevel object to add to the hash table
 *
 * Adds a service levels to the module. If the name already exists, it will be
 * replaced by this entry and will release a reference on the previous entry.
 *
 * Since: 1.0
 */
void
modulemd_module_add_servicelevel (ModulemdModule *self,
                                  ModulemdServiceLevel *servicelevel)
{
  const gchar *name = NULL;
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  if (!servicelevel)
    {
      return;
    }

  name = modulemd_servicelevel_get_name (servicelevel);
  if (!name)
    {
      /* Uh oh; this servicelevel is missing its name.
       * We will log a warning when those are enabled and then skip it.
       */
      g_warning ("Attempted to add a servicelevel with a NULL name");
      return;
    }

  g_hash_table_replace (
    self->servicelevels, g_strdup (name), g_object_ref (servicelevel));

  g_object_notify_by_pspec (G_OBJECT (self), md_properties[MD_PROP_SL]);
}


/**
 * modulemd_module_get_servicelevels:
 *
 * Retrieves the service levels for the module
 *
 * Returns: (element-type utf8 ModulemdServiceLevel) (transfer none): A
 * hash table containing the service levels.
 *
 * Deprecated: 1.1
 * Use peek_servicelevels() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_servicelevels)
GHashTable *
modulemd_module_get_servicelevels (ModulemdModule *self)
{
  return modulemd_module_peek_servicelevels (self);
}


/**
 * modulemd_module_peek_servicelevels:
 *
 * Retrieves the service levels for the module
 *
 * Returns: (element-type utf8 ModulemdServiceLevel) (transfer none): A
 * hash table containing the service levels.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_peek_servicelevels (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->servicelevels;
}


/**
 * modulemd_module_dup_servicelevels:
 *
 * Retrieves the service levels for the module
 *
 * Returns: (element-type utf8 ModulemdServiceLevel) (transfer container): A
 * hash table containing the service levels.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_dup_servicelevels (ModulemdModule *self)
{
  GHashTable *servicelevels = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  servicelevels =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  g_hash_table_iter_init (&iter, self->servicelevels);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_replace (
        servicelevels,
        g_strdup ((gchar *)key),
        modulemd_servicelevel_copy (MODULEMD_SERVICELEVEL (value)));
    }

  return servicelevels;
}


/**
 * modulemd_module_set_stream:
 * @stream: (nullable): the module stream.
 *
 * Sets the "stream" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_stream() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_stream)
const gchar *
modulemd_module_get_stream (ModulemdModule *self)
{
  return modulemd_module_peek_stream (self);
}


/**
 * modulemd_module_peek_stream:
 *
 * Retrieves the "stream" for modulemd.
 *
 * Returns: A string containing the "stream" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_module_peek_stream (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->stream;
}


/**
 * modulemd_module_dup_stream:
 *
 * Retrieves a copy of the "stream" for modulemd.
 *
 * Returns: A copy of the string containing the "stream" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_module_dup_stream (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_strdup (self->stream);
}


/**
 * modulemd_module_set_summary:
 * @summary: (nullable): the module summary.
 *
 * Sets the "summary" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_summary() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_summary)
const gchar *
modulemd_module_get_summary (ModulemdModule *self)
{
  return modulemd_module_peek_summary (self);
}


/**
 * modulemd_module_peek_summary:
 *
 * Retrieves the "summary" for modulemd.
 *
 * Returns: A string containing the "summary" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_module_peek_summary (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->summary;
}


/**
 * modulemd_module_dup_summary:
 *
 * Retrieves a copy of the "summary" for modulemd.
 *
 * Returns: A copy of the string containing the "summary" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_module_dup_summary (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_strdup (self->summary);
}


/**
 * modulemd_module_set_tracker:
 * @tracker: (nullable): the module tracker.
 *
 * Sets the "tracker" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_tracker() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_tracker)
const gchar *
modulemd_module_get_tracker (ModulemdModule *self)
{
  return modulemd_module_peek_tracker (self);
}


/**
 * modulemd_module_peek_tracker:
 *
 * Retrieves the "tracker" for modulemd.
 *
 * Returns: A string containing the "tracker" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_module_peek_tracker (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->tracker;
}


/**
 * modulemd_module_dup_tracker:
 *
 * Retrieves a copy of the "tracker" for modulemd.
 *
 * Returns: A copy of the string containing the "tracker" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_module_dup_tracker (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return g_strdup (self->tracker);
}


/**
 * modulemd_module_set_version
 * @version: the module version
 *
 * Sets the "version" property.
 *
 * Since: 1.0
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
 *
 * Deprecated: 1.1
 * Use peek_version() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_version)
const guint64
modulemd_module_get_version (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), 0);

  return modulemd_module_peek_version (self);
}


/**
 * modulemd_module_peek_version:
 *
 * Retrieves the "version" for modulemd.
 *
 * Returns: A 64-bit unsigned integer containing the "version" property.
 *
 * Since: 1.1
 */
guint64
modulemd_module_peek_version (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), 0);

  return self->version;
}


/**
 * modulemd_module_set_xmd:
 * @xmd: (nullable) (element-type utf8 GVariant): Extensible metadata block
 *
 * Sets the 'xmd' property.
 *
 * Since: 1.0
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
 * Returns: (element-type utf8 GVariant) (transfer none): A hash table
 * containing the "xmd" property.
 *
 * Deprecated: 1.1
 * Use peek_xmd() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_module_peek_xmd)
GHashTable *
modulemd_module_get_xmd (ModulemdModule *self)
{
  return modulemd_module_peek_xmd (self);
}


/**
 * modulemd_module_peek_xmd:
 *
 * Retrieves the "xmd" for modulemd.
 *
 * Returns: (element-type utf8 GVariant) (transfer none): A hash table
 * containing the "xmd" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_peek_xmd (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->xmd;
}


/**
 * modulemd_module_dup_xmd:
 *
 * Retrieves a copy of the "xmd" for modulemd.
 *
 * Returns: (element-type utf8 GVariant) (transfer container): A hash table
 * containing the "xmd" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_module_dup_xmd (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return _modulemd_hash_table_deep_variant_copy (self->xmd);
}


/**
 * modulemd_module_copy:
 *
 * Make a copy of the current module
 *
 * Returns: (transfer full): A deep copy of this #ModulemdModule
 *
 * Since: 1.1
 */
ModulemdModule *
modulemd_module_copy (ModulemdModule *self)
{
  guint64 mdversion = modulemd_module_peek_mdversion (self);
  ModulemdModule *copy = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);
  g_return_val_if_fail (mdversion, NULL);

  copy = modulemd_module_new ();

  /* Set mdversion first */
  modulemd_module_set_mdversion (copy, self->mdversion);

  modulemd_module_set_arch (copy, self->arch);

  modulemd_module_set_buildopts (copy, self->buildopts);

  modulemd_module_set_community (copy, self->community);

  modulemd_module_set_content_licenses (copy, self->content_licenses);

  modulemd_module_set_context (copy, self->context);

  modulemd_module_set_description (copy, self->description);

  modulemd_module_set_documentation (copy, self->documentation);

  modulemd_module_set_module_components (copy, self->module_components);

  modulemd_module_set_module_licenses (copy, self->module_licenses);

  modulemd_module_set_name (copy, self->name);

  modulemd_module_set_profiles (copy, self->profiles);

  modulemd_module_set_rpm_api (copy, self->rpm_api);

  modulemd_module_set_rpm_artifacts (copy, self->rpm_artifacts);

  modulemd_module_set_rpm_components (copy, self->rpm_components);

  modulemd_module_set_rpm_filter (copy, self->rpm_filter);

  modulemd_module_set_servicelevels (copy, self->servicelevels);

  modulemd_module_set_stream (copy, self->stream);

  modulemd_module_set_summary (copy, self->summary);

  modulemd_module_set_tracker (copy, self->tracker);

  modulemd_module_set_version (copy, self->version);

  modulemd_module_set_xmd (copy, self->xmd);


  /* Version-specific content */
  if (mdversion == MD_VERSION_1)
    {
      modulemd_module_set_buildrequires (copy, self->buildrequires);
      modulemd_module_set_requires (copy, self->requires);
      if (modulemd_module_peek_eol (self))
        {
          modulemd_module_set_eol (copy, self->eol);
        }
    }
  else if (mdversion >= MD_VERSION_2)
    {
      modulemd_module_set_dependencies (copy, self->dependencies);
    }


  return copy;
}


/**
 * modulemd_module_dup_nsvc:
 *
 * Return the unique module identifier.
 *
 * Returns: a string describing the unique module identifier in the form:
 * "NAME:STREAM:VERSION[:CONTEXT]". This string is owned by the caller and
 * must be freed with g_free().
 */
gchar *
modulemd_module_dup_nsvc (ModulemdModule *self)
{
  gchar *nsvc = NULL;
  const gchar *name = modulemd_module_peek_name (self);
  const gchar *stream = modulemd_module_peek_stream (self);
  guint64 version = modulemd_module_peek_version (self);
  const gchar *context = modulemd_module_peek_context (self);

  if (!name || !stream || !version)
    {
      /* Mandatory field is missing */
      return NULL;
    }

  if (context)
    {
      nsvc = g_strdup_printf (
        "%s:%s:%" PRIx64 ":%s", name, stream, version, context);
    }
  else
    {
      nsvc = g_strdup_printf ("%s:%s:%" PRIx64, name, stream, version);
    }

  return nsvc;
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

    case MD_PROP_BUILDOPTS:
      modulemd_module_set_buildopts (self, g_value_get_object (value));
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
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      modulemd_module_set_rpm_buildopts (self, g_value_get_boxed (value));
      G_GNUC_END_IGNORE_DEPRECATIONS
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
      g_value_take_string (value, modulemd_module_dup_arch (self));
      break;

    case MD_PROP_BUILDOPTS:
      g_value_take_object (value, modulemd_module_get_buildopts (self));
      break;

    case MD_PROP_BUILDREQUIRES:
      g_value_take_boxed (value, modulemd_module_dup_buildrequires (self));
      break;

    case MD_PROP_COMMUNITY:
      g_value_take_string (value, modulemd_module_dup_community (self));
      break;

    case MD_PROP_CONTENT_LIC:
      g_value_take_object (value, modulemd_module_dup_content_licenses (self));
      break;

    case MD_PROP_CONTEXT:
      g_value_take_string (value, modulemd_module_dup_context (self));
      break;

    case MD_PROP_DEPS:
      g_value_set_boxed (value, modulemd_module_peek_dependencies (self));
      break;

    case MD_PROP_DESC:
      g_value_take_string (value, modulemd_module_dup_description (self));
      break;

    case MD_PROP_DOCS:
      g_value_take_string (value, modulemd_module_dup_documentation (self));
      break;

    case MD_PROP_EOL:
      g_value_take_boxed (value, modulemd_module_dup_eol (self));
      break;

    case MD_PROP_MDVERSION:
      g_value_set_uint64 (value, modulemd_module_peek_mdversion (self));
      break;

    case MD_PROP_MODULE_COMPONENTS:
      g_value_take_boxed (value, modulemd_module_dup_module_components (self));
      break;

    case MD_PROP_MODULE_LIC:
      g_value_take_object (value, modulemd_module_dup_module_licenses (self));
      break;

    case MD_PROP_NAME:
      g_value_take_string (value, modulemd_module_dup_name (self));
      break;

    case MD_PROP_PROFILES:
      g_value_take_boxed (value, modulemd_module_dup_profiles (self));
      break;

    case MD_PROP_REQUIRES:
      g_value_take_boxed (value, modulemd_module_dup_requires (self));
      break;

    case MD_PROP_RPM_API:
      g_value_take_object (value, modulemd_module_dup_rpm_api (self));
      break;

    case MD_PROP_RPM_ARTIFACTS:
      g_value_take_object (value, modulemd_module_dup_rpm_artifacts (self));
      break;

    case MD_PROP_RPM_BUILDOPTS:
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      g_value_take_boxed (value, modulemd_module_dup_rpm_buildopts (self));
      G_GNUC_END_IGNORE_DEPRECATIONS
      break;

    case MD_PROP_RPM_COMPONENTS:
      g_value_take_boxed (value, modulemd_module_dup_rpm_components (self));
      break;

    case MD_PROP_RPM_FILTER:
      g_value_take_object (value, modulemd_module_dup_rpm_filter (self));
      break;

    case MD_PROP_SL:
      g_value_take_boxed (value, modulemd_module_dup_servicelevels (self));
      break;

    case MD_PROP_STREAM:
      g_value_take_string (value, modulemd_module_dup_stream (self));
      break;

    case MD_PROP_SUMMARY:
      g_value_take_string (value, modulemd_module_dup_summary (self));
      break;

    case MD_PROP_TRACKER:
      g_value_take_string (value, modulemd_module_dup_tracker (self));
      break;

    case MD_PROP_VERSION:
      g_value_set_uint64 (value, modulemd_module_peek_version (self));
      break;

    case MD_PROP_XMD:
      g_value_take_boxed (value, modulemd_module_dup_xmd (self));
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
  g_clear_pointer (&self->buildopts, g_object_unref);
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

  md_properties[MD_PROP_BUILDOPTS] =
    g_param_spec_object ("buildopts",
                         "Build options for the module",
                         "Assorted instructions for the build system on how "
                         "to build this module.",
                         MODULEMD_TYPE_BUILDOPTS,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModule:buildrequires: (type GLib.HashTable(utf8,utf8))
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
   * ModulemdModule:dependencies: (type GLib.PtrArray(ModulemdDependencies))
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
   * ModulemdModule:components-module: (type GLib.HashTable(utf8,ModulemdComponentModule))
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
   * ModulemdModule:profiles: (type GLib.HashTable(utf8,ModulemdProfile))
   */
  md_properties[MD_PROP_PROFILES] =
    g_param_spec_boxed ("profiles",
                        "Module Profiles",
                        "A dictionary property representing the module "
                        "profiles.",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModule:requires: (type GLib.HashTable(utf8,utf8))
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
   * ModulemdModule:rpm-buildopts: (type GLib.HashTable(utf8,utf8))
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
   * ModulemdModule:components-rpm: (type GLib.HashTable(utf8,ModulemdComponentRpm))
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
   * ModulemdModule:servicelevels: (type GLib.HashTable(utf8,ModulemdServiceLevel))
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
   * ModulemdModule:xmd: (type GLib.HashTable(utf8,GVariant))
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
  self->buildopts = modulemd_buildopts_new ();

  self->buildrequires =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  self->module_components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  self->rpm_components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->dependencies = g_ptr_array_new_with_free_func (g_object_unref);

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
 *
 * Since: 1.0
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
 * Return value: a new #ModulemdModule. When no longer needed, free it with
 * g_object_unref().
 *
 * Since: 1.0
 */
ModulemdModule *
modulemd_module_new_from_file (const gchar *yaml_file)
{
  return modulemd_module_new_from_file_ext (yaml_file, NULL, NULL);
}


/**
 * modulemd_module_new_from_file_ext:
 * @yaml_file: A YAML file containing the module metadata. If this file
 * contains more than one module, only the first will be loaded.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 *
 * Allocates a new #ModulemdModule from a file.
 *
 * Return value: a new #ModulemdModule. When no longer needed, free it with
 * g_object_unref().
 *
 * Since: 1.4
 */
ModulemdModule *
modulemd_module_new_from_file_ext (const gchar *yaml_file,
                                   GPtrArray **failures,
                                   GError **error)
{
  ModulemdModule *module = NULL;
  ModulemdModule **modules = NULL;
  GPtrArray *data = NULL;

  if (!parse_yaml_file (yaml_file, &data, failures, error))
    {
      return NULL;
    }

  modules = mmd_yaml_dup_modules (data);
  module = modules[0];

  /* This old implementation needs to ignore extra_data, so just free it. */
  g_clear_pointer (&data, g_ptr_array_unref);

  if (module)
    {
      for (gsize i = 1; modules[i]; i++)
        {
          g_object_unref (modules[i]);
        }
    }

  g_clear_pointer (&data, g_object_unref);
  g_clear_pointer (&modules, g_free);
  return module;
}

/**
 * modulemd_module_new_all_from_file:
 * @yaml_file: A YAML file containing the module metadata.
 * @_modules: (out) (array zero-terminated=1) (element-type ModulemdModule) (transfer container):
 * A zero-terminated array of modules contained in this document.
 *
 * Allocates a list of new #ModulemdModule from a file.
 *
 * Since: 1.0
 * Deprecated: 1.2
 * Use Modulemd.objects_from_file() instead
 */
void
modulemd_module_new_all_from_file (const gchar *yaml_file,
                                   ModulemdModule ***_modules)
{
  GError *error = NULL;
  GPtrArray *data = NULL;

  if (!parse_yaml_file (yaml_file, &data, NULL, &error))
    {
      g_debug ("Error parsing YAML: %s", error->message);
      g_error_free (error);
      return;
    }

  *_modules = mmd_yaml_dup_modules (data);

  /* This old implementation needs to ignore extra_data, so just free it. */
  g_clear_pointer (&data, g_ptr_array_unref);
}


/**
 * modulemd_module_new_all_from_file_ext:
 * @yaml_file: A YAML file containing the module metadata.
 * @data: (out) (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of objects read from the YAML stream.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Since: 1.1
 * Deprecated: 1.2
 * Use Modulemd.objects_from_file() instead
 */
void
modulemd_module_new_all_from_file_ext (const gchar *yaml_file,
                                       GPtrArray **data)
{
  GError *error = NULL;

  if (!parse_yaml_file (yaml_file, data, NULL, &error))
    {
      g_debug ("Error parsing YAML: %s", error->message);
      g_error_free (error);
      return;
    }
}


/**
 * modulemd_module_new_from_string:
 * @yaml_string: A YAML string containing the module metadata. If this string
 * contains more than one module, only the first will be loaded.
 *
 * Allocates a new #ModulemdModule from a string.
 *
 * Return value: a new #ModulemdModule.
 *
 * Since: 1.0
 */
ModulemdModule *
modulemd_module_new_from_string (const gchar *yaml_string)
{
  return modulemd_module_new_from_string_ext (yaml_string, NULL, NULL);
}


/**
 * modulemd_module_new_from_string_ext:
 * @yaml_string: A YAML string containing the module metadata. If this string
 * contains more than one module, only the first will be loaded.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 *
 * Allocates a new #ModulemdModule from a string.
 *
 * Return value: a new #ModulemdModule.
 *
 * Since: 1.4
 */
ModulemdModule *
modulemd_module_new_from_string_ext (const gchar *yaml_string,
                                     GPtrArray **failures,
                                     GError **error)
{
  ModulemdModule *module = NULL;
  ModulemdModule **modules = NULL;
  GPtrArray *data = NULL;

  if (!parse_yaml_string (yaml_string, &data, failures, error))
    {
      return NULL;
    }

  modules = mmd_yaml_dup_modules (data);

  module = modules[0];

  /* This old implementation needs to ignore extra_data, so just free it. */
  g_clear_pointer (&data, g_ptr_array_unref);

  if (module)
    {
      for (gsize i = 1; modules[i]; i++)
        {
          g_object_unref (modules[i]);
        }
    }
  g_free (modules);

  return module;
}


/**
 * modulemd_module_new_all_from_string:
 * @yaml_string: A YAML string containing the module metadata.
 * @_modules: (out) (array zero-terminated=1) (element-type ModulemdModule) (transfer container):
 * A zero-terminated array of modules contained in this document.
 *
 * Allocates a list of new #ModulemdModule from a string.
 *
 * Since: 1.0
 * Deprecated: 1.2
 * Use Modulemd.objects_from_string() instead
 */
void
modulemd_module_new_all_from_string (const gchar *yaml_string,
                                     ModulemdModule ***_modules)
{
  GError *error = NULL;
  GPtrArray *data = NULL;

  if (!parse_yaml_string (yaml_string, &data, NULL, &error))
    {
      g_debug ("Error parsing YAML: %s", error->message);
      g_error_free (error);
      return;
    }

  *_modules = mmd_yaml_dup_modules (data);

  /* This old implementation needs to ignore extra_data, so just free it. */
  g_clear_pointer (&data, g_ptr_array_unref);
}


/**
 * modulemd_module_new_all_from_string_ext:
 * @yaml_string: A YAML string containing the module metadata.
 * @data: (out) (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of objects read from the YAML stream.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Since: 1.1
 * Deprecated: 1.2
 * Use Modulemd.objects_from_string() instead
 */
void
modulemd_module_new_all_from_string_ext (const gchar *yaml_string,
                                         GPtrArray **data)
{
  GError *error = NULL;

  if (!parse_yaml_string (yaml_string, data, NULL, &error))
    {
      g_debug ("Error parsing YAML: %s", error->message);
      g_error_free (error);
      return;
    }
}


/**
 * modulemd_module_new_from_stream:
 * @stream: A YAML stream containing the module metadata. If this file
 * contains more than one module, only the first will be loaded.
 *
 * Allocates a new #ModulemdModule from a file.
 *
 * Return value: a new #ModulemdModule. When no longer needed, free it with
 * g_object_unref().
 *
 * Since: 1.4
 */
ModulemdModule *
modulemd_module_new_from_stream (FILE *stream, GError **error)
{
  return modulemd_module_new_from_stream_ext (stream, NULL, error);
}


/**
 * modulemd_module_new_from_stream_ext:
 * @stream: A YAML stream containing the module metadata. If this file
 * contains more than one module, only the first will be loaded.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 *
 * Allocates a new #ModulemdModule from a file.
 *
 * Return value: a new #ModulemdModule. When no longer needed, free it with
 * g_object_unref().
 *
 * Since: 1.4
 */
ModulemdModule *
modulemd_module_new_from_stream_ext (FILE *stream,
                                     GPtrArray **failures,
                                     GError **error)
{
  GObject *object = NULL;
  ModulemdModule *module = NULL;
  g_autoptr (GPtrArray) data = NULL;

  if (!parse_yaml_stream (stream, &data, failures, error))
    {
      return NULL;
    }

  for (gsize i = 0; i < data->len; i++)
    {
      object = g_ptr_array_index (data, i);
      if (MODULEMD_IS_MODULE (object))
        {
          module = MODULEMD_MODULE (g_object_ref (object));
          break;
        }
    }

  if (!module)
    {
      g_set_error (error,
                   MODULEMD_MODULE_ERROR,
                   MODULEMD_MODULE_ERROR_MISSING_CONTENT,
                   "Provided YAML file contained no valid module objects");
    }

  return module;
}


/**
 * modulemd_module_dump:
 * @yaml_file: A string containing the path to the output file
 *
 * Writes this module out to a YAML document on disk.
 *
 * Since: 1.0
 */
void
modulemd_module_dump (ModulemdModule *self, const gchar *yaml_file)
{
  GError *error = NULL;
  GPtrArray *objects = g_ptr_array_new ();

  g_ptr_array_add (objects, self);

  if (!emit_yaml_file (objects, yaml_file, &error))
    {
      g_debug ("Error emitting YAML file: %s", error->message);
      g_error_free (error);
    }

  g_ptr_array_unref (objects);
}

/**
 * modulemd_module_dumps:
 *
 * Writes this module out to a YAML document string.
 *
 * Return value: A string containing a YAML representation of this module.
 *
 * Since: 1.0
 */
gchar *
modulemd_module_dumps (ModulemdModule *self)
{
  GError *error = NULL;
  gchar *yaml = NULL;
  GPtrArray *objects = g_ptr_array_new ();

  g_ptr_array_add (objects, self);

  if (!emit_yaml_string (objects, &yaml, &error))
    {
      g_debug ("Error emitting YAML string: %s", error->message);
      g_error_free (error);
      yaml = NULL;
    }

  g_ptr_array_unref (objects);
  return yaml;
}

/**
 * modulemd_module_dump_all:
 * @module_array: (array zero-terminated=1) (element-type GObject) (transfer none):
 * A zero-terminated array of modules to be output
 *
 * This function writes out a file containing one or more YAML documents
 * generated from the supplied modules.
 *
 * Since: 1.0
 *
 * Deprecated: 1.2
 * Use Modulemd.dump() instead.
 */
void
modulemd_module_dump_all (GPtrArray *module_array, const gchar *yaml_file)
{
  GError *error = NULL;

  if (!emit_yaml_file (module_array, yaml_file, &error))
    {
      g_debug ("Error emitting YAML file: %s", error->message);
      g_error_free (error);
    }
}

/**
 * modulemd_module_dumps_all:
 * @module_array: (array zero-terminated=1) (element-type GObject) (transfer none):
 * A zero-terminated array of modules to be output
 *
 * This function returns an allocated string containing one or more YAML
 * documents generated from the supplied modules.
 *
 * Return value: A string containing a YAML representation of all provided modules.
 *
 * Since: 1.0
 *
 * Deprecated: 1.2
 * Use Modulemd.dump() instead.
 */
gchar *
modulemd_module_dumps_all (GPtrArray *module_array)
{
  GError *error = NULL;
  gchar *yaml = NULL;

  if (!emit_yaml_string (module_array, &yaml, &error))
    {
      g_debug ("Error emitting YAML string: %s", error->message);
      g_error_free (error);
      yaml = NULL;
    }

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
  eol = modulemd_module_peek_eol (self);
  if (eol && g_date_valid (eol))
    {
      sl = modulemd_servicelevel_new ();
      modulemd_servicelevel_set_eol (sl, eol);
      modulemd_servicelevel_set_name (sl, "rawhide");

      modulemd_module_add_servicelevel (self, sl);
      g_clear_pointer (&sl, g_object_unref);
    }

  /* Upgrade the build and runtime requirements */
  v2_dep = modulemd_dependencies_new ();


  /* First do BuildRequires */
  buildrequires = modulemd_module_peek_buildrequires (self);
  g_hash_table_iter_init (&iter, buildrequires);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      modulemd_dependencies_add_buildrequires_single (
        v2_dep, (const gchar *)key, (const gchar *)value);
    }

  /* Now add runtime Requires */
  requires = modulemd_module_peek_requires (self);
  g_hash_table_iter_init (&iter, requires);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      modulemd_dependencies_add_requires_single (
        v2_dep, (const gchar *)key, (const gchar *)value);
    }

  deps = g_ptr_array_new ();
  g_ptr_array_add (deps, v2_dep);

  modulemd_module_set_mdversion (self, MD_VERSION_2);
  modulemd_module_set_dependencies (self, deps);

  g_clear_pointer (&deps, g_ptr_array_unref);
  g_clear_pointer (&v2_dep, g_object_unref);

  return TRUE;
}

/**
 * modulemd_module_upgrade:
 * Upgrade the module to the latest supported version
 *
 * This function takes content imported from earlier versions of the modulemd
 * format and upgrades it into the most recent version.
 *
 * Return value: TRUE if the upgrade was performed successfully. If this
 * function returns FALSE, the internal state of the data is undefined and
 * should not be used further.
 *
 * Since: 1.0
 */

gboolean
modulemd_module_upgrade (ModulemdModule *self)
{
  gboolean result = FALSE;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self), FALSE);

  result = modulemd_module_upgrade_full (self, MD_VERSION_LATEST);

  return result;
}

static gboolean
modulemd_module_upgrade_full (ModulemdModule *self, guint64 version)
{
  gboolean result = FALSE;
  guint64 mdversion;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self), FALSE);

  mdversion = modulemd_module_peek_mdversion (self);

  while (mdversion < version)
    {
      switch (mdversion + 1)
        {
        case MD_VERSION_1:
          /* No upgrade needed for v1 */
          break;

        case MD_VERSION_2:
          result = _modulemd_upgrade_v1_to_v2 (self);
          if (!result)
            goto done;
          break;

          /* Future upgrades go here */

        default:
          g_error ("Programming error: no such version %" PRIx64, version);
          result = FALSE;
          goto done;
        }
      mdversion++;
    }

  result = TRUE;
done:
  return result;
}
