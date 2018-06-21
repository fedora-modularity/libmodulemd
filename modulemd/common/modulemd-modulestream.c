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
#include "modulemd-modulestream.h"
#include "modulemd-yaml.h"
#include "modulemd-util.h"
#include <glib.h>
#include <glib/gprintf.h>
#include <yaml.h>
#include <inttypes.h>

GQuark
modulemd_modulestream_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-module-error-quark");
}

enum
{
  PROP_0,

  PROP_ARCH,
  PROP_BUILDOPTS,
  PROP_BUILDREQUIRES,
  PROP_COMMUNITY,
  PROP_CONTENT_LIC,
  PROP_CONTEXT,
  PROP_DEPS,
  PROP_DESC,
  PROP_DOCS,
  PROP_EOL,
  PROP_MDVERSION,
  PROP_MODULE_LIC,
  PROP_MODULE_COMPONENTS,
  PROP_NAME,
  PROP_PROFILES,
  PROP_RPM_API,
  PROP_RPM_ARTIFACTS,
  PROP_RPM_COMPONENTS,
  PROP_RPM_FILTER,
  PROP_REQUIRES,
  PROP_SL,
  PROP_STREAM,
  PROP_SUMMARY,
  PROP_TRACKER,
  PROP_VERSION,
  PROP_XMD,

  MD_N_PROPERTIES
};

static GParamSpec *properties[MD_N_PROPERTIES] = {
  NULL,
};

struct _ModulemdModuleStream
{
  GObject parent_instance;

  /* == Members == */
  gchar *arch;
  ModulemdBuildopts *buildopts;
  GHashTable *buildrequires;
  gchar *community;
  ModulemdSimpleSet *content_licenses;
  gchar *context;
  GPtrArray *dependencies;
  gchar *description;
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
  GHashTable *rpm_components;
  ModulemdSimpleSet *rpm_filter;
  GHashTable *servicelevels;
  gchar *stream;
  gchar *summary;
  gchar *tracker;
  guint64 version;
  GHashTable *xmd;
};

G_DEFINE_TYPE (ModulemdModuleStream, modulemd_modulestream, G_TYPE_OBJECT)


/**
 * modulemd_modulestream_new:
 *
 * Allocates a new #ModulemdModuleStream.
 *
 * Return value: a new #ModulemdModuleStream.
 *
 * Since: 1.6
 */
ModulemdModuleStream *
modulemd_modulestream_new (void)
{
  return g_object_new (MODULEMD_TYPE_MODULESTREAM, NULL);
}


static void
_modulemd_modulestream_copy_internal (ModulemdModuleStream *dest,
                                      ModulemdModuleStream *src)
{
  /* Set mdversion first */
  modulemd_modulestream_set_mdversion (dest, src->mdversion);

  modulemd_modulestream_set_arch (dest, src->arch);

  modulemd_modulestream_set_buildopts (dest, src->buildopts);

  modulemd_modulestream_set_community (dest, src->community);

  modulemd_modulestream_set_content_licenses (dest, src->content_licenses);

  modulemd_modulestream_set_context (dest, src->context);

  modulemd_modulestream_set_description (dest, src->description);

  modulemd_modulestream_set_documentation (dest, src->documentation);

  modulemd_modulestream_set_module_components (dest, src->module_components);

  modulemd_modulestream_set_module_licenses (dest, src->module_licenses);

  modulemd_modulestream_set_name (dest, src->name);

  modulemd_modulestream_set_profiles (dest, src->profiles);

  modulemd_modulestream_set_rpm_api (dest, src->rpm_api);

  modulemd_modulestream_set_rpm_artifacts (dest, src->rpm_artifacts);

  modulemd_modulestream_set_rpm_components (dest, src->rpm_components);

  modulemd_modulestream_set_rpm_filter (dest, src->rpm_filter);

  modulemd_modulestream_set_servicelevels (dest, src->servicelevels);

  modulemd_modulestream_set_stream (dest, src->stream);

  modulemd_modulestream_set_summary (dest, src->summary);

  modulemd_modulestream_set_tracker (dest, src->tracker);

  modulemd_modulestream_set_version (dest, src->version);

  modulemd_modulestream_set_xmd (dest, src->xmd);


  /* Version-specific content */
  if (src->mdversion == MD_VERSION_1)
    {
      modulemd_modulestream_set_buildrequires (dest, src->buildrequires);
      modulemd_modulestream_set_requires (dest, src->requires);
      if (modulemd_modulestream_peek_eol (src))
        {
          modulemd_modulestream_set_eol (dest, src->eol);
        }
    }
  else if (src->mdversion >= MD_VERSION_2)
    {
      modulemd_modulestream_set_dependencies (dest, src->dependencies);
    }
}


/**
 * modulemd_modulestream_copy:
 *
 * Make a copy of the current module
 *
 * Returns: (transfer full): A deep copy of this #ModulemdModuleStream
 *
 * Since: 1.1
 */
ModulemdModuleStream *
modulemd_modulestream_copy (ModulemdModuleStream *self)
{
  guint64 mdversion = modulemd_modulestream_get_mdversion (self);
  g_autoptr (ModulemdModuleStream) copy = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);
  g_return_val_if_fail (mdversion, NULL);

  copy = modulemd_modulestream_new ();

  _modulemd_modulestream_copy_internal (copy, self);

  return g_object_ref (copy);
}


/**
 * modulemd_modulestream_import_from_file:
 * @yaml_file: A YAML file containing the module metadata. If this file
 * contains more than one subdocument, the entire document will be parsed but
 * only the first document will be returned, if it is a valid module.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 *
 * Populates this #ModulemdModuleStream with data from a YAML file.
 *
 * Return value:
 * False if a parse error occurred and sets @error and @failures.
 * False if the first subdocument was not a module and sets @error
 *
 * Since: 1.6
 */
gboolean
modulemd_modulestream_import_from_file (ModulemdModuleStream *self,
                                        const gchar *yaml_file,
                                        GPtrArray **failures,
                                        GError **error)
{
  g_autoptr (GPtrArray) data = NULL;

  if (!parse_yaml_file (yaml_file, &data, failures, error))
    {
      return FALSE;
    }

  if (data->len < 1)
    {
      g_set_error (error,
                   MODULEMD_MODULESTREAM_ERROR,
                   MODULEMD_MODULESTREAM_ERROR_MISSING_CONTENT,
                   "Provided YAML contained no valid subdocuments");
      return FALSE;
    }
  else if (!MODULEMD_IS_MODULESTREAM (g_ptr_array_index (data, 0)))
    {
      g_set_error (error,
                   MODULEMD_MODULESTREAM_ERROR,
                   MODULEMD_MODULESTREAM_ERROR_MISSING_CONTENT,
                   "Provided YAML did not begin with a module document");
      return FALSE;
    }

  _modulemd_modulestream_copy_internal (self, g_ptr_array_index (data, 0));

  return TRUE;
}


/**
 * modulemd_modulestream_dump:
 * @yaml_file: A string containing the path to the output file
 *
 * Writes this module stream out to a YAML document on disk.
 *
 * Returns: False if the file could not be written and sets @error.
 *
 * Since: 1.6
 */
gboolean
modulemd_modulestream_dump (ModulemdModuleStream *self,
                            const gchar *yaml_file,
                            GError **error)
{
  g_autoptr (GPtrArray) objects = g_ptr_array_new ();

  g_ptr_array_add (objects, self);

  if (!emit_yaml_file (objects, yaml_file, error))
    {
      g_debug ("Error emitting YAML file: %s", (*error)->message);
      return FALSE;
    }

  return TRUE;
}


