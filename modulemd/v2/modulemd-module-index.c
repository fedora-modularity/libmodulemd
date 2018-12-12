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

#include <errno.h>
#include <glib.h>
#include <yaml.h>

#include "modulemd-module-index.h"
#include "modulemd-subdocument-info.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-private.h"
#include "private/modulemd-defaults-private.h"
#include "private/modulemd-defaults-v1-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-module-index-private.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-translation-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdModuleIndex
{
  GObject parent_instance;

  GHashTable *modules;

  ModulemdDefaultsVersionEnum defaults_mdversion;
  ModulemdModuleStreamVersionEnum stream_mdversion;
};

G_DEFINE_TYPE (ModulemdModuleIndex, modulemd_module_index, G_TYPE_OBJECT)


ModulemdModuleIndex *
modulemd_module_index_new (void)
{
  return g_object_new (MODULEMD_TYPE_MODULE_INDEX, NULL);
}


static void
modulemd_module_index_finalize (GObject *object)
{
  ModulemdModuleIndex *self = (ModulemdModuleIndex *)object;

  g_clear_pointer (&self->modules, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_module_index_parent_class)->finalize (object);
}

static void
modulemd_module_index_get_property (GObject *object,
                                    guint prop_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_module_index_set_property (GObject *object,
                                    guint prop_id,
                                    const GValue *value,
                                    GParamSpec *pspec)
{
  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_module_index_class_init (ModulemdModuleIndexClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_module_index_finalize;
  object_class->get_property = modulemd_module_index_get_property;
  object_class->set_property = modulemd_module_index_set_property;
}


static void
modulemd_module_index_init (ModulemdModuleIndex *self)
{
  self->modules =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}


static ModulemdModule *
get_or_create_module (ModulemdModuleIndex *self, const gchar *module_name)
{
  ModulemdModule *module = g_hash_table_lookup (self->modules, module_name);
  if (module == NULL)
    {
      module = modulemd_module_new (module_name);
      g_hash_table_insert (self->modules, g_strdup (module_name), module);
    }
  return module;
}


static gboolean
add_subdoc (ModulemdModuleIndex *self,
            ModulemdSubdocumentInfo *subdoc,
            gboolean strict,
            gboolean autogen_module_name,
            GError **error)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;
  g_autofree gchar *name = NULL;

  switch (modulemd_subdocument_info_get_doctype (subdoc))
    {
    case MODULEMD_YAML_DOC_MODULESTREAM:
      switch (modulemd_subdocument_info_get_mdversion (subdoc))
        {
        case MD_MODULESTREAM_VERSION_ONE:
          stream = MODULEMD_MODULE_STREAM (
            modulemd_module_stream_v1_parse_yaml (subdoc, strict, error));
          break;

        case MD_MODULESTREAM_VERSION_TWO:
          stream =
            (ModulemdModuleStream *)modulemd_module_stream_v2_parse_yaml (
              subdoc, strict, error);
          break;

        default:
          g_set_error (error,
                       MODULEMD_YAML_ERROR,
                       MODULEMD_YAML_ERROR_PARSE,
                       "Invalid mdversion for a stream object");
          return FALSE;
        }

      if (stream == NULL)
        return FALSE;

      if (autogen_module_name &&
          !modulemd_module_stream_get_module_name (stream))
        {
          name = g_strdup_printf ("__unnamed_module_%d",
                                  g_hash_table_size (self->modules) + 1);
          modulemd_module_stream_set_module_name (stream, name);
          g_clear_pointer (&name, g_free);
        }

      if (autogen_module_name &&
          !modulemd_module_stream_get_stream_name (stream))
        {
          name = g_strdup_printf ("__unnamed_stream_%d",
                                  g_hash_table_size (self->modules) + 1);
          modulemd_module_stream_set_stream_name (stream, name);
          g_clear_pointer (&name, g_free);
        }


      if (!modulemd_module_index_add_module_stream (self, stream, error))
        return FALSE;

      break;

    case MODULEMD_YAML_DOC_DEFAULTS:
      switch (modulemd_subdocument_info_get_mdversion (subdoc))
        {
        case MD_DEFAULTS_VERSION_ONE:
          defaults = (ModulemdDefaults *)modulemd_defaults_v1_parse_yaml (
            subdoc, strict, error);
          if (defaults == NULL)
            return FALSE;
          if (!modulemd_module_index_add_defaults (self, defaults, error))
            return FALSE;
          break;

        default:
          g_set_error (error,
                       MODULEMD_YAML_ERROR,
                       MODULEMD_YAML_ERROR_PARSE,
                       "Invalid mdversion for a defaults object");
          return FALSE;
        }
      break;

    case MODULEMD_YAML_DOC_TRANSLATIONS:
      translation = modulemd_translation_parse_yaml (subdoc, strict, error);
      if (translation == NULL)
        return FALSE;
      if (!modulemd_module_index_add_translation (self, translation, error))
        return FALSE;
      break;

    default:
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_PARSE,
                   "Invalid doctype encountered");
      return FALSE;
    }

  return TRUE;
}


