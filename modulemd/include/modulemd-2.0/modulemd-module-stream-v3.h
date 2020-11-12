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

#include "modulemd-buildopts.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-component.h"
#include "modulemd-module-stream.h"
#include "modulemd-profile.h"
#include "modulemd-rpm-map-entry.h"
#include "modulemd-obsoletes.h"
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-module-stream-v3
 * @title: Modulemd.ModuleStreamV3
 * @stability: stable
 * @short_description: The data to represent a stream of a module as described
 * by a modulemd YAML document of version 3.
 */

#define MODULEMD_TYPE_MODULE_STREAM_V3 (modulemd_module_stream_v3_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdModuleStreamV3,
                      modulemd_module_stream_v3,
                      MODULEMD,
                      MODULE_STREAM_V3,
                      ModulemdModuleStream)


/**
 * modulemd_module_stream_v3_new:
 * @module_name: (in) (nullable): The name of this module.
 * @module_stream: (in) (nullable): The name of this module stream.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStreamV3 object,
 * with the specified module and stream names, if provided.
 *
 * Since: 2.10
 */
ModulemdModuleStreamV3 *
modulemd_module_stream_v3_new (const gchar *module_name,
                               const gchar *module_stream);


/* ===== Properties ====== */


/**
 * modulemd_module_stream_v3_set_arch:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @arch: (in): The module artifact architecture.
 *
 * Set the module artifact architecture.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_arch (ModulemdModuleStreamV3 *self,
                                    const gchar *arch);


/**
 * modulemd_module_stream_v3_get_arch:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer none): The module artifact architecture.
 *
 * Since: 2.10
 */
const gchar *
modulemd_module_stream_v3_get_arch (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_set_buildopts:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @buildopts: (in) (transfer none): A #ModulemdBuildopts object describing
 * build options that apply globally to components in this module.
 *
 * Set build options for this module's components.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_buildopts (ModulemdModuleStreamV3 *self,
                                         ModulemdBuildopts *buildopts);


/**
 * modulemd_module_stream_v3_get_buildopts:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer none): The build options for this module's components.
 *
 * Since: 2.10
 */
