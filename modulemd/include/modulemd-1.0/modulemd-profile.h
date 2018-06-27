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

#ifndef MODULEMD_PROFILE_H
#define MODULEMD_PROFILE_H

#include "modulemd.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_PROFILE modulemd_profile_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdProfile, modulemd_profile, MODULEMD, PROFILE, GObject)

ModulemdProfile *
modulemd_profile_new (void);

void
modulemd_profile_set_description (ModulemdProfile *self,
                                  const gchar *description);

MMD_DEPRECATED_FOR (modulemd_profile_peek_description)
const gchar *
modulemd_profile_get_description (ModulemdProfile *self);

const gchar *
modulemd_profile_peek_description (ModulemdProfile *self);
gchar *
modulemd_profile_dup_description (ModulemdProfile *self);

void
modulemd_profile_set_name (ModulemdProfile *self, const gchar *name);

MMD_DEPRECATED_FOR (modulemd_profile_peek_name)
const gchar *
modulemd_profile_get_name (ModulemdProfile *self);

const gchar *
modulemd_profile_peek_name (ModulemdProfile *self);
gchar *
modulemd_profile_dup_name (ModulemdProfile *self);

void
modulemd_profile_set_rpms (ModulemdProfile *self, ModulemdSimpleSet *rpms);

MMD_DEPRECATED_FOR (modulemd_profile_peek_rpms)
ModulemdSimpleSet *
modulemd_profile_get_rpms (ModulemdProfile *self);

ModulemdSimpleSet *
modulemd_profile_peek_rpms (ModulemdProfile *self);
ModulemdSimpleSet *
modulemd_profile_dup_rpms (ModulemdProfile *self);

void
modulemd_profile_add_rpm (ModulemdProfile *self, const gchar *rpm);

void
modulemd_profile_remove_rpm (ModulemdProfile *self, const gchar *rpm);

ModulemdProfile *
modulemd_profile_copy (ModulemdProfile *self);

G_END_DECLS

#endif /* MODULEMD_PROFILE_H */
