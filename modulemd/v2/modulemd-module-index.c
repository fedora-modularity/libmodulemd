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

#include "modulemd-module-index.h"
#include "modulemd-subdocument-info.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-private.h"
#include "private/modulemd-defaults-v1-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-translation-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdModuleIndex
{
  GObject parent_instance;

  GHashTable *modules;
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
            GError **error)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;

  switch (modulemd_subdocument_info_get_doctype (subdoc))
    {
    case MODULEMD_YAML_DOC_MODULESTREAM:
      switch (modulemd_subdocument_info_get_mdversion (subdoc))
        {
        case MD_MODULESTREAM_VERSION_ONE:
          stream = MODULEMD_MODULE_STREAM (
            modulemd_module_stream_v1_parse_yaml (subdoc, error));
          if (stream == NULL)
            return FALSE;
          if (!modulemd_module_index_add_module_stream (self, stream, error))
            return FALSE;
          break;

        case MD_MODULESTREAM_VERSION_TWO:
          stream =
            (ModulemdModuleStream *)modulemd_module_stream_v2_parse_yaml (
              subdoc, error);
          if (stream == NULL)
            return FALSE;
          if (!modulemd_module_index_add_module_stream (self, stream, error))
            return FALSE;
          break;

        default:
          g_set_error (error,
                       MODULEMD_YAML_ERROR,
                       MODULEMD_YAML_ERROR_PARSE,
                       "Invalid mdversion for a stream object");
          return FALSE;
        }
      break;

    case MODULEMD_YAML_DOC_DEFAULTS:
      switch (modulemd_subdocument_info_get_mdversion (subdoc))
        {
        case MD_DEFAULTS_VERSION_ONE:
          defaults = (ModulemdDefaults *)modulemd_defaults_v1_parse_yaml (
            subdoc, error);
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
      translation = modulemd_translation_parse_yaml (subdoc, error);
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


static gboolean
modulemd_module_index_update_from_parser (ModulemdModuleIndex *self,
                                          yaml_parser_t *parser,
                                          GPtrArray **failures,
                                          GError **error)
{
  gboolean done = FALSE;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  MMD_INIT_YAML_EVENT (event);

  *failures = g_ptr_array_new_full (0, g_object_unref);

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
            }
          else
            {
              /* Initial parsing worked, parse further */
              if (!add_subdoc (self, subdoc, error))
                return FALSE;
              g_clear_pointer (&subdoc, g_object_unref);
            }
          break;

        case YAML_STREAM_END_EVENT: done = TRUE; break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in document stream");
          break;
        }

      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
dump_defaults (ModulemdModule *module, yaml_emitter_t *emitter, GError **error)
{
  ModulemdDefaults *defaults = modulemd_module_get_defaults (module);

  if (defaults == NULL)
    return TRUE; /* Nothing to dump -> all a success */

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

  /*
   * Make sure we get a stable sorting by sorting jsut before dumping.
   */
  g_ptr_array_sort (streams, compare_stream_SVC);

  for (i = 0; i < streams->len; i++)
    {
      stream = (ModulemdModuleStream *)g_ptr_array_index (streams, i);

      if (modulemd_module_stream_get_mdversion (stream) ==
          MD_MODULESTREAM_VERSION_ONE)
        {
          /* TODO: Emit stream v1 */
        }
      else if (modulemd_module_stream_get_mdversion (stream) ==
               MD_MODULESTREAM_VERSION_TWO)
        {
          /* TODO: Emit stream v2 */
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
                                        GPtrArray **failures,
                                        GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  g_autoptr (FILE) yaml_stream = NULL;

  yaml_stream = g_fopen (yaml_file, "rb");

  return modulemd_module_index_update_from_stream (
    self, yaml_stream, failures, error);
}


gboolean
modulemd_module_index_update_from_string (ModulemdModuleIndex *self,
                                          const gchar *yaml_string,
                                          GPtrArray **failures,
                                          GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  MMD_INIT_YAML_PARSER (parser);

  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string, strlen (yaml_string));

  return modulemd_module_index_update_from_parser (
    self, &parser, failures, error);
}


gboolean
modulemd_module_index_update_from_stream (ModulemdModuleIndex *self,
                                          FILE *yaml_stream,
                                          GPtrArray **failures,
                                          GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  MMD_INIT_YAML_PARSER (parser);

  yaml_parser_set_input_file (&parser, yaml_stream);

  return modulemd_module_index_update_from_parser (
    self, &parser, failures, error);
}


const gchar *
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
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  modulemd_module_add_stream (
    get_or_create_module (self,
                          modulemd_module_stream_get_module_name (stream)),
    stream);
  return TRUE;
}


gboolean
modulemd_module_index_add_defaults (ModulemdModuleIndex *self,
                                    ModulemdDefaults *defaults,
                                    GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  modulemd_module_set_defaults (
    get_or_create_module (self, modulemd_defaults_get_module_name (defaults)),
    defaults);
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
