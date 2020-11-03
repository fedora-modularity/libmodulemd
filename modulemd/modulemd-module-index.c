/*
 * This file is part of libmodulemd
 * Copyright (C) 2018-2020 Red Hat, Inc.
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
#include <inttypes.h>
#include <stdio.h>
#include <yaml.h>

#ifdef HAVE_RPMIO
#include <rpm/rpmio.h>
#endif

#include "modulemd-compression.h"
#include "modulemd-errors.h"
#include "modulemd-module-index.h"
#include "modulemd-subdocument-info.h"
#include "private/glib-extensions.h"
#include "private/modulemd-compression-private.h"
#include "private/modulemd-defaults-private.h"
#include "private/modulemd-defaults-v1-private.h"
#include "private/modulemd-module-index-private.h"
#include "private/modulemd-module-private.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-packager-v3.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-translation-private.h"
#include "private/modulemd-obsoletes-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"


#define MMD_YAML_SUFFIX ".yaml"


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
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdPackagerV3) packager = NULL;
  g_autoptr (ModulemdTranslation) translation = NULL;
  g_autoptr (ModulemdObsoletes) obsoletes = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;
  g_autofree gchar *name = NULL;
  ModulemdYamlDocumentTypeEnum doctype =
    modulemd_subdocument_info_get_doctype (subdoc);

  switch (doctype)
    {
    case MODULEMD_YAML_DOC_PACKAGER:
      if (modulemd_subdocument_info_get_mdversion (subdoc) <
          MD_PACKAGER_VERSION_TWO)
        {
          g_set_error (error,
                       MODULEMD_YAML_ERROR,
                       MMD_YAML_ERROR_PARSE,
                       "Invalid mdversion for a packager document");
          return FALSE;
        }

      if (modulemd_subdocument_info_get_mdversion (subdoc) ==
          MD_PACKAGER_VERSION_THREE)
        {
          packager = modulemd_packager_v3_parse_yaml (subdoc, error);

          /* TODO: Determine which stream version to convert the packager
           * object into and do so here.
           */
          break;
        }

      /* Fall through intentionally
       * We will handle the v2 packager format below
       */

    case MODULEMD_YAML_DOC_MODULESTREAM:
      switch (modulemd_subdocument_info_get_mdversion (subdoc))
        {
        case MD_MODULESTREAM_VERSION_ONE:
          stream = MODULEMD_MODULE_STREAM (
            modulemd_module_stream_v1_parse_yaml (subdoc, strict, error));
          break;

        case MD_MODULESTREAM_VERSION_TWO:
          stream =
            MODULEMD_MODULE_STREAM (modulemd_module_stream_v2_parse_yaml (
              subdoc, strict, doctype == MODULEMD_YAML_DOC_PACKAGER, error));
          break;

        case MD_MODULESTREAM_VERSION_THREE:
          if (doctype == MODULEMD_YAML_DOC_MODULESTREAM)
            {
              stream = MODULEMD_MODULE_STREAM (
                modulemd_module_stream_v3_parse_yaml (subdoc, strict, error));
            }
          break;

        default:
          g_set_error (error,
                       MODULEMD_YAML_ERROR,
                       MMD_YAML_ERROR_PARSE,
                       "Invalid mdversion for a stream object");
          return FALSE;
        }

      if (stream == NULL)
        {
          return FALSE;
        }

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

      if (!modulemd_module_stream_validate (stream, &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return FALSE;
        }

      if (!modulemd_module_index_add_module_stream (self, stream, error))
        {
          return FALSE;
        }

      break;

    case MODULEMD_YAML_DOC_DEFAULTS:
      switch (modulemd_subdocument_info_get_mdversion (subdoc))
        {
        case MD_DEFAULTS_VERSION_ONE:
          defaults = (ModulemdDefaults *)modulemd_defaults_v1_parse_yaml (
            subdoc, strict, error);
          if (defaults == NULL)
            {
              return FALSE;
            }

          if (!modulemd_defaults_validate (defaults, &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }

          if (!modulemd_module_index_add_defaults (self, defaults, error))
            {
              return FALSE;
            }
          break;

        default:
          g_set_error (error,
                       MODULEMD_YAML_ERROR,
                       MMD_YAML_ERROR_PARSE,
                       "Invalid mdversion for a defaults object");
          return FALSE;
        }
      break;

    case MODULEMD_YAML_DOC_TRANSLATIONS:
      translation = modulemd_translation_parse_yaml (subdoc, strict, error);
      if (translation == NULL)
        {
          return FALSE;
        }

      if (!modulemd_translation_validate (translation, &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return FALSE;
        }

      if (!modulemd_module_index_add_translation (self, translation, error))
        {
          return FALSE;
        }
      break;

    case MODULEMD_YAML_DOC_OBSOLETES:
      obsoletes = modulemd_obsoletes_parse_yaml (subdoc, strict, error);
      if (obsoletes == NULL)
        {
          return FALSE;
        }

      if (!modulemd_obsoletes_validate (obsoletes, &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return FALSE;
        }

      if (!modulemd_module_index_add_obsoletes (self, obsoletes, error))
        {
          return FALSE;
        }
      break;

    default:
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_PARSE,
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
    {
      *failures = g_ptr_array_new_with_free_func (g_object_unref);
    }

  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_STREAM_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Did not encounter stream start");
    }

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
    {
      return TRUE; /* Nothing to dump -> all a success */
    }

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
        {
          return FALSE;
        }
    }
  else
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
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
        {
          return FALSE;
        }
    }

  return TRUE;
}

