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
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-build-config
 * @title: Modulemd.BuildConfig
 * @stability: stable
 * @short_description: The data to represent a configuration for building a
 * module stream.
 */

#define MODULEMD_TYPE_BUILD_CONFIG (modulemd_build_config_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdBuildConfig, modulemd_build_config, MODULEMD, BUILD_CONFIG, GObject)


/**
 * modulemd_build_config_new:
 *
 * Initialize a new #ModulemdBuildConfig representing a module build
 * configuration.
 *
 * Since: 2.11
 */
ModulemdBuildConfig *
modulemd_build_config_new (void);


/**
 * modulemd_build_config_set_context:
 * @self: This #ModulemdBuildConfig object.
 * @context: A string of up to ten alphanumeric characters.
 *
 * Set the context that this build configuration produces.
 *
 * Note: For consistency in the API, this function does not validate the input
 * @context. This validation will be performed as part of the
 * modulemd_build_config_validate() routine where it can be reported cleanly.
 *
 * Since: 2.11
 */
void
modulemd_build_config_set_context (ModulemdBuildConfig *self,
                                   const gchar *context);


/**
 * modulemd_build_config_get_context:
 * @self: This #ModulemdBuildConfig object.
 *
 * Get the context that this build configuration produces.
 *
 * Note: This function returns the context as stored internally. If you want to
 * be sure that it is in the correct format, call
 * modulemd_build_config_validate() first.
 *
 * Returns: (transfer none): The string representing the context that this
 * build configuration produces.
 *
 * Since: 2.11
 */
const gchar *
modulemd_build_config_get_context (ModulemdBuildConfig *self);


/**
 * modulemd_build_config_set_platform:
 * @self: This #ModulemdBuildConfig object.
 * @platform: A string of up to ten alphanumeric characters.
 *
 * Set the platform that this build configuration applies to.
 *
 * Since: 2.11
 */
void
modulemd_build_config_set_platform (ModulemdBuildConfig *self,
                                    const gchar *platform);


/**
 * modulemd_build_config_get_platform:
 * @self: This #ModulemdBuildConfig object.
 *
 * Get the platform that this build configuration applies to.
 *
 * Returns: (transfer none): The string representing the platform that this
 * build configuration applies to.
 *
 * Since: 2.11
 */
const gchar *
modulemd_build_config_get_platform (ModulemdBuildConfig *self);


/**
 * modulemd_build_config_set_stream:
 * @self: This #ModulemdBuildConfig object.
 * @stream: A stream name to use as an override for this configuration.
 *
 * Set the stream name override for this build configuration.
 *
 * Since: 2.15
 */
void
modulemd_build_config_set_stream (ModulemdBuildConfig *self,
                                  const gchar *stream);


/**
 * modulemd_build_config_get_stream:
 * @self: This #ModulemdBuildConfig object.
 *
 * Get the stream override for this build configuration.
 *
 * Returns: (transfer none): The string representing the overridden stream
 * name.
 *
 * Since name: 2.15
 */
const gchar *
modulemd_build_config_get_stream (ModulemdBuildConfig *self);


/**
 * modulemd_build_config_add_runtime_requirement:
 * @self: (in): This #ModulemdBuildConfig object.
 * @module_name: (in): The name of the module to depend on.
 * @stream_name: (in): The name of the module stream to depend on.
 *
 * Add a build-time dependency for this module.
 *
 * Since: 2.11
 */
void
modulemd_build_config_add_runtime_requirement (ModulemdBuildConfig *self,
                                               const gchar *module_name,
                                               const gchar *stream_name);


/**
 * modulemd_build_config_remove_runtime_requirement:
 * @self: (in): This #ModulemdBuildConfig object.
 * @module_name: (in): The name of the module to be removed.
 *
 * Remove a run-time dependency for this module.
 *
 * Since: 2.11
 */
void
modulemd_build_config_remove_runtime_requirement (ModulemdBuildConfig *self,
                                                  const gchar *module_name);


/**
 * modulemd_build_config_clear_runtime_requirements
 * @self: (in): This #ModulemdBuildConfig object.
 *
 * Remove all run-time dependencies for this module.
 *
 * Since: 2.11
 */
void
modulemd_build_config_clear_runtime_requirements (ModulemdBuildConfig *self);


/**
 * modulemd_build_config_get_runtime_requirement_stream:
 * @self: (in): This #ModulemdBuildConfig object.
 * @module_name: (in): The name of the module this module depends on.
 *
 * Returns: (transfer none): The name of the stream matching this module name
 * in the run-time dependencies.
 *
 * Since: 2.11
 */
const gchar *
modulemd_build_config_get_runtime_requirement_stream (
  ModulemdBuildConfig *self, const gchar *module_name);


/**
 * modulemd_build_config_get_runtime_modules_as_strv:
 * @self: (in): This #ModulemdBuildConfig object.
 *
 * Returns: (transfer full): An ordered #GStrv list of module names that this
 * module depends on at run-time.
 *
 * Since: 2.11
 */