/**
 * modulemd_modulestream_import_from_string:
 * @yaml_string: A YAML string containing the module metadata. If this string
 * contains more than one subdocument, the entire document will be parsed but
 * only the first document will be returned, if it is a valid module.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 *
 * Populates this #ModulemdModuleStream with data from a YAML string.
 *
 * Return value:
 * False if a parse error occurred and sets @error and @failures.
 * False if the first subdocument was not a module and sets @error
 *
 * Since: 1.6
 */
gboolean
modulemd_modulestream_import_from_string (ModulemdModuleStream *self,
                                          const gchar *yaml_string,
                                          GPtrArray **failures,
                                          GError **error)
{
  g_autoptr (GPtrArray) data = NULL;

  if (!parse_yaml_string (yaml_string, &data, failures, error))
    {
      return FALSE;
    }

  if (data->len < 1 || !MODULEMD_IS_MODULESTREAM (g_ptr_array_index (data, 0)))
    {
      g_set_error (error,
                   MODULEMD_MODULESTREAM_ERROR,
                   MODULEMD_MODULESTREAM_ERROR_MISSING_CONTENT,
                   "Provided YAML did not begin with a module document");
      return FALSE;
    }

  _modulemd_modulestream_copy_internal (self, g_ptr_array_index (data, 0));

  return TRUE;
}


/**
 * modulemd_modulestream_dumps:
 *
 * Writes this module out to a YAML document string.
 *
 * Returns: (transfer full): The serialized YAML representation of this stream
 * document or NULL and sets @error appropriately. The returned string must be
 * freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_dumps (ModulemdModuleStream *self, GError **error)
{
  gchar *yaml = NULL;
  g_autoptr (GPtrArray) objects = g_ptr_array_new ();

  g_ptr_array_add (objects, self);

  if (!emit_yaml_string (objects, &yaml, error))
    {
      g_debug ("Error emitting YAML file: %s", (*error)->message);
      g_clear_pointer (&yaml, g_free);
    }

  return yaml;
}


/**
 * modulemd_modulestream_import_from_stream:
 * @stream: A YAML stream containing the module metadata. If this stream
 * contains more than one subdocument, the entire document will be parsed but
 * only the first document will be returned, if it is a valid module.
 * @failures: (element-type ModulemdSubdocument) (transfer container) (out):
 * An array containing any subdocuments from the YAML file that failed to
 * parse. This must be freed with g_ptr_array_unref().
 *
 * Populates this #ModulemdModuleStream with data from a YAML stream.
 *
 * Return value:
 * False if a parse error occurred and sets @error and @failures.
 * False if the first subdocument was not a module and sets @error
 *
 * Since: 1.6
 */
gboolean
modulemd_modulestream_import_from_stream (ModulemdModuleStream *self,
                                          FILE *stream,
                                          GPtrArray **failures,
                                          GError **error)
{
  g_autoptr (GPtrArray) data = NULL;

  if (!parse_yaml_stream (stream, &data, failures, error))
    {
      return FALSE;
    }

  if (data->len < 1 || !MODULEMD_IS_MODULESTREAM (g_ptr_array_index (data, 0)))
    {
      g_set_error (error,
                   MODULEMD_MODULESTREAM_ERROR,
                   MODULEMD_MODULESTREAM_ERROR_MISSING_CONTENT,
                   "Provided YAML did not begin with a module document");
      return FALSE;
    }

  _modulemd_modulestream_copy_internal (self, g_ptr_array_index (data, 0));

  return TRUE;
}


static gboolean
_modulemd_upgrade_v1_to_v2 (ModulemdModuleStream *self)
{
  const GDate *eol = NULL;
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_autoptr (GHashTable) buildrequires = NULL;
  g_autoptr (GHashTable) requires = NULL;
  g_autoptr (ModulemdDependencies) v2_dep = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_autoptr (GPtrArray) deps = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), FALSE);

  /* Upgrade the EOL field to a "rawhide" servicelevel*/
  eol = modulemd_modulestream_peek_eol (self);
  if (eol && g_date_valid (eol))
    {
      sl = modulemd_servicelevel_new ();
      modulemd_servicelevel_set_eol (sl, eol);
      modulemd_servicelevel_set_name (sl, "rawhide");

      modulemd_modulestream_add_servicelevel (self, sl);
    }

  /* Upgrade the build and runtime requirements */
  v2_dep = modulemd_dependencies_new ();


  /* First do BuildRequires */
  buildrequires = modulemd_modulestream_get_buildrequires (self);
  g_hash_table_iter_init (&iter, buildrequires);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      modulemd_dependencies_add_buildrequires_single (
        v2_dep, (const gchar *)key, (const gchar *)value);
    }

  /* Now add runtime Requires */
  requires = modulemd_modulestream_get_requires (self);
  g_hash_table_iter_init (&iter, requires);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      modulemd_dependencies_add_requires_single (
        v2_dep, (const gchar *)key, (const gchar *)value);
    }

  deps = g_ptr_array_new ();
  g_ptr_array_add (deps, v2_dep);

  modulemd_modulestream_set_mdversion (self, MD_VERSION_2);
  modulemd_modulestream_set_dependencies (self, deps);

  return TRUE;
}


static gboolean
modulemd_modulestream_upgrade_full (ModulemdModuleStream *self,
                                    guint64 version)
{
  gboolean result = FALSE;
  guint64 mdversion;

  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), FALSE);

  mdversion = modulemd_modulestream_get_mdversion (self);

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


/**
 * modulemd_modulestream_upgrade:
 * Upgrade the module to the latest supported version
 *
 * This function takes content imported from earlier versions of the modulemd
 * format and upgrades it into the most recent version.
 *
 * Return value: TRUE if the upgrade was performed successfully. If this
 * function returns FALSE, the internal state of the data is undefined and
 * should not be used further.
 *
 * Since: 1.6
 */
gboolean
modulemd_modulestream_upgrade (ModulemdModuleStream *self)
{
  gboolean result = FALSE;

  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), FALSE);

  result = modulemd_modulestream_upgrade_full (self, MD_VERSION_LATEST);

  return result;
}


/**
 * modulemd_modulestream_set_arch:
 * @arch: (nullable): the module artifact architecture.
 *
 * Sets the "arch" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_arch (ModulemdModuleStream *self, const gchar *arch)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (g_strcmp0 (self->arch, arch) != 0)
    {
      g_free (self->arch);
      self->arch = g_strdup (arch);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ARCH]);
    }
}

/**
 * modulemd_modulestream_get_arch:
 *
 * Retrieves the "arch" for modulemd.
 *
 * Returns: (transfer full): A string containing the "arch" property. This
 * string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_arch (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return g_strdup (self->arch);
}

/**
 * modulemd_modulestream_peek_arch: (skip)
 *
 * Retrieves the "arch" for modulemd.
 *
 * Returns: A string containing the "arch" property. This string must not be
 * modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_arch (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->arch;
}


/**
 * modulemd_modulestream_set_buildopts:
 * @buildopts: (nullable) (transfer none): A #ModulemdBuildopts object
 *
 * Copies a #ModulemdBuildopts object into the module. This object contains
 * additional instructions to the build system required to build this module.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_buildopts (ModulemdModuleStream *self,
                                     ModulemdBuildopts *buildopts)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (!buildopts || MODULEMD_IS_BUILDOPTS (buildopts));

  g_clear_pointer (&self->buildopts, g_object_unref);
  if (buildopts)
    {
      self->buildopts = modulemd_buildopts_copy (buildopts);
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BUILDOPTS]);
}


/**
 * modulemd_modulestream_get_buildopts:
 *
 * Get a copy of the #ModulemdBuildopts object
 *
 * Returns: (transfer full): a copy of the #ModulemdBuildopts object. This
 * object must be freed with g_object_unref() when the caller is finished with
 * it. This function will return NULL if no buildopts have been set.
 *
 * Since: 1.6
 */
