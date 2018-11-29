/*
 * This file is part of libmodulemd
 * Copyright (C) 2018 Red Hat, Inc.
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

#include <glib-object.h>
#include "modulemd-buildopts.h"
#include "modulemd-component.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-deprecated.h"
#include "modulemd-module-stream.h"
#include "modulemd-profile.h"
#include "modulemd-service-level.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-module-stream-v1
 * @title: Modulemd.ModuleStreamV1
 * @stability: stable
 * @short_description: The data to represent a stream of a module as described
 * by a modulemd YAML document of version 1.
 */

#define MODULEMD_TYPE_MODULE_STREAM_V1 (modulemd_module_stream_v1_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdModuleStreamV1,
                      modulemd_module_stream_v1,
                      MODULEMD,
                      MODULE_STREAM_V1,
                      ModulemdModuleStream)


/**
 * modulemd_module_stream_v1_new:
 * @module_name: (in) (nullable): The name of this module.
 * @module_stream: (in) (nullable): The name of this module stream.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStreamV1 object,
 * with the specified module and stream names, if provided.
 *
 * Since: 2.0
 */
ModulemdModuleStreamV1 *
modulemd_module_stream_v1_new (const gchar *module_name,
                               const gchar *module_stream);


/* ===== Properties ====== */


/**
 * modulemd_module_stream_v1_set_arch:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @arch: (in): The module artifact architecture.
 *
 * Set the module artifact architecture.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_set_arch (ModulemdModuleStreamV1 *self,
                                    const gchar *arch);


/**
 * modulemd_module_stream_v1_get_arch:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer none): The module artifact architecture.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_v1_get_arch (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_set_buildopts:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @buildopts: (in) (transfer none): A #ModulemdBuildOpts object describing
 * build options that apply globally to components in this module.
 *
 * Set build options for this module's components.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_set_buildopts (ModulemdModuleStreamV1 *self,
                                         ModulemdBuildopts *buildopts);


/**
 * modulemd_module_stream_v1_get_buildopts:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer none): The build options for this module's components.
 *
 * Since: 2.0
 */
