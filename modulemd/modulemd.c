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

#include "modulemd.h"
#include "config.h"

#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-packager-v3-private.h"

#include <errno.h>
#include <inttypes.h>

const gchar *
modulemd_get_version (void)
{
  return LIBMODULEMD_VERSION;
}


static ModulemdModuleIndex *
verify_load (int ret,
             ModulemdModuleIndex *idx,
             GPtrArray *failures,
             GError **error,
             GError **nested_error);

ModulemdModuleIndex *
modulemd_load_file (const gchar *yaml_file, GError **error)
{
  gboolean ret;
  g_autoptr (ModulemdModuleIndex) idx = NULL;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (GPtrArray) failures = NULL;

  g_return_val_if_fail (yaml_file, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  failures = g_ptr_array_new_with_free_func (g_object_unref);
  idx = modulemd_module_index_new ();

  ret = modulemd_module_index_update_from_file (
    idx, yaml_file, FALSE, &failures, &nested_error);
  return verify_load (ret, idx, failures, error, &nested_error);
}


ModulemdModuleIndex *
modulemd_load_string (const gchar *yaml_string, GError **error)
{
  gboolean ret;
  g_autoptr (ModulemdModuleIndex) idx = NULL;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (GPtrArray) failures = NULL;

  g_return_val_if_fail (yaml_string, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  failures = g_ptr_array_new_with_free_func (g_object_unref);
  idx = modulemd_module_index_new ();

  ret = modulemd_module_index_update_from_string (
    idx, yaml_string, FALSE, &failures, &nested_error);
  return verify_load (ret, idx, failures, error, &nested_error);
}


static ModulemdModuleIndex *
verify_load (gboolean ret,
             ModulemdModuleIndex *idx,
             GPtrArray *failures,
             GError **error,
             GError **nested_error)
{
  if (!ret)
    {
      if (*nested_error)
        {
          g_propagate_error (error, g_steal_pointer (nested_error));
          return NULL;
        }
      else if (failures && failures->len)
        {
          modulemd_subdocument_info_debug_dump_failures (failures);
          g_set_error (error,
                       MODULEMD_ERROR,
                       MMD_ERROR_VALIDATE,
                       "One or more YAML subdocuments were invalid.");
          return NULL;
        }

      /* Should never get here, must be a programming error */
      g_set_error (
        error, MODULEMD_ERROR, MMD_ERROR_VALIDATE, "Unknown internal error.");
      g_return_val_if_reached (NULL);
    }

  return g_object_ref (idx);
}

static GType
modulemd_read_packager_from_parser (yaml_parser_t *parser,
                                    GObject **object,
                                    GError **error);

GType
modulemd_read_packager_file (const gchar *yaml_path,
                             GObject **object,
                             GError **error)
{
  MMD_INIT_YAML_PARSER (parser);
  g_autoptr (FILE) yaml_stream = NULL;
  gint err;

  g_return_val_if_fail (yaml_path, G_TYPE_INVALID);
  g_return_val_if_fail (object, G_TYPE_INVALID);
  g_return_val_if_fail (error == NULL || *error == NULL, G_TYPE_INVALID);

  errno = 0;
  yaml_stream = g_fopen (yaml_path, "rbe");
  err = errno;

  if (!yaml_stream)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_OPEN,
                   "%s",
                   g_strerror (err));
      return G_TYPE_INVALID;
    }

  yaml_parser_set_input_file (&parser, yaml_stream);

  return modulemd_read_packager_from_parser (&parser, object, error);
}

GType
modulemd_read_packager_string (const gchar *yaml_string,
                               GObject **object,
                               GError **error)
{
  MMD_INIT_YAML_PARSER (parser);

  g_return_val_if_fail (yaml_string, G_TYPE_INVALID);
  g_return_val_if_fail (object, G_TYPE_INVALID);
  g_return_val_if_fail (error == NULL || *error == NULL, G_TYPE_INVALID);

  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string, strlen (yaml_string));

  return modulemd_read_packager_from_parser (&parser, object, error);
}

