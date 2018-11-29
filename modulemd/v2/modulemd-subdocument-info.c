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

#include "modulemd-subdocument-info.h"
#include "private/glib-extensions.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define SD_DEFAULT_STRING "__SUBDOCUMENT_INFO_VALUE_UNSET__"

struct _ModulemdSubdocumentInfo
{
  GObject parent_instance;

  enum ModulemdYamlDocumentType doctype;
  guint64 mdversion;
  GError *error;
  gchar *contents;
};

G_DEFINE_TYPE (ModulemdSubdocumentInfo,
               modulemd_subdocument_info,
               G_TYPE_OBJECT)


ModulemdSubdocumentInfo *
modulemd_subdocument_info_new (void)
{
  return g_object_new (MODULEMD_TYPE_SUBDOCUMENT_INFO, NULL);
}


ModulemdSubdocumentInfo *
modulemd_subdocument_info_copy (ModulemdSubdocumentInfo *self)
{
  g_autoptr (ModulemdSubdocumentInfo) s = NULL;
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self), NULL);

  s = modulemd_subdocument_info_new ();
  modulemd_subdocument_info_set_doctype (
    s, modulemd_subdocument_info_get_doctype (self));
  modulemd_subdocument_info_set_mdversion (
    s, modulemd_subdocument_info_get_mdversion (self));
  modulemd_subdocument_info_set_gerror (
    s, modulemd_subdocument_info_get_gerror (self));
  modulemd_subdocument_info_set_yaml (
    s, modulemd_subdocument_info_get_yaml (self));

  return g_steal_pointer (&s);
}


static void
modulemd_subdocument_info_finalize (GObject *object)
{
  ModulemdSubdocumentInfo *self = (ModulemdSubdocumentInfo *)object;

  g_clear_pointer (&self->error, g_error_free);
  g_clear_pointer (&self->contents, g_free);

  G_OBJECT_CLASS (modulemd_subdocument_info_parent_class)->finalize (object);
}


void
modulemd_subdocument_info_set_yaml (ModulemdSubdocumentInfo *self,
                                    const gchar *yaml)
{
  g_return_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self));

  printf ("Setting YAML: %s\n", yaml);

  g_clear_pointer (&self->contents, g_free);
  self->contents = g_strdup (yaml);
}


const gchar *
modulemd_subdocument_info_get_yaml (ModulemdSubdocumentInfo *self)
{
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self), NULL);

  return self->contents;
}


void
modulemd_subdocument_info_set_gerror (ModulemdSubdocumentInfo *self,
                                      const GError *error)
{
  g_return_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self));

  g_clear_pointer (&self->error, g_error_free);
  if (error)
    {
      self->error = g_error_copy (error);
    }
  else
    {
      self->error = NULL;
    }
}


const GError *
modulemd_subdocument_info_get_gerror (ModulemdSubdocumentInfo *self)
{
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self), NULL);

  return self->error;
}


void
modulemd_subdocument_info_set_doctype (ModulemdSubdocumentInfo *self,
                                       enum ModulemdYamlDocumentType doctype)
{
  g_return_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self));

  self->doctype = doctype;
}


enum ModulemdYamlDocumentType
modulemd_subdocument_info_get_doctype (ModulemdSubdocumentInfo *self)
{
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self),
                        MODULEMD_YAML_DOC_UNKNOWN);

  return self->doctype;
}


void
modulemd_subdocument_info_set_mdversion (ModulemdSubdocumentInfo *self,
                                         guint64 mdversion)
{
  g_return_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self));

  self->mdversion = mdversion;
}


guint64
modulemd_subdocument_info_get_mdversion (ModulemdSubdocumentInfo *self)
{
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self), 0);

  return self->mdversion;
}


gboolean
modulemd_subdocument_info_get_data_parser (ModulemdSubdocumentInfo *self,
                                           yaml_parser_t *parser,
                                           GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT_INFO (self), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  MMD_INIT_YAML_EVENT (event);
  MODULEMD_INIT_TRACE ();
  gsize depth = 0;

  yaml_parser_set_input_string (
    parser, (const unsigned char *)self->contents, strlen (self->contents));

  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_STREAM_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Subdocument did not begin with a STREAM_START.");
    }
  yaml_event_delete (&event);

  /* The second event must be the document start */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_DOCUMENT_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Subdocument did not begin with a DOCUMENT_START.");
    }
  yaml_event_delete (&event);

  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Subdocument did not begin with a MAPPING_START.");
    }
  yaml_event_delete (&event);

  while (TRUE)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
      if (event.type == 0)
        {
          g_set_error (error,
                       MODULEMD_YAML_ERROR,
                       MODULEMD_YAML_ERROR_UNPARSEABLE,
                       "Unexpected end while waiting for data");
          return FALSE;
        }

      switch (event.type)
        {
        case YAML_SCALAR_EVENT:
          if (depth == 0 && g_str_equal (event.data.scalar.value, "data"))
            {
              /* We have arrived at the "data". Return. */
              return TRUE;
            }
          break;

        case YAML_SEQUENCE_START_EVENT:
        case YAML_MAPPING_START_EVENT: depth++; break;

        case YAML_SEQUENCE_END_EVENT:
        case YAML_MAPPING_END_EVENT:
          depth--;
          /* Fall through intentional */

        default:
          if (depth == 0)
            {
              g_set_error (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_UNPARSEABLE,
                           "Unexpected event while waiting for data");
              return FALSE;
            }
          break;
        }
      yaml_event_delete (&event);
    }

  return FALSE;
}


static void
modulemd_subdocument_info_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
  // ModulemdSubdocumentInfo *self = MODULEMD_SUBDOCUMENT_INFO (object);

  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_subdocument_info_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
  // ModulemdSubdocumentInfo *self = MODULEMD_SUBDOCUMENT_INFO (object);

  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_subdocument_info_class_init (ModulemdSubdocumentInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_subdocument_info_finalize;
  object_class->get_property = modulemd_subdocument_info_get_property;
  object_class->set_property = modulemd_subdocument_info_set_property;
}


static void
modulemd_subdocument_info_init (ModulemdSubdocumentInfo *self)
{
  /* Nothing to init */
}
