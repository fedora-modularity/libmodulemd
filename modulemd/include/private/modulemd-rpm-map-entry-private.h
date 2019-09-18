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

#include <glib.h>
#include <yaml.h>

/**
 * SECTION: modulemd-rpm-map-entry-private
 * @title: Modulemd.RpmMapEntry (Private)
 * @stability: Private
 * @short_description: #ModulemdRpmMapEntry methods that should be used only
 * by internal consumers.
 */

/**
 * modulemd_rpm_map_entry_parse_yaml
 * @parser: (inout): A libyaml parser object positioned at the beginning of an
 * RpmMapEntry's mapping entry in the YAML document.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for parsing error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdComponentRpm object
 * read from the YAML. NULL if a parse error occurred and sets @error
 * appropriately.
 * Since: 2.2
 */
ModulemdRpmMapEntry *
modulemd_rpm_map_entry_parse_yaml (yaml_parser_t *parser,
                                   gboolean strict,
                                   GError **error);


/**
 * modulemd_rpm_map_entry_emit_yaml:
 * @self: This #ModulemdRpmMapEntry object.
 * @emitter: (inout): A libyaml emitter object positioned where a
 * #ModulemdRpmMapEntry belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Return: TRUE if the entry was emitted successfully. FALSE and sets @error
 * appropriately if the YAML could not be emitted.
 *
 * Since: 2.2
 */
gboolean
modulemd_rpm_map_entry_emit_yaml (ModulemdRpmMapEntry *self,
                                  yaml_emitter_t *emitter,
                                  GError **error);


/**
 * modulemd_rpm_map_entry_equals_wrapper:
 * @a: A const void pointer.
 * @b: A const void pointer.
 *
 * Returns: TRUE if the two entries are both pointers to #ModulemdRpmMapEntry
 * objects containing equivalent data. FALSE if they differ.
 *
 * Since: 2.5
 */
gboolean
modulemd_rpm_map_entry_equals_wrapper (const void *a, const void *b);


/**
 * modulemd_RpmMapEntry_hash_table_equals_wrapper:
 * @a: A const void pointer.
 * @b: A const void pointer.
 *
 * Returns: TRUE if the two entries are equivalent #GHashTable<!-- -->s of
 * pointers to #ModulemdRpmMapEntry objects containing equivalent data. FALSE
 * if they differ.
 *
 * Since: 2.5
 */
gboolean
modulemd_RpmMapEntry_hash_table_equals_wrapper (const void *a, const void *b);