ModulemdBuildopts *
modulemd_module_stream_v3_get_buildopts (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_set_community:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @community: (in): The upstream community website for this module.
 *
 * Set the module community website address.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_community (ModulemdModuleStreamV3 *self,
                                         const gchar *community);


/**
 * modulemd_module_stream_v3_get_community:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer none): The module community website address.
 *
 * Since: 2.10
 */
const gchar *
modulemd_module_stream_v3_get_community (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_set_description:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @description: (in) (nullable): The untranslated description of this module.
 *
 * Set the module description.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_description (ModulemdModuleStreamV3 *self,
                                           const gchar *description);


/**
 * modulemd_module_stream_v3_get_description:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @locale: (in) (nullable): The name of the locale to use when translating
 * the string. If NULL, it will determine the locale with a system call to
 * `setlocale(LC_MESSAGES, NULL)` and return that. If the caller wants the
 * untranslated string, they should pass `"C"` for the locale.
 *
 * Returns: (transfer none): The module description, translated to the
 * requested locale if available. Translation information is managed by the
 * #ModulemdTranslation and #ModulemdTranslationEntry objects.
 *
 * Since: 2.10
 */
const gchar *
modulemd_module_stream_v3_get_description (ModulemdModuleStreamV3 *self,
                                           const gchar *locale);


/**
 * modulemd_module_stream_v3_set_documentation:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @documentation: (in): The upstream documentation website for this module.
 *
 * Set the module documentation website address.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_documentation (ModulemdModuleStreamV3 *self,
                                             const gchar *documentation);


/**
 * modulemd_module_stream_v3_get_documentation:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer none): The module documentation website address.
 *
 * Since: 2.10
 */
const gchar *
modulemd_module_stream_v3_get_documentation (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_set_summary:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @summary: (in) (nullable): The untranslated summary of this module.
 *
 * Set the module summary.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_summary (ModulemdModuleStreamV3 *self,
                                       const gchar *summary);


/**
 * modulemd_module_stream_v3_get_summary:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @locale: (in) (nullable): The name of the locale to use when translating
 * the string. If NULL, it will determine the locale with a system call to
 * `setlocale(LC_MESSAGES, NULL)` and return that. If the caller wants the
 * untranslated string, they should pass `"C"` for the locale.
 *
 * Returns: (transfer none): The module summary, translated to the requested
 * locale if available. Translation information is managed by the
 * #ModulemdTranslation and #ModulemdTranslationEntry objects.
 *
 * Since: 2.10
 */
const gchar *
modulemd_module_stream_v3_get_summary (ModulemdModuleStreamV3 *self,
                                       const gchar *locale);


/**
 * modulemd_module_stream_v3_set_tracker:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @tracker: (in): The upstream bug tracker website for this module.
 *
 * Set the module bug tracker website address.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_tracker (ModulemdModuleStreamV3 *self,
                                       const gchar *tracker);


/**
 * modulemd_module_stream_v3_get_tracker:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer none): The module bug tracker website address.
 *
 * Since: 2.10
 */
const gchar *
modulemd_module_stream_v3_get_tracker (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_set_platform:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @platform: (in): The buildroot and runtime platform for this module.
 *
 * Set the The buildroot and runtime platform for this module.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_platform (ModulemdModuleStreamV3 *self,
                                        const gchar *platform);


/**
 * modulemd_module_stream_v3_get_platform:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer none): The buildroot and runtime platform for this module.
 *
 * Since: 2.10
 */
const gchar *
modulemd_module_stream_v3_get_platform (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_obsoletes_resolved:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer none): The #ModulemdObsoletes information associated with
 * this object. If the associated obsoletes has reset attribute set, this
 * function doesn't return it. From outside obsoletes with reset looks like
 * there is no obsoletes set for this stream. Every obsoletes (even with reset) can
 * be accessed from the streams module.
 *
 * Since: 2.10
 */
ModulemdObsoletes *
modulemd_module_stream_v3_get_obsoletes_resolved (
  ModulemdModuleStreamV3 *self);


/* ===== Non-property Methods ===== */


/**
 * modulemd_module_stream_v3_add_component:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @component: (in) (transfer none): A #ModulemdComponent to be added to this
 * module stream.
 *
 * Add a component definition to the module.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_add_component (ModulemdModuleStreamV3 *self,
                                         ModulemdComponent *component);


/**
 * modulemd_module_stream_v3_remove_module_component:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @component_name: (in): The name of the component to remove from the module
 * stream.
 *
 * Remove a component from this module stream.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_remove_module_component (
  ModulemdModuleStreamV3 *self, const gchar *component_name);


/**
 * modulemd_module_stream_v3_clear_module_components:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all module components from this module stream.
 */
void
modulemd_module_stream_v3_clear_module_components (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_remove_rpm_component:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @component_name: (in): The name of the component to remove from the module
 * stream.
 *
 * Remove a component from this module stream.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_remove_rpm_component (ModulemdModuleStreamV3 *self,
                                                const gchar *component_name);


/**
 * modulemd_module_stream_v3_clear_rpm_components:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all RPM components from this module stream.
 */
void
modulemd_module_stream_v3_clear_rpm_components (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_module_component_names_as_strv: (rename-to modulemd_module_stream_v3_get_module_component_names)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of module component names
 * included in this stream.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_module_component_names_as_strv (
  ModulemdModuleStreamV3 *self);

/**
 * modulemd_module_stream_v3_get_rpm_component_names_as_strv: (rename-to modulemd_module_stream_v3_get_rpm_component_names)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of RPM component names
 * included in this stream.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_rpm_component_names_as_strv (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_module_component:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @component_name: (in): The name of the component to retrieve.
 *
 * Returns: (transfer none): The module component matching @component_name if
 * it exists, else NULL.
 *
 * Since: 2.10
 */
ModulemdComponentModule *
modulemd_module_stream_v3_get_module_component (ModulemdModuleStreamV3 *self,
                                                const gchar *component_name);


/**
 * modulemd_module_stream_v3_get_rpm_component:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @component_name: (in): The name of the component to retrieve.
 *
 * Returns: (transfer none): The RPM component matching @component_name if it
 * exists, else NULL.
 *
 * Since: 2.10
 */
ModulemdComponentRpm *
modulemd_module_stream_v3_get_rpm_component (ModulemdModuleStreamV3 *self,
                                             const gchar *component_name);


/**
 * modulemd_module_stream_v3_add_content_license:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @license: (in): A license under which one or more of the components of this
 * module stream are distributed.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_add_content_license (ModulemdModuleStreamV3 *self,
                                               const gchar *license);


/**
 * modulemd_module_stream_v3_add_module_license:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @license: (in): A license under which this module stream is distributed.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_add_module_license (ModulemdModuleStreamV3 *self,
                                              const gchar *license);


/**
 * modulemd_module_stream_v3_remove_content_license:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @license: (in): A license to remove from the list. Has no effect if the
 * license is not present.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_remove_content_license (ModulemdModuleStreamV3 *self,
                                                  const gchar *license);


/**
 * modulemd_module_stream_v3_remove_module_license:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @license: (in): A license to remove from the list. Has no effect if the
 * license is not present.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_remove_module_license (ModulemdModuleStreamV3 *self,
                                                 const gchar *license);


/**
 * modulemd_module_stream_v3_clear_content_licenses:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all content licenses.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_clear_content_licenses (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_clear_module_licenses:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all module licenses.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_clear_module_licenses (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_content_licenses_as_strv: (rename-to modulemd_module_stream_v3_get_content_licenses)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of licenses under which one
 * or more components of this module stream are released.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_content_licenses_as_strv (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_module_licenses_as_strv: (rename-to modulemd_module_stream_v3_get_module_licenses)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of licenses under which
 * this module stream is released.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_module_licenses_as_strv (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_add_profile:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @profile: (in) (transfer none): A #ModulemdProfile for this module stream.
 *
 * Adds a profile definition to this module stream.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_add_profile (ModulemdModuleStreamV3 *self,
                                       ModulemdProfile *profile);


/**
 * modulemd_module_stream_v3_clear_profiles:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all profiles from this module stream.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_clear_profiles (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_profile_names_as_strv: (rename-to modulemd_module_stream_v3_get_profile_names)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of profile names associated
 * with this module stream.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_profile_names_as_strv (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_profile:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @profile_name: (in): The name of a profile to retrieve.
 *
 * Returns: (transfer none): The requested profile definition if present in the
 * module stream. NULL otherwise.
 *
 * Since: 2.10
 */
ModulemdProfile *
modulemd_module_stream_v3_get_profile (ModulemdModuleStreamV3 *self,
                                       const gchar *profile_name);


/**
 * modulemd_module_stream_v3_search_profiles:
 * @self: This #ModulemdModuleStreamV3 object.
 * @profile_pattern: (nullable): A globbing pattern to locate one or more
 * profiles in this #ModulemdModuleStreamV3 object. The names will be compared
 * using [fnmatch(3)](https://www.mankier.com/3/fnmatch).
 *
 * Returns: (transfer container) (element-type ModulemdProfile): The list of
 * #ModulemdProfile objects whose name matched @profile_pattern. This function
 * cannot fail, but it may return a zero-length list if no matches were found.
 * The returned profiles will be sorted alphabetically by profile name.
 */
GPtrArray *
modulemd_module_stream_v3_search_profiles (ModulemdModuleStreamV3 *self,
                                           const gchar *profile_pattern);


/**
 * modulemd_module_stream_v3_add_rpm_api:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @rpm: (in): The name of a binary RPM present in this module that is
 * considered stable public API.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_add_rpm_api (ModulemdModuleStreamV3 *self,
                                       const gchar *rpm);


/**
 * modulemd_module_stream_v3_remove_rpm_api:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @rpm: (in): A binary RPM name to remove from the list of stable public API.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_remove_rpm_api (ModulemdModuleStreamV3 *self,
                                          const gchar *rpm);


/**
 * modulemd_module_stream_v3_clear_rpm_api:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all RPMs from the list of stable public API.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_clear_rpm_api (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_rpm_api_as_strv: (rename-to modulemd_module_stream_v3_get_rpm_api)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of binary RPM names that
 * form the public API of this module stream.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_rpm_api_as_strv (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_add_rpm_artifact:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @nevr: (in): The NEVR of a binary RPM present in this module stream.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_add_rpm_artifact (ModulemdModuleStreamV3 *self,
                                            const gchar *nevr);


/**
 * modulemd_module_stream_v3_remove_rpm_artifact:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @nevr: (in): An RPM NEVR to remove from the list of artifacts.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_remove_rpm_artifact (ModulemdModuleStreamV3 *self,
                                               const gchar *nevr);


/**
 * modulemd_module_stream_v3_clear_rpm_artifacts:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all NPM NEVRs from the list of artifacts.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_clear_rpm_artifacts (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_rpm_artifacts_as_strv: (rename-to modulemd_module_stream_v3_get_rpm_artifacts)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of RPM NEVRs that are
 * included in this module stream.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_rpm_artifacts_as_strv (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_set_rpm_artifact_map_entry:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @entry: (in): The RPM map entry to save to the stream.
 * @digest: (in): A string representing the digest algorithm used to generate
 * the @checksum.
 * @checksum: (in): An RPM artifact checksum.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_rpm_artifact_map_entry (
  ModulemdModuleStreamV3 *self,
  ModulemdRpmMapEntry *entry,
  const gchar *digest,
  const gchar *checksum);


/**
 * modulemd_module_stream_v3_get_rpm_artifact_map_entry:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @digest: (in): A string representing the digest algorithm used to generate
 * the @checksum.
 * @checksum: (in): An RPM artifact checksum.
 *
 * Returns: (transfer none): The #ModulemdRpmMapEntry object associated with
 * the provided @checksum generated by the provided @digest.
 *
 * Since: 2.10
 */
ModulemdRpmMapEntry *
modulemd_module_stream_v3_get_rpm_artifact_map_entry (
  ModulemdModuleStreamV3 *self, const gchar *digest, const gchar *checksum);


/**
 * modulemd_module_stream_v3_add_rpm_filter:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @rpm: (in): The name of a binary RPM to filter out of this module stream.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_add_rpm_filter (ModulemdModuleStreamV3 *self,
                                          const gchar *rpm);


/**
 * modulemd_module_stream_v3_remove_rpm_filter:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @rpm: (in): A binary RPM name to remove from the filter list.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_remove_rpm_filter (ModulemdModuleStreamV3 *self,
                                             const gchar *rpm);


/**
 * modulemd_module_stream_v3_clear_rpm_filters:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all RPMs from the filter list.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_clear_rpm_filters (ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_rpm_filters_as_strv: (rename-to modulemd_module_stream_v3_get_rpm_filters)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of binary RPM names that
 * are filtered out of this module stream.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_rpm_filters_as_strv (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_add_buildtime_requirement:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @module_name: (in): The name of the module to depend on.
 * @module_stream: (in): The name of the module stream to depend on.
 *
 * Add a build-time dependency for this module.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_add_buildtime_requirement (
  ModulemdModuleStreamV3 *self,
  const gchar *module_name,
  const gchar *module_stream);

/**
 * modulemd_module_stream_v3_add_runtime_requirement:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @module_name: (in): The name of the module to depend on.
 * @module_stream: (in): The name of the module stream to depend on.
 *
 * Add a runtime dependency for this module.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_add_runtime_requirement (
  ModulemdModuleStreamV3 *self,
  const gchar *module_name,
  const gchar *module_stream);


/**
 * modulemd_module_stream_v3_remove_buildtime_requirement:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @module_name: (in): The name of the module to be removed.
 *
 * Remove a build-time dependency for this module.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_remove_buildtime_requirement (
  ModulemdModuleStreamV3 *self, const gchar *module_name);


/**
 * modulemd_module_stream_v3_remove_runtime_requirement:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @module_name: (in): The name of the module to be removed.
 *
 * Remove a runtime dependency for this module.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_remove_runtime_requirement (
  ModulemdModuleStreamV3 *self, const gchar *module_name);


/**
 * modulemd_module_stream_v3_clear_buildtime_requirements
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all buildtime dependencies for this module.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_clear_buildtime_requirements (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_clear_runtime_requirements
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Remove all runtime dependencies for this module.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_clear_runtime_requirements (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_buildtime_modules_as_strv: (rename-to modulemd_module_stream_v3_get_buildtime_modules)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of module names that this
 * module depends on at build-time.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_buildtime_modules_as_strv (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_runtime_modules_as_strv: (rename-to modulemd_module_stream_v3_get_runtime_modules)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer full): An ordered #GStrv list of module names that this
 * module depends on at runtime.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_runtime_modules_as_strv (
  ModulemdModuleStreamV3 *self);


/**
 * modulemd_module_stream_v3_get_buildtime_requirement_stream:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @module_name: (in): The name of the module this module depends on.
 *
 * Returns: (transfer none): The name of the stream matching this module name
 * in the build-time dependencies.
 *
 * Since: 2.10
 */
const gchar *
modulemd_module_stream_v3_get_buildtime_requirement_stream (
  ModulemdModuleStreamV3 *self, const gchar *module_name);


/**
 * modulemd_module_stream_v3_get_runtime_requirement_stream:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @module_name: (in): The name of the module this module depends on.
 *
 * Returns: (transfer none): The name of the stream matching this module name
 * in the runtime dependencies.
 *
 * Since: 2.10
 */
const gchar *
modulemd_module_stream_v3_get_runtime_requirement_stream (
  ModulemdModuleStreamV3 *self, const gchar *module_name);


/**
 * modulemd_module_stream_v3_get_buildtime_requirement_streams_as_strv: (rename-to modulemd_module_stream_v3_get_buildtime_requirement_streams)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @module_name: (in): The name of the module this module depends on.
 *
 * Returns: (transfer full): NULL if @module_name is not a build-time dependency
 * of @self, else a #GStrv list containing a single element that is the name of
 * the stream matching this module name in the build-time dependencies.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_buildtime_requirement_streams_as_strv (
  ModulemdModuleStreamV3 *self, const gchar *module_name);


/**
 * modulemd_module_stream_v3_get_runtime_requirement_streams_as_strv: (rename-to modulemd_module_stream_v3_get_runtime_requirement_streams)
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @module_name: (in): The name of the module this module depends on.
 *
 * Returns: (transfer full): NULL if @module_name is not a run-time dependency
 * of @self, else a #GStrv list containing a single element that is the name of
 * the stream matching this module name in the run-time dependencies.
 *
 * Since: 2.10
 */
GStrv
modulemd_module_stream_v3_get_runtime_requirement_streams_as_strv (
  ModulemdModuleStreamV3 *self, const gchar *module_name);


/**
 * modulemd_module_stream_v3_set_xmd:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @xmd: (in) (transfer none): A #GVariant representing arbitrary YAML.
 *
 * Sets the eXtensible MetaData (XMD) for this module. XMD is arbitrary YAML
 * data that will be set and returned as-is (with the exception that the
 * ordering of mapping keys is not defined). Useful for carrying private data.
 *
 * This function assumes ownership of the XMD #GVariant and thus should not be
 * freed by the caller.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_set_xmd (ModulemdModuleStreamV3 *self,
                                   GVariant *xmd);


/**
 * modulemd_module_stream_v3_get_xmd:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer none): The extensible metadata block as a #GVariant.
 */
GVariant *
modulemd_module_stream_v3_get_xmd (ModulemdModuleStreamV3 *self);


G_END_DECLS
