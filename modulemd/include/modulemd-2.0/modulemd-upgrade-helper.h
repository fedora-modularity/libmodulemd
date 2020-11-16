/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat, Inc.
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
 * SECTION: modulemd-upgrade-helper
 * @title: Modulemd.UpgradeHelper
 * @stability: stable
 * @short_description: Helpers to provide cues to #ModulemdModuleStream
 * upgrades.
 */

#define MODULEMD_TYPE_UPGRADE_HELPER (modulemd_upgrade_helper_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdUpgradeHelper,
                      modulemd_upgrade_helper,
                      MODULEMD,
                      UPGRADE_HELPER,
                      GObject)


/**
 * modulemd_upgrade_helper_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdUpgradeHelper object.
 * This object must be freed with g_object_unref().
 *
 * Since: 2.10
 */
ModulemdUpgradeHelper *
modulemd_upgrade_helper_new (void);


/**
 * modulemd_upgrade_helper_add_known_stream:
 * @self: (in): This #ModulemdUpgradeHelper object.
 * @module_name: (in): The name of the known module being added.
 * @stream_name: (in): The name of the known module stream being added.
 *
 * This function adds a `module:stream` entry to the #ModulemdUpgradeHelper.
 * It will be used if and when libmodulemd needs to upgrade a
 * #ModulemdModuleStreamV2 object to a #ModulemdModuleStreamV3 object if it
 * encounters a module dependency that is specified as either `[ ]`
 * (all streams) or `[ -streamname ]` (all but some exclusions).
 *
 * When using the python bindings, a simpler way to set these values is to call
 * ```
 * helper = Modulemd.UpgradeHelper.new()
 * helper.set_known_streams({"module_name": ["stream1", "stream2"]}
 * ```
 *
 * Since: 2.10
 */
void
modulemd_upgrade_helper_add_known_stream (ModulemdUpgradeHelper *self,
                                          const gchar *module_name,
                                          const gchar *stream_name);


/**
 * modulemd_upgrade_helper_get_known_modules_as_strv: (rename-to modulemd_upgrade_helper_get_known_modules)
 * @self: (in): This #ModulemdUpgradeHelper
 *
 * Returns: (transfer full): A list of known modules to provide clues to the
 * stream upgrade process.
 *
 * Since: 2.10
 */
GStrv
modulemd_upgrade_helper_get_known_modules_as_strv (
  ModulemdUpgradeHelper *self);


/**
 * modulemd_upgrade_helper_get_known_streams_as_strv: (rename-to modulemd_upgrade_helper_get_known_streams)
 * @self: (in): This #ModulemdUpgradeHelper
 * @module_name: (in): The name of the module to return a list of known streams
 * for.
 *
 * Returns: (transfer full): A list of known streams to provide clues to the
 * stream upgrade process.
 *
 * Since: 2.10
 */
GStrv
modulemd_upgrade_helper_get_known_streams_as_strv (ModulemdUpgradeHelper *self,
                                                   const gchar *module_name);

G_END_DECLS