ModulemdBuildopts *
modulemd_modulestream_get_buildopts (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return modulemd_buildopts_copy (self->buildopts);
}


/**
 * modulemd_modulestream_peek_buildopts: (skip)
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
modulemd_modulestream_peek_buildopts (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->buildopts;
}


/**
 * modulemd_modulestream_set_buildrequires:
 * @buildrequires: (nullable) (element-type utf8 utf8) (transfer none): The
 * requirements to build this module.
 *
 * Sets the 'buildrequires' property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_buildrequires (ModulemdModuleStream *self,
                                         GHashTable *buildrequires)
{
  GHashTableIter iter;
  gpointer module_name, stream_name;
  guint64 version;

  version = modulemd_modulestream_get_mdversion (self);

  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
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

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BUILDREQUIRES]);
}

/**
 * modulemd_modulestream_get_buildrequires:
 *
 * Retrieves the "buildrequires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing the "buildrequires" property. This table must be freed with
 * g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_buildrequires (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return _modulemd_hash_table_deep_str_copy (self->buildrequires);
}


/**
 * modulemd_modulestream_set_community:
 * @community: (nullable) (transfer none): the module community.
 *
 * Sets the "community" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_community (ModulemdModuleStream *self,
                                     const gchar *community)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (g_strcmp0 (self->community, community) != 0)
    {
      g_free (self->community);
      self->community = g_strdup (community);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COMMUNITY]);
    }
}

/**
 * modulemd_modulestream_get_community:
 *
 * Retrieves the "community" for modulemd.
 *
 * Returns: (transfer full): A string containing the "community" property. This
 * string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_community (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return g_strdup (self->community);
}


/**
 * modulemd_modulestream_peek_community: (skip)
 *
 * Retrieves the "community" for modulemd.
 *
 * Returns: A string containing the "community" property. This string must not
 * be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_community (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->community;
}


/**
 * modulemd_modulestream_set_content_licenses:
 * @licenses: (nullable) (transfer none): A #ModuleSimpleSet: The licenses under
 * which the components of this module are released.
 *
 * Sets the content_licenses property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_content_licenses (ModulemdModuleStream *self,
                                            ModulemdSimpleSet *licenses)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (!licenses || MODULEMD_IS_SIMPLESET (licenses));

  modulemd_simpleset_copy (licenses, &self->content_licenses);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CONTENT_LIC]);
}

/**
 * modulemd_modulestream_get_content_licenses:
 *
 * Retrieves the "content_licenses" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of licenses in the
 * "content_licenses" property. The returned #SimpleSet must be freed with
 * g_object_unref().
 *
 * Since: 1.6
 */
ModulemdSimpleSet *
modulemd_modulestream_get_content_licenses (ModulemdModuleStream *self)
{
  ModulemdSimpleSet *set = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  modulemd_simpleset_copy (self->content_licenses, &set);

  return set;
}


/**
 * modulemd_modulestream_peek_content_licenses: (skip)
 *
 * Retrieves the "content_licenses" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of licenses in the
 * "content_licenses" property. This #SimpleSet must not be modified or freed.
 *
 * Since: 1.6
 */
const ModulemdSimpleSet *
modulemd_modulestream_peek_content_licenses (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->content_licenses;
}


/**
 * modulemd_modulestream_set_context:
 * @context: (nullable) (transfer none): the module artifact architecture.
 *
 * Sets the "context" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_context (ModulemdModuleStream *self,
                                   const gchar *context)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (g_strcmp0 (self->context, context) != 0)
    {
      g_free (self->context);
      self->context = g_strdup (context);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CONTEXT]);
    }
}

/**
 * modulemd_modulestream_get_context:
 *
 * Retrieves the "context" for modulemd.
 *
 * Returns: (transfer full): A string containing the "context" property. This
 * string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_context (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return g_strdup (self->context);
}


/**
 * modulemd_modulestream_peek_context: (skip)
 *
 * Retrieves the "context" for modulemd.
 *
 * Returns: (transfer none): A string containing the "context" property. This
 * string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_context (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->context;
}


/**
 * modulemd_modulestream_set_dependencies:
 * @deps: (array zero-terminated=1) (element-type ModulemdDependencies) (transfer none) (nullable):
 * The list of #ModulemdDependencies for this module stream.
 *
 * Sets the list of #ModulemdDependencies objects for this module stream.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_dependencies (ModulemdModuleStream *self,
                                        GPtrArray *deps)
{
  gsize i = 0;
  ModulemdDependencies *copy = NULL;
  guint64 mdversion;

  mdversion = modulemd_modulestream_get_mdversion (self);

  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

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

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DEPS]);
}


/**
 * modulemd_modulestream_add_dependencies:
 * @dep: (transfer none): A #ModulemdDependencies object to add to this module
 *
 * Helper function to populate the dependencies list
 *
 * Since: 1.6
 */
void
modulemd_modulestream_add_dependencies (ModulemdModuleStream *self,
                                        ModulemdDependencies *dep)
{
  ModulemdDependencies *copy = NULL;

  guint64 mdversion;

  mdversion = modulemd_modulestream_get_mdversion (self);

  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (mdversion && mdversion < MD_VERSION_2)
    {
      g_debug ("Incompatible modulemd version");
      return;
    }

  modulemd_dependencies_copy (dep, &copy);
  g_ptr_array_add (self->dependencies, g_object_ref (copy));
  g_clear_pointer (&copy, g_object_unref);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DEPS]);
}


/**
 * modulemd_modulestream_get_dependencies:
 *
 * Returns: (element-type ModulemdDependencies) (transfer full): The list
 * of dependency objects for this module. This list must be freed with
 * g_ptr_array_unref().
 *
 * Since: 1.6
 */
