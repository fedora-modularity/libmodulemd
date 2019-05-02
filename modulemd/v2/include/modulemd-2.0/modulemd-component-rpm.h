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
#include "modulemd-component.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-component-rpm
 * @title: Modulemd.ComponentRpm
 * @stability: stable
 * @short_description: A rpm component that goes into a module stream.
 */
#define MODULEMD_TYPE_COMPONENT_RPM (modulemd_component_rpm_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdComponentRpm,
                      modulemd_component_rpm,
                      MODULEMD,
                      COMPONENT_RPM,
                      ModulemdComponent)


/**
 * modulemd_component_rpm_new:
 * @key: (not nullable): The key of this module component. Used when looking up
 * components from a #ModulemdModuleStream.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdComponentRpm object.
 *
 * Since: 2.0
 */
ModulemdComponentRpm *
modulemd_component_rpm_new (const gchar *key);


/**
 * modulemd_component_rpm_add_restricted_arch:
 * @self: This #ModulemdComponentRpm object.
 * @arch: An architecture on which this package should be available.
 *
 * Restrict the list of architectures on which this RPM will be available. It may be called any number of times to indicate support on additional architectures. Use `reset_arches()` to return to "all architectures".
 *
 * Since: 2.0
 */
void
modulemd_component_rpm_add_restricted_arch (ModulemdComponentRpm *self,
                                            const gchar *arch);


/**
 * modulemd_component_rpm_reset_arches:
 * @self: This #ModulemdComponentRpm object.
 *
 * Indicate that this RPM component is available on all arches.
 *
 * Since: 2.0
 */
void
modulemd_component_rpm_reset_arches (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_get_arches_as_strv: (rename-to modulemd_component_rpm_get_arches)
 * @self: This #ModulemdComponentRpm object.
 *
 * Returns: (transfer full): A list of architectures on which this RPM should be available.
 *
 * Since: 2.0
 */
GStrv
modulemd_component_rpm_get_arches_as_strv (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_add_multilib_arch:
 * @self: This #ModulemdComponentRpm object.
 * @arch: An architecture on which this package should be multilib.
 *
 * Add an architectures on which this RPM will be multilib. It may be called any number of times. Use `reset_multilib_arches()` to return to "no architectures".
 *
 * Since: 2.0
 */
void
modulemd_component_rpm_add_multilib_arch (ModulemdComponentRpm *self,
                                          const gchar *arch);


/**
 * modulemd_component_rpm_reset_multilib_arches:
 * @self: This #ModulemdComponentRpm object.
 *
 * Indicate that this RPM component is multilib on no architectures.
 *
 * Since: 2.0
 */
void
modulemd_component_rpm_reset_multilib_arches (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_get_multilib_arches_as_strv: (rename-to modulemd_component_rpm_get_multilib_arches)
 * @self: This #ModulemdComponentRpm object.
 *
 * Returns: (transfer full): A list of architectures on which multilib should be available.
 *
 * Since: 2.0
 */
GStrv
modulemd_component_rpm_get_multilib_arches_as_strv (
  ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_set_cache:
 * @self: This #ModulemdComponentRpm object.
 * @cache: (in) (nullable): The lookaside cache URL.
 *
 * Since: 2.0
 */
void
modulemd_component_rpm_set_cache (ModulemdComponentRpm *self,
                                  const gchar *cache);


/**
 * modulemd_component_rpm_get_cache:
 * @self: This #ModulemdComponentRpm object.
 *
 * Returns: (transfer none): The lookaside cache URL.
 *
 * Since: 2.0
 */
const gchar *
modulemd_component_rpm_get_cache (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_set_ref:
 * @self: This #ModulemdComponentRpm object.
 * @ref: (in) (nullable): The commit ID in the SCM repository.
 *
 * Since: 2.0
 */
void
modulemd_component_rpm_set_ref (ModulemdComponentRpm *self, const gchar *ref);


/**
 * modulemd_component_rpm_get_ref:
 * @self: This #ModulemdComponentRpm object
 *
 * Returns: (transfer none): The commit ID in the SCM repository.
 *
 * Since: 2.0
 */
const gchar *
modulemd_component_rpm_get_ref (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_set_repository:
 * @self: This #ModulemdComponentRpm object
 * @repository: (in) (nullable): The URI of the SCM repository.
 *
 * Since: 2.0
 */
void
modulemd_component_rpm_set_repository (ModulemdComponentRpm *self,
                                       const gchar *repository);


/**
 * modulemd_component_rpm_get_repository:
 * @self: This #ModulemdComponentRpm object
 *
 * Returns: (transfer none): The URI of the SCM repository.
 *
 * Since: 2.0
 */
const gchar *
modulemd_component_rpm_get_repository (ModulemdComponentRpm *self);

G_END_DECLS
