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

#ifndef MODULEMD_UTIL_H
#define MODULEMD_UTIL_H

#include "modulemd.h"

G_BEGIN_DECLS

GHashTable *
_modulemd_hash_table_deep_str_copy (GHashTable *orig);

GHashTable *
_modulemd_hash_table_deep_obj_copy (GHashTable *orig);

GHashTable *
_modulemd_hash_table_deep_variant_copy (GHashTable *orig);

gint
_modulemd_strcmp_sort (gconstpointer a, gconstpointer b);

GPtrArray *
_modulemd_ordered_str_keys (GHashTable *htable, GCompareFunc compare_func);

GList *
_modulemd_ordered_int64_keys (GHashTable *htable);

void
modulemd_variant_unref (void *ptr);

gboolean
modulemd_validate_nevra (const gchar *nevra);

G_END_DECLS

#endif /* MODULEMD_UTIL_H */