GPtrArray *
modulemd_modulestream_get_dependencies (ModulemdModuleStream *self)
{
  GPtrArray *dependencies = NULL;
  ModulemdDependencies *copy = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  dependencies =
    g_ptr_array_new_full (self->dependencies->len, g_object_unref);

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
 * modulemd_modulestream_peek_dependencies: (skip)
 *
 * Returns: (element-type ModulemdDependencies) (transfer none): The list
 * of dependency objects for this module. This list and its contents must not
 * be modified or freed.
 *
 * Since: 1.6
 */
const GPtrArray *
modulemd_modulestream_peek_dependencies (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->dependencies;
}


/**
 * modulemd_modulestream_set_description:
 * @description: (transfer none) (nullable): the module description.
 *
 * Sets the "description" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_description (ModulemdModuleStream *self,
                                       const gchar *description)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (g_strcmp0 (self->description, description) != 0)
    {
      g_free (self->description);
      self->description = g_strdup (description);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DESC]);
    }
}

/**
 * modulemd_modulestream_get_description:
 *
 * Retrieves the "description" for modulemd.
 *
 * Returns: (transfer full): A string containing the "description" property.
 * This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_description (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return g_strdup (self->description);
}


/**
 * modulemd_modulestream_peek_description: (skip)
 *
 * Retrieves the "description" for modulemd.
 *
 * Returns: (transfer none): A string containing the "description" property.
 * This string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_description (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->description;
}


/**
 * modulemd_modulestream_set_documentation:
 * @documentation: (transfer none) (nullable): the module documentation.
 *
 * Sets the "documentation" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_documentation (ModulemdModuleStream *self,
                                         const gchar *documentation)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (g_strcmp0 (self->documentation, documentation) != 0)
    {
      g_free (self->documentation);
      self->documentation = g_strdup (documentation);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DOCS]);
    }
}

/**
 * modulemd_modulestream_get_documentation:
 *
 * Retrieves the "documentation" for modulemd.
 *
 * Returns: (transfer full): A string containing the "documentation" property.
 * This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_documentation (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return g_strdup (self->documentation);
}


/**
 * modulemd_modulestream_peek_documentation: (skip)
 *
 * Retrieves the "documentation" for modulemd.
 *
 * Returns: (transfer none): A string containing the "documentation" property.
 * This string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_documentation (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->documentation;
}


/**
 * modulemd_modulestream_set_eol:
 * @date: (transfer none) (nullable): The end-of-life date of the module
 *
 * Sets the "eol" property.
 *
 * Note: This property is obsolete. Use "servicelevels" instead.  This will fail
 * on modulemd files using the version 2 or later formats.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_eol (ModulemdModuleStream *self, const GDate *date)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (modulemd_modulestream_get_mdversion (self) < 2);

  if (!date)
    {
      gboolean previously_valid = g_date_valid (self->eol);

      g_date_clear (self->eol, 1);

      if (previously_valid)
        {
          g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EOL]);
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

      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EOL]);
    }
}


/**
 * modulemd_modulestream_get_eol:
 *
 * Retrieves the "eol" property.
 *
 * Note: This property is obsolete. Use "servicelevels" instead. This will fail
 * on modulemd files using the version 2 or later formats.
 *
 * Returns: (transfer full): A #GDate containing the "EOL" date.
 * This #GDate must be freed with g_date_free().
 *
 * Since: 1.6
 */
GDate *
modulemd_modulestream_get_eol (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  if (!g_date_valid (self->eol))
    {
      return NULL;
    }

  return g_date_new_dmy (g_date_get_day (self->eol),
                         g_date_get_month (self->eol),
                         g_date_get_year (self->eol));
}


/**
 * modulemd_modulestream_peek_eol: (skip)
 *
 * Retrieves the "eol" property.
 *
 * Note: This property is obsolete. Use "servicelevels" instead. This will fail
 * on modulemd files using the version 2 or later formats.
 *
 * Returns: (transfer none): A #GDate containing the "EOL" date. This #GDate
 * must not be modified or freed.
 *
 * Since: 1.6
 */
const GDate *
modulemd_modulestream_peek_eol (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  if (!g_date_valid (self->eol))
    {
      return NULL;
    }

  return self->eol;
}


/**
 * modulemd_modulestream_set_mdversion
 * @mdversion: the metadata version
 *
 * Sets the "mdversion" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_mdversion (ModulemdModuleStream *self,
                                     const guint64 mdversion)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (self->mdversion != mdversion)
    {
      self->mdversion = mdversion;
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MDVERSION]);
    }
}


/**
 * modulemd_modulestream_get_mdversion:
 *
 * Retrieves the "mdversion" for modulemd.
 *
 * Returns: A 64-bit unsigned integer containing the "mdversion" property.
 *
 * Since: 1.6
 */
guint64
modulemd_modulestream_get_mdversion (ModulemdModuleStream *self)
{
  return self->mdversion;
}


/**
 * modulemd_modulestream_add_module_component:
 * @component: (transfer none): A #ModulemdComponentModule
 *
 * Adds a #ModulemdComponentModule to the "module_components" hash table.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_add_module_component (ModulemdModuleStream *self,
                                            ModulemdComponentModule *component)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (MODULEMD_IS_COMPONENT_MODULE (component));

  g_hash_table_replace (
    self->module_components,
    g_strdup (modulemd_component_get_name ((ModulemdComponent *)component)),
    MODULEMD_COMPONENT_MODULE (
      modulemd_component_copy (MODULEMD_COMPONENT (component))));

  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_MODULE_COMPONENTS]);
}


/**
 * modulemd_modulestream_clear_module_components:
 *
 * Remove all entries from the "module_components" hash table.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_clear_module_components (ModulemdModuleStream *self)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  g_hash_table_remove_all (self->module_components);

  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_MODULE_COMPONENTS]);
}


/**
 * modulemd_modulestream_set_module_components:
 * @components: (transfer none) (nullable) (element-type utf8 ModulemdComponentModule):
 * The hash table of module components that comprise this module. The keys are
 * the module name, the values are a #ModulemdComponentModule containing
 * information about that module.
 *
 * Sets the module_components property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_module_components (ModulemdModuleStream *self,
                                             GHashTable *components)
{
  GHashTableIter iter;
  gpointer key, value;
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if ((!components || g_hash_table_size (components) == 0) &&
      g_hash_table_size (self->module_components) == 0)
    {
      /* Nothing to do; don't send notification */
      return;
    }

  /* For any other case, we'll assume a full replacement */
  modulemd_modulestream_clear_module_components (self);

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
            MODULEMD_COMPONENT_MODULE (
              modulemd_component_copy (MODULEMD_COMPONENT (value))));
        }
    }
  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_MODULE_COMPONENTS]);
}

/**
 * modulemd_modulestream_get_module_components:
 *
 * Retrieves the "module-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentModule) (transfer full): A hash
 * table containing the "module-components" property. This table must be freed
 * with g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_module_components (ModulemdModuleStream *self)
{
  GHashTable *components = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

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
 * modulemd_modulestream_set_module_licenses:
 * @licenses: (transfer none) (nullable): A #ModuleSimpleSet: The licenses under
 * which the components of this module are released.
 *
 * Sets the module_licenses property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_module_licenses (ModulemdModuleStream *self,
                                           ModulemdSimpleSet *licenses)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (!licenses || MODULEMD_IS_SIMPLESET (licenses));

  modulemd_simpleset_copy (licenses, &self->module_licenses);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_LIC]);
}

/**
 * modulemd_modulestream_get_module_licenses:
 *
 * Retrieves the "module_licenses" for modulemd
 *
 * Returns: (transfer full): a #ModulemdSimpleSet containing the set of
 * licenses in the "module_licenses" property. This #ModulemdSimpleSet must be
 * freed with g_object_unref().
 *
 * Since: 1.6
 */
ModulemdSimpleSet *
modulemd_modulestream_get_module_licenses (ModulemdModuleStream *self)
{
  ModulemdSimpleSet *licenses = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  modulemd_simpleset_copy (self->module_licenses, &licenses);

  return licenses;
}


/**
 * modulemd_modulestream_peek_module_licenses: (skip)
 *
 * Retrieves the "module_licenses" for modulemd
 *
 * Returns: (transfer none): a #ModulemdSimpleSet containing the set of
 * licenses in the "module_licenses" property.
 *
 * Since: 1.6
 */
const ModulemdSimpleSet *
modulemd_modulestream_peek_module_licenses (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->module_licenses;
}