ModulemdBuildopts *
modulemd_module_stream_v1_get_buildopts (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_set_community:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @community: (in): The upstream community website for this module.
 *
 * Set the module community website address.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_set_community (ModulemdModuleStreamV1 *self,
                                         const gchar *community);


/**
 * modulemd_module_stream_v1_get_community:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer none): The module community website address.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_v1_get_community (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_set_description:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @description: (in): The untranslated description of this module.
 *
 * Set the module description.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_set_description (ModulemdModuleStreamV1 *self,
                                           const gchar *description);


/**
 * modulemd_module_stream_v1_get_description:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer none): The module description.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_v1_get_description (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_set_documentation:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @documentation: (in): The upstream documentation website for this module.
 *
 * Set the module documentation website address.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_set_documentation (ModulemdModuleStreamV1 *self,
                                             const gchar *documentation);


/**
 * modulemd_module_stream_v1_get_documentation:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer none): The module documentation website address.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_v1_get_documentation (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_set_summary:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @summary: (in): The untranslated summary of this module.
 *
 * Set the module summary.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_set_summary (ModulemdModuleStreamV1 *self,
                                       const gchar *summary);


/**
 * modulemd_module_stream_v1_get_summary:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer none): The module summary.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_v1_get_summary (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_set_tracker:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @tracker: (in): The upstream bug tracker website for this module.
 *
 * Set the module bug tracker website address.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_set_tracker (ModulemdModuleStreamV1 *self,
                                       const gchar *tracker);


/**
 * modulemd_module_stream_v1_get_tracker:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer none): The module bug tracker website address.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_v1_get_tracker (ModulemdModuleStreamV1 *self);


/* ===== Non-property Methods ===== */


/**
 * modulemd_module_stream_v1_add_component:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @component: (in) (transfer none): A #ModulemdComponent to be added to this
 * module stream.
 *
 * Add a component definition to the module.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_component (ModulemdModuleStreamV1 *self,
                                         ModulemdComponent *component);


/**
 * modulemd_module_stream_v1_remove_module_component:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @component_name: (in): The name of the component to remove from the module
 * stream.
 *
 * Remove a component from this module stream.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_remove_module_component (
  ModulemdModuleStreamV1 *self, const gchar *component_name);


/**
 * modulemd_module_stream_v1_remove_rpm_component:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @component_name: (in): The name of the component to remove from the module
 * stream.
 *
 * Remove a component from this module stream.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_remove_rpm_component (ModulemdModuleStreamV1 *self,
                                                const gchar *component_name);


/**
 * modulemd_module_stream_v1_get_module_component_names_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of module component names included
 * in this stream.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_module_component_names_as_strv (
  ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_rpm_component_names_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of RPM component names included
 * in this stream.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_rpm_component_names_as_strv (
  ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_get_module_component:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @component_name: (in): The name of the component to retrieve.
 *
 * Returns: (transfer none): The module component matching @component name if
 * it exists, else NULL.
 *
 * Since: 2.0
 */
ModulemdComponentModule *
modulemd_module_stream_v1_get_module_component (ModulemdModuleStreamV1 *self,
                                                const gchar *component_name);


/**
 * modulemd_module_stream_v1_get_rpm_component:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @component_name: (in): The name of the component to retrieve.
 *
 * Returns: (transfer none): The RPM component matching @component name if it
 * exists, else NULL.
 *
 * Since: 2.0
 */
ModulemdComponentRpm *
modulemd_module_stream_v1_get_rpm_component (ModulemdModuleStreamV1 *self,
                                             const gchar *component_name);


/**
 * modulemd_module_stream_v1_add_content_license:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @license: (in): A license under which one or more of the components of this
 * module stream are distributed.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_content_license (ModulemdModuleStreamV1 *self,
                                               const gchar *license);


/**
 * modulemd_module_stream_v1_add_module_license:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @license: (in): A license under which this module stream is distributed.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_module_license (ModulemdModuleStreamV1 *self,
                                              const gchar *license);


/**
 * modulemd_module_stream_v1_remove_content_license:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @license: (in): A license to remove from the list. Has no effect if the
 * license is not present.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_remove_content_license (ModulemdModuleStreamV1 *self,
                                                  const gchar *license);


/**
 * modulemd_module_stream_v1_remove_module_license:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @license: (in): A license to remove from the list. Has no effect if the
 * license is not present.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_remove_module_license (ModulemdModuleStreamV1 *self,
                                                 const gchar *license);


/**
 * modulemd_module_stream_v1_get_content_licenses_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of licenses under which one or
 * more components of this module stream are released.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_content_licenses_as_strv (
  ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_get_module_licenses_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of licenses under which this
 * module stream is released.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_module_licenses_as_strv (
  ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_add_profile:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @profile: (in) (transfer none): A #ModulemdProfile for this module stream.
 *
 * Adds a profile definition to this module stream.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_profile (ModulemdModuleStreamV1 *self,
                                       ModulemdProfile *profile);


/**
 * modulemd_module_stream_v1_clear_profiles:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Removes all profiles from this module stream.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_clear_profiles (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_get_profile_names_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of profile names associated with
 * this module stream.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_profile_names_as_strv (
  ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_get_profile:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @profile_name: (in): The name of a profile to retrieve.
 *
 * Returns: (transfer none): The requested profile definition if present in the
 * module stream. NULL otherwise.
 *
 * Since: 2.0
 */
ModulemdProfile *
modulemd_module_stream_v1_get_profile (ModulemdModuleStreamV1 *self,
                                       const gchar *profile_name);


/**
 * modulemd_module_stream_v1_add_rpm_api:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @rpm: (in): The name of a binary RPM present in this module that is
 * considered stable public API.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_rpm_api (ModulemdModuleStreamV1 *self,
                                       const gchar *rpm);


/**
 * modulemd_module_stream_v1_remove_rpm_api:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @rpm: (in): A binary RPM name to remove from the list of stable public API.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_remove_rpm_api (ModulemdModuleStreamV1 *self,
                                          const gchar *rpm);


/**
 * modulemd_module_stream_v1_get_rpm_api_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of binary RPM names that forms
 * the public API of this module stream.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_rpm_api_as_strv (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_add_rpm_artifact:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @nevr: (in): The NEVR of a binary RPM present in this module stream.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_rpm_artifact (ModulemdModuleStreamV1 *self,
                                            const gchar *nevr);


/**
 * modulemd_module_stream_v1_remove_rpm_artifact:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @nevr: (in): An RPM NEVR to remove from the list of artifacts.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_remove_rpm_artifact (ModulemdModuleStreamV1 *self,
                                               const gchar *nevr);


/**
 * modulemd_module_stream_v1_get_rpm_artifacts_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of RPM NEVRs are included in this
 * module stream.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_rpm_artifacts_as_strv (
  ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_add_rpm_filter:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @rpm: (in): The name of a binary RPM to filter out of this module stream.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_rpm_filter (ModulemdModuleStreamV1 *self,
                                          const gchar *rpm);


/**
 * modulemd_module_stream_v1_remove_rpm_filter:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @rpm: (in): A binary RPM name to remove from the filter list.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_remove_rpm_filter (ModulemdModuleStreamV1 *self,
                                             const gchar *rpm);


/**
 * modulemd_module_stream_v1_get_rpm_filters_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of binary RPM names that are
 * filtered out of this module stream.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_rpm_filters_as_strv (
  ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_add_servicelevel:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @servicelevel: (in) (transfer none): A #ModulemdServiceLevel for this module stream.
 *
 * Adds a servicelevel definition to this module stream.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_servicelevel (
  ModulemdModuleStreamV1 *self, ModulemdServiceLevel *servicelevel);


/**
 * modulemd_module_stream_v1_clear_servicelevels:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Removes all servicelevels from this module stream.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_clear_servicelevels (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_get_servicelevel_names_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of servicelevel names associated with
 * this module stream.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_servicelevel_names_as_strv (
  ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_get_servicelevel:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @servicelevel_name: (in): The name of a servicelevel to retrieve.
 *
 * Returns: (transfer none): The requested servicelevel definition if present in the
 * module stream. NULL otherwise.
 *
 * Since: 2.0
 */
ModulemdServiceLevel *
modulemd_module_stream_v1_get_servicelevel (ModulemdModuleStreamV1 *self,
                                            const gchar *servicelevel_name);


/**
 * modulemd_module_stream_v1_set_eol:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @eol: (in): The end-of-life date for the "rawhide" service level.
 *
 * Comptibility function with early iterations of modulemd v1. This function is
 * a wrapper for Modulemd.ModuleStreamV1.add_servicelevel("rawhide", eol).
 *
 * Since: 2.0
 * Deprecated: 2.0
 */
MMD_DEPRECATED_FOR (modulemd_module_stream_v1_add_servicelevel)
void
modulemd_module_stream_v1_set_eol (ModulemdModuleStreamV1 *self, GDate *eol);


/**
 * modulemd_module_stream_v1_get_eol:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Compatibility function with early iterations of modulemd v1. This function
 * is a wrapper for Modulemd.ModuleStreamV1.get_servicelevel("rawhide").
 *
 * Returns: (transfer none): The end-of-life date for the "rawhide" service
 * level.
 *
 * Since: 2.0
 * Deprecated: 2.0
 */
MMD_DEPRECATED_FOR (modulemd_module_stream_v1_get_servicelevel)
GDate *
modulemd_module_stream_v1_get_eol (ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_add_buildtime_requirement:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @module_name: (in): The name of the module to depend on.
 * @module_stream: (in): The name of the module stream to depend on.
 *
 * Add a build-time dependency for this module.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_buildtime_requirement (
  ModulemdModuleStreamV1 *self,
  const gchar *module_name,
  const gchar *module_stream);

/**
 * modulemd_module_stream_v1_add_runtime_requirement:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @module_name: (in): The name of the module to depend on.
 * @module_stream: (in): The name of the module stream to depend on.
 *
 * Add a runtime dependency for this module.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_add_runtime_requirement (
  ModulemdModuleStreamV1 *self,
  const gchar *module_name,
  const gchar *module_stream);


/**
 * modulemd_module_stream_v1_remove_buildtime_requirement:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @module_name: (in): The name of the module to be removed.
 *
 * Remove a build-time dependency for this module.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_remove_buildtime_requirement (
  ModulemdModuleStreamV1 *self, const gchar *module_name);


/**
 * modulemd_module_stream_v1_remove_runtime_requirement:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @module_name: (in): The name of the module to be removed.
 *
 * Remove a runtime dependency for this module.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_remove_runtime_requirement (
  ModulemdModuleStreamV1 *self, const gchar *module_name);


/**
 * modulemd_module_stream_v1_get_buildtime_modules_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of module names that this module
 * depends on at build-time.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_buildtime_modules_as_strv (
  ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_get_runtime_modules_as_strv:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer full): An ordered list of module names that this module
 * depends on at runtime.
 *
 * Since: 2.0
 */
GStrv
modulemd_module_stream_v1_get_runtime_modules_as_strv (
  ModulemdModuleStreamV1 *self);


/**
 * modulemd_module_stream_v1_get_buildtime_requirement_stream:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @module_name: (in): The name of the module this module depends on.
 *
 * Returns: (transfer none): The name of the stream matching this module name
 * in the build-time dependencies.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_v1_get_buildtime_requirement_stream (
  ModulemdModuleStreamV1 *self, const gchar *module_name);


/**
 * modulemd_module_stream_get_runtime_requirement_stream:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @module_name: (in): The name of the module this module depends on.
 *
 * Returns: (transfer none): The name of the stream matching this module name
 * in the runtime dependencies.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_v1_get_runtime_requirement_stream (
  ModulemdModuleStreamV1 *self, const gchar *module_name);


/**
 * modulemd_module_stream_v1_set_xmd:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 * @xmd: (in) (transfer full): A #GVariant representing arbitrary YAML.
 *
 * Sets the eXtensible MetaData (XMD) for this module. XMD is arbitrary YAML
 * data that will be set and returned as-is (with the exception that the
 * ordering of mapping keys is not defined). Useful for carrying private data.
 *
 * This function assumes ownership of the XMD #GVariant and thus should not be
 * freed by the caller.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v1_set_xmd (ModulemdModuleStreamV1 *self,
                                   GVariant *xmd);


/**
 * modulemd_module_stream_v1_get_xmd:
 * @self: (in): This #ModulemdModuleStreamV1 object.
 *
 * Returns: (transfer none): The extensible metadata block as a GVariant.
 */
GVariant *
modulemd_module_stream_v1_get_xmd (ModulemdModuleStreamV1 *self);


G_END_DECLS
