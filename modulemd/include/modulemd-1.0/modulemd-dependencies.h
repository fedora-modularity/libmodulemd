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

#ifndef MODULEMD_DEPENDENCIES_H
#define MODULEMD_DEPENDENCIES_H

#include "modulemd.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_DEPENDENCIES (modulemd_dependencies_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdDependencies, modulemd_dependencies, MODULEMD, DEPENDENCIES, GObject)

ModulemdDependencies *
modulemd_dependencies_new (void);

void
modulemd_dependencies_add_buildrequires (ModulemdDependencies *self,
                                         const gchar *module,
                                         const gchar **streams);

void
modulemd_dependencies_add_buildrequires_single (ModulemdDependencies *self,
                                                const gchar *module,
                                                const gchar *stream);

void
modulemd_dependencies_set_buildrequires (ModulemdDependencies *self,
                                         GHashTable *buildrequires);

MMD_DEPRECATED_FOR (modulemd_dependencies_peek_buildrequires)
GHashTable *
modulemd_dependencies_get_buildrequires (ModulemdDependencies *self);

GHashTable *
modulemd_dependencies_peek_buildrequires (ModulemdDependencies *self);
GHashTable *
modulemd_dependencies_dup_buildrequires (ModulemdDependencies *self);

void
modulemd_dependencies_add_requires (ModulemdDependencies *self,
                                    const gchar *module,
                                    const gchar **streams);

void
modulemd_dependencies_add_requires_single (ModulemdDependencies *self,
                                           const gchar *module,
                                           const gchar *stream);

void
modulemd_dependencies_set_requires (ModulemdDependencies *self,
                                    GHashTable *requires);

MMD_DEPRECATED_FOR (modulemd_dependencies_peek_requires)
GHashTable *
modulemd_dependencies_get_requires (ModulemdDependencies *self);

GHashTable *
modulemd_dependencies_peek_requires (ModulemdDependencies *self);

GHashTable *
modulemd_dependencies_dup_requires (ModulemdDependencies *self);

void
modulemd_dependencies_copy (ModulemdDependencies *self,
                            ModulemdDependencies **dest);

G_END_DECLS

#endif /* MODULEMD_DEPENDENCIES_H */