static gboolean
dump_obsoletes (ModulemdModule *module,
                yaml_emitter_t *emitter,
                GError **error)
{
  ModulemdObsoletes *o = NULL;
  gsize i = 0;
  GPtrArray *obsoletes = modulemd_module_get_obsoletes (module);

  for (i = 0; i < obsoletes->len; i++)
    {
      o = g_ptr_array_index (obsoletes, i);

      if (!modulemd_obsoletes_emit_yaml (o, emitter, error))
        {
          return FALSE;
        }
    }

  return TRUE;
}


static gint
compare_stream_SVCA (gconstpointer a, gconstpointer b)
{
  ModulemdModuleStream *a_ = *(ModulemdModuleStream **)a;
  ModulemdModuleStream *b_ = *(ModulemdModuleStream **)b;
  g_autofree gchar *a_nsvca = modulemd_module_stream_get_NSVCA_as_string (a_);
  g_autofree gchar *b_nsvca = modulemd_module_stream_get_NSVCA_as_string (b_);
  return g_strcmp0 (a_nsvca, b_nsvca);
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
  g_ptr_array_sort (streams, compare_stream_SVCA);

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
            {
              return FALSE;
            }
        }
      else if (modulemd_module_stream_get_mdversion (stream) ==
               MD_MODULESTREAM_VERSION_TWO)
        {
          if (!modulemd_module_stream_v2_emit_yaml (
                MODULEMD_MODULE_STREAM_V2 (stream), emitter, error))
            {
              return FALSE;
            }
        }
      else if (modulemd_module_stream_get_mdversion (stream) ==
               MD_MODULESTREAM_VERSION_THREE)
        {
          if (!modulemd_module_stream_v3_emit_yaml (
                MODULEMD_MODULE_STREAM_V3 (stream), emitter, error))
            {
              return FALSE;
            }
        }
      else
        {
          g_set_error_literal (error,
                               MODULEMD_ERROR,
                               MMD_ERROR_VALIDATE,
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

  if (modules->len == 0)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Index contains no modules.");
      return FALSE;
    }

  if (!mmd_emitter_start_stream (emitter, error))
    {
      return FALSE;
    }

  for (i = 0; i < modules->len; i++)
    {
      module = modulemd_module_index_get_module (
        self, g_ptr_array_index (modules, i));

      if (!dump_defaults (module, emitter, error))
        {
          return FALSE;
        }

      if (!dump_obsoletes (module, emitter, error))
        {
          return FALSE;
        }

      if (!dump_translations (module, emitter, error))
        {
          return FALSE;
        }

      if (!dump_streams (module, emitter, error))
        {
          return FALSE;
        }
    }

  if (!mmd_emitter_end_stream (emitter, error))
    {
      return FALSE;
    }

  return TRUE;
}


