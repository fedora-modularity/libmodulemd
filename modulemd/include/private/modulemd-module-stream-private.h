/*
 * This file is part of libmodulemd
 * Copyright (C) 2017-2020 Stephen Gallagher
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

#include "modulemd-module-index.h"
#include "modulemd-module-stream.h"
#include "modulemd-translation-entry.h"
#include "modulemd-translation.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-yaml.h"
#include <glib-object.h>

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


/**
 * modulemd_module_stream_validate_component_rpm_arches:
 * @components: (in): A #GHashTable of #ModulemdComponent objects.
 * @module_arches: (in): A #GStrv list of arches for which to build the module.
 * @error: (out): A #GError that will return the reason for a validation error.
 *
 * Verifies that for each of the #ModulemdComponent RPM objects in @components,
 * any arches specified must be a subset of @module_arches. If @module_arches
 * is empty, all arches are allowed.
 *
 * Returns: TRUE if the component objects passed validation. FALSE and sets
 * @error appropriately if validation fails.
 *
 * Since: 2.9
 */
gboolean
modulemd_module_stream_validate_component_rpm_arches (GHashTable *components,
                                                      GStrv module_arches,
                                                      GError **error);


/* Some macros used for copy operations */
/**
 * STREAM_UPGRADE_IF_SET_FULL:
 * @oldversion: The stream version of @src. Must be literal "v1" or "v2"
 * without the quotes.
 * @newversion: The stream version of @dest. Must be literal "v1" or "v2"
 * without the quotes.
 * @dest: (out): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object
 * that is the destination to which @property is to be copied.
 * @src: (in): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object that
 * is the source from which @property is to be copied.
 * @property: The name of the property to copy. Must be the literal property
 * name, in lower case, without quotes.
 * @locale...: (in): An optional locale that can be provided when @property has
 * possible translations.
 *
 * DIRECT USE OF THIS MACRO SHOULD BE AVOIDED. This is the internal
 * implementation for %STREAM_COPY_IF_SET, %STREAM_UPGRADE_IF_SET,
 * %STREAM_COPY_IF_SET_WITH_LOCALE, and %STREAM_UPGRADE_IF_SET_WITH_LOCALE
 * which should be used instead.
 *
 * This is a helper macro to simplify the coding when copying/upgrading
 * properties between #ModulemdModuleStreamV1 and #ModulemdModuleStreamV2
 * objects.
 *
 * Does nothing if the @src @property is NULL.
 *
 * Since: 2.0
 */
