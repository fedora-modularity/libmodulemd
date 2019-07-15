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
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"

G_BEGIN_DECLS


/**
 * SECTION: modulemd-module-stream-private
 * @title: Modulemd.ModuleStream (Private)
 * @stability: private
 * @short_description: #ModulemdModuleStream methods that should only be used
 * by internal consumers.
 */


/**
 * modulemd_module_stream_set_module_name:
 * @self: (in): This #ModulemdModuleStream object.
 * @module_name: The module name this object represents.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_set_module_name (ModulemdModuleStream *self,
                                        const gchar *module_name);

/**
 * modulemd_module_stream_set_stream_name:
 * @self: (in): This #ModulemdModuleStream object.
 * @stream_name: The stream name this object represents.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_set_stream_name (ModulemdModuleStream *self,
                                        const gchar *stream_name);

/**
 * modulemd_module_stream_associate_translation:
 * @self: (in): This #ModulemdModuleStream object.
 * @translation: (in): The #ModulemdTranslation information to associate with
 * this object.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_associate_translation (
  ModulemdModuleStream *self, ModulemdTranslation *translation);

/**
 * modulemd_module_stream_get_translation:
 * @self: (in): This #ModulemdModuleStream object.
 *
 * Returns: The #ModulemdTranslation information to associated with this object.
 *
 * Since: 2.0
 */
ModulemdTranslation *
modulemd_module_stream_get_translation (ModulemdModuleStream *self);

/**
 * modulemd_module_stream_get_translation_entry:
 * @self: (in): This #ModulemdModuleStream object.
 * @locale: The locale of the translation to retrieve.
 *
 * Returns: (transfer none): The module stream #ModulemdTranslationEntry for
 * the requested locale, or NULL if the locale was unknown.
 *
 * Since: 2.0
 */
ModulemdTranslationEntry *
modulemd_module_stream_get_translation_entry (ModulemdModuleStream *self,
                                              const gchar *locale);

/**
 * modulemd_module_stream_validate_components:
 * @components: (in): A #GHashTable of #ModulemdComponent objects.
 * @error: (out): A #GError that will return the reason for a validation error.
 *
 * Verifies that each of the #ModulemdComponent objects in @components
 * validates correctly via modulemd_component_validate(), verifies that any
 * buildafter components are also present in @components, and verifies that
 * buildorder and buildafter aren't mixed.
 *
 * Returns: TRUE if the component objects passed validation. FALSE and sets
 * @error appropriately if validation fails.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_stream_validate_components (GHashTable *components,
                                            GError **error);


/* Some macros used for copy operations */
#define STREAM_COPY_IF_SET(version, dest, src, property)                      \
  do                                                                          \
    {                                                                         \
      if (modulemd_module_stream_##version##_get_##property (src) != NULL)    \
        modulemd_module_stream_##version##_set_##property (                   \
          dest, modulemd_module_stream_##version##_get_##property (src));     \
    }                                                                         \
  while (0)

#define STREAM_UPGRADE_IF_SET(oldversion, newversion, dest, src, property)    \
  do                                                                          \
    {                                                                         \
      if (modulemd_module_stream_##oldversion##_get_##property (src) != NULL) \
        modulemd_module_stream_##newversion##_set_##property (                \
          dest, modulemd_module_stream_##oldversion##_get_##property (src));  \
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

#define STREAM_UPGRADE_IF_SET_WITH_LOCALE(                                    \
  oldversion, newversion, dest, src, property)                                \
  do                                                                          \
    {                                                                         \
      if (modulemd_module_stream_##oldversion##_get_##property (src, "C") !=  \
          NULL)                                                               \
        modulemd_module_stream_##newversion##_set_##property (                \
          dest,                                                               \
          modulemd_module_stream_##oldversion##_get_##property (src, "C"));   \
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

/**
 * modulemd_module_stream_emit_yaml_base:
 * @self: This #ModulemdModuleStream object.
 * @emitter: (inout): A libyaml emitter object positioned where a Module Stream
 * document belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Emit the common non-version specific YAML components for the Module Stream
 * document.
 *
 * Returns: TRUE if the #ModulemdModuleStream components were emitted
 * successfully. FALSE and sets @error appropriately if the YAML could not be
 * emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_stream_emit_yaml_base (ModulemdModuleStream *self,
                                       yaml_emitter_t *emitter,
                                       GError **error);

G_END_DECLS