gboolean
modulemd_module_index_update_from_file_ext (ModulemdModuleIndex *self,
                                            const gchar *yaml_file,
                                            gboolean strict,
                                            gboolean autogen_module_name,
                                            GPtrArray **failures,
                                            GError **error)
{
  if (*failures == NULL)
    {
      *failures = g_ptr_array_new_full (0, g_object_unref);
    }

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  MMD_INIT_YAML_PARSER (parser);
  int saved_errno;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (GError) nested_error = NULL;
  int fd;
  ModulemdCompressionTypeEnum comtype;
  g_autofree gchar *fmode = NULL;

  yaml_stream = g_fopen (yaml_file, "rbe");
  saved_errno = errno;

  if (yaml_stream == NULL)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_OPEN,
                   "Failed to open file: %s",
                   g_strerror (saved_errno));
      return FALSE;
    }

  /* To avoid TOCTOU race conditions, do everything from the same opened
   * file
   */
  fd = fileno (yaml_stream);

  /* Determine if the file is compressed */
  comtype = modulemd_detect_compression (yaml_file, fd, &nested_error);
  if (comtype == MODULEMD_COMPRESSION_TYPE_DETECTION_FAILED)
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }
  if (comtype == MODULEMD_COMPRESSION_TYPE_NO_COMPRESSION ||
      comtype == MODULEMD_COMPRESSION_TYPE_UNKNOWN_COMPRESSION)
    {
      /* If it's not compressed (or we can't figure out what compression is in
       * use), just use the libyaml function. It's fast and will fail quickly
       * if the file is unreadable.
       */

      yaml_parser_set_input_file (&parser, yaml_stream);

      return modulemd_module_index_update_from_parser (
        self, &parser, strict, autogen_module_name, failures, error);
    }

#ifdef HAVE_RPMIO
  /* We're handling a compressed input file, so we'll use librpm's "rpmio"
   * suite of tools to deal with it. We need to construct a special "mode"
   * argument to pass to Fdopen().
   */
  FD_t rpmio_fd = NULL;
  g_auto (FD_t) fd_dup = NULL;

  fmode = modulemd_get_rpmio_fmode ("r", comtype);
  if (!fmode)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_FILE_ACCESS,
                   "Unable to construct rpmio fmode from comtype [%d]",
                   comtype);
      return FALSE;
    }

  /* Create an rpmio file descriptor object from the current file descriptor,
   * setting the appropriate "mode" so that librpm will read it with the
   * correct handlers.
   */
  fd_dup = fdDup (fd);
  saved_errno = errno;
  if (!fd_dup)
    {
      g_set_error (
        error,
        MODULEMD_ERROR,
        MMD_ERROR_NOT_IMPLEMENTED,
        "Cannot open compressed file. Error in rpmio::fdDup(%d): %s",
        fd,
        strerror (saved_errno));
      return FALSE;
    }

  g_debug ("Calling rpmio::Fdopen (%p, %s)", fd_dup, fmode);
  rpmio_fd = Fdopen (fd_dup, fmode);

  if (!rpmio_fd)
    {
      g_set_error_literal (
        error,
        MODULEMD_ERROR,
        MMD_ERROR_NOT_IMPLEMENTED,
        "Cannot open compressed file. Error in rpmio::Fdopen().");
      return FALSE;
    }

  g_debug ("rpmio::Fdopen (%p, %s) succeeded", fd_dup, fmode);

  return modulemd_module_index_update_from_custom (
    self, compressed_stream_read_fn, rpmio_fd, strict, failures, error);

#else /* HAVE_RPMIO */
  g_set_error_literal (
    error,
    MODULEMD_ERROR,
    MMD_ERROR_NOT_IMPLEMENTED,
    "Cannot open compressed file. libmodulemd was not compiled "
    "with rpmio support.");
  return FALSE;
#endif /* HAVE_RPMIO */
}


gboolean
modulemd_module_index_update_from_file (ModulemdModuleIndex *self,
                                        const gchar *yaml_file,
                                        gboolean strict,
                                        GPtrArray **failures,
                                        GError **error)
{
  return modulemd_module_index_update_from_file_ext (
    self, yaml_file, strict, FALSE, failures, error);
}


gboolean
modulemd_module_index_update_from_string (ModulemdModuleIndex *self,
                                          const gchar *yaml_string,
                                          gboolean strict,
                                          GPtrArray **failures,
                                          GError **error)
{
  if (*failures == NULL)
    {
      *failures = g_ptr_array_new_full (0, g_object_unref);
    }

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  if (!yaml_string)
    {
      g_set_error (
        error, MODULEMD_ERROR, MMD_YAML_ERROR_OPEN, "No string provided");
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
    {
      *failures = g_ptr_array_new_full (0, g_object_unref);
    }

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  if (!yaml_stream)
    {
      g_set_error (
        error, MODULEMD_ERROR, MMD_YAML_ERROR_OPEN, "No stream provided");
      return FALSE;
    }

  MMD_INIT_YAML_PARSER (parser);

  yaml_parser_set_input_file (&parser, yaml_stream);

  return modulemd_module_index_update_from_parser (
    self, &parser, strict, FALSE, failures, error);
}


gboolean
modulemd_module_index_update_from_custom (ModulemdModuleIndex *self,
                                          ModulemdReadHandler custom_read_fn,
                                          void *custom_pvt_data,
                                          gboolean strict,
                                          GPtrArray **failures,
                                          GError **error)
{
  if (*failures == NULL)
    {
      *failures = g_ptr_array_new_full (0, g_object_unref);
    }

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);
  g_return_val_if_fail (custom_read_fn, FALSE);

  MMD_INIT_YAML_PARSER (parser);

  yaml_parser_set_input (&parser, custom_read_fn, custom_pvt_data);

  return modulemd_module_index_update_from_parser (
    self, &parser, strict, FALSE, failures, error);
}


