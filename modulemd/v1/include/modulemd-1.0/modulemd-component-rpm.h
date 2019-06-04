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

#ifndef MODULEMD_COMPONENT_RPM_H
#define MODULEMD_COMPONENT_RPM_H

#include "modulemd-component.h"
#include "modulemd-deprecated.h"
#include "modulemd-simpleset.h"

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-component-rpm
 * @title: Modulemd.ComponentRpm
 * @short_description: An RPM component that goes into a module stream.
 */

#define MODULEMD_TYPE_COMPONENT_RPM (modulemd_component_rpm_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdComponentRpm,
                      modulemd_component_rpm,
                      MODULEMD,
                      COMPONENT_RPM,
                      ModulemdComponent)


/**
 * modulemd_component_rpm_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdComponentRpm. This must
 * be freed with g_object_unref().
 *
 * Since: 1.0
 */
ModulemdComponentRpm *
modulemd_component_rpm_new (void);


/**
 * modulemd_component_rpm_set_arches:
 * @arches: (nullable): a #ModulemdSimpleSet: A set of architectures on which
 * this RPM package should be available. An empty set means the package is
 * available on all supported architectures.
 *
 * Since: 1.0
 */
void
modulemd_component_rpm_set_arches (ModulemdComponentRpm *self,
                                   ModulemdSimpleSet *arches);


/**
 * modulemd_component_rpm_get_arches:
 *
 * Retrieves the set of arches for this component.
 *
 * Returns: (transfer none): A #ModulemdSimpleSet containing the set of
 * supported architectures for this component.
 *
 * Deprecated: 1.1
 * Use peek_arches() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_component_rpm_peek_arches)
ModulemdSimpleSet *
modulemd_component_rpm_get_arches (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_peek_arches:
 *
 * Retrieves the set of arches for this component.
 *
 * Returns: (transfer none): A #ModulemdSimpleSet containing the set of
 * supported architectures for this component.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_component_rpm_peek_arches (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_dup_arches:
 *
 * Retrieves a copy of the set of arches for this component.
 *
 * Returns: (transfer full): A #ModulemdSimpleSet containing the set of
 * supported architectures for this component.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_component_rpm_dup_arches (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_set_cache
 * @cache: (nullable): A string: The URL of the lookaside cache where this package's
 * sources are stored.
 *
 * Since: 1.0
 */
void
modulemd_component_rpm_set_cache (ModulemdComponentRpm *self,
                                  const gchar *cache);


/**
 * modulemd_component_rpm_get_cache:
 *
 * Retrieves the lookaside cache URL.
 *
 * Returns: A string containing the URL to the lookaside cache.
 *
 * Deprecated: 1.1
 * Use peek_cache() instead.
 */
MMD_DEPRECATED_FOR (modulemd_component_rpm_peek_cache)
const gchar *
modulemd_component_rpm_get_cache (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_peek_cache:
 *
 * Retrieves the lookaside cache URL.
 *
 * Returns: A string containing the URL to the lookaside cache.
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_rpm_peek_cache (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_dup_cache:
 *
 * Retrieves a copy of the lookaside cache URL.
 *
 * Returns: A copy of the string containing the URL to the lookaside cache.
 *
 * Since: 1.1
 */
gchar *
modulemd_component_rpm_dup_cache (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_set_multilib:
 * @multilib: (nullable): a #ModulemdSimpleSet: A set of architectures on which
 * this RPM package should be available as multilib. An empty set means the
 * package is not available as multilib on any architecture.
 *
 * Since: 1.0
 */
void
modulemd_component_rpm_set_multilib (ModulemdComponentRpm *self,
                                     ModulemdSimpleSet *multilib);


/**
 * modulemd_component_rpm_get_multilib:
 *
 * Retrieves the set of multilib for this component.
 *
 * Returns: (transfer none): A #ModulemdSimpleSet containing the set of
 * supported multilib architectures for this component.
 *
 * Deprecated: 1.1
 * Use peek_multilib() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_component_rpm_peek_multilib)
ModulemdSimpleSet *
modulemd_component_rpm_get_multilib (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_peek_multilib:
 *
 * Retrieves the set of multilib for this component.
 *
 * Returns: (transfer none): A #ModulemdSimpleSet containing the set of
 * supported multilib architectures for this component.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_component_rpm_peek_multilib (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_dup_multilib:
 *
 * Retrieves a copy of the set of multilib for this component.
 *
 * Returns: (transfer full): A #ModulemdSimpleSet containing the set of
 * supported multilib architectures for this component.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_component_rpm_dup_multilib (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_set_ref
 * @ref: (nullable): A string: The particular repository commit hash, branch or tag name
 * used in this module.
 *
 * Since: 1.0
 */
void
modulemd_component_rpm_set_ref (ModulemdComponentRpm *self, const gchar *ref);


/**
 * modulemd_component_rpm_get_ref:
 *
 * Retrieves the repository ref.
 *
 * Returns: A string containing the repository ref.
 *
 * Deprecated: 1.1
 * Use peek_ref() instead.
 */
MMD_DEPRECATED_FOR (modulemd_component_rpm_peek_ref)
const gchar *
modulemd_component_rpm_get_ref (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_peek_ref:
 *
 * Retrieves the repository ref.
 *
 * Returns: A string containing the repository ref.
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_rpm_peek_ref (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_dup_ref:
 *
 * Retrieves a copy of the repository ref.
 *
 * Returns: A copy of the string containing the repository ref.
 *
 * Since: 1.1
 */
gchar *
modulemd_component_rpm_dup_ref (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_set_repository
 * @repository: (nullable): A string: The VCS repository with the RPM SPEC file, patches and other
 * package data.
 *
 * Since: 1.0
 */
void
modulemd_component_rpm_set_repository (ModulemdComponentRpm *self,
                                       const gchar *repository);


/**
 * modulemd_component_rpm_get_repository:
 *
 * Retrieves the repository location.
 *
 * Returns: A string containing the repository location.
 *
 * Deprecated: 1.1
 * Use peek_repository() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_component_rpm_peek_repository)
const gchar *
modulemd_component_rpm_get_repository (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_peek_repository:
 *
 * Retrieves the repository location.
 *
 * Returns: A string containing the repository location.
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_rpm_peek_repository (ModulemdComponentRpm *self);


/**
 * modulemd_component_rpm_dup_repository:
 *
 * Retrieves a copy of the repository location.
 *
 * Returns: A copy of the string containing the repository location.
 *
 * Since: 1.1
 */
gchar *
modulemd_component_rpm_dup_repository (ModulemdComponentRpm *self);

G_END_DECLS

#endif /* MODULEMD_COMPONENT_RPM_H */
