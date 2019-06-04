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
#include "modulemd-module-index.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-module-index-private
 * @title: Modulemd.ModuleIndex (Private)
 * @stability: Private
 * @short_description: #ModulemdModuleIndex methods that should be used only
 * by internal consumers.
 */


/**
 * modulemd_module_index_update_from_parser:
 * @self: (in): This #ModulemdModuleIndex object
 * @parser: (inout): An initialized YAML parser that has not yet processed any
 * events.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @autogen_module_name: (in): When parsing a module stream that contains no
 * module name or stream name, whether to autogenerate one or not. This option
 * should be used only for validation tools such as modulemd-validator. Normal
 * public routines should always set this to FALSE.
 * @failures: (out) (element-type ModulemdSubdocumentInfo) (transfer container):
 * An array containing any subdocuments from the YAML file that failed to parse.
 * See #ModulemdSubdocumentInfo for more details. If the array is NULL, it will
 * be allocated by this function. If it is non-NULL, this function will append
 * to it.
 * @error: (out): A GError containing additional information if this function
 * fails in a way that prevents program continuation.
 *
 * Returns: TRUE if the update was successful. Returns FALSE and sets failures
 * approriately if any of the YAML subdocuments were invalid or sets @error if
 * there was a fatal parse error.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_index_update_from_parser (ModulemdModuleIndex *self,
                                          yaml_parser_t *parser,
                                          gboolean strict,
                                          gboolean autogen_module_name,
                                          GPtrArray **failures,
                                          GError **error);


/**
 * modulemd_module_index_merge:
 * @from: (in) (transfer none): The #ModulemdModuleIndex whose contents are
 * being merged in.
 * @into: (inout) (transfer none): The #ModulemdModuleIndex whose contents are
 * being merged updated by those from @from.
 * @override: (in): In the event that the contents cannot be merged, this
 * argument specifies whether the contents of @from will supersede those from
 * @into. For specifics of how this works, see the Description section for
 * #ModulemdModuleIndexMerger.
 * @error: (out): If the merge fails, this will return a #GError explaining the
 * reason for it.
 *
 * Returns: TRUE if the two #ModulemdModuleIndex objects could be merged
 * without conflicts. FALSE and sets @error appropriately if the merge fails.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_index_merge (ModulemdModuleIndex *from,
                             ModulemdModuleIndex *into,
                             gboolean override,
                             GError **error);

G_END_DECLS