/*
 * modules_from_directory:
 * @path: A directory containing one or more modulemd YAML documents
 * @file_suffix: A file suffix to limit the files to be read. Pass "" if you
 * need to read all files.
 * @strict: Whether to fail on unknown fields
 * @strict_default_streams: Whether to fail on default stream merges.
 * @error: Error return value
 */
static ModulemdModuleIndex *
modules_from_directory (const gchar *path,
                        const gchar *file_suffix,
                        gboolean strict,
                        gboolean strict_default_streams,
                        GError **error)
{
  const gchar *filename = NULL;
  g_autoptr (GDir) dir = NULL;
  g_autofree gchar *filepath = NULL;
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (ModulemdModuleIndex) intermediate = NULL;
  g_autoptr (GPtrArray) failures = NULL;
  g_autoptr (GError) nested_error = NULL;

  index = modulemd_module_index_new ();

  /* Open the directory */
  dir = g_dir_open (path, 0, &nested_error);
  if (!dir)
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }

  while ((filename = g_dir_read_name (dir)) != NULL)
    {
      if (g_str_has_suffix (filename, file_suffix))
        {
          intermediate = modulemd_module_index_new ();

          filepath = g_build_path ("/", path, filename, NULL);
          g_debug ("Reading modulemd from %s", filepath);
          if (!modulemd_module_index_update_from_file (
                intermediate, filepath, strict, &failures, error))
            {
              return FALSE;
            }

          if (!modulemd_module_index_merge (intermediate,
                                            index,
                                            FALSE,
                                            strict_default_streams,
                                            &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }

          g_clear_pointer (&failures, g_ptr_array_unref);
          g_clear_pointer (&filepath, g_free);
          g_clear_object (&intermediate);
        }
    }

  return g_steal_pointer (&index);
}


gboolean
modulemd_module_index_update_from_defaults_directory (
  ModulemdModuleIndex *self,
  const gchar *path,
  gboolean strict,
  const gchar *overrides_path,
  GError **error)
{
  g_autoptr (ModulemdModuleIndex) defaults_idx = NULL;
  g_autoptr (ModulemdModuleIndex) override_idx = NULL;
  g_autoptr (GError) nested_error = NULL;

  /* Read the regular path first */
  defaults_idx = modules_from_directory (
    path, MMD_YAML_SUFFIX, strict, strict, &nested_error);
  if (!defaults_idx)
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }

  /* If an override path was provided, use that too */
  if (overrides_path)
    {
      override_idx = modules_from_directory (
        overrides_path, MMD_YAML_SUFFIX, strict, strict, &nested_error);
      if (!override_idx)
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return FALSE;
        }

      if (!modulemd_module_index_merge (
            override_idx, defaults_idx, TRUE, strict, &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return FALSE;
        }
    }

  /* Now that we've verified that the content in the two paths is compatible,
   * attempt to merge it into the existing index.
   */
  if (!modulemd_module_index_merge (
        defaults_idx, self, TRUE, strict, &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }


  return TRUE;
}


gchar *
modulemd_module_index_dump_to_string (ModulemdModuleIndex *self,
                                      GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), NULL);

  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  if (!modulemd_module_index_dump_to_emitter (self, &emitter, error))
    {
      return NULL;
    }

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


