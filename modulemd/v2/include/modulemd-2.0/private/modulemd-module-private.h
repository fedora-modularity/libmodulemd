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
#include <yaml.h>

#include "modulemd-module.h"
#include "modulemd-translation.h"

/**
 * SECTION: modulemd-module-private
 * @title: Modulemd.Module (Private)
 * @stability: Private
 * @short_description: #ModulemdModule methods that should be used only
 * by internal consumers.
 */


/**
 * modulemd_module_new:
 * @module_name: (in) (not nullable): The name of the module
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModule
 *
 * Since: 2.0
 */
ModulemdModule *
modulemd_module_new (const gchar *module_name);


/**
 * modulemd_module_set_defaults:
 * @self: This #ModulemdModule object
 * @defaults: A #ModulemdDefaults object to associate with this #ModulemdModule. If the
 * module_name in the #ModulemdDefaults object does not match this module, it will be silently
 * ignored.
 *
 * Since: 2.0
 */
void
modulemd_module_set_defaults (ModulemdModule *self,
                              ModulemdDefaults *defaults);


/**
 * modulemd_module_associate_translation:
 * @self: This #ModulemdModule object
 * @translation: (in): A #ModulemdTranslation object which is copied into the
 * #ModulemdModule object.
 *
 * Since: 2.0
 */
void
modulemd_module_add_translation (ModulemdModule *self,
                                 ModulemdTranslation *translation);


/**
 * modulemd_module_get_translated_streams:
 * @self: This #ModulemdModule object
 *
 * Returns: (transfer container): A list of streams for which translations have
 * been added, sorted by stream name.
 *
 * Since: 2.0
 */
GPtrArray *
modulemd_module_get_translated_streams (ModulemdModule *self);


/**
 * modulemd_module_get_translations:
 * @self: This #ModulemdModule object
 * @stream: The stream to look up translations for.
 *
 * Returns: (transfer none): The set of translations attached to streams.
 *
 * Since: 2.0
 */
ModulemdTranslation *
modulemd_module_get_translation (ModulemdModule *self, const gchar *stream);


/**
 * modulemd_module_add_stream:
 * @self: This #ModulemdModule object
 * @stream: A #ModulemdModuleStream object to associate with this #ModulemdModule.
 *
 * Since: 2.0
 */
void
modulemd_module_add_stream (ModulemdModule *self,
                            ModulemdModuleStream *stream);
