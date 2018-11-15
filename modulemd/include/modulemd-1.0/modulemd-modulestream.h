/*
 * This file is part of libmodulemd
 * Copyright (C) 2017-2018 Stephen Gallagher
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#pragma once

#include "modulemd-buildopts.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-defaults.h"
#include "modulemd-dependencies.h"
#include "modulemd-profile.h"
#include "modulemd-servicelevel.h"
#include "modulemd-simpleset.h"
#include "modulemd-translation.h"

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-modulestream
 * @title: Modulemd.ModuleStream
 * @short_description: The data to represent a stream of a module as described
 * by a modulemd YAML document.
 */

#define MODULEMD_MODULESTREAM_ERROR modulemd_modulestream_error_quark ()
GQuark
modulemd_modulestream_error_quark (void);

enum ModulemdModuleStreamError
{
  MODULEMD_MODULESTREAM_ERROR_MISSING_CONTENT,
};

#define MODULEMD_TYPE_MODULESTREAM modulemd_modulestream_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdModuleStream, modulemd_modulestream, MODULEMD, MODULESTREAM, GObject)


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
modulemd_modulestream_new (void);


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
modulemd_modulestream_copy (ModulemdModuleStream *self);


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
                                        GError **error);


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
                            GError **error);


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
                                          GError **error);


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
modulemd_modulestream_dumps (ModulemdModuleStream *self, GError **error);


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
                                          GError **error);


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
modulemd_modulestream_upgrade (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_set_arch:
 * @arch: (nullable): the module artifact architecture.
 *
 * Sets the "arch" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_arch (ModulemdModuleStream *self, const gchar *arch);


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
modulemd_modulestream_get_arch (ModulemdModuleStream *self);


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
modulemd_modulestream_peek_arch (ModulemdModuleStream *self);


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
                                     ModulemdBuildopts *buildopts);


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
modulemd_modulestream_get_buildopts (ModulemdModuleStream *self);


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
modulemd_modulestream_peek_buildopts (ModulemdModuleStream *self);


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
                                         GHashTable *buildrequires);


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
modulemd_modulestream_get_buildrequires (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_peek_buildrequires: (skip)
 *
 * Retrieves the "buildrequires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer none): A hash table
 * containing the "buildrequires" property.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_peek_buildrequires (ModulemdModuleStream *self);


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
                                     const gchar *community);


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
modulemd_modulestream_get_community (ModulemdModuleStream *self);


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
modulemd_modulestream_peek_community (ModulemdModuleStream *self);


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
                                            ModulemdSimpleSet *licenses);


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
modulemd_modulestream_get_content_licenses (ModulemdModuleStream *self);


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
ModulemdSimpleSet *
modulemd_modulestream_peek_content_licenses (ModulemdModuleStream *self);


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
                                   const gchar *context);


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
modulemd_modulestream_get_context (ModulemdModuleStream *self);


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
modulemd_modulestream_peek_context (ModulemdModuleStream *self);


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
                                        GPtrArray *deps);


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
                                        ModulemdDependencies *dep);

/**
 * modulemd_modulestream_get_dependencies:
 *
 * Returns: (element-type ModulemdDependencies) (transfer container): The list
 * of dependency objects for this module. This list must be freed with
 * g_ptr_array_unref().
 *
 * Since: 1.6
 */
GPtrArray *
modulemd_modulestream_get_dependencies (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_peek_dependencies: (skip)
 *
 * Returns: (element-type ModulemdDependencies) (transfer none): The list
 * of dependency objects for this module. This list and its contents must not
 * be modified or freed.
 *
 * Since: 1.6
 */
GPtrArray *
modulemd_modulestream_peek_dependencies (ModulemdModuleStream *self);


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
                                       const gchar *description);


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
modulemd_modulestream_get_description (ModulemdModuleStream *self);