gboolean
modulemd_module_index_dump_to_custom (ModulemdModuleIndex *self,
                                      ModulemdWriteHandler custom_write_fn,
                                      void *custom_pvt_data,
                                      GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  MMD_INIT_YAML_EMITTER (emitter);
  yaml_emitter_set_output (&emitter, custom_write_fn, custom_pvt_data);

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


GPtrArray *
modulemd_module_index_search_streams (ModulemdModuleIndex *self,
                                      const gchar *module_name,
                                      const gchar *stream_name,
                                      const gchar *version,
                                      const gchar *context,
                                      const gchar *arch)
{
  g_autoptr (GPtrArray) module_names = NULL;
  g_autoptr (GPtrArray) module_streams = NULL;
  const gchar *mname = NULL;
  ModulemdModule *module = NULL;

  module_names =
    modulemd_ordered_str_keys (self->modules, modulemd_strcmp_sort);

  module_streams = g_ptr_array_new ();
  for (guint i = 0; i < module_names->len; i++)
    {
      mname = g_ptr_array_index (module_names, i);
      g_debug ("Searching through %s", mname);

      module = modulemd_module_index_get_module (self, mname);
      if (!module)
        {
          /* Since we're iterating through keys we just retrieved, this should
           * be impossible. If we get here, it must be a bug.
           */
          g_assert_not_reached ();
          continue;
        }

      if (!modulemd_fnmatch (module_name,
                             modulemd_module_get_module_name (module)))
        {
          g_debug ("%s did not match %s",
                   modulemd_module_get_module_name (module),
                   module_name);
          continue;
        }

      g_ptr_array_extend_and_steal (
        module_streams,
        modulemd_module_search_streams_by_glob (
          module, stream_name, version, context, arch));
    }

  g_debug ("Module stream count: %d", module_streams->len);

  return g_steal_pointer (&module_streams);
}


GPtrArray *
modulemd_module_index_search_streams_by_nsvca_glob (ModulemdModuleIndex *self,
                                                    const gchar *nsvca_pattern)
{
  g_autoptr (GPtrArray) module_names = NULL;
  g_autoptr (GPtrArray) module_streams = NULL;
  const gchar *mname = NULL;
  ModulemdModule *module = NULL;

  module_names =
    modulemd_ordered_str_keys (self->modules, modulemd_strcmp_sort);

  module_streams = g_ptr_array_new ();
  for (guint i = 0; i < module_names->len; i++)
    {
      mname = g_ptr_array_index (module_names, i);
      g_debug ("Searching through %s", mname);

      module = modulemd_module_index_get_module (self, mname);
      if (!module)
        {
          /* Since we're iterating through keys we just retrieved, this should
           * be impossible. If we get here, it must be a bug.
           */
          g_assert_not_reached ();
          continue;
        }

      g_ptr_array_extend_and_steal (
        module_streams,
        modulemd_module_search_streams_by_nsvca_glob (module, nsvca_pattern));
    }

  g_debug ("Module stream count: %d", module_streams->len);

  return g_steal_pointer (&module_streams);
}


GPtrArray *
modulemd_module_index_search_rpms (ModulemdModuleIndex *self,
                                   const gchar *nevra_pattern)
{
  g_autoptr (GPtrArray) module_names = NULL;
  g_autoptr (GPtrArray) found_streams = NULL;
  GPtrArray *module_streams = NULL;
  const gchar *mname = NULL;
  ModulemdModule *module = NULL;
  ModulemdModuleStream *stream = NULL;

  module_names =
    modulemd_ordered_str_keys (self->modules, modulemd_strcmp_sort);

  found_streams = g_ptr_array_new ();
  for (guint i = 0; i < module_names->len; i++)
    {
      mname = g_ptr_array_index (module_names, i);
      g_debug ("Searching through %s", mname);

      module = modulemd_module_index_get_module (self, mname);
      if (!module)
        {
          /* Since we're iterating through keys we just retrieved, this should
           * be impossible. If we get here, it must be a bug.
           */
          g_assert_not_reached ();
          continue;
        }

      module_streams = modulemd_module_get_all_streams (module);
      for (guint j = 0; j < module_streams->len; j++)
        {
          stream = g_ptr_array_index (module_streams, j);
          if (modulemd_module_stream_includes_nevra (stream, nevra_pattern))
            {
              g_ptr_array_add (found_streams, stream);
            }
        }
    }

  g_debug ("Module stream count: %d", found_streams->len);

  return g_steal_pointer (&found_streams);
}


gboolean
modulemd_module_index_remove_module (ModulemdModuleIndex *self,
                                     const gchar *module_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  return g_hash_table_remove (self->modules, module_name);
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
                   MMD_ERROR_MISSING_REQUIRED,
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
  gpointer key;
  gpointer value;
  g_autoptr (ModulemdModule) module = NULL;
  g_autoptr (GError) nested_error = NULL;

  if (mdversion < self->stream_mdversion)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_UPGRADE,
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
modulemd_module_index_add_obsoletes (ModulemdModuleIndex *self,
                                     ModulemdObsoletes *obsoletes,
                                     GError **error)
{
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE_INDEX (self), FALSE);

  modulemd_module_add_obsoletes (
    get_or_create_module (self,
                          modulemd_obsoletes_get_module_name (obsoletes)),
    obsoletes);
  return TRUE;
}


GHashTable *
modulemd_module_index_get_default_streams_as_hash_table (
  ModulemdModuleIndex *self, const gchar *intent)
{
  GHashTable *defaults = NULL;
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  ModulemdDefaults *defs = NULL;
  const gchar *def_stream_name = NULL;

  defaults = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  g_hash_table_iter_init (&iter, self->modules);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      defs = modulemd_module_get_defaults (MODULEMD_MODULE (value));
      if (defs)
        {
          switch (modulemd_defaults_get_mdversion (defs))
            {
            case MD_DEFAULTS_VERSION_ONE:
              def_stream_name = modulemd_defaults_v1_get_default_stream (
                MODULEMD_DEFAULTS_V1 (defs), intent);
              if (def_stream_name)
                {
                  /* This module has a default stream. Add it to the table */
                  g_hash_table_replace (
                    defaults, g_strdup (key), g_strdup (def_stream_name));
                }
              break;

            default:
              /* This should be impossible and suggests that we somehow added
               * a corrupt defaults object. We will ignore it and continue to
               * return valid entries.
               */
              g_warning ("Encountered an unknown defaults mdversion: %" PRIu64,
                         modulemd_defaults_get_mdversion (defs));
              break;
            }
        }
    }

  return defaults;
}


