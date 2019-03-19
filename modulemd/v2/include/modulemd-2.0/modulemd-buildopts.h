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

G_BEGIN_DECLS

/**
 * SECTION: modulemd-buildopts
 * @title: Modulemd.Buildopts
 * @stability: stable
 * @short_description: Provides hints to the build-system on how to build this module.
 */

#define MODULEMD_TYPE_BUILDOPTS (modulemd_buildopts_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdBuildopts, modulemd_buildopts, MODULEMD, BUILDOPTS, GObject)

/**
 * modulemd_buildopts_new:
 *
 * Returns: (transfer full): A newly-allocated #ModuleBuildopts. This object must be freed with g_object_unref().
 *
 * Since: 2.0
 */
ModulemdBuildopts *
modulemd_buildopts_new (void);


/**
 * modulemd_buildopts_equals:
 * @self_1: A #ModulemdBuildopts
 * @self_2: A #ModulemdBuildopts
 *
 * Check for equality for 2 ModulemdBuildopts objects.
 *
 * Returns: TRUE if both objects are equal, FALSE otherwise.
 *
 * Since: 2.2
 */
gboolean
modulemd_buildopts_equals (ModulemdBuildopts *self_1,
                           ModulemdBuildopts *self_2);


/**
 * modulemd_buildopts_copy:
 * @self: This #ModulemdBuildopts
 *
 * Create a copy of this #ModulemdBuildopts object.
 *
 * Returns: (transfer full): a copied #ModulemdBuildopts object
 *
 * Since: 2.0
 */
ModulemdBuildopts *
modulemd_buildopts_copy (ModulemdBuildopts *self);


/**
 * modulemd_buildopts_set_rpm_macros:
 * @self: This #ModulemdBuildopts
 * @rpm_macros: A string containing RPM build macros in the form that they would appear in an RPM macros file on-disk.
 *
 * Since: 2.0
 */
void
modulemd_buildopts_set_rpm_macros (ModulemdBuildopts *self,
                                   const gchar *rpm_macros);


/**
 * modulemd_buildopts_get_rpm_macros:
 * @self: This #ModulemdBuildopts
 *
 * Returns: (transfer none): A string containing RPM build macros in the form that they would appear in an RPM macros file on-disk.
 *
 * Since: 2.0
 */
const gchar *
modulemd_buildopts_get_rpm_macros (ModulemdBuildopts *self);


/**
 * modulemd_buildopts_add_rpm_to_whitelist:
 * @self: This #ModulemdBuildopts
 * @rpm: An RPM name to add to the whitelist.
 *
 * Since: 2.0
 */
void
modulemd_buildopts_add_rpm_to_whitelist (ModulemdBuildopts *self,
                                         const gchar *rpm);


/**
 * modulemd_buildopts_remove_rpm_from_whitelist:
 * @self: This #ModulemdBuildopts
 * @rpm: An RPM name to remove from the whitelist.
 *
 * Since: 2.0
 */
void
modulemd_buildopts_remove_rpm_from_whitelist (ModulemdBuildopts *self,
                                              const gchar *rpm);


/**
 * modulemd_buildopts_get_rpm_whitelist_as_strv: (rename-to modulemd_buildopts_get_rpm_whitelist)
 * @self: This #ModulemdBuildopts
 *
 * Returns: (transfer full): An ordered list of all RPMs in the whitelist.
 *
 * Since: 2.0
 */
gchar **
modulemd_buildopts_get_rpm_whitelist_as_strv (ModulemdBuildopts *self);

G_END_DECLS
