/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat, Inc.
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

#include "modulemd-build-config.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-profile.h"
#include "modulemd-subdocument-info.h"
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-packager-v3
 * @title: Modulemd.PackagerV3
 * @stability: Private
 * @short_description: Internal representation of the modulemd-packager v3
 * format.
 */

#define MODULEMD_TYPE_PACKAGER_V3 (modulemd_packager_v3_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdPackagerV3, modulemd_packager_v3, MODULEMD, PACKAGER_V3, GObject)


/**
 * MMD_PACKAGER_DEFAULT_MODULE_LICENSE:
 *
 * The ModulePackager v3 default module metadata license.
 *
 * Since: 2.11
 */
#define MMD_PACKAGER_DEFAULT_MODULE_LICENSE "MIT"

/**
 * ModulemdPackagerVersionEnum:
 * @MD_PACKAGER_VERSION_ERROR: Represents an error handling module stream
 * version.
 * @MD_PACKAGER_VERSION_UNSET: Represents an unset module stream version.
 * @MD_PACKAGER_VERSION_TWO: Represents v2 of the #ModulemdPackager
 * metadata format. Since: 2.11
 * @MD_PACKAGER_VERSION_THREE: Represents v3 of the #ModulemdPackager
 * metadata format. Since: 2.11
 * @MD_PACKAGER_VERSION_LATEST: Represents the highest-supported version of
 * the #ModulemdPackager metadata format.
 *
 * Since: 2.11
 */
typedef enum
{
  MD_PACKAGER_VERSION_ERROR = -1,
  MD_PACKAGER_VERSION_UNSET = 0,

  /* There is no Packager v1 */
  MD_PACKAGER_VERSION_TWO = 2,
  MD_PACKAGER_VERSION_THREE = 3,

  MD_PACKAGER_VERSION_LATEST = MD_PACKAGER_VERSION_THREE
} ModulemdPackagerVersionEnum;


/**
 * modulemd_packager_v3_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdPackagerV3 object. This
 * object must be freed with g_object_unref().
 *
 * Since: 2.11
 */
ModulemdPackagerV3 *
modulemd_packager_v3_new (void);


/**
 * modulemd_packager_v3_copy:
 *
 * Returns: (transfer full): A newly-allocated deep-copy of this
 * #ModulemdPackagerV3 object. This object must be freed with g_object_unref().
 *
 * Since: 2.11
 */
ModulemdPackagerV3 *
modulemd_packager_v3_copy (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_set_module_name:
 * @self: This #ModulemdPackagerV3 object.
 * @module_name: The name of the module.
 *
 * Sets the module name that this #ModulemdPackagerV3 object references.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_set_module_name (ModulemdPackagerV3 *self,
                                      const gchar *module_name);


/**
 * modulemd_packager_v3_get_module_name:
 * @self: This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer none): The module name.
 *
 * Since: 2.11
 */
const gchar *
modulemd_packager_v3_get_module_name (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_set_stream_name:
 * @self: This #ModulemdPackagerV3 object.
 * @stream_name: The name of the module.
 *
 * Sets the stream name that this #ModulemdPackagerV3 object references.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_set_stream_name (ModulemdPackagerV3 *self,
                                      const gchar *stream_name);


/**
 * modulemd_packager_v3_get_stream_name:
 * @self: This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer none): The module stream name.
 *
 * Since: 2.11
 */
const gchar *
modulemd_packager_v3_get_stream_name (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_set_summary:
 * @self: This #ModulemdPackagerV3 object.
 * @summary: (nullable): A short description of the module.
 *
 * Sets the module's short description.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_set_summary (ModulemdPackagerV3 *self,
                                  const gchar *summary);


/**
 * modulemd_packager_v3_get_summary:
 * @self: This #ModulemdPackagerV3 object.
 *
 * Returns: The short description of the module.
 *
 * Since: 2.11
 */
const gchar *
modulemd_packager_v3_get_summary (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_set_description:
 * @self: This #ModulemdPackagerV3 object.
 * @description: (nullable): A complete description of the module.
 *
 * Sets the module's long description.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_set_description (ModulemdPackagerV3 *self,
                                      const gchar *description);


/**
 * modulemd_packager_v3_get_description:
 * @self: This #ModulemdPackagerV3 object.
 *
 * Returns: The long description of the module.
 *
 * Since: 2.11
 */