gboolean
modulemd_module_index_upgrade_defaults (ModulemdModuleIndex *self,
                                        ModulemdDefaultsVersionEnum mdversion,
                                        GError **error)
{
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  g_autoptr (ModulemdModule) module = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;
  ModulemdDefaultsVersionEnum returned_mdversion = MD_DEFAULTS_VERSION_UNSET;
  g_autoptr (GError) nested_error = NULL;

  if (mdversion < self->defaults_mdversion)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_UPGRADE,
                   "Downgrades not permitted. mdversion %i < current %i",
                   mdversion,
                   self->defaults_mdversion);
      return FALSE;
    }

  if (mdversion > MD_DEFAULTS_VERSION_LATEST)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_UPGRADE,
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
        {
          g_clear_object (&module);
          continue;
        }

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
                             gboolean strict_default_streams,
                             GError **error)
{
  MODULEMD_INIT_TRACE ();
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  const gchar *module_name = NULL;
  const gchar *trans_stream = NULL;
  ModulemdModule *module = NULL;
  ModulemdModule *into_module = NULL;
  GPtrArray *streams = NULL;
  ModulemdModuleStream *stream = NULL;
  g_autoptr (GPtrArray) translations = NULL;
  ModulemdTranslation *translation = NULL;
  ModulemdTranslation *current_translation = NULL;
  ModulemdObsoletes *obsoletes = NULL;
  ModulemdDefaults *defaults = NULL;
  ModulemdDefaults *into_defaults = NULL;
  g_autoptr (ModulemdDefaults) merged_defaults = NULL;
  g_autoptr (GError) nested_error = NULL;
  guint i;
  g_autoptr (GPtrArray) translated_stream_names = NULL;
  GPtrArray *obsoletes_array = NULL;
  gchar *translated_stream_name = NULL;
  g_autofree gchar *nsvca = NULL;


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
          nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);

          if (!modulemd_module_index_add_module_stream (
                into, stream, &nested_error))
            {
              g_info ("Could not add stream %s due to %s",
                      nsvca,
                      nested_error->message);
              g_clear_error (&nested_error);
            }
          g_clear_pointer (&nsvca, g_free);
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
          merged_defaults = modulemd_defaults_merge (
            defaults, into_defaults, strict_default_streams, &nested_error);
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

      /* Merge obsoletes for this module */
      g_debug ("Prioritizer: merging obsoletes for %s", module_name);
      obsoletes_array = modulemd_module_get_obsoletes (module);
      for (i = 0; i < obsoletes_array->len; i++)
        {
          obsoletes = g_ptr_array_index (obsoletes_array, i);

          if (obsoletes)
            {
              /* Add obsoletes, overriding if we enounter one with
               * identical module, stream, context and modified time.
               */
              if (!modulemd_module_index_add_obsoletes (
                    into, obsoletes, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }
        }

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