gboolean
modulemd_module_index_update_from_parser (ModulemdModuleIndex *self,
                                          yaml_parser_t *parser,
                                          gboolean strict,
                                          gboolean autogen_module_name,
                                          GPtrArray **failures,
                                          GError **error)
{
  gboolean done = FALSE;
  gboolean all_passed = TRUE;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  MMD_INIT_YAML_EVENT (event);

  if (*failures == NULL)
    *failures = g_ptr_array_new_with_free_func (g_object_unref);

  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_STREAM_START_EVENT)
    MMD_YAML_ERROR_EVENT_EXIT_BOOL (
      error, event, "Did not encounter stream start");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_DOCUMENT_START_EVENT:
          /* One more subdocument to parse */
          subdoc = modulemd_yaml_parse_document_type (parser);
          if (modulemd_subdocument_info_get_gerror (subdoc) != NULL)
            {
              /* Add to failures and ignore */
              g_ptr_array_add (*failures, g_steal_pointer (&subdoc));
              all_passed = FALSE;
            }
          else
            {
              /* Initial parsing worked, parse further */
              if (!add_subdoc (
                    self, subdoc, strict, autogen_module_name, error))
                {
                  modulemd_subdocument_info_set_gerror (subdoc, *error);
                  g_clear_pointer (error, g_error_free);
                  /* Add to failures and ignore */
                  g_ptr_array_add (*failures, g_steal_pointer (&subdoc));
                  all_passed = FALSE;
                }
            }
          g_clear_pointer (&subdoc, g_object_unref);
          break;

        case YAML_STREAM_END_EVENT: done = TRUE; break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in document stream");
          break;
        }

      yaml_event_delete (&event);
    }

  return all_passed;
}


static gboolean
dump_defaults (ModulemdModule *module, yaml_emitter_t *emitter, GError **error)
{
  ModulemdDefaults *defaults = modulemd_module_get_defaults (module);
  g_autoptr (GError) nested_error = NULL;

  if (defaults == NULL)
    return TRUE; /* Nothing to dump -> all a success */

  if (!modulemd_defaults_validate (defaults, &nested_error))
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Could not validate defaults to emit: ");
      return FALSE;
    }

  if (modulemd_defaults_get_mdversion (defaults) == MD_DEFAULTS_VERSION_ONE)
    {
      if (!modulemd_defaults_v1_emit_yaml (
            (ModulemdDefaultsV1 *)defaults, emitter, error))
        return FALSE;
    }
  else
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MODULEMD_ERROR_VALIDATE,
                           "Provided defaults is not a recognized version");
      return FALSE;
    }

  return TRUE;
}


static gboolean
dump_translations (ModulemdModule *module,
                   yaml_emitter_t *emitter,
                   GError **error)
{
  ModulemdTranslation *translation = NULL;
  gsize i = 0;
  g_autoptr (GPtrArray) streams =
    modulemd_module_get_translated_streams (module);

  for (i = 0; i < streams->len; i++)
    {
      translation = modulemd_module_get_translation (
        module, g_ptr_array_index (streams, i));

      if (!modulemd_translation_emit_yaml (translation, emitter, error))
        return FALSE;
    }

  return TRUE;
}