/**
 * modulemd_modulestream_set_name:
 * @name: (transfer none) (nullable): the module name.
 *
 * Sets the "name" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_name (ModulemdModuleStream *self, const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (g_strcmp0 (self->name, name) != 0)
    {
      g_free (self->name);
      self->name = g_strdup (name);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
    }
}

/**
 * modulemd_modulestream_get_name:
 *
 * Retrieves the "name" for modulemd.
 *
 * Returns: (transfer full): A string containing the "name" property. This string
 * must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_name (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return g_strdup (self->name);
}


/**
 * modulemd_modulestream_peek_name: (skip)
 *
 * Retrieves the "name" for modulemd.
 *
 * Returns: (transfer none): A string containing the "name" property. This
 * string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_name (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->name;
}


/**
 * modulemd_modulestream_add_profile:
 * @profile: (transfer none): A #ModulemdProfile
 *
 * Adds a #ModulemdProfile definition to this module.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_add_profile (ModulemdModuleStream *self,
                                   ModulemdProfile *profile)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (MODULEMD_IS_PROFILE (profile));

  g_hash_table_replace (
    self->profiles,
    g_strdup (modulemd_profile_get_name ((ModulemdProfile *)profile)),
    modulemd_profile_copy (profile));

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PROFILES]);
}


/**
 * modulemd_modulestream_clear_profiles:
 *
 * Remove all entries from the "profiles" hash table.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_clear_profiles (ModulemdModuleStream *self)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  g_hash_table_remove_all (self->profiles);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PROFILES]);
}


/**
 * modulemd_modulestream_set_profiles:
 * @profiles: (transfer none) (nullable) (element-type utf8 ModulemdProfile):
 * The profiles available for this module.
 *
 * Sets the 'profiles' property.
 *
 * Since: 1.6
 */

void
modulemd_modulestream_set_profiles (ModulemdModuleStream *self,
                                    GHashTable *profiles)
{
  GHashTableIter iter;
  gpointer key, value;

  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if ((!profiles || g_hash_table_size (profiles) == 0) &&
      g_hash_table_size (self->profiles) == 0)
    {
      /* Nothing to do; don't send notification */
      return;
    }

  /* For any other case, we'll assume a full replacement */
  modulemd_modulestream_clear_profiles (self);

  if (profiles)
    {
      g_hash_table_iter_init (&iter, profiles);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          g_hash_table_replace (
            self->profiles,
            g_strdup (modulemd_profile_get_name ((ModulemdProfile *)value)),
            modulemd_profile_copy ((ModulemdProfile *)value));
        }
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PROFILES]);
}


/**
 * modulemd_modulestream_get_profiles:
 *
 * Retrieves the "profiles" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdProfile) (transfer full): A hash
 * table containing the "profiles" property. This table must be freed with
 * g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_profiles (ModulemdModuleStream *self)
{
  GHashTable *profiles = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

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
 * modulemd_modulestream_set_requires:
 * @requires: (transfer none) (nullable) (element-type utf8 utf8): The
 * requirements to run this module
 *
 * Sets the 'requires' property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_requires (ModulemdModuleStream *self,
                                    GHashTable *requires)
{
  GHashTableIter iter;
  gpointer module_name, stream_name;
  guint64 version;

  version = modulemd_modulestream_get_mdversion (self);

  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
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
  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REQUIRES]);
}


/**
 * modulemd_modulestream_get_requires:
 *
 * Retrieves the "requires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer full): A hash table
 * containing the "requires" property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later. This table must be freed with
 * g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_requires (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return _modulemd_hash_table_deep_str_copy (self->requires);
}


/**
 * modulemd_modulestream_set_rpm_api:
 * @apis: (transfer none) (nullable): A #ModuleSimpleSet: The set of binary RPM
 * packages that form the public API for this module.
 *
 * Sets the rpm_api property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_rpm_api (ModulemdModuleStream *self,
                                   ModulemdSimpleSet *apis)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (!apis || MODULEMD_IS_SIMPLESET (apis));

  modulemd_simpleset_copy (apis, &self->rpm_api);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_API]);
}


/**
 * modulemd_modulestream_get_rpm_api:
 *
 * Retrieves the "rpm_api" for modulemd
 *
 * Returns: (transfer full): a #ModulemdSimpleSet containing the set of binary
 * RPM packages in the "rpm_api" property. This #ModulemdSimpleSet must be freed
 * with g_object_unref().
 *
 * Since: 1.6
 */
ModulemdSimpleSet *
modulemd_modulestream_get_rpm_api (ModulemdModuleStream *self)
{
  ModulemdSimpleSet *api = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  modulemd_simpleset_copy (self->rpm_api, &api);
  return api;
}


/**
 * modulemd_modulestream_peek_rpm_api: (skip)
 *
 * Retrieves the "rpm_api" for modulemd
 *
 * Returns: (transfer none): a #ModulemdSimpleSet containing the set of binary
 * RPM packages in the "rpm_api" property. This #ModulemdSimpleSet must not be
 * modified or freed.
 *
 * Since: 1.6
 */
const ModulemdSimpleSet *
modulemd_modulestream_peek_rpm_api (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->rpm_api;
}


/**
 * modulemd_modulestream_set_rpm_artifacts:
 * @artifacts: (transfer none) (nullable): A #ModuleSimpleSet: The set of binary
 * RPM packages that are contained in this module. Generally populated by the
 * module build service.
 *
 * Sets the rpm_artifacts property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_rpm_artifacts (ModulemdModuleStream *self,
                                         ModulemdSimpleSet *artifacts)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (!artifacts || MODULEMD_IS_SIMPLESET (artifacts));

  modulemd_simpleset_copy (artifacts, &self->rpm_artifacts);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_ARTIFACTS]);
}


/**
 * modulemd_modulestream_get_rpm_artifacts:
 *
 * Retrieves the "rpm_artifacts" for modulemd
 *
 * Returns: (transfer full): a #SimpleSet containing the set of binary RPMs
 * contained in this module.
 *
 * Since: 1.6
 */
ModulemdSimpleSet *
modulemd_modulestream_get_rpm_artifacts (ModulemdModuleStream *self)
{
  ModulemdSimpleSet *artifacts = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  modulemd_simpleset_copy (self->rpm_artifacts, &artifacts);

  return artifacts;
}


/**
 * modulemd_modulestream_peek_rpm_artifacts: (skip)
 *
 * Retrieves the "rpm_artifacts" for modulemd
 *
 * Returns: (transfer none): a #SimpleSet containing the set of binary RPMs
 * contained in this module.
 *
 * Since: 1.6
 */
const ModulemdSimpleSet *
modulemd_modulestream_peek_rpm_artifacts (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->rpm_artifacts;
}


/**
 * modulemd_modulestream_add_rpm_component:
 * @component: (transfer none): A #ModulemdComponentRpm
 *
 * Adds a #ModulemdComponentRpm to the "rpm_components" hash table.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_add_rpm_component (ModulemdModuleStream *self,
                                         ModulemdComponentRpm *component)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (component));

  g_hash_table_replace (
    self->rpm_components,
    g_strdup (modulemd_component_get_name (MODULEMD_COMPONENT (component))),
    MODULEMD_COMPONENT_RPM (
      modulemd_component_copy (MODULEMD_COMPONENT (component))));

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_COMPONENTS]);
}


/**
 * modulemd_modulestream_clear_rpm_components:
 *
 * Remove all entries from the "rpm_components" hash table.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_clear_rpm_components (ModulemdModuleStream *self)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  g_hash_table_remove_all (self->rpm_components);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_COMPONENTS]);
}


/**
 * modulemd_modulestream_set_rpm_components:
 * @components: (transfer none) (nullable) (element-type utf8 ModulemdComponentRpm):
 * The hash table of module components that comprise this module. The keys are
 * the module name, the values are a #ModulemdComponentRpm containing
 * information about that module.
 *
 * Sets the rpm_components property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_rpm_components (ModulemdModuleStream *self,
                                          GHashTable *components)
{
  GHashTableIter iter;
  gpointer key, value;
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if ((!components || g_hash_table_size (components) == 0) &&
      g_hash_table_size (self->rpm_components) == 0)
    {
      /* Nothing to do; don't send notification */
      return;
    }

  /* For any other case, we'll assume a full replacement */
  modulemd_modulestream_clear_rpm_components (self);

  if (components)
    {
      g_hash_table_iter_init (&iter, components);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          /* Throw an error if the name of the key and the name the component
           * has internally are different.
           */
          g_hash_table_replace (
            self->rpm_components,
            g_strdup (
              modulemd_component_get_name (MODULEMD_COMPONENT (value))),

            MODULEMD_COMPONENT_RPM (
              modulemd_component_copy (MODULEMD_COMPONENT (value))));
        }
    }
  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_COMPONENTS]);
}