const gchar *
modulemd_packager_v3_get_description (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_add_module_license:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @license: (in): A license under which this module stream is distributed.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_add_module_license (ModulemdPackagerV3 *self,
                                         const gchar *license);


/**
 * modulemd_packager_v3_remove_module_license:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @license: (in): A license to remove from the list. Has no effect if the
 * license is not present.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_remove_module_license (ModulemdPackagerV3 *self,
                                            const gchar *license);


/**
 * modulemd_packager_v3_clear_module_licenses:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Remove all module licenses from @self
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_clear_module_licenses (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_get_module_licenses_as_strv:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer full): A #GStrv of module licenses associated with this
 * module stream.
 *
 * Since: 2.11
 */
GStrv
modulemd_packager_v3_get_module_licenses_as_strv (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_set_xmd:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @xmd: (in) (transfer none): A #GVariant representing arbitrary YAML.
 *
 * Sets the eXtensible MetaData (XMD) for this module. XMD is arbitrary YAML
 * data that will be set and returned as-is (with the exception that the
 * ordering of mapping keys is not defined). Useful for carrying private data.
 *
 * This function assumes ownership of the XMD #GVariant and thus should not be
 * freed by the caller.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_set_xmd (ModulemdPackagerV3 *self, GVariant *xmd);


/**
 * modulemd_packager_v3_get_xmd:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer none): The extensible metadata block as a #GVariant.
 *
 * Since: 2.11
 */
GVariant *
modulemd_packager_v3_get_xmd (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_add_build_config:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @buildconfig: (in): A #ModulemdBuildConfig to include.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_add_build_config (ModulemdPackagerV3 *self,
                                       ModulemdBuildConfig *buildconfig);


/**
 * modulemd_packager_v3_clear_build_configs:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Remove all added #ModulemdBuildConfig objects from @self
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_clear_build_configs (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_get_build_config_contexts_as_strv:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer full): A list of contexts associated with the build
 * configurations.
 *
 * Since: 2.11
 */
GStrv
modulemd_packager_v3_get_build_config_contexts_as_strv (
  ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_get_build_config:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @context: (in): The context of the #ModulemdBuildConfig to retrieve
 * from @self.
 *
 * Returns: (transfer none): A #ModulemdBuildConfig with the provided @context
 * or NULL if it was not present.
 *
 * Since: 2.11
 */
ModulemdBuildConfig *
modulemd_packager_v3_get_build_config (ModulemdPackagerV3 *self,
                                       const gchar *context);


/**
 * modulemd_packager_v3_set_community:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @community: (in): The upstream community website for this module.
 *
 * Set the module community website address.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_set_community (ModulemdPackagerV3 *self,
                                    const gchar *community);


/**
 * modulemd_packager_v3_get_community:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer none): The module community website address.
 *
 * Since: 2.11
 */
const gchar *
modulemd_packager_v3_get_community (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_set_documentation:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @documentation: (in): The upstream documentation website for this module.
 *
 * Set the module documentation website address.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_set_documentation (ModulemdPackagerV3 *self,
                                        const gchar *documentation);


/**
 * modulemd_packager_v3_get_documentation:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer none): The module documentation website address.
 *
 * Since: 2.11
 */
const gchar *
modulemd_packager_v3_get_documentation (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_set_tracker:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @tracker: (in): The upstream bug tracker website for this module.
 *
 * Set the module bug tracker website address.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_set_tracker (ModulemdPackagerV3 *self,
                                  const gchar *tracker);


/**
 * modulemd_packager_v3_get_tracker:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer none): The module bug tracker website address.
 *
 * Since: 2.11
 */
const gchar *
modulemd_packager_v3_get_tracker (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_add_profile:
 * @self: This #ModulemdPackagerV3 object.
 * @profile: The new #ModulemdProfile to add.
 *
 * Adds a #ModulemdProfile to this #ModulemdPackagerV3 object.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_add_profile (ModulemdPackagerV3 *self,
                                  ModulemdProfile *profile);


/**
 * modulemd_packager_v3_clear_profiles:
 * @self: This #ModulemdPackagerV3 object.
 *
 * Removes all #ModulemdProfile objects from this #ModulemdPackagerV3 object.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_clear_profiles (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_get_profile_names_as_strv:
 * @self: This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of profile names associated
 * with this #ModulemdPackagerV3 object.
 *
 * Since: 2.11
 */
GStrv
modulemd_packager_v3_get_profile_names_as_strv (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_get_profile:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @profile_name: (in): The name of a profile to retrieve.
 *
 * Returns: (transfer none): The requested profile definition if present in the
 * #ModulemdPackagerV3 object. NULL otherwise.
 *
 * Since: 2.11
 */
ModulemdProfile *
modulemd_packager_v3_get_profile (ModulemdPackagerV3 *self,
                                  const gchar *profile_name);


/**
 * modulemd_packager_v3_add_rpm_api:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @rpm: (in): The name of a binary RPM present in this module that is
 * considered stable public API.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_add_rpm_api (ModulemdPackagerV3 *self, const gchar *rpm);


/**
 * modulemd_packager_v3_remove_rpm_api:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @rpm: (in): A binary RPM name to remove from the list of stable public API.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_remove_rpm_api (ModulemdPackagerV3 *self,
                                     const gchar *rpm);


/**
 * modulemd_packager_v3_clear_rpm_api:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Remove all RPMs from the list of stable public API.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_clear_rpm_api (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_replace_rpm_api:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @set: (in): A #GHashTable set of binary RPMs present in this module stream that is
 * considered stable public API.
 *
 * Any existing API RPMs associated with module stream @self are removed and
 * replaced by @set.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_replace_rpm_api (ModulemdPackagerV3 *self,
                                      GHashTable *set);


/**
 * modulemd_packager_v3_get_rpm_api_as_strv:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of binary RPM names that
 * form the public API of this module stream.
 *
 * Since: 2.11
 */
GStrv
modulemd_packager_v3_get_rpm_api_as_strv (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_add_rpm_filter:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @rpm: (in): The name of a binary RPM to filter out of this module stream.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_add_rpm_filter (ModulemdPackagerV3 *self,
                                     const gchar *rpm);


/**
 * modulemd_packager_v3_remove_rpm_filter:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @rpm: (in): A binary RPM name to remove from the filter list.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_remove_rpm_filter (ModulemdPackagerV3 *self,
                                        const gchar *rpm);


/**
 * modulemd_packager_v3_clear_rpm_filters:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Remove all RPMs from the filter list.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_clear_rpm_filters (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_get_rpm_filters_as_strv:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of binary RPM names that
 * are filtered out of this module stream.
 *
 * Since: 2.11
 */
GStrv
modulemd_packager_v3_get_rpm_filters_as_strv (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_replace_rpm_filters:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @set: (in): A #GHashTable set of names of binary RPMs to filter out of this
 * module stream.
 *
 * Any existing filtered binary RPM names associated with module stream @self
 * are removed and replaced by @set.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_replace_rpm_filters (ModulemdPackagerV3 *self,
                                          GHashTable *set);


/**
 * modulemd_packager_v3_add_component:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @component: (in) (transfer none): A #ModulemdComponent to be added to this
 * module stream.
 *
 * Add a component definition to the module.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_add_component (ModulemdPackagerV3 *self,
                                    ModulemdComponent *component);


/**
 * modulemd_packager_v3_remove_module_component:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @component_name: (in): The name of the component to remove from the module
 * stream.
 *
 * Remove a component from this module stream.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_remove_module_component (ModulemdPackagerV3 *self,
                                              const gchar *component_name);


/**
 * modulemd_packager_v3_clear_module_components:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Remove all module components from this module stream.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_clear_module_components (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_remove_rpm_component:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @component_name: (in): The name of the component to remove from the module
 * stream.
 *
 * Remove a component from this module stream.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_remove_rpm_component (ModulemdPackagerV3 *self,
                                           const gchar *component_name);


/**
 * modulemd_packager_v3_clear_rpm_components:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Remove all RPM components from this module stream.
 *
 * Since: 2.11
 */
void
modulemd_packager_v3_clear_rpm_components (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_get_module_component_names_as_strv: (rename-to modulemd_packager_v3_get_module_component_names)
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of module component names
 * included in this stream.
 *
 * Since: 2.11
 */
GStrv
modulemd_packager_v3_get_module_component_names_as_strv (
  ModulemdPackagerV3 *self);

/**
 * modulemd_packager_v3_get_rpm_component_names_as_strv: (rename-to modulemd_packager_v3_get_rpm_component_names)
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of RPM component names
 * included in this stream.
 *
 * Since: 2.11
 */
GStrv
modulemd_packager_v3_get_rpm_component_names_as_strv (
  ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_get_module_component:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @component_name: (in): The name of the component to retrieve.
 *
 * Returns: (transfer none): The module component matching @component_name if
 * it exists, else NULL.
 *
 * Since: 2.11
 */
ModulemdComponentModule *
modulemd_packager_v3_get_module_component (ModulemdPackagerV3 *self,
                                           const gchar *component_name);


/**
 * modulemd_packager_v3_get_rpm_component:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @component_name: (in): The name of the component to retrieve.
 *
 * Returns: (transfer none): The RPM component matching @component_name if it
 * exists, else NULL.
 *
 * Since: 2.11
 */
ModulemdComponentRpm *
modulemd_packager_v3_get_rpm_component (ModulemdPackagerV3 *self,
                                        const gchar *component_name);

G_END_DECLS
