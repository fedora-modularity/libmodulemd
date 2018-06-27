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

#ifndef MODULEMD_COMPONENT_MODULE_H
#define MODULEMD_COMPONENT_MODULE_H

#include "modulemd.h"
#include "modulemd-component.h"
#include "modulemd-simpleset.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_COMPONENT_MODULE (modulemd_component_module_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdComponentModule,
                      modulemd_component_module,
                      MODULEMD,
                      COMPONENT_MODULE,
                      ModulemdComponent)

ModulemdComponentModule *
modulemd_component_module_new (void);

void
modulemd_component_module_set_ref (ModulemdComponentModule *self,
                                   const gchar *ref);

MMD_DEPRECATED_FOR (modulemd_component_module_peek_ref)
const gchar *
modulemd_component_module_get_ref (ModulemdComponentModule *self);

const gchar *
modulemd_component_module_peek_ref (ModulemdComponentModule *self);
gchar *
modulemd_component_module_dup_ref (ModulemdComponentModule *self);

void
modulemd_component_module_set_repository (ModulemdComponentModule *self,
                                          const gchar *repository);

MMD_DEPRECATED_FOR (modulemd_component_module_peek_repository)
const gchar *
modulemd_component_module_get_repository (ModulemdComponentModule *self);

const gchar *
modulemd_component_module_peek_repository (ModulemdComponentModule *self);
gchar *
modulemd_component_module_dup_repository (ModulemdComponentModule *self);

G_END_DECLS

#endif /* MODULEMD_COMPONENT_MODULE_H */