/**
 * modulemd_modulestream_get_rpm_components:
 *
 * Retrieves the "rpm-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentRpm) (transfer full): A hash
 * table containing the "rpm-components" property. This table must be freed with
 * g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_rpm_components (ModulemdModuleStream *self)
{
  GHashTable *components = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

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
 * modulemd_modulestream_set_rpm_filter:
 * @filter: (transfer none) (nullable): A #ModuleSimpleSet: The set of binary
 * RPM packages that are explicitly filtered out of this module.
 *
 * Sets the rpm_artifacts property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_rpm_filter (ModulemdModuleStream *self,
                                      ModulemdSimpleSet *filter)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  g_return_if_fail (!filter || MODULEMD_IS_SIMPLESET (filter));

  modulemd_simpleset_copy (filter, &self->rpm_filter);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_FILTER]);
}


/**
 * modulemd_modulestream_get_rpm_filter:
 *
 * Retrieves the "rpm_filter" for modulemd
 *
 * Returns: (transfer full): a #ModulemdSimpleSet containing the set of binary
 * RPMs filtered out of this module. This #ModulemdSimpleSet must be freed with
 * g_object_unref().
 *
 * Since: 1.6
 */
ModulemdSimpleSet *
modulemd_modulestream_get_rpm_filter (ModulemdModuleStream *self)
{
  ModulemdSimpleSet *filters = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  modulemd_simpleset_copy (self->rpm_filter, &filters);

  return filters;
}


/**
 * modulemd_modulestream_peek_rpm_filter: (skip)
 *
 * Retrieves the "rpm_filter" for modulemd
 *
 * Returns: (transfer none): a #ModulemdSimpleSet containing the set of binary
 * RPMs filtered out of this module. This #ModulemdSimpleSet must not be
 * modified or freed.
 *
 * Since: 1.6
 */
const ModulemdSimpleSet *
modulemd_modulestream_peek_rpm_filter (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->rpm_filter;
}

/**
 * modulemd_modulestream_clear_servicelevels:
 *
 * Remove all entries from the "servicelevels" hash table
 *
 * Since: 1.6
 */
void
modulemd_modulestream_clear_servicelevels (ModulemdModuleStream *self)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  g_hash_table_remove_all (self->servicelevels);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SL]);
}


/**
 * modulemd_modulestream_set_servicelevels:
 * @servicelevels: (transfer none) (nullable) (element-type utf8 ModulemdServiceLevel):
 * A hash table of #ModulemdServiceLevel objects
 *
 * Sets the service levels for the module.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_servicelevels (ModulemdModuleStream *self,
                                         GHashTable *servicelevels)
{
  GHashTableIter iter;
  gpointer key, value;
  const gchar *name = NULL;
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if ((!servicelevels || g_hash_table_size (servicelevels) == 0) &&
      g_hash_table_size (self->servicelevels))
    {
      /* Nothing to do; don't send notification */
      return;
    }

  /* For any other case, we'll assume a full replacement */
  modulemd_modulestream_clear_servicelevels (self);

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

          g_hash_table_replace (
            self->servicelevels,
            g_strdup (name),
            modulemd_servicelevel_copy (MODULEMD_SERVICELEVEL (value)));
        }
    }
  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SL]);
}


/**
 * modulemd_modulestream_add_servicelevel:
 * @servicelevel: (transfer none): A #ModulemdServiceLevel object to add to the hash table
 *
 * Adds a service levels to the module. If the name already exists, it will be
 * replaced by this entry and will release a reference on the previous entry.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_add_servicelevel (ModulemdModuleStream *self,
                                        ModulemdServiceLevel *servicelevel)
{
  const gchar *name = NULL;
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

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

  g_hash_table_replace (self->servicelevels,
                        g_strdup (name),
                        modulemd_servicelevel_copy (servicelevel));

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SL]);
}


/**
 * modulemd_modulestream_get_servicelevels:
 *
 * Retrieves the service levels for the module
 *
 * Returns: (element-type utf8 ModulemdServiceLevel) (transfer full): A
 * hash table containing the service levels. This table must be freed with
 * g_hash_table_unref().
 *
 * Deprecated: 1.1
 * Use peek_servicelevels() instead.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_servicelevels (ModulemdModuleStream *self)
{
  GHashTable *servicelevels = NULL;
  GHashTableIter iter;
  gpointer key, value;
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

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
 * modulemd_modulestream_set_stream:
 * @stream: (transfer none) (nullable): the module stream.
 *
 * Sets the "stream" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_stream (ModulemdModuleStream *self,
                                  const gchar *stream)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (g_strcmp0 (self->stream, stream) != 0)
    {
      g_free (self->stream);
      self->stream = g_strdup (stream);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STREAM]);
    }
}


/**
 * modulemd_modulestream_get_stream:
 *
 * Retrieves the "stream" for modulemd.
 *
 * Returns: A string containing the "stream" property. This string must be freed
 * with g_free().
 *
 * Deprecated: 1.1
 * Use peek_stream() instead.
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_stream (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return g_strdup (self->stream);
}


/**
 * modulemd_modulestream_peek_stream: (skip)
 *
 * Retrieves the "stream" for modulemd.
 *
 * Returns: (transfer none): A string containing the "stream" property. This
 * string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_stream (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->stream;
}


/**
 * modulemd_modulestream_set_summary:
 * @summary: (transfer none) (nullable): the module summary.
 *
 * Sets the "summary" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_summary (ModulemdModuleStream *self,
                                   const gchar *summary)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (g_strcmp0 (self->summary, summary) != 0)
    {
      g_free (self->summary);
      self->summary = g_strdup (summary);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SUMMARY]);
    }
}


/**
 * modulemd_modulestream_get_summary:
 *
 * Retrieves the "summary" for modulemd.
 *
 * Returns: A string containing the "summary" property. This string must be
 * freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_summary (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return g_strdup (self->summary);
}


/**
 * modulemd_modulestream_peek_summary: (skip)
 *
 * Retrieves the "summary" for modulemd.
 *
 * Returns: (transfer none): A string containing the "summary" property. This
 * string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_summary (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->summary;
}


/**
 * modulemd_modulestream_set_tracker:
 * @tracker: (transfer none) (nullable): the module tracker.
 *
 * Sets the "tracker" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_tracker (ModulemdModuleStream *self,
                                   const gchar *tracker)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

  if (g_strcmp0 (self->tracker, tracker) != 0)
    {
      g_free (self->tracker);
      self->tracker = g_strdup (tracker);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TRACKER]);
    }
}


/**
 * modulemd_modulestream_get_tracker:
 *
 * Retrieves the "tracker" for modulemd.
 *
 * Returns: (transfer full): A string containing the "tracker" property. This
 * string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_tracker (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return g_strdup (self->tracker);
}


/**
 * modulemd_modulestream_peek_tracker:
 *
 * Retrieves the "tracker" for modulemd.
 *
 * Returns: (transfer none): A string containing the "tracker" property. This
 * string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_tracker (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return self->tracker;
}


/**
 * modulemd_modulestream_set_version
 * @version: the module version
 *
 * Sets the "version" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_version (ModulemdModuleStream *self,
                                   const guint64 version)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));
  if (self->version != version)
    {
      self->version = version;
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VERSION]);
    }
}


/**
 * modulemd_modulestream_get_version:
 *
 * Retrieves the "version" for modulemd.
 *
 * Returns: A 64-bit unsigned integer containing the "version" property.
 *
 * Since: 1.6
 */
