/* modulemd-util.h
 *
 * Copyright (C) 2017 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

G_END_DECLS

#endif /* MODULEMD_UTIL_H */
