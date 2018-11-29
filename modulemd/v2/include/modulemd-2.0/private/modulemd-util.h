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

#pragma once

#include <glib.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-util
 * @title: Modulemd Utility Functions
 * @stability: private
 * @short_description: Provides private utility functions for use within
 * libmodulemd.
 */


#define MODULEMD_ERROR modulemd_error_quark ()
GQuark
modulemd_error_quark (void);

enum ModulemdError
{
  MODULEMD_ERROR_UPGRADE,
  MODULEMD_ERROR_VALIDATE,
  MODULEMD_ERROR_FILE_ACCESS
};

typedef struct _modulemd_tracer
{
  gchar *function_name;
} modulemd_tracer;

modulemd_tracer *
modulemd_trace_init (const gchar *function_name);


void
modulemd_trace_free (modulemd_tracer *tracer);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (modulemd_tracer, modulemd_trace_free);

#define MODULEMD_INIT_TRACE()                                                 \
  g_autoptr (modulemd_tracer) tracer = modulemd_trace_init (__func__);        \
  do                                                                          \
    {                                                                         \
      (void)(tracer);                                                         \
    }                                                                         \
  while (0)

G_END_DECLS


GHashTable *
modulemd_hash_table_deep_str_copy (GHashTable *orig);

GHashTable *
modulemd_hash_table_deep_set_copy (GHashTable *orig);

GHashTable *
modulemd_hash_table_deep_str_set_copy (GHashTable *orig);

gint
modulemd_strcmp_sort (gconstpointer a, gconstpointer b);

GPtrArray *
modulemd_ordered_str_keys (GHashTable *htable, GCompareFunc compare_func);

gchar **
modulemd_ordered_str_keys_as_strv (GHashTable *htable);

GVariant *
modulemd_variant_deep_copy (GVariant *variant);

void
modulemd_hash_table_unref (void *table);