static gint
compare_stream_SVC (gconstpointer a, gconstpointer b)
{
  ModulemdModuleStream *a_ = *(ModulemdModuleStream **)a;
  ModulemdModuleStream *b_ = *(ModulemdModuleStream **)b;
  g_autofree gchar *a_nsvc = modulemd_module_stream_get_nsvc_as_string (a_);
  g_autofree gchar *b_nsvc = modulemd_module_stream_get_nsvc_as_string (b_);
  return g_strcmp0 (a_nsvc, b_nsvc);
}


static gboolean
dump_streams (ModulemdModule *module, yaml_emitter_t *emitter, GError **error)
{
  ModulemdModuleStream *stream = NULL;
  gsize i = 0;
  GPtrArray *streams = modulemd_module_get_all_streams (module);
  g_autoptr (GError) nested_error = NULL;

  /*
   * Make sure we get a stable sorting by sorting just before dumping.
   */
  g_ptr_array_sort (streams, compare_stream_SVC);

  for (i = 0; i < streams->len; i++)
    {
      stream = (ModulemdModuleStream *)g_ptr_array_index (streams, i);

      if (!modulemd_module_stream_validate (stream, &nested_error))
        {
          g_propagate_prefixed_error (error,
                                      g_steal_pointer (&nested_error),
                                      "Could not validate stream to emit: ");
          return FALSE;
        }

      if (modulemd_module_stream_get_mdversion (stream) ==
          MD_MODULESTREAM_VERSION_ONE)
        {
          if (!modulemd_module_stream_v1_emit_yaml (
                MODULEMD_MODULE_STREAM_V1 (stream), emitter, error))
            return FALSE;
        }
      else if (modulemd_module_stream_get_mdversion (stream) ==
               MD_MODULESTREAM_VERSION_TWO)
        {
          if (!modulemd_module_stream_v2_emit_yaml (
                MODULEMD_MODULE_STREAM_V2 (stream), emitter, error))
            return FALSE;
        }
      else
        {
          g_set_error_literal (error,
                               MODULEMD_ERROR,
                               MODULEMD_ERROR_VALIDATE,
                               "Provided stream is not a recognized version");
          return FALSE;
        }
    }

  return TRUE;
}


static gboolean
modulemd_module_index_dump_to_emitter (ModulemdModuleIndex *self,
                                       yaml_emitter_t *emitter,
                                       GError **error)
{
  ModulemdModule *module = NULL;
  gsize i;
  g_autoptr (GPtrArray) modules =
    modulemd_ordered_str_keys (self->modules, modulemd_strcmp_sort);

  if (!mmd_emitter_start_stream (emitter, error))
    return FALSE;

  for (i = 0; i < modules->len; i++)
    {
      module = modulemd_module_index_get_module (
        self, g_ptr_array_index (modules, i));

      if (!dump_defaults (module, emitter, error))
        return FALSE;

      if (!dump_translations (module, emitter, error))
        return FALSE;

      if (!dump_streams (module, emitter, error))
        return FALSE;
    }

  if (!mmd_emitter_end_stream (emitter, error))
    return FALSE;

  return TRUE;
}


gboolean
modulemd_module_index_update_from_file (ModulemdModuleIndex *self,
                                        const gchar *yaml_file,
                                        gboolean strict,
                                        GPtrArray **failures,
                                        GError **error)
{
  if (*failures == NULL)
    *failures = g_ptr_array_new_full (0, g_object_unref);

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  int saved_errno;
  g_autoptr (FILE) yaml_stream = NULL;

  yaml_stream = g_fopen (yaml_file, "rb");
  saved_errno = errno;

  if (yaml_stream == NULL)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_YAML_ERROR_OPEN,
                   "Failed to open file: %s",
                   g_strerror (saved_errno));
      return FALSE;
    }

  return modulemd_module_index_update_from_stream (
    self, yaml_stream, strict, failures, error);
}


