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
#include "modulemd-obsoletes.h"


G_BEGIN_DECLS

/**
 * SECTION: modulemd-module-private
 * @title: Modulemd.Module (Private)
 * @stability: Private
 * @short_description: #ModulemdModule methods that should be used only
 * by internal consumers.
 */


/**
 * modulemd_module_new:
 * @module_name: (in) (not nullable): The name of the module.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModule object.
 *
 * Since: 2.0
 */
ModulemdModule *
modulemd_module_new (const gchar *module_name);


/**
 * modulemd_module_set_defaults:
 * @self: (in): This #ModulemdModule object.
 * @defaults: (in): A #ModulemdDefaults object to associate with this
 * #ModulemdModule. If the module_name in the #ModulemdDefaults object does not
 * match this module, it will be rejected.
 * @index_mdversion: (in): The #ModulemdDefaultsVersionEnum of the highest
 * defaults version added so far in the #ModulemdModuleIndex. If non-zero,
 * perform an upgrade to this version while adding @defaults to @self. If
 * the @defaults already has a higher version, just copy it.
 * @error: (out): A #GError containing information about why this function
 * failed.
 *
 * This function takes a defaults object, upgrades it to @index_mdversion if it
 * is lower and adds it to the #ModulemdModule. If it cannot upgrade it safely
 * or the defaults are not for this module, it will return an appropriate
 * error.
 *
 * Returns: The mdversion of the defaults that were added. Returns
 * %MD_DEFAULTS_VERSION_ERROR and sets @error if the default name didn't match
 * or the Defaults object couldn't be upgraded successfully to
 * the @index_mdversion. Returns %MD_DEFAULTS_VERSION_UNSET if @defaults was
 * NULL.
 *
 * Since: 2.0
 */
ModulemdDefaultsVersionEnum
modulemd_module_set_defaults (ModulemdModule *self,
                              ModulemdDefaults *defaults,
                              ModulemdDefaultsVersionEnum index_mdversion,
                              GError **error);


/**
 * modulemd_module_add_translation:
 * @self: This #ModulemdModule object.
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
 * @self: This #ModulemdModule object.
 *
 * Returns: (transfer container): A list of streams for which translations have
 * been added, sorted by stream name.
 *
 * Since: 2.0
 */
GPtrArray *
modulemd_module_get_translated_streams (ModulemdModule *self);


/**
 * modulemd_module_add_obsoletes:
 * @self: This #ModulemdModule object.
 * @obsoletes: (in): A #ModulemdObsoletes object which is copied into the
 * #ModulemdModule object.
 *
 * This function copies the @obsoletes object into @self. In addition if @obsoletes is
 * the newest active obsoletes for existing #ModulemdModuleStream in @self, the stream is
 * upgraded to at least version two and @obsoletes is associated with it.
 *
 * Since: 2.10
 */
void
modulemd_module_add_obsoletes (ModulemdModule *self,
                               ModulemdObsoletes *obsoletes);


/**
 * modulemd_module_add_stream:
 * @self: This #ModulemdModule object.
 * @stream: A #ModulemdModuleStream object to associate with this
 * #ModulemdModule. A stream added to a #ModulemdModule must have a module
 * name and stream name set on it or it will be rejected. If the module name
 * does not match this module, it will also be rejected.
 * @index_mdversion: (in): The #ModulemdModuleStreamVersionEnum stream_mdversion
 * of the #ModulemdModuleIndex to which @stream is being added. If the version
 * of @stream is less than @index_mdversion, an upgrade to this version will be
 * performed while adding @stream to @self. If @stream already has the same
 * version, it is just copied. When obsoletes is present for @stream it must be
 * set to at least version two.
 * @error: (out): A #GError containing information about why this function
 * failed.
 *
 * This function takes a stream object, upgrades it to index_mdversion if
 * needed (if the module contains active obsoletes for this @stream it is upgraded
 * to at least version two) and then adds it to the #ModulemdModule. If it cannot upgrade it
 * safely or the defaults are not for this module, it will return an
 * appropriate error.
 *
 * Returns: The mdversion of the stream that was added, which will be
 * @index_mdversion unless an error occurred. Returns
 * %MD_MODULESTREAM_VERSION_ERROR and sets @error if the module name didn't
 * match, the module and stream names were unset or the stream object couldn't
 * be upgraded successfully to the @index_mdversion. Returns
 * %MD_MODULESTREAM_VERSION_UNSET if @stream was NULL.
 *
 * Since: 2.0
 */
ModulemdModuleStreamVersionEnum
modulemd_module_add_stream (ModulemdModule *self,
                            ModulemdModuleStream *stream,
                            ModulemdModuleStreamVersionEnum index_mdversion,
                            GError **error);


/**
 * modulemd_module_upgrade_streams:
 * @self: This #ModulemdModule object.
 * @mdversion: The metadata version to upgrade to.
 * @error: (out): A #GError containing the reason a stream failed to upgrade.
 *
 * Returns: TRUE if all upgrades completed successfully. FALSE and sets @error
 * if an upgrade error occurs, including attempts to downgrade a stream.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_upgrade_streams (ModulemdModule *self,
                                 ModulemdModuleStreamVersionEnum mdversion,
                                 GError **error);

G_END_DECLS
