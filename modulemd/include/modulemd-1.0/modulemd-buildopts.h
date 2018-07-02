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

/**
 * SECTION: modulemd-buildopts
 * @title: Modulemd.Buildopts
 * @short_description: Provides hints to the build-system on how to build this
 * module.
 */

#define MODULEMD_TYPE_BUILDOPTS (modulemd_buildopts_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdBuildopts, modulemd_buildopts, MODULEMD, BUILDOPTS, GObject)

/**
 * modulemd_buildopts_new:
 *
 * Returns: (transfer full): a newly-allocated #ModulemdBuildopts. This object
 * must be freed with g_object_unref().
 *
 * Since: 1.5
 */
ModulemdBuildopts *
modulemd_buildopts_new (void);


/**
 * modulemd_buildopts_set_rpm_macros:
 * @macros: (nullable): A string containing RPM build macros in the form that
 * they would appear in an RPM macros file on-disk.
 *
 * Assigns RPM macros for the build-system.
 *
 * Since: 1.5
 */
void
modulemd_buildopts_set_rpm_macros (ModulemdBuildopts *self,
                                   const gchar *macros);


/**
 * modulemd_buildopts_get_rpm_macros:
 *
 * Retrieves a copy of the string containing RPM build macros in the form that
 * they would appear in an RPM macros file on-disk.
 *
 * Returns: a copy of the string containing RPM build macros in the form that
 * they would appear in an RPM macros file on-disk. The caller must free the
 * returned string with g_free() once finished with it. This function may
 * return NULL if no RPM macros have been set.
 *
 * Since: 1.5
 */
gchar *
modulemd_buildopts_get_rpm_macros (ModulemdBuildopts *self);


/**
 * modulemd_buildopts_set_rpm_whitelist:
 * @whitelist: (array zero-terminated=1) (transfer none): The set of RPM names
 * for the whitelist.
 *
 * This will make a copy of all of the unique items in @whitelist.
 *
 * Since: 1.5
 */
void
modulemd_buildopts_set_rpm_whitelist (ModulemdBuildopts *self,
                                      GStrv whitelist);


/**
 * modulemd_buildopts_set_rpm_whitelist_simpleset:
 * @whitelist: (transfer none): The #ModulemdSimpleSet set of RPM names
 * for the whitelist.
 *
 * This will make a copy of all of the unique items in @whitelist.
 *
 * Since: 1.5
 */
void
modulemd_buildopts_set_rpm_whitelist_simpleset (ModulemdBuildopts *self,
                                                ModulemdSimpleSet *whitelist);


/**
 * modulemd_buildopts_get_rpm_whitelist:
 *
 * Returns a copy of the whitelist.
 *
 * Returns: (array zero-terminated=1) (transfer full): The set of RPM names
 * for the whitelist. May return NULL if no whitelist is stored.
 *
 * Since: 1.5
 */
GStrv
modulemd_buildopts_get_rpm_whitelist (ModulemdBuildopts *self);


/**
 * modulemd_buildopts_get_rpm_whitelist_simpleset:
 *
 * Returns a copy of the whitelist as a #ModulemdSimpleset
 *
 * Returns: (transfer full): The #ModulemdSimpleSet of RPM names
 * for the whitelist. May return NULL if no whitelist is stored.
 *
 * Since: 1.5
 */
ModulemdSimpleSet *
modulemd_buildopts_get_rpm_whitelist_simpleset (ModulemdBuildopts *self);


/**
 * modulemd_buildopts_copy:
 *
 * Make a deep copy of this #ModulemdBuildopts object.
 *
 * Returns: (transfer full): A deep copy of this #ModulemdBuildopts object.
 * This value must be freed with g_object_unref().
 *
 * Since: 1.5
 */
ModulemdBuildopts *
modulemd_buildopts_copy (ModulemdBuildopts *self);

G_END_DECLS