gboolean
modulemd_module_index_update_from_string (ModulemdModuleIndex *self,
                                          const gchar *yaml_string,
                                          gboolean strict,
                                          GPtrArray **failures,
                                          GError **error)
{
  if (*failures == NULL)
    *failures = g_ptr_array_new_full (0, g_object_unref);

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  if (!yaml_string)
    {
      g_set_error (
        error, MODULEMD_ERROR, MODULEMD_YAML_ERROR_OPEN, "No string provided");
      return FALSE;
    }

  MMD_INIT_YAML_PARSER (parser);

  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string, strlen (yaml_string));

  return modulemd_module_index_update_from_parser (
    self, &parser, strict, FALSE, failures, error);
}


gboolean
modulemd_module_index_update_from_stream (ModulemdModuleIndex *self,
                                          FILE *yaml_stream,
                                          gboolean strict,
                                          GPtrArray **failures,
                                          GError **error)
{
  if (*failures == NULL)
    *failures = g_ptr_array_new_full (0, g_object_unref);

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  if (!yaml_stream)
    {
      g_set_error (
        error, MODULEMD_ERROR, MODULEMD_YAML_ERROR_OPEN, "No stream provided");
      return FALSE;
    }

  MMD_INIT_YAML_PARSER (parser);

  yaml_parser_set_input_file (&parser, yaml_stream);

  return modulemd_module_index_update_from_parser (
    self, &parser, strict, FALSE, failures, error);
}


gchar *
modulemd_module_index_dump_to_string (ModulemdModuleIndex *self,
                                      GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), NULL);

  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  if (!modulemd_module_index_dump_to_emitter (self, &emitter, error))
    return NULL;

  return g_steal_pointer (&yaml_string->str);
}


gboolean
modulemd_module_index_dump_to_stream (ModulemdModuleIndex *self,
                                      FILE *yaml_stream,
                                      GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  MMD_INIT_YAML_EMITTER (emitter);
  yaml_emitter_set_output_file (&emitter, yaml_stream);

  return modulemd_module_index_dump_to_emitter (self, &emitter, error);
}


GStrv
modulemd_module_index_get_module_names_as_strv (ModulemdModuleIndex *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->modules);
}


ModulemdModule *
modulemd_module_index_get_module (ModulemdModuleIndex *self,
                                  const gchar *module_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), NULL);

  return g_hash_table_lookup (self->modules, module_name);
}


gboolean
modulemd_module_index_add_module_stream (ModulemdModuleIndex *self,
                                         ModulemdModuleStream *stream,
                                         GError **error)
{
  g_autoptr (GError) nested_error = NULL;
  ModulemdModuleStreamVersionEnum mdversion = MD_MODULESTREAM_VERSION_UNSET;
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  if (!modulemd_module_stream_get_module_name (stream) ||
      !modulemd_module_stream_get_stream_name (stream))
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "The module and stream names are required when adding to "
                   "ModuleIndex.");
      return FALSE;
    }

  mdversion = modulemd_module_add_stream (
    get_or_create_module (self,
                          modulemd_module_stream_get_module_name (stream)),
    stream,
    self->stream_mdversion,
    &nested_error);

  if (mdversion == MD_MODULESTREAM_VERSION_ERROR)
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }

  if (mdversion > self->stream_mdversion)
    {
      /* Upgrade any streams we've already seen to this version */
      g_debug ("Upgrading all streams to version %i", mdversion);
      if (!modulemd_module_index_upgrade_streams (
            self, mdversion, &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return FALSE;
        }
    }

  return TRUE;
}