/**
 * modulemd_modulestream_get_localized_description:
 * @locale: (transfer none) (nullable): Specify the locale for the description.
 * If NULL is passed, it will attempt to use the LC_MESSAGES locale. If "C" is
 * passed or if the locale has no translation available, it will treat it as
 * untranslated.
 *
 * Returns: (transfer full): A string containing the "description" property,
 * translated into the language specified by @locale if possible. This string
 * must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_localized_description (ModulemdModuleStream *self,
                                                 const gchar *locale);


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
modulemd_modulestream_peek_description (ModulemdModuleStream *self);


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
                                         const gchar *documentation);


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
modulemd_modulestream_get_documentation (ModulemdModuleStream *self);


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
modulemd_modulestream_peek_documentation (ModulemdModuleStream *self);


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
modulemd_modulestream_set_eol (ModulemdModuleStream *self, const GDate *date);


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
modulemd_modulestream_get_eol (ModulemdModuleStream *self);


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
modulemd_modulestream_peek_eol (ModulemdModuleStream *self);


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
                                     const guint64 mdversion);


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
modulemd_modulestream_get_mdversion (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_add_module_component:
 * @component: (transfer none): A #ModulemdComponentModule
 *
 * Adds a #ModulemdComponentModule to the "module_components" hash table.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_add_module_component (
  ModulemdModuleStream *self, ModulemdComponentModule *component);


/**
 * modulemd_modulestream_clear_module_components:
 *
 * Remove all entries from the "module_components" hash table.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_clear_module_components (ModulemdModuleStream *self);


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
                                             GHashTable *components);


/**
 * modulemd_modulestream_get_module_components:
 *
 * Retrieves the "module-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentModule) (transfer container): A hash
 * table containing the "module-components" property. This table must be freed
 * with g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_module_components (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_peek_module_components: (skip)
 *
 * Retrieves the "module-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentModule) (transfer none): A hash
 * table containing the "module-components" property.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_peek_module_components (ModulemdModuleStream *self);


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
                                           ModulemdSimpleSet *licenses);


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
modulemd_modulestream_get_module_licenses (ModulemdModuleStream *self);


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
ModulemdSimpleSet *
modulemd_modulestream_peek_module_licenses (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_set_name:
 * @name: (transfer none) (nullable): the module name.
 *
 * Sets the "name" property.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_name (ModulemdModuleStream *self, const gchar *name);


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
modulemd_modulestream_get_name (ModulemdModuleStream *self);


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
modulemd_modulestream_peek_name (ModulemdModuleStream *self);


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
                                   ModulemdProfile *profile);


/**
 * modulemd_modulestream_clear_profiles:
 *
 * Remove all entries from the "profiles" hash table.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_clear_profiles (ModulemdModuleStream *self);


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
                                    GHashTable *profiles);


/**
 * modulemd_modulestream_get_profiles:
 *
 * Retrieves the "profiles" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdProfile) (transfer container): A hash
 * table containing the "profiles" property. This table must be freed with
 * g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_profiles (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_peek_profiles: (skip)
 *
 * Retrieves the "profiles" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdProfile) (transfer none): A hash
 * table containing the "profiles" property.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_peek_profiles (ModulemdModuleStream *self);


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
                                    GHashTable *requires);


/**
 * modulemd_modulestream_get_requires:
 *
 * Retrieves the "requires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing the "requires" property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later. This table must be freed with
 * g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_requires (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_peek_requires: (skip)
 *
 * Retrieves the "requires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer none): A hash table
 * containing the "requires" property. This function was deprecated and is not
 * valid for modulemd files of version 2 or later.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_peek_requires (ModulemdModuleStream *self);


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
                                   ModulemdSimpleSet *apis);


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
modulemd_modulestream_get_rpm_api (ModulemdModuleStream *self);


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
ModulemdSimpleSet *
modulemd_modulestream_peek_rpm_api (ModulemdModuleStream *self);


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
                                         ModulemdSimpleSet *artifacts);

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
modulemd_modulestream_get_rpm_artifacts (ModulemdModuleStream *self);


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
ModulemdSimpleSet *
modulemd_modulestream_peek_rpm_artifacts (ModulemdModuleStream *self);


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
                                         ModulemdComponentRpm *component);


/**
 * modulemd_modulestream_clear_rpm_components:
 *
 * Remove all entries from the "rpm_components" hash table.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_clear_rpm_components (ModulemdModuleStream *self);


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
                                          GHashTable *components);


/**
 * modulemd_modulestream_get_rpm_components:
 *
 * Retrieves the "rpm-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentRpm) (transfer container): A hash
 * table containing the "rpm-components" property. This table must be freed with
 * g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_rpm_components (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_peek_rpm_components: (skip)
 *
 * Retrieves the "rpm-components" for modulemd.
 *
 * Returns: (element-type utf8 ModulemdComponentRpm) (transfer none): A hash
 * table containing the "rpm-components" property.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_peek_rpm_components (ModulemdModuleStream *self);


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
                                      ModulemdSimpleSet *filter);


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
modulemd_modulestream_get_rpm_filter (ModulemdModuleStream *self);


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
ModulemdSimpleSet *
modulemd_modulestream_peek_rpm_filter (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_clear_servicelevels:
 *
 * Remove all entries from the "servicelevels" hash table
 *
 * Since: 1.6
 */
