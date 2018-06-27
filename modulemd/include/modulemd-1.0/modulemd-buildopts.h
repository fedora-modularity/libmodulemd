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

#pragma once

#include "modulemd.h"
#include "modulemd-simpleset.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_BUILDOPTS (modulemd_buildopts_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdBuildopts, modulemd_buildopts, MODULEMD, BUILDOPTS, GObject)

ModulemdBuildopts *
modulemd_buildopts_new (void);

void
modulemd_buildopts_set_rpm_macros (ModulemdBuildopts *self,
                                   const gchar *macros);

gchar *
modulemd_buildopts_get_rpm_macros (ModulemdBuildopts *self);

void
modulemd_buildopts_set_rpm_whitelist (ModulemdBuildopts *self,
                                      GStrv whitelist);

void
modulemd_buildopts_set_rpm_whitelist_simpleset (ModulemdBuildopts *self,
                                                ModulemdSimpleSet *whitelist);

GStrv
modulemd_buildopts_get_rpm_whitelist (ModulemdBuildopts *self);

ModulemdSimpleSet *
modulemd_buildopts_get_rpm_whitelist_simpleset (ModulemdBuildopts *self);

ModulemdBuildopts *
modulemd_buildopts_copy (ModulemdBuildopts *self);

G_END_DECLS