gboolean
modulemd_module_index_upgrade_streams (
  ModulemdModuleIndex *self,
  ModulemdModuleStreamVersionEnum mdversion,
  GError **error)
{
  GHashTableIter iter;
  gpointer key, value;
  g_autoptr (ModulemdModule) module = NULL;
  g_autoptr (GError) nested_error = NULL;

  if (mdversion < self->stream_mdversion)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_UPGRADE,
                   "Downgrades not permitted. mdversion %i < current %i",
                   mdversion,
                   self->stream_mdversion);
      return FALSE;
    }

  g_hash_table_iter_init (&iter, self->modules);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      module = g_object_ref (MODULEMD_MODULE (value));

      /* Skip any module without streams */
      if (modulemd_module_get_all_streams (module)->len == 0)
        {
          g_clear_object (&module);
          continue;
        }

      if (!modulemd_module_upgrade_streams (module, mdversion, &nested_error))
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Error upgrading streams for module %s",
            modulemd_module_get_module_name (module));
          return FALSE;
        }

      g_clear_object (&module);
    }

  self->stream_mdversion = mdversion;

  return TRUE;
}


gboolean
modulemd_module_index_add_defaults (ModulemdModuleIndex *self,
                                    ModulemdDefaults *defaults,
                                    GError **error)
{
  g_autoptr (GError) nested_error = NULL;
  ModulemdDefaultsVersionEnum mdversion = MD_DEFAULTS_VERSION_UNSET;

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  mdversion = modulemd_module_set_defaults (
    get_or_create_module (self, modulemd_defaults_get_module_name (defaults)),
    defaults,
    self->defaults_mdversion,
    &nested_error);
  if (mdversion == MD_DEFAULTS_VERSION_ERROR)
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }

  if (mdversion > self->defaults_mdversion)
    {
      /* Upgrade any defaults we've already seen to this version */
      g_debug ("Upgrading all defaults to version %i", mdversion);
      if (!modulemd_module_index_upgrade_defaults (
            self, mdversion, &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return FALSE;
        }
    }

  return TRUE;
}


gboolean
modulemd_module_index_upgrade_defaults (ModulemdModuleIndex *self,
                                        ModulemdDefaultsVersionEnum mdversion,
                                        GError **error)
{
  GHashTableIter iter;
  gpointer key, value;
  g_autoptr (ModulemdModule) module = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;
  ModulemdDefaultsVersionEnum returned_mdversion = MD_DEFAULTS_VERSION_UNSET;
  g_autoptr (GError) nested_error = NULL;

  if (mdversion < self->defaults_mdversion)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_UPGRADE,
                   "Downgrades not permitted. mdversion %i < current %i",
                   mdversion,
                   self->defaults_mdversion);
      return FALSE;
    }

  if (mdversion > MD_DEFAULTS_VERSION_LATEST)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_UPGRADE,
                   "Unknown Defaults metadata version %i",
                   mdversion);
      return FALSE;
    }

  g_hash_table_iter_init (&iter, self->modules);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      module = g_object_ref (MODULEMD_MODULE (value));
      defaults = modulemd_module_get_defaults (module);

      /* Skip any module without defaults */
      if (!defaults)
        continue;
      g_object_ref (defaults);

      returned_mdversion = modulemd_module_set_defaults (
        module, defaults, self->defaults_mdversion, &nested_error);
      if (returned_mdversion != mdversion)
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Error upgrading previously-added defaults: ");
          return FALSE;
        }
      g_clear_object (&defaults);
      g_clear_object (&module);
    }

  self->defaults_mdversion = mdversion;

  return TRUE;
}


gboolean
modulemd_module_index_add_translation (ModulemdModuleIndex *self,
                                       ModulemdTranslation *translation,
                                       GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  modulemd_module_add_translation (
    get_or_create_module (self,
                          modulemd_translation_get_module_name (translation)),
    translation);
  return TRUE;
}


