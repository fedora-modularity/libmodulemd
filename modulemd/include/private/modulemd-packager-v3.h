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

#include <glib-object.h>

#include "modulemd-2.0/modulemd-component-module.h"
#include "modulemd-2.0/modulemd-component-rpm.h"
#include "modulemd-2.0/modulemd-profile.h"
#include "modulemd-2.0/modulemd-subdocument-info.h"

#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-module-stream-v3-private.h"
#include "private/modulemd-build-config.h"
#include "private/modulemd-module-index-private.h"

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
 * ModulemdPackagerVersionEnum:
 * @MD_PACKAGER_VERSION_ERROR: Represents an error handling module stream
 * version.
 * @MD_PACKAGER_VERSION_UNSET: Represents an unset module stream version.
 * @MD_PACKAGER_VERSION_TWO: Represents v2 of the #ModulemdPackager
 * metadata format. Since: 2.10
 * @MD_PACKAGER_VERSION_THREE: Represents v3 of the #ModulemdPackager
 * metadata format. Since: 2.10
 * @MD_PACKAGER_VERSION_LATEST: Represents the highest-supported version of
 * the #ModulemdPackager metadata format.
 *
 * Since: 2.10
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
 * Since: 2.10
 */
ModulemdPackagerV3 *
modulemd_packager_v3_new (void);


/**
 * modulemd_packager_v3_copy:
 *
 * Returns: (transfer full): A newly-allocated deep-copy of this
 * #ModulemdPackagerV3 object. This object must be freed with g_object_unref().
 *
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
 */
const gchar *
modulemd_packager_v3_get_description (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_add_module_license:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @license: (in): A license under which this module stream is distributed.
 *
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
 */
void
modulemd_packager_v3_clear_module_licenses (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_get_module_licenses:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer full): A #GStrv of module licenses associated with this
 * module stream.
 *
 * Since: 2.10
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
 * Since: 2.10
 */
void
modulemd_packager_v3_set_xmd (ModulemdPackagerV3 *self, GVariant *xmd);


/**
 * modulemd_packager_v3_get_xmd:
 * @self: (in): This #ModulemdPackagerV3 object.
 *
 * Returns: (transfer none): The extensible metadata block as a #GVariant.
 *
 * Since: 2.10
 */
GVariant *
modulemd_packager_v3_get_xmd (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_add_build_config:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @buildconfig: (in): A #ModulemdBuildConfig to include
 *
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
 */
void
modulemd_packager_v3_add_rpm_api (ModulemdPackagerV3 *self, const gchar *rpm);


/**
 * modulemd_packager_v3_remove_rpm_api:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @rpm: (in): A binary RPM name to remove from the list of stable public API.
 *
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
 */
GStrv
modulemd_packager_v3_get_rpm_api_as_strv (ModulemdPackagerV3 *self);


/**
 * modulemd_packager_v3_add_rpm_filter:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @rpm: (in): The name of a binary RPM to filter out of this module stream.
 *
 * Since: 2.10
 */
void
modulemd_packager_v3_add_rpm_filter (ModulemdPackagerV3 *self,
                                     const gchar *rpm);


/**
 * modulemd_packager_v3_remove_rpm_filter:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @rpm: (in): A binary RPM name to remove from the filter list.
 *
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
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
 * Since: 2.10
 */
ModulemdComponentRpm *
modulemd_packager_v3_get_rpm_component (ModulemdPackagerV3 *self,
                                        const gchar *component_name);


/**
 * modulemd_packager_v3_to_defaults:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @defaults_ptr: (out): (transfer-full): A pointer to a pointer to a new
 * #ModulemdDefaults object. Must be a valid pointer to a NULL object when
 * called.
 * @error: (out): A #GError that will return the reason for a conversion error.
 *
 * Sets @defaults_ptr to point to a newly-allocated #ModulemdDefaults object
 * corresponding to the #ModulemdPackagerV3 object @self if @self contains any
 * profiles marked as default. Leaves @defaults_ptr pointing to NULL if @self
 * contained no default profiles.
 *
 * Returns: TRUE if the conversion succeeded, including the case where there
 * @self contains no default profiles. FALSE otherwise and @error will be set.
 *
 * Since: 2.10
 */
gboolean
modulemd_packager_v3_to_defaults (ModulemdPackagerV3 *self,
                                  ModulemdDefaults **defaults,
                                  GError **error);

/**
 * modulemd_packager_v3_to_stream_v2:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @error: (out): A #GError that will return the reason for a conversion error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStreamV2 object
 * corresponding to the #ModulemdPackagerV3 object @self. NULL if there was an
 * error doing the mapping and sets @error appropriately.
 *
 * Since: 2.10
 */
ModulemdModuleStreamV2 *
modulemd_packager_v3_to_stream_v2 (ModulemdPackagerV3 *self, GError **error);

/**
 * modulemd_packager_v3_to_stream_v2_ext:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @error: (out): A #GError that will return the reason for a conversion error.
 *
 * Note: If buildopts (#ModulemdBuildopts) are in use in one or more build
 * configurations in the #ModulemdPackagerV3 object @self, only the buildopts
 * present in the first listed configuration (if any) will be applied to the
 * #ModulemdModuleStreamV2 object in the returned index.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleIndex object
 * containing a #ModulemdModuleStreamV2 object and possibly a
 * #ModulemdDefaults object corresponding to the #ModulemdPackagerV3 object
 * @self. NULL if there was an error doing the mapping and sets @error
 * appropriately.
 *
 * Since: 2.10
 */
ModulemdModuleIndex *
modulemd_packager_v3_to_stream_v2_ext (ModulemdPackagerV3 *self,
                                       GError **error);

/**
 * modulemd_packager_v3_to_stream_v3:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @error: (out): A #GError that will return the reason for a conversion error.
 *
 * This will fail if the #ModulemdPackagerV3 object maps to multiple
 * #ModulemdModuleStreamV3 objects.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStreamV3 object
 * corresponding to the #ModulemdPackagerV3 object @self. NULL if there was an
 * error doing the mapping and sets @error appropriately.
 *
 * Since: 2.10
 */
ModulemdModuleStreamV3 *
modulemd_packager_v3_to_stream_v3 (ModulemdPackagerV3 *self, GError **error);

/**
 * modulemd_packager_v3_to_stream_v2_ext:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @error: (out): A #GError that will return the reason for a conversion error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleIndex object
 * containing one or more #ModulemdModuleStreamV3 objects and possibly a
 * #ModulemdDefaults object corresponding to the #ModulemdPackagerV3 object
 * @self. NULL if there was an error doing the mapping and sets @error
 * appropriately.
 *
 * Since: 2.10
 */
ModulemdModuleIndex *
modulemd_packager_v3_to_stream_v3_ext (ModulemdPackagerV3 *self,
                                       GError **error);


/**
 * modulemd_packager_v3_parse_yaml:
 * @subdoc: (in): A #ModulemdSubdocumentInfo representing a packager v3
 * document.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Parse a #ModulemdPackagerV3 document. This parser always operates in strict
 * mode, since it should only be used as input for a build-system.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdPackagerV3 object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.10
 */
ModulemdPackagerV3 *
modulemd_packager_v3_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                 GError **error);


G_END_DECLS