#define STREAM_UPGRADE_IF_SET_FULL(                                           \
  oldversion, newversion, dest, src, property, locale...)                     \
  do                                                                          \
    {                                                                         \
      if (modulemd_module_stream_##oldversion##_get_##property (              \
            src, ##locale) != NULL)                                           \
        modulemd_module_stream_##newversion##_set_##property (                \
          dest,                                                               \
          modulemd_module_stream_##oldversion##_get_##property (src,          \
                                                                ##locale));   \
    }                                                                         \
  while (0)

/**
 * STREAM_COPY_IF_SET:
 * @version: The stream version being copied. Must be literal "v1" or "v2"
 * without the quotes.
 * @dest: (out): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object
 * that is the destination to which @property is to be copied.
 * @src: (in): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object that
 * is the source from which @property is to be copied.
 * @property: The name of the property to copy. Must be the literal property
 * name, in lower case, without quotes.
 *
 * This is a convenience macro to simplify the coding when copying properties
 * between #ModulemdModuleStream objects when both @src and @dest are the same
 * version.
 *
 * Does nothing if the @src @property is NULL.
 *
 * Since: 2.0
 */
#define STREAM_COPY_IF_SET(version, dest, src, property)                      \
  STREAM_UPGRADE_IF_SET_FULL (version, version, dest, src, property)

/**
 * STREAM_UPGRADE_IF_SET:
 * @oldversion: The stream version of @src. Must be literal "v1" or "v2"
 * without the quotes.
 * @newversion: The stream version of @dest. Must be literal "v1" or "v2"
 * without the quotes.
 * @dest: (out): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object
 * that is the destination to which @property is to be copied.
 * @src: (in): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object that
 * is the source from which @property is to be copied.
 * @property: The name of the property to copy. Must be the literal property
 * name, in lower case, without quotes.
 *
 * This is a convenience macro to simplify the coding when copying properties
 * between #ModulemdModuleStreamV1 and #ModulemdModuleStreamV2 objects when
 * @src and @dest are different versions.
 *
 * Does nothing if the @src @property is NULL.
 *
 * Since: 2.0
 */
#define STREAM_UPGRADE_IF_SET(oldversion, newversion, dest, src, property)    \
  STREAM_UPGRADE_IF_SET_FULL (oldversion, newversion, dest, src, property)

/**
 * STREAM_COPY_IF_SET_WITH_LOCALE:
 * @version: The stream version being copied. Must be literal "v1" or "v2"
 * without the quotes.
 * @dest: (out): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object
 * that is the destination to which @property is to be copied.
 * @src: (in): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object that
 * is the source from which @property is to be copied.
 * @property: The name of the property to copy. Must be the literal property
 * name, in lower case, without quotes.
 *
 * This is a convenience macro to simplify the coding when copying properties
 * between #ModulemdModuleStreamV1 and #ModulemdModuleStreamV2 objects when
 * both @src and @dest are the same version and @property has possible
 * translations. Only the untranslated (`"C"` locale) version of @property will
 * be copied.
 *
 * Does nothing if the @src @property is NULL.
 *
 * Since: 2.0
 */
#define STREAM_COPY_IF_SET_WITH_LOCALE(version, dest, src, property)          \
  STREAM_UPGRADE_IF_SET_FULL (version, version, dest, src, property, "C")

/**
 * STREAM_UPGRADE_IF_SET_WITH_LOCALE:
 * @oldversion: The stream version of @src. Must be literal "v1" or "v2"
 * without the quotes.
 * @newversion: The stream version of @dest. Must be literal "v1" or "v2"
 * without the quotes.
 * @dest: (out): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object
 * that is the destination to which @property is to be copied.
 * @src: (in): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object that
 * is the source from which @property is to be copied.
 * @property: The name of the property to copy. Must be the literal property
 * name, in lower case, without quotes.
 *
 * This is a convenience macro to simply the coding when copying properties
 * between #ModulemdModuleStreamV1 and #ModulemdModuleStreamV2 objects when
 * @src and @dest are different versions and @property has possible
 * translations. Only the untranslated (`"C"` locale) version of @property will
 * be copied.
 *
 * Does nothing if the @src @property is NULL.
 *
 * Since: 2.0
 */
#define STREAM_UPGRADE_IF_SET_WITH_LOCALE(                                    \
  oldversion, newversion, dest, src, property)                                \
  STREAM_UPGRADE_IF_SET_FULL (oldversion, newversion, dest, src, property, "C")

/**
 * STREAM_REPLACE_HASHTABLE:
 * @version: The stream version being replaced. Must be literal "v1" or "v2"
 * without the quotes.
 * @dest: (out): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object
 * that is the destination at which @property is being replaced.
 * @src: (in): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object that
 * is the source from which @property is being replaced.
 * @property: The name of the #GHashTable property to replace. Must be the
 * literal property name, in lower case, without quotes.
 *
 * This is a convenience macro to simply the coding when replacing #GHashTable
 * properties of #ModulemdModuleStreamV1 and #ModulemdModuleStreamV2 objects
 * when both @src and @dest are the same version.
 *
 * Since: 2.0
 */
#define STREAM_REPLACE_HASHTABLE(version, dest, src, property)                \
  do                                                                          \
    {                                                                         \
      modulemd_module_stream_##version##_replace_##property (dest,            \
                                                             src->property);  \
    }                                                                         \
  while (0)

