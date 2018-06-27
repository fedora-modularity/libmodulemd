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

#include <glib-object.h>
#include "modulemd-component.h"
#include "modulemd-simpleset.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_COMPONENT_RPM (modulemd_component_rpm_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdComponentRpm,
                      modulemd_component_rpm,
                      MODULEMD,
                      COMPONENT_RPM,
                      ModulemdComponent)

ModulemdComponentRpm *
modulemd_component_rpm_new (void);

void
modulemd_component_rpm_set_arches (ModulemdComponentRpm *self,
                                   ModulemdSimpleSet *arches);
ModulemdSimpleSet *
modulemd_component_rpm_get_arches (ModulemdComponentRpm *self);
ModulemdSimpleSet *
modulemd_component_rpm_peek_arches (ModulemdComponentRpm *self);
ModulemdSimpleSet *
modulemd_component_rpm_dup_arches (ModulemdComponentRpm *self);

void
modulemd_component_rpm_set_cache (ModulemdComponentRpm *self,
                                  const gchar *cache);
const gchar *
modulemd_component_rpm_get_cache (ModulemdComponentRpm *self);
const gchar *
modulemd_component_rpm_peek_cache (ModulemdComponentRpm *self);
gchar *
modulemd_component_rpm_dup_cache (ModulemdComponentRpm *self);

void
modulemd_component_rpm_set_multilib (ModulemdComponentRpm *self,
                                     ModulemdSimpleSet *multilib);
ModulemdSimpleSet *
modulemd_component_rpm_get_multilib (ModulemdComponentRpm *self);
ModulemdSimpleSet *
modulemd_component_rpm_peek_multilib (ModulemdComponentRpm *self);
ModulemdSimpleSet *
modulemd_component_rpm_dup_multilib (ModulemdComponentRpm *self);

void
modulemd_component_rpm_set_ref (ModulemdComponentRpm *self, const gchar *ref);
const gchar *
modulemd_component_rpm_get_ref (ModulemdComponentRpm *self);
const gchar *
modulemd_component_rpm_peek_ref (ModulemdComponentRpm *self);
gchar *
modulemd_component_rpm_dup_ref (ModulemdComponentRpm *self);

void
modulemd_component_rpm_set_repository (ModulemdComponentRpm *self,
                                       const gchar *repository);
const gchar *
modulemd_component_rpm_get_repository (ModulemdComponentRpm *self);
const gchar *
modulemd_component_rpm_peek_repository (ModulemdComponentRpm *self);
gchar *
modulemd_component_rpm_dup_repository (ModulemdComponentRpm *self);

G_END_DECLS

#endif /* MODULEMD_COMPONENT_RPM_H */
