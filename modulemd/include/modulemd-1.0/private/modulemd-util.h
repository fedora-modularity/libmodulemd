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
#include "glib.h"

G_BEGIN_DECLS

#define MODULEMD_ERROR modulemd_error_quark ()
GQuark
modulemd_error_quark (void);

enum ModulemdError
{
  MODULEMD_ERROR_PROGRAMMING
};

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


typedef struct _modulemd_tracer
{
  gchar *function_name;
} modulemd_tracer;

modulemd_tracer *
modulemd_trace_init (const gchar *function_name);

void
modulemd_trace_free (modulemd_tracer *tracer);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (modulemd_tracer, modulemd_trace_free);

#define MODULEMD_INIT_TRACE                                                   \
  g_autoptr (modulemd_tracer) tracer = modulemd_trace_init (__func__);        \
  do                                                                          \
    {                                                                         \
      (void)(tracer);                                                         \
    }                                                                         \
  while (0);


GPtrArray *
_modulemd_index_serialize (GHashTable *index, GError **error);


ModulemdTranslationEntry *
_get_locale_entry (ModulemdTranslation *translation, const gchar *_locale);


GHashTable *
module_index_from_data (GPtrArray *data, GError **error);


GPtrArray *
convert_modulestream_to_module (GPtrArray *objects);


G_END_DECLS

#endif /* MODULEMD_UTIL_H */
