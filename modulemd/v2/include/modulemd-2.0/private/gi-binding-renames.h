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

/*
 * This file contains empty "target" functions for renames.
 * This is required because gobject-introspection requires a defined target for
 * any renames, but they don't have to be implemented.
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * modulemd_buildopts_get_rpm_whitelist: (skip)
 */
gchar **
modulemd_buildopts_get_rpm_whitelist (ModulemdBuildopts *self);

/**
 * modulemd_component_rpm_get_arches: (skip)
 */
GStrv
modulemd_component_rpm_get_arches (ModulemdComponentRpm *self);

/**
 * modulemd_component_rpm_get_multilib_arches: (skip)
 */
GStrv
modulemd_component_rpm_get_multilib_arches (ModulemdComponentRpm *self);

/**
 * modulemd_defaults_v1_get_streams_with_default_profiles: (skip)
 */
GStrv
modulemd_defaults_v1_get_streams_with_default_profiles (
  ModulemdDefaultsV1 *self, const gchar *intent);

/**
 * modulemd_defaults_v1_get_default_profiles_for_stream: (skip)
 */
GStrv
modulemd_defaults_v1_get_default_profiles_for_stream (ModulemdDefaultsV1 *self,
                                                      const gchar *stream_name,
                                                      const gchar *intent);

/**
 * modulemd_dependencies_get_buildtime_modules: (skip)
 */
GStrv
modulemd_dependencies_get_buildtime_modules (ModulemdDependencies *self);

/**
 * modulemd_dependencies_get_buildtime_streams: (skip)
 */
GStrv
modulemd_dependencies_get_buildtime_streams (ModulemdDependencies *self,
                                             const gchar *module);

/**
 * modulemd_dependencies_get_runtime_modules: (skip)
 */
GStrv
modulemd_dependencies_get_runtime_modules (ModulemdDependencies *self);

/**
 * modulemd_dependencies_get_runtime_streams: (skip)
 */
GStrv
modulemd_dependencies_get_runtime_streams (ModulemdDependencies *self,
                                           const gchar *module);

/**
 * modulemd_module_get_streams_by_stream_name: (skip)
 */
GPtrArray *
modulemd_module_get_streams_by_stream_name (ModulemdModule *self,
                                            const gchar *stream_name);

/**
 * modulemd_module_stream_get_nsvc: (skip)
 */
gchar *
modulemd_module_stream_get_nsvc (ModulemdModuleStream *self);

/**
 * modulemd_module_stream_v1_get_module_component_names: (skip)
 */
GStrv
modulemd_module_stream_v1_get_module_component_names (
  ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_rpm_component_names: (skip)
 */
GStrv
modulemd_module_stream_v1_get_rpm_component_names (
  ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_content_licenses: (skip)
 */
GStrv
modulemd_module_stream_v1_get_content_licenses (ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_module_licenses: (skip)
 */
GStrv
modulemd_module_stream_v1_get_module_licenses (ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_profile_names: (skip)
 */
GStrv
modulemd_module_stream_v1_get_profile_names (ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_rpm_api: (skip)
 */
GStrv
modulemd_module_stream_v1_get_rpm_api (ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_rpm_artifacts: (skip)
 */
GStrv
modulemd_module_stream_v1_get_rpm_artifacts (ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_rpm_filters: (skip)
 */
GStrv
modulemd_module_stream_v1_get_rpm_filters (ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_servicelevel_names: (skip)
 */
GStrv
modulemd_module_stream_v1_get_servicelevel_names (
  ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_buildtime_modules: (skip)
 */
GStrv
modulemd_module_stream_v1_get_buildtime_modules (ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v1_get_runtime_modules: (skip)
 */
GStrv
modulemd_module_stream_v1_get_runtime_modules (ModulemdModuleStreamV1 *self);

/**
 * modulemd_module_stream_v2_get_module_component_names: (skip)
 */
GStrv
modulemd_module_stream_v2_get_module_component_names (
  ModulemdModuleStreamV2 *self);

/**
 * modulemd_module_stream_v2_get_rpm_component_names: (skip)
 */
GStrv
modulemd_module_stream_v2_get_rpm_component_names (
  ModulemdModuleStreamV2 *self);

/**
 * modulemd_module_stream_v2_get_content_licenses: (skip)
 */
GStrv
modulemd_module_stream_v2_get_content_licenses (ModulemdModuleStreamV2 *self);

/**
 * modulemd_module_stream_v2_get_module_licenses: (skip)
 */
GStrv
modulemd_module_stream_v2_get_module_licenses (ModulemdModuleStreamV2 *self);

/**
 * modulemd_module_stream_v2_get_profile_names: (skip)
 */
GStrv
modulemd_module_stream_v2_get_profile_names (ModulemdModuleStreamV2 *self);

/**
 * modulemd_module_stream_v2_get_rpm_api: (skip)
 */
GStrv
modulemd_module_stream_v2_get_rpm_api (ModulemdModuleStreamV2 *self);

/**
 * modulemd_module_stream_v2_get_rpm_artifacts: (skip)
 */
GStrv
modulemd_module_stream_v2_get_rpm_artifacts (ModulemdModuleStreamV2 *self);

/**
 * modulemd_module_stream_v2_get_rpm_filters: (skip)
 */
GStrv
modulemd_module_stream_v2_get_rpm_filters (ModulemdModuleStreamV2 *self);

/**
 * modulemd_module_stream_v2_get_servicelevel_names: (skip)
 */
GStrv
modulemd_module_stream_v2_get_servicelevel_names (
  ModulemdModuleStreamV2 *self);

/**
 * modulemd_profile_get_rpms: (skip)
 */
gchar **
modulemd_profile_get_rpms (ModulemdProfile *self);

/**
 * modulemd_translation_entry_get_profiles: (skip)
 */
gchar **
modulemd_translation_entry_get_profiles (ModulemdTranslationEntry *self);

/**
 * modulemd_translation_get_locales: (skip)
 */
GStrv
modulemd_translation_get_locales (ModulemdTranslation *self);

/**
 * modulemd_module_index_get_module_names: (skip)
 */
GStrv
modulemd_module_index_get_module_names (ModulemdModuleIndex *self);

G_END_DECLS
