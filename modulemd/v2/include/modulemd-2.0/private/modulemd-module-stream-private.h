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

#include <glib-object.h>
#include "modulemd-module-stream.h"
#include "modulemd-translation.h"
#include "modulemd-translation-entry.h"
#include "private/modulemd-yaml.h"

G_BEGIN_DECLS


/**
 * SECTION: modulemd-module-stream-private
 * @title: Modulemd.ModuleStream (Private)
 * @stability: private
 * @short_description: #ModulemdModuleStream methods that should only be used
 * by internal consumers.
 */


#define MODULESTREAM_PLACEHOLDER "__MODULESTREAM_PLACEHOLDER__"


void
modulemd_module_stream_set_module_name (ModulemdModuleStream *self,
                                        const gchar *module_name);

void
modulemd_module_stream_set_stream_name (ModulemdModuleStream *self,
                                        const gchar *stream_name);

void
modulemd_module_stream_associate_translation (
  ModulemdModuleStream *self, ModulemdTranslation *translation);

ModulemdTranslation *
modulemd_module_stream_get_translation (ModulemdModuleStream *self);

ModulemdTranslationEntry *
modulemd_module_stream_get_translation_entry (ModulemdModuleStream *self,
                                              const gchar *locale);


/* Some macros used for copy operations */
#define STREAM_COPY_IF_SET(version, dest, src, property)                      \
  do                                                                          \
    {                                                                         \
      if (modulemd_module_stream_##version##_get_##property (src) != NULL)    \
        modulemd_module_stream_##version##_set_##property (                   \
          dest, modulemd_module_stream_##version##_get_##property (src));     \
    }                                                                         \
  while (0)

#define STREAM_COPY_IF_SET_WITH_LOCALE(version, dest, src, property)          \
  do                                                                          \
    {                                                                         \
      if (modulemd_module_stream_##version##_get_##property (src, "C") !=     \
          NULL)                                                               \
        modulemd_module_stream_##version##_set_##property (                   \
          dest,                                                               \
          modulemd_module_stream_##version##_get_##property (src, "C"));      \
    }                                                                         \
  while (0)

#define STREAM_REPLACE_HASHTABLE(version, dest, src, property)                \
  do                                                                          \
    {                                                                         \
      modulemd_module_stream_##version##_replace_##property (dest,            \
                                                             src->property);  \
    }                                                                         \
  while (0)

#define COPY_HASHTABLE_BY_VALUE_ADDER(dest, src, property, adder)             \
  do                                                                          \
    {                                                                         \
      GHashTableIter iter;                                                    \
      gpointer value;                                                         \
      g_hash_table_iter_init (&iter, src->property);                          \
      while (g_hash_table_iter_next (&iter, NULL, &value))                    \
        {                                                                     \
          adder (dest, value);                                                \
        }                                                                     \
    }                                                                         \
  while (0)

gboolean
modulemd_module_stream_emit_yaml_base (ModulemdModuleStream *self,
                                       yaml_emitter_t *emitter,
                                       GError **error);

G_END_DECLS