GStrv
modulemd_build_config_get_runtime_modules_as_strv (ModulemdBuildConfig *self);


/**
 * modulemd_build_config_add_buildtime_requirement:
 * @self: (in): This #ModulemdBuildConfig object.
 * @module_name: (in): The name of the module to depend on.
 * @stream_name: (in): The name of the module stream to depend on.
 *
 * Add a build-time dependency for this module.
 *
 * Since: 2.11
 */
void
modulemd_build_config_add_buildtime_requirement (ModulemdBuildConfig *self,
                                                 const gchar *module_name,
                                                 const gchar *stream_name);


/**
 * modulemd_build_config_remove_buildtime_requirement:
 * @self: (in): This #ModulemdBuildConfig object.
 * @module_name: (in): The name of the module to be removed.
 *
 * Remove a build-time dependency for this module.
 *
 * Since: 2.11
 */
void
modulemd_build_config_remove_buildtime_requirement (ModulemdBuildConfig *self,
                                                    const gchar *module_name);


/**
 * modulemd_build_config_clear_buildtime_requirements
 * @self: (in): This #ModulemdBuildConfig object.
 *
 * Remove all build-time dependencies for this module.
 *
 * Since: 2.11
 */
void
modulemd_build_config_clear_buildtime_requirements (ModulemdBuildConfig *self);


/**
 * modulemd_build_config_get_buildtime_requirement_stream:
 * @self: (in): This #ModulemdBuildConfig object.
 * @module_name: (in): The name of the module this module depends on.
 *
 * Returns: (transfer none): The name of the stream matching this module name
 * in the build-time dependencies.
 *
 * Since: 2.11
 */
const gchar *
modulemd_build_config_get_buildtime_requirement_stream (
  ModulemdBuildConfig *self, const gchar *module_name);


/**
 * modulemd_build_config_get_buildtime_modules_as_strv:
 * @self: (in): This #ModulemdBuildConfig object.
 *
 * Returns: (transfer full): An ordered #GStrv list of module names that this
 * module depends on at build-time.
 *
 * Since: 2.11
 */
GStrv
modulemd_build_config_get_buildtime_modules_as_strv (
  ModulemdBuildConfig *self);


/**
 * modulemd_build_config_set_buildopts:
 * @self: (in): This #ModulemdBuildConfig object.
 * @buildopts: (in) (transfer none): A #ModulemdBuildopts object describing
 * build options that apply globally to components in this module.
 *
 * Set build options for this module's components.
 *
 * Since: 2.11
 */
void
modulemd_build_config_set_buildopts (ModulemdBuildConfig *self,
                                     ModulemdBuildopts *buildopts);


/**
 * modulemd_build_config_get_buildopts:
 * @self: (in): This #ModulemdBuildConfig object.
 *
 * Returns: (transfer none): The build options for this module's components.
 *
 * Since: 2.11
 */
ModulemdBuildopts *
modulemd_build_config_get_buildopts (ModulemdBuildConfig *self);


/**
 * modulemd_build_config_validate:
 * @self: (in): This #ModulemdBuildConfig object.
 * @error: (out): A #GError explaining any validation failure.
 *
 * Determine if this #ModulemdBuildConfig is valid according to the YAML
 * specification.
 *
 * Returns: TRUE if validation passes. Returns FALSE and sets @error
 * appropriately on validation failure.
 *
 * Since: 2.11
 */

gboolean
modulemd_build_config_validate (ModulemdBuildConfig *self, GError **error);


/**
 * modulemd_build_config_copy:
 * @self: (in): This #ModulemdBuildConfig object
 *
 * Returns: (transfer full): A deep copy of @self
 *
 * Since: 2.11
 */
ModulemdBuildConfig *
modulemd_build_config_copy (ModulemdBuildConfig *self);


/**
 * modulemd_build_config_equals:
 * @self_1: A pointer to a #ModulemdBuildConfig object.
 * @self_2: A pointer to a #ModulemdBuildConfig object.
 *
 * Returns: TRUE, if @self_1 and @self_2 are pointers to #ModulemdBuildConfig
 * objects containing equivalent data. FALSE, otherwise.
 *
 * Since: 2.11
 */

gboolean
modulemd_build_config_equals (ModulemdBuildConfig *self_1,
                              ModulemdBuildConfig *self_2);


/**
 * modulemd_build_config_compare:
 * @self_1: (in): A pointer to a #ModulemdBuildConfig object.
 * @self_2: (in): A pointer to a #ModulemdBuildConfig object.
 *
 * Returns: Less than zero if @self_1 sorts less than @self_2, zero for equal,
 * greater than zero if @self_1 is greater than @self_2.
 *
 * Since: 2.11
 */
gint
modulemd_build_config_compare (ModulemdBuildConfig *self_1,
                               ModulemdBuildConfig *self_2);

G_END_DECLS