guint64
modulemd_modulestream_get_version (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), 0);

  return self->version;
}


/**
 * modulemd_modulestream_set_xmd:
 * @xmd: (transfer none) (nullable) (element-type utf8 GVariant): Extensible
 * metadata block
 *
 * Sets the 'xmd' property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_xmd (ModulemdModuleStream *self, GHashTable *xmd)
{
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (self));

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
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_XMD]);
    }
}


/**
 * modulemd_modulestream_get_xmd:
 *
 * Retrieves the "xmd" for modulemd.
 *
 * Returns: (element-type utf8 GVariant) (transfer full): A hash table
 * containing the "xmd" property.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_xmd (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULESTREAM (self), NULL);

  return _modulemd_hash_table_deep_variant_copy (self->xmd);
}


/**
 * modulemd_modulestream_get_nsvc:
 *
 * Return the unique module identifier.
 *
 * Returns: a string describing the unique module identifier in the form:
 * "NAME:STREAM:VERSION[:CONTEXT]". This string is owned by the caller and
 * must be freed with g_free().
 */
gchar *
modulemd_modulestream_get_nsvc (ModulemdModuleStream *self)
{
  gchar *nsvc = NULL;
  const gchar *name = modulemd_modulestream_peek_name (self);
  const gchar *stream = modulemd_modulestream_peek_stream (self);
  guint64 version = modulemd_modulestream_get_version (self);
  const gchar *context = modulemd_modulestream_peek_context (self);

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
modulemd_modulestream_set_property (GObject *gobject,
                                    guint property_id,
                                    const GValue *value,
                                    GParamSpec *pspec)
{
  ModulemdModuleStream *self = MODULEMD_MODULESTREAM (gobject);

  switch (property_id)
    {
    case PROP_ARCH:
      modulemd_modulestream_set_arch (self, g_value_get_string (value));
      break;

    case PROP_BUILDOPTS:
      modulemd_modulestream_set_buildopts (self, g_value_get_object (value));
      break;

    case PROP_BUILDREQUIRES:
      modulemd_modulestream_set_buildrequires (self,
                                               g_value_get_boxed (value));
      break;

    case PROP_COMMUNITY:
      modulemd_modulestream_set_community (self, g_value_get_string (value));
      break;

    case PROP_CONTENT_LIC:
      modulemd_modulestream_set_content_licenses (self,
                                                  g_value_get_object (value));
      break;

    case PROP_CONTEXT:
      modulemd_modulestream_set_context (self, g_value_get_string (value));
      break;

    case PROP_DEPS:
      modulemd_modulestream_set_dependencies (self, g_value_get_boxed (value));
      break;

    case PROP_DESC:
      modulemd_modulestream_set_description (self, g_value_get_string (value));
      break;

    case PROP_DOCS:
      modulemd_modulestream_set_documentation (self,
                                               g_value_get_string (value));
      break;

    case PROP_EOL:
      modulemd_modulestream_set_eol (self, g_value_get_boxed (value));
      break;

    case PROP_MDVERSION:
      modulemd_modulestream_set_mdversion (self, g_value_get_uint64 (value));
      break;

    case PROP_MODULE_COMPONENTS:
      modulemd_modulestream_set_module_components (self,
                                                   g_value_get_boxed (value));
      break;

    case PROP_MODULE_LIC:
      modulemd_modulestream_set_module_licenses (self,
                                                 g_value_get_object (value));
      break;

    case PROP_NAME:
      modulemd_modulestream_set_name (self, g_value_get_string (value));
      break;

    case PROP_PROFILES:
      modulemd_modulestream_set_profiles (self, g_value_get_boxed (value));
      break;

    case PROP_REQUIRES:
      modulemd_modulestream_set_requires (self, g_value_get_boxed (value));
      break;

    case PROP_RPM_API:
      modulemd_modulestream_set_rpm_api (self, g_value_get_object (value));
      break;

    case PROP_RPM_ARTIFACTS:
      modulemd_modulestream_set_rpm_artifacts (self,
                                               g_value_get_object (value));
      break;

    case PROP_RPM_COMPONENTS:
      modulemd_modulestream_set_rpm_components (self,
                                                g_value_get_boxed (value));
      break;

    case PROP_RPM_FILTER:
      modulemd_modulestream_set_rpm_filter (self, g_value_get_object (value));
      break;

    case PROP_SL:
      modulemd_modulestream_set_servicelevels (self,
                                               g_value_get_boxed (value));
      break;

    case PROP_STREAM:
      modulemd_modulestream_set_stream (self, g_value_get_string (value));
      break;

    case PROP_SUMMARY:
      modulemd_modulestream_set_summary (self, g_value_get_string (value));
      break;

    case PROP_TRACKER:
      modulemd_modulestream_set_tracker (self, g_value_get_string (value));
      break;

    case PROP_VERSION:
      modulemd_modulestream_set_version (self, g_value_get_uint64 (value));
      break;

    case PROP_XMD:
      modulemd_modulestream_set_xmd (self, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_modulestream_get_property (GObject *gobject,
                                    guint property_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  ModulemdModuleStream *self = MODULEMD_MODULESTREAM (gobject);

  switch (property_id)
    {
    case PROP_ARCH:
      g_value_take_string (value, modulemd_modulestream_get_arch (self));
      break;

    case PROP_BUILDOPTS:
      g_value_take_object (value, modulemd_modulestream_get_buildopts (self));
      break;

    case PROP_BUILDREQUIRES:
      g_value_take_boxed (value,
                          modulemd_modulestream_get_buildrequires (self));
      break;

    case PROP_COMMUNITY:
      g_value_take_string (value, modulemd_modulestream_get_community (self));
      break;

    case PROP_CONTENT_LIC:
      g_value_take_object (value,
                           modulemd_modulestream_get_content_licenses (self));
      break;

    case PROP_CONTEXT:
      g_value_take_string (value, modulemd_modulestream_get_context (self));
      break;

    case PROP_DEPS:
      g_value_take_boxed (value,
                          modulemd_modulestream_get_dependencies (self));
      break;

    case PROP_DESC:
      g_value_take_string (value,
                           modulemd_modulestream_get_description (self));
      break;

    case PROP_DOCS:
      g_value_take_string (value,
                           modulemd_modulestream_get_documentation (self));
      break;

    case PROP_EOL:
      g_value_take_boxed (value, modulemd_modulestream_get_eol (self));
      break;

    case PROP_MDVERSION:
      g_value_set_uint64 (value, modulemd_modulestream_get_mdversion (self));
      break;

    case PROP_MODULE_COMPONENTS:
      g_value_take_boxed (value,
                          modulemd_modulestream_get_module_components (self));
      break;

    case PROP_MODULE_LIC:
      g_value_take_object (value,
                           modulemd_modulestream_get_module_licenses (self));
      break;

    case PROP_NAME:
      g_value_take_string (value, modulemd_modulestream_get_name (self));
      break;

    case PROP_PROFILES:
      g_value_take_boxed (value, modulemd_modulestream_get_profiles (self));
      break;

    case PROP_REQUIRES:
      g_value_take_boxed (value, modulemd_modulestream_get_requires (self));
      break;

    case PROP_RPM_API:
      g_value_take_object (value, modulemd_modulestream_get_rpm_api (self));
      break;

    case PROP_RPM_ARTIFACTS:
      g_value_take_object (value,
                           modulemd_modulestream_get_rpm_artifacts (self));
      break;

    case PROP_RPM_COMPONENTS:
      g_value_take_boxed (value,
                          modulemd_modulestream_get_rpm_components (self));
      break;

    case PROP_RPM_FILTER:
      g_value_take_object (value, modulemd_modulestream_get_rpm_filter (self));
      break;

    case PROP_SL:
      g_value_take_boxed (value,
                          modulemd_modulestream_get_servicelevels (self));
      break;

    case PROP_STREAM:
      g_value_take_string (value, modulemd_modulestream_get_stream (self));
      break;

    case PROP_SUMMARY:
      g_value_take_string (value, modulemd_modulestream_get_summary (self));
      break;

    case PROP_TRACKER:
      g_value_take_string (value, modulemd_modulestream_get_tracker (self));
      break;

    case PROP_VERSION:
      g_value_set_uint64 (value, modulemd_modulestream_get_version (self));
      break;

    case PROP_XMD:
      g_value_take_boxed (value, modulemd_modulestream_get_xmd (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_modulestream_finalize (GObject *gobject)
{
  ModulemdModuleStream *self = (ModulemdModuleStream *)gobject;

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
  g_clear_pointer (&self->rpm_components, g_hash_table_unref);
  g_clear_pointer (&self->rpm_filter, g_object_unref);
  g_clear_pointer (&self->servicelevels, g_hash_table_unref);
  g_clear_pointer (&self->stream, g_free);
  g_clear_pointer (&self->summary, g_free);
  g_clear_pointer (&self->tracker, g_free);
  g_clear_pointer (&self->xmd, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_modulestream_parent_class)->finalize (gobject);
}

static void
modulemd_modulestream_class_init (ModulemdModuleStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = modulemd_modulestream_set_property;
  object_class->get_property = modulemd_modulestream_get_property;

  object_class->finalize = modulemd_modulestream_finalize;

  properties[PROP_ARCH] =
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

  properties[PROP_BUILDOPTS] =
    g_param_spec_object ("buildopts",
                         "Build options for the module",
                         "Assorted instructions for the build system on how "
                         "to build this module.",
                         MODULEMD_TYPE_BUILDOPTS,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModuleStream:buildrequires: (type GLib.HashTable(utf8,utf8))
   */
  properties[PROP_BUILDREQUIRES] =
    g_param_spec_boxed ("buildrequires",
                        "Module BuildRequires",
                        "A dictionary property representing the required "
                        "build dependencies of the module. Keys are the "
                        "required module names (strings), values are their "
                        "required stream names (also strings).",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_COMMUNITY] =
    g_param_spec_string ("community",
                         "Module Community",
                         "A string property representing a link to the "
                         "upstream community for this module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_CONTENT_LIC] =
    g_param_spec_object ("content-licenses",
                         "Module Content Licenses",
                         "The set of licenses under which the contents "
                         "of this module are released.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_CONTEXT] =
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
   * ModulemdModuleStream:dependencies: (type GLib.PtrArray(ModulemdDependencies))
   */
  properties[PROP_DEPS] =
    g_param_spec_boxed ("dependencies",
                        "Module Dependencies",
                        "A list of build and runtime requirements needed by "
                        "this module.",
                        G_TYPE_PTR_ARRAY,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_DESC] =
    g_param_spec_string ("description",
                         "Module Description",
                         "A string property representing a detailed "
                         "description of the module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_DOCS] =
    g_param_spec_string ("documentation",
                         "Module Documentation",
                         "A string property representing a link to the "
                         "upstream documentation for this module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModuleStream:eol: (type GLib.Date)
   */
  properties[PROP_EOL] =
    g_param_spec_boxed ("eol",
                        "End of Life",
                        "An ISO-8601 compatible YYYY-MM-DD value representing "
                        "the end-of-life date of this module. This field is "
                        "obsolete; use 'servicelevels' instead.",
                        G_TYPE_DATE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_MDVERSION] =
    g_param_spec_uint64 ("mdversion",
                         "Module Metadata Version",
                         "An int property representing the metadata "
                         "format version used.",
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModuleStream:components-module: (type GLib.HashTable(utf8,ModulemdComponentModule))
   */
  properties[PROP_MODULE_COMPONENTS] =
    g_param_spec_boxed ("components-module",
                        "Module Components",
                        "The module components that define this module.",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_MODULE_LIC] =
    g_param_spec_object ("module-licenses",
                         "Module Licenses",
                         "The set of licenses under which this module is "
                         "released.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Module Name",
                         "A string property representing the name of "
                         "the module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModuleStream:profiles: (type GLib.HashTable(utf8,ModulemdProfile))
   */
  properties[PROP_PROFILES] =
    g_param_spec_boxed ("profiles",
                        "Module Profiles",
                        "A dictionary property representing the module "
                        "profiles.",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModuleStream:requires: (type GLib.HashTable(utf8,utf8))
   */
  properties[PROP_REQUIRES] =
    g_param_spec_boxed ("requires",
                        "Module Requires",
                        "A dictionary property representing the required "
                        "dependencies of the module. Keys are the "
                        "required module names (strings), values are their "
                        "required stream names (also strings).",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_RPM_API] =
    g_param_spec_object ("rpm-api",
                         "Module API - RPMs",
                         "The RPMs that make up the public API of this "
                         "module.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_RPM_ARTIFACTS] =
    g_param_spec_object ("rpm-artifacts",
                         "Module artifacts - RPMs",
                         "The RPMs that make up the output artifacts for "
                         "this module.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModuleStream:components-rpm: (type GLib.HashTable(utf8,ModulemdComponentRpm))
   */
  properties[PROP_RPM_COMPONENTS] =
    g_param_spec_boxed ("components-rpm",
                        "RPM Components",
                        "The RPM components that define this module.",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_RPM_FILTER] =
    g_param_spec_object ("rpm-filter",
                         "Module filter - RPMs",
                         "The RPMs that are explicitly filtered out of "
                         "this module.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModuleStream:servicelevels: (type GLib.HashTable(utf8,ModulemdServiceLevel))
   */
  properties[PROP_SL] =
    g_param_spec_boxed ("servicelevels",
                        "Service Levels",
                        "A dictionary of service levels that apply to this "
                        "module.",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_STREAM] =
    g_param_spec_string ("stream",
                         "Module Stream",
                         "A string property representing the stream name "
                         "of the module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_SUMMARY] =
    g_param_spec_string ("summary",
                         "Module Short Description",
                         "A string property representing a short summary "
                         "of the module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_TRACKER] =
    g_param_spec_string ("tracker",
                         "Module Bug Tracker",
                         "A string property representing a link to the "
                         "upstream bug tracker for this module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_VERSION] =
    g_param_spec_uint64 ("version",
                         "Module Version",
                         "An integer property representing the version of "
                         "the module.",
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModuleStream:xmd: (type GLib.HashTable(utf8,GVariant))
   */
  properties[PROP_XMD] =
    g_param_spec_boxed ("xmd",
                        "Extensible Metadata Block",
                        "A dictionary of user-defined keys and values. "
                        "Optional.  Defaults to an empty dictionary. ",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (
    object_class, MD_N_PROPERTIES, properties);
}

static void
modulemd_modulestream_init (ModulemdModuleStream *self)
{
  /* Allocate the members */
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
  self->rpm_filter = modulemd_simpleset_new ();

  self->servicelevels =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->xmd = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_variant_unref);
}