gboolean
modulemd_module_index_merge (ModulemdModuleIndex *from,
                             ModulemdModuleIndex *into,
                             gboolean override,
                             GError **error)
{
  MODULEMD_INIT_TRACE ();
  GHashTableIter iter;
  gpointer key, value;
  const gchar *module_name = NULL;
  const gchar *trans_stream = NULL;
  ModulemdModule *module = NULL;
  ModulemdModule *into_module = NULL;
  GPtrArray *streams = NULL;
  ModulemdModuleStream *stream = NULL;
  g_autoptr (GPtrArray) translations = NULL;
  ModulemdTranslation *translation = NULL;
  ModulemdTranslation *current_translation = NULL;
  ModulemdDefaults *defaults = NULL;
  ModulemdDefaults *into_defaults = NULL;
  g_autoptr (ModulemdDefaults) merged_defaults = NULL;
  g_autoptr (GError) nested_error = NULL;
  guint i;
  g_autoptr (GPtrArray) translated_stream_names = NULL;
  gchar *translated_stream_name = NULL;


  /* Loop through each module in the Index */
  g_hash_table_iter_init (&iter, from->modules);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      module_name = (const gchar *)key;
      g_debug ("Merging module %s", module_name);

      module = MODULEMD_MODULE (value);
      into_module = get_or_create_module (into, module_name);

      /* Copy all module streams for this module
       * The module streams have "version" and "context" to disambiguate them,
       * so we have documented that if there are two modules with differing
       * content and the same NSVC, the operation is undefined.
       * As such, we'll just assume it's safe to add every stream. If there are
       * duplicates, they'll be deduplicated by replacing the previously-
       * existing entry.
       */
      g_debug ("Prioritizer: merging streams for %s", module_name);
      streams = modulemd_module_get_all_streams (module);
      for (i = 0; i < streams->len; i++)
        {
          stream = g_ptr_array_index (streams, i);

          if (!modulemd_module_index_add_module_stream (
                into, stream, &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
        }


      /* Merge any defaults entry for this module */
      g_debug ("Prioritizer: merging defaults for %s", module_name);
      defaults = modulemd_module_get_defaults (module);
      into_defaults = modulemd_module_get_defaults (into_module);
      if (override && defaults)
        {
          /* If we've been told to override (we're at a higher priority level),
           * then just replace the current defaults with the new one
           */
          if (!modulemd_module_index_add_defaults (
                into, defaults, &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
        }
      else if (!defaults)
        {
          /* No defaults to merge in right now, just continue */
        }
      else if (defaults && !into_defaults)
        {
          /* There are no defaults on the target module yet. Copy these */
          if (!modulemd_module_index_add_defaults (
                into, defaults, &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
        }
      else
        {
          merged_defaults =
            modulemd_defaults_merge (defaults, into_defaults, &nested_error);
          if (!merged_defaults)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }

          /* Add the new, merged defaults to the index */
          if (!modulemd_module_index_add_defaults (
                into, merged_defaults, &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
          g_clear_object (&merged_defaults);
        }

      /* Merge translations for this module */
      g_debug ("Prioritizer: merging translations for %s", module_name);
      translated_stream_names =
        modulemd_module_get_translated_streams (module);
      for (i = 0; i < translated_stream_names->len; i++)
        {
          translated_stream_name =
            g_ptr_array_index (translated_stream_names, i);
          translation =
            modulemd_module_get_translation (module, translated_stream_name);
          trans_stream = modulemd_translation_get_module_stream (translation);
          current_translation =
            modulemd_module_get_translation (into_module, trans_stream);

          if (!current_translation ||
              modulemd_translation_get_modified (translation) >
                modulemd_translation_get_modified (current_translation))
            {
              /* There was no translation for this stream name or we just found
               * a newer version of it, so set it on the index.
               */
              if (!modulemd_module_index_add_translation (
                    into, translation, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }
        }
      g_clear_pointer (&translated_stream_names, g_ptr_array_unref);

      g_debug ("Prioritizer: all documents merged for %s", module_name);
      g_clear_pointer (&translations, g_ptr_array_unref);
    }
  return TRUE;
}


ModulemdDefaultsVersionEnum
modulemd_module_index_get_defaults_mdversion (ModulemdModuleIndex *self)
{
  return self->defaults_mdversion;
}


ModulemdModuleStreamVersionEnum
modulemd_module_index_get_stream_mdversion (ModulemdModuleIndex *self)
{
  return self->stream_mdversion;
}
