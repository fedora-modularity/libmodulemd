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

#include "modulemd-2.0/modulemd-packager-v3.h"
#include <glib-object.h>
#include <yaml.h>


G_BEGIN_DECLS

/**
 * SECTION: modulemd-packager-v3-private
 * @title: Modulemd.PackagerV3 (Private)
 * @stability: Private
 * @short_description: #ModulemdPackagerV3 methods that should only be used by
 * internal consumers.
 */


/**
 * modulemd_packager_v3_to_defaults:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @defaults_ptr: (out): (transfer-full): A pointer to a pointer to a new
 * #ModulemdDefaults object. Must be a valid pointer to a NULL object when
 * called.
 * @error: (out): A #GError that will return the reason for a conversion error.
 *
 * Sets @defaults_ptr to point to a newly-allocated #ModulemdDefaults object
 * corresponding to the #ModulemdPackagerV3 object @self if @self contains any
 * profiles marked as default. Leaves @defaults_ptr pointing to NULL if @self
 * contained no default profiles.
 *
 * Returns: TRUE if the conversion succeeded, including the case where there
 * @self contains no default profiles. FALSE otherwise and @error will be set.
 *
 * Since: 2.11
 */
gboolean
modulemd_packager_v3_to_defaults (ModulemdPackagerV3 *self,
                                  ModulemdDefaults **defaults_ptr,
                                  GError **error);


/**
 * modulemd_packager_v3_to_stream_v2:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @error: (out): A #GError that will return the reason for a conversion error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStreamV2 object
 * corresponding to the #ModulemdPackagerV3 object @self. NULL if there was an
 * error doing the mapping and sets @error appropriately.
 *
 * Since: 2.11
 */
ModulemdModuleStreamV2 *
modulemd_packager_v3_to_stream_v2 (ModulemdPackagerV3 *self, GError **error);

/**
 * modulemd_packager_v3_to_stream_v2_ext:
 * @self: (in): This #ModulemdPackagerV3 object.
 * @error: (out): A #GError that will return the reason for a conversion error.
 *
 * Note: If buildopts (#ModulemdBuildopts) are in use in one or more build
 * configurations in the #ModulemdPackagerV3 object @self, only the buildopts
 * present in the first listed configuration (if any) will be applied to the
 * #ModulemdModuleStreamV2 object in the returned index.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleIndex object
 * containing a #ModulemdModuleStreamV2 object and possibly a
 * #ModulemdDefaults object corresponding to the #ModulemdPackagerV3 object
 * @self. NULL if there was an error doing the mapping and sets @error
 * appropriately.
 *
 * Since: 2.11
 */
ModulemdModuleIndex *
modulemd_packager_v3_to_stream_v2_ext (ModulemdPackagerV3 *self,
                                       GError **error);


/**
 * modulemd_packager_v3_parse_yaml:
 * @subdoc: (in): A #ModulemdSubdocumentInfo representing a packager v3
 * document.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Parse a #ModulemdPackagerV3 document. This parser always operates in strict
 * mode, since it should only be used as input for a build-system.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdPackagerV3 object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.11
 */
ModulemdPackagerV3 *
modulemd_packager_v3_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                 GError **error);


/**
 * modulemd_packager_v3_emit_yaml:
 * @self: This #ModulemdPackagerV3 object.
 * @emitter: (inout): A libyaml emitter object positioned where the data
 * section of a #ModulemdPackagerV3 belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the modulemd-packager v3 document was emitted successfully.
 * FALSE and sets @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.11
 */
gboolean
modulemd_packager_v3_emit_yaml (ModulemdPackagerV3 *self,
                                yaml_emitter_t *emitter,
                                GError **error);


G_END_DECLS