void
modulemd_modulestream_clear_servicelevels (ModulemdModuleStream *self);


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
                                         GHashTable *servicelevels);


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
                                        ModulemdServiceLevel *servicelevel);


/**
 * modulemd_modulestream_get_servicelevels:
 *
 * Retrieves the service levels for the module
 *
 * Returns: (element-type utf8 ModulemdServiceLevel) (transfer container): A
 * hash table containing the service levels. This table must be freed with
 * g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_servicelevels (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_peek_servicelevels: (skip)
 *
 * Retrieves the service levels for the module
 *
 * Returns: (element-type utf8 ModulemdServiceLevel) (transfer none): A
 * hash table containing the service levels.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_peek_servicelevels (ModulemdModuleStream *self);


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
                                  const gchar *stream);


/**
 * modulemd_modulestream_get_stream:
 *
 * Retrieves the "stream" for modulemd.
 *
 * Returns: A string containing the "stream" property. This string must be freed
 * with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_stream (ModulemdModuleStream *self);


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
modulemd_modulestream_peek_stream (ModulemdModuleStream *self);


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
                                   const gchar *summary);


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
modulemd_modulestream_get_summary (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_get_localized_summary:
 * @locale: (transfer none) (nullable): Specify the locale for the summary. If
 * NULL is passed, it will attempt to use the LC_MESSAGES locale. If "C" is
 * passed or if the locale has no translation available, it will treat it as
 * untranslated.
 *
 * Returns: (transfer full): A string containing the "summary" property,
 * translated into the language specified by @locale if possible. This string
 * must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_modulestream_get_localized_summary (ModulemdModuleStream *self,
                                             const gchar *locale);


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
modulemd_modulestream_peek_summary (ModulemdModuleStream *self);


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
                                   const gchar *tracker);


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
modulemd_modulestream_get_tracker (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_peek_tracker: (skip)
 *
 * Retrieves the "tracker" for modulemd.
 *
 * Returns: (transfer none): A string containing the "tracker" property. This
 * string must not be modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_modulestream_peek_tracker (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_set_translation:
 * @translation: (transfer none) (nullable): A #ModulemdTranslation associated
 * with this module and stream which provides translated strings. If the passed
 * #ModulemdTranslation does not match this module and stream, it will have no
 * effect, but a warning will be emitted.
 *
 * Since: 1.6
 */
void
modulemd_modulestream_set_translation (ModulemdModuleStream *self,
                                       ModulemdTranslation *translation);


/**
 * modulemd_modulestream_get_translation:
 *
 * Returns: (transfer full): A #ModulemdTranslation object associated with this
 * module and stream.
 *
 * Since: 1.6
 */
ModulemdTranslation *
modulemd_modulestream_get_translation (ModulemdModuleStream *self);


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
                                   const guint64 version);


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
modulemd_modulestream_get_version (ModulemdModuleStream *self);


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
modulemd_modulestream_set_xmd (ModulemdModuleStream *self, GHashTable *xmd);


/**
 * modulemd_modulestream_get_xmd:
 *
 * Retrieves the "xmd" for modulemd.
 *
 * Returns: (element-type utf8 GVariant) (transfer container): A hash table
 * containing the "xmd" property.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_get_xmd (ModulemdModuleStream *self);


/**
 * modulemd_modulestream_peek_xmd: (skip)
 *
 * Retrieves the "xmd" for modulemd.
 *
 * Returns: (element-type utf8 GVariant) (transfer none): A hash table
 * containing the "xmd" property.
 *
 * Since: 1.6
 */
GHashTable *
modulemd_modulestream_peek_xmd (ModulemdModuleStream *self);


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
modulemd_modulestream_get_nsvc (ModulemdModuleStream *self);

G_END_DECLS