/**
 * COPY_HASHTABLE_BY_VALUE_ADDER:
 * @dest: (out): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object
 * that is the destination to which @property is to be copied.
 * @src: (in): A #ModulemdModuleStreamV1 or #ModulemdModuleStreamV2 object that
 * is the source from which @property is to be copied.
 * @property: The name of the #GHashTable property to copy. Must be the literal
 * property name, in lower case, without quotes.
 * @adder: (in): A pointer to a method of @dest that supports add-on property
 * values.
 *
 * This is a convenience macro to simply the coding when copying #GHashTable
 * properties between #ModulemdModuleStreamV1 and #ModulemdModuleStreamV2
 * objects when the property is set by using add-on values.
 *
 * Since: 2.0
 */
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


/**
 * modulemd_module_stream_includes_nevra:
 * @self: This #ModulemdModuleStream object.
 * @nevra_pattern: (not nullable): A [glob](https://www.mankier.com/3/glob)
 * pattern to match against the NEVRA strings of the rpm artifacts in @self.
 *
 * Returns: TRUE if this stream includes at least one RPM artifact that
 * matches @nevra_pattern. FALSE otherwise.
 *
 * Since: 2.9
 */
gboolean
modulemd_module_stream_includes_nevra (ModulemdModuleStream *self,
                                       const gchar *nevra_pattern);


/**
 * modulemd_module_stream_set_autogen_module_name:
 * @self: (in): A pointer to a #ModulemdModuleStream object
 * @id: (in): An unsigned integer to be used as a unique identifier if a module
 * name is generated.
 *
 * If @self already has a module name set, this function does nothing.
 * Otherwise, a module name will be generated and set for @self.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_set_autogen_module_name (ModulemdModuleStream *self,
                                                guint id);

/**
 * modulemd_module_stream_set_autogen_stream_name:
 * @self: (in): A pointer to a #ModulemdModuleStream object
 * @id: (in): An unsigned integer to be used as a unique identifier if a stream
 * name is generated.
 *
 * If @self already has a stream name set, this function does nothing.
 * Otherwise, a stream name will be generated and set for @self.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_set_autogen_stream_name (ModulemdModuleStream *self,
                                                guint id);

/**
 * modulemd_module_stream_is_autogen_module_name:
 * @self: (in): A pointer to a #ModulemdModuleStream object
 *
 * Returns: TRUE if @self has a module name that matches the format used if
 * modulemd_module_stream_set_autogen_module_name() created the name.
 * Otherwise FALSE.
 *
 * Since: 2.10
 */
gboolean
modulemd_module_stream_is_autogen_module_name (ModulemdModuleStream *self);

/**
 * modulemd_module_stream_is_autogen_stream_name:
 * @self: (in): A pointer to a #ModulemdModuleStream object
 *
 * Returns: TRUE if @self has a stream name that matches the format used if
 * modulemd_module_stream_set_autogen_stream_name() created the name.
 * Otherwise FALSE.
 *
 * Since: 2.10
 */
gboolean
modulemd_module_stream_is_autogen_stream_name (ModulemdModuleStream *self);

/**
 * modulemd_module_stream_clear_autogen_module_name:
 * @self: (in): A pointer to a #ModulemdModuleStream object
 *
 * Clears @self's module name if it matches the format used if
 * modulemd_module_stream_set_autogen_module_name() created the name, else
 * does nothing.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_clear_autogen_module_name (ModulemdModuleStream *self);

/**
 * modulemd_module_stream_clear_autogen_stream_name:
 * @self: (in): A pointer to a #ModulemdModuleStream object
 *
 * Clears @self's stream name if it matches the format used if
 * modulemd_module_stream_set_autogen_stream_name() created the name, else
 * does nothing.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_clear_autogen_stream_name (ModulemdModuleStream *self);

/**
 * modulemd_module_stream_upgrade_v1_to_v2:
 * @from (in): A #ModulemdModuleStreamV1 object.
 * @error: (out): A #GError that will return the reason for an upgrade error.
 *
 * Return an upgraded copy of this object. Does not modify the original.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStreamV2 copy of
 * @from upgraded to V2. Returns NULL and sets @error appropriately if the
 * upgrade could not be completed automatically.
 *
 * Since: 2.11
 */
ModulemdModuleStream *
modulemd_module_stream_upgrade_v1_to_v2 (ModulemdModuleStream *from);


G_END_DECLS