static GType
modulemd_read_packager_from_parser (yaml_parser_t *parser,
                                    GObject **object,
                                    GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (ModulemdModuleStreamV1) stream_v1 = NULL;
  g_autoptr (ModulemdModuleStreamV2) stream_v2 = NULL;
  g_autoptr (ModulemdPackagerV3) packager_v3 = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  ModulemdYamlDocumentTypeEnum doctype;
  const GError *gerror = NULL;
  g_autoptr (GObject) return_object = NULL;
  GType return_type = G_TYPE_INVALID;

  /* The first event must be the stream start */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return G_TYPE_INVALID;
    }
  if (event.type != YAML_STREAM_START_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_PARSE,
                           "YAML didn't begin with STREAM_START.");
      return G_TYPE_INVALID;
    }
  yaml_event_delete (&event);

  /* The second event must be the document start */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return G_TYPE_INVALID;
    }
  if (event.type != YAML_DOCUMENT_START_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_PARSE,
                           "YAML didn't begin with STREAM_START.");
      return G_TYPE_INVALID;
    }
  yaml_event_delete (&event);

  subdoc = modulemd_yaml_parse_document_type (parser);
  gerror = modulemd_subdocument_info_get_gerror (subdoc);
  if (gerror)
    {
      g_set_error (error,
                   gerror->domain,
                   gerror->code,
                   "Parse error identifying document type and version: %s",
                   gerror->message);
      return G_TYPE_INVALID;
    }


  doctype = modulemd_subdocument_info_get_doctype (subdoc);

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
          return G_TYPE_INVALID;
        }

      if (modulemd_subdocument_info_get_mdversion (subdoc) ==
          MD_PACKAGER_VERSION_THREE)
        {
          packager_v3 = modulemd_packager_v3_parse_yaml (subdoc, error);
          if (!packager_v3)
            {
              return G_TYPE_INVALID;
            }

          return_object = (GObject *)g_steal_pointer (&packager_v3);
          return_type = MODULEMD_TYPE_PACKAGER_V3;
          break;
        }

      /* Falling through intentionally: packager V2 format is handled below */

    case MODULEMD_YAML_DOC_MODULESTREAM:
      switch (modulemd_subdocument_info_get_mdversion (subdoc))
        {
        case MD_MODULESTREAM_VERSION_ONE:
          stream_v1 =
            modulemd_module_stream_v1_parse_yaml (subdoc, FALSE, error);
          if (!stream_v1)
            {
              return G_TYPE_INVALID;
            }

          stream_v2 = MODULEMD_MODULE_STREAM_V2 (
            modulemd_module_stream_upgrade_v1_to_v2 (
              MODULEMD_MODULE_STREAM (stream_v1)));
          if (!stream_v2)
            {
              /* This should be impossible, since there are no failure returns
               * from modulemd_module_stream_upgrade_v1_to_v2()
               */
              g_set_error (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_UPGRADE,
                           "Upgrading to v2 failed for an unknown reason");
              return G_TYPE_INVALID;
            }

          return_object = (GObject *)g_steal_pointer (&stream_v2);
          return_type = MODULEMD_TYPE_MODULE_STREAM_V2;
          break;

        case MD_MODULESTREAM_VERSION_TWO:
          stream_v2 = modulemd_module_stream_v2_parse_yaml (
            subdoc, FALSE, doctype == MODULEMD_YAML_DOC_PACKAGER, error);
          if (!stream_v2)
            {
              return G_TYPE_INVALID;
            }

          return_object = (GObject *)g_steal_pointer (&stream_v2);
          return_type = MODULEMD_TYPE_MODULE_STREAM_V2;
          break;

        default:
          g_set_error (error,
                       MODULEMD_YAML_ERROR,
                       MMD_YAML_ERROR_PARSE,
                       "Invalid mdversion (%" PRIu64
                       ") for a modulemd[-stream] document",
                       modulemd_subdocument_info_get_mdversion (subdoc));
          return G_TYPE_INVALID;
        }
      break;

    default:
      g_set_error (
        error,
        MODULEMD_YAML_ERROR,
        MMD_YAML_ERROR_PARSE,
        "Expected `document: modulemd[-stream] or modulemd-packager`, got %d",
        modulemd_subdocument_info_get_doctype (subdoc));
      return G_TYPE_INVALID;
    }

  /* The last event must be the stream end */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return G_TYPE_INVALID;
    }

  if (event.type != YAML_STREAM_END_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_PARSE,
                           "YAML contained more than a single subdocument");
      return G_TYPE_INVALID;
    }
  yaml_event_delete (&event);

  *object = g_steal_pointer (&return_object);
  return return_type;
}
