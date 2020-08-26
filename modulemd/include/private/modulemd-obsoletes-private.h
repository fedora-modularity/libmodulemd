/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat
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

#include "modulemd-obsoletes.h"
#include "modulemd-subdocument-info.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-obsoletes-private
 * @title: Modulemd.Obsoletes (Private)
 * @stability: private
 * @short_description: #ModulemdObsoletes methods that should only be used by
 * internal consumers.
 */

/**
 * modulemd_obsoletes_get_mdversion:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: The metadata mdversion of this #ModulemdObsoletes object.
 *
 * Since: 2.10
 */
guint64
modulemd_obsoletes_get_mdversion (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_get_module_name:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: The module name to which this #ModulemdObsoletes object applies.
 *
 * Since: 2.10
 */
const gchar *
modulemd_obsoletes_get_module_name (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_get_module_stream:
 * @self: (in): This #ModulemdObsoletes object.
 *
 * Returns: The stream name to which this #ModulemdObsoletes object applies.
 *
 * Since: 2.10
 */
const gchar *
modulemd_obsoletes_get_module_stream (ModulemdObsoletes *self);

/**
 * modulemd_obsoletes_set_message:
 * @self: This #ModulemdObsoletes object.
 * @message: (in): A string describing the change, reason, etc.
 *
 * Since: 2.10
 */
void
modulemd_obsoletes_set_message (ModulemdObsoletes *self, const gchar *message);

/**
 * modulemd_obsoletes_set_obsoleted_by_module_name:
 * @self: This #ModulemdObsoletes object.
 * @obsoleted_by_module_name: (in): The module name of obsoleting stream.
 *
 * Has to be set together with obsoleted by module stream.
 *
 * Since: 2.10
 */
void
modulemd_obsoletes_set_obsoleted_by_module_name (
  ModulemdObsoletes *self, const gchar *obsoleted_by_module_name);

/**
 * modulemd_obsoletes_set_obsoleted_by_module_stream:
 * @self: This #ModulemdObsoletes object.
 * @obsoleted_by_module_stream: (in): The module stream of obsoleting stream.
 *
 * Has to be set together with obsoleted by module name.
 *
 * Since: 2.10
 */
void
modulemd_obsoletes_set_obsoleted_by_module_stream (
  ModulemdObsoletes *self, const gchar *obsoleted_by_module_stream);

/**
 * modulemd_obsoletes_parse_yaml:
 * @subdoc: (in): A #ModulemdSubdocumentInfo representing a obsoletes * document
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdObsoletes object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.10
 */
ModulemdObsoletes *
modulemd_obsoletes_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                               gboolean strict,
                               GError **error);

/**
 * modulemd_obsoletes_emit_yaml:
 * @self: (in): This #ModulemdObsoletes object.
 * @emitter: (inout): A libyaml emitter object positioned where obsoletes data
 * belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the obsoletes data was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.10
 */
gboolean
modulemd_obsoletes_emit_yaml (ModulemdObsoletes *self,
                              yaml_emitter_t *emitter,
                              GError **error);


G_END_DECLS
