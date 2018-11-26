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
#include <inttypes.h>
#include "modulemd-module-stream.h"
#include "modulemd-module-stream-v1.h"
#include "modulemd-module-stream-v2.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

typedef struct
{
  gchar *module_name;
  gchar *stream_name;
  guint64 version;
  gchar *context;
} ModulemdModuleStreamPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ModulemdModuleStream,
                                     modulemd_module_stream,
                                     G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_MDVERSION,
  PROP_MODULE_NAME,
  PROP_STREAM_NAME,
  PROP_VERSION,
  PROP_CONTEXT,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdModuleStream *
modulemd_module_stream_new (guint64 mdversion,
                            const gchar *module_name,
                            const gchar *module_stream)
{
  switch (mdversion)
    {
    case MD_MODULESTREAM_VERSION_ONE:
      return MODULEMD_MODULE_STREAM (
        modulemd_module_stream_v1_new (module_name, module_stream));

    case MD_MODULESTREAM_VERSION_TWO:
      return MODULEMD_MODULE_STREAM (
        modulemd_module_stream_v2_new (module_name, module_stream));

    default:
      /* Other versions have not yet been implemented */
      return NULL;
    }
}


static ModulemdModuleStream *
modulemd_module_stream_read_yaml (yaml_parser_t *parser,
                                  const gchar *module_name,
                                  const gchar *module_stream,
                                  GError **error);


ModulemdModuleStream *
modulemd_module_stream_read_file (const gchar *path,
                                  const gchar *module_name,
                                  const gchar *module_stream,
                                  GError **error)
{
  MMD_INIT_YAML_PARSER (parser);
  g_autoptr (FILE) yaml_stream = NULL;
  gint err;

  g_return_val_if_fail (path, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  errno = 0;
  yaml_stream = g_fopen (path, "rb");
  err = errno;

  if (!yaml_stream)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_FILE_ACCESS,
                   "%s",
                   g_strerror (err));
      return NULL;
    }

  yaml_parser_set_input_file (&parser, yaml_stream);

  return modulemd_module_stream_read_yaml (
    &parser, module_name, module_stream, error);
}


ModulemdModuleStream *
modulemd_module_stream_read_string (const gchar *yaml_string,
                                    const gchar *module_name,
                                    const gchar *module_stream,
                                    GError **error)
{
  MMD_INIT_YAML_PARSER (parser);

  g_return_val_if_fail (yaml_string, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  yaml_parser_set_input_string (
    &parser, (const unsigned char *)yaml_string, strlen (yaml_string));

  return modulemd_module_stream_read_yaml (
    &parser, module_name, module_stream, error);
}


ModulemdModuleStream *
modulemd_module_stream_read_stream (FILE *stream,
                                    const gchar *module_name,
                                    const gchar *module_stream,
                                    GError **error)
{
  MMD_INIT_YAML_PARSER (parser);
  yaml_parser_set_input_file (&parser, stream);

  return modulemd_module_stream_read_yaml (
    &parser, module_name, module_stream, error);
}


static ModulemdModuleStream *
modulemd_module_stream_read_yaml (yaml_parser_t *parser,
                                  const gchar *module_name,
                                  const gchar *module_stream,
                                  GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  enum ModulemdYamlDocumentType doctype = MODULEMD_YAML_DOC_UNKNOWN;
  guint64 mdversion = 0;
  g_autofree gchar *data = NULL;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdModuleStream) stream = NULL;

  /* The first event must be the stream start */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return NULL;
    }
  if (event.type != YAML_STREAM_START_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_PARSE,
                           "YAML didn't begin with STREAM_START.");
      return NULL;
    }
  yaml_event_delete (&event);

  if (!modulemd_yaml_parse_document_type (
        parser, &doctype, &mdversion, &data, &nested_error))
    {
      g_propagate_prefixed_error (
        error,
        nested_error,
        "Parse error identifying document type and version: ");
      return NULL;
    }

  if (doctype != MODULEMD_YAML_DOC_MODULESTREAM)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_PARSE,
                   "Expected `document: modulemd`, got %d",
                   doctype);
      return NULL;
    }

  /* TODO: Read mdversion and parse 'data' with the appropriate subclass */
  switch (mdversion)
    {
    case MD_MODULESTREAM_VERSION_ONE:
      /* stream = MODULEMD_MODULESTREAM(modulemd_module_stream_v1_parse_yaml (data)); */
      break;

    case MD_MODULESTREAM_VERSION_TWO:
      /* stream = MODULEMD_MODULESTREAM(modulemd_module_stream_v2_parse_yaml (data)); */
      break;

    default:
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_PARSE,
                   "Unknown ModuleStream version: %" PRIu64,
                   mdversion);
      break;
    }

  /* The last event must be the stream end */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_PARSE,
                           "YAML contained more than a single subdocument");
      return NULL;
    }

  if (event.type != YAML_STREAM_END_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return NULL;
    }
  yaml_event_delete (&event);

  return stream;
}


static void
modulemd_module_stream_finalize (GObject *object)
{
  ModulemdModuleStream *self = (ModulemdModuleStream *)object;
  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  g_clear_pointer (&priv->module_name, g_free);
  g_clear_pointer (&priv->stream_name, g_free);
  g_clear_pointer (&priv->context, g_free);

  G_OBJECT_CLASS (modulemd_module_stream_parent_class)->finalize (object);
}


static ModulemdModuleStream *
modulemd_module_stream_default_copy (ModulemdModuleStream *self,
                                     const gchar *module_name,
                                     const gchar *module_stream)
{
  g_autoptr (ModulemdModuleStream) copy;
  const gchar *module = NULL;
  const gchar *stream = NULL;

  if (!self)
    return NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), NULL);

  /* If the module name was passed in, replace the original with it */
  if (module_name)
    {
      module = module_name;
    }
  else
    {
      module = modulemd_module_stream_get_module_name (self);
    }

  /* If the stream name was passed in, replace the original with it */
  if (module_stream)
    {
      stream = module_stream;
    }
  else
    {
      stream = modulemd_module_stream_get_stream_name (self);
    }

  /* The base implementation of the copy */
  copy = modulemd_module_stream_new (
    modulemd_module_stream_get_mdversion (self), module, stream);

  modulemd_module_stream_set_version (
    copy, modulemd_module_stream_get_version (self));
  modulemd_module_stream_set_context (
    copy, modulemd_module_stream_get_context (self));

  return g_steal_pointer (&copy);
}


ModulemdModuleStream *
modulemd_module_stream_copy (ModulemdModuleStream *self,
                             const gchar *module_name,
                             const gchar *module_stream)
{
  ModulemdModuleStreamClass *klass;

  if (!self)
    return NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), NULL);

  klass = MODULEMD_MODULE_STREAM_GET_CLASS (self);
  g_return_val_if_fail (klass->copy, NULL);

  return klass->copy (self, module_name, module_stream);
}


ModulemdModuleStream *
modulemd_module_stream_upgrade (ModulemdModuleStream *self,
                                guint64 mdversion,
                                GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), NULL);
  g_return_val_if_fail (
    mdversion && mdversion <= MD_MODULESTREAM_VERSION_LATEST, NULL);
  /* TODO */
  return NULL;
}


static gboolean
modulemd_module_stream_default_validate (ModulemdModuleStream *self,
                                         GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), FALSE);

  guint64 mdversion = modulemd_module_stream_get_mdversion (self);
  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  if (mdversion == 0)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MODULEMD_ERROR_VALIDATE,
                           "Metadata version is unset.");
      return FALSE;
    }
  else if (mdversion > MD_MODULESTREAM_VERSION_LATEST)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MODULEMD_ERROR_VALIDATE,
                           "Unknown metadata version.");
      return FALSE;
    }

  if (!priv->module_name ||
      g_str_equal (priv->module_name, MODULESTREAM_PLACEHOLDER))
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MODULEMD_ERROR_VALIDATE,
                           "Module name is unset.");
      return FALSE;
    }

  return TRUE;
}

gboolean
modulemd_module_stream_validate (ModulemdModuleStream *self, GError **error)
{
  ModulemdModuleStreamClass *klass;

  if (!self)
    return FALSE;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), FALSE);

  klass = MODULEMD_MODULE_STREAM_GET_CLASS (self);
  g_return_val_if_fail (klass->validate, FALSE);

  return klass->validate (self, error);
}


guint64
modulemd_module_stream_get_mdversion (ModulemdModuleStream *self)
{
  ModulemdModuleStreamClass *klass;

  if (!self)
    return FALSE;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), 0);

  klass = MODULEMD_MODULE_STREAM_GET_CLASS (self);
  g_return_val_if_fail (klass->get_mdversion, 0);

  return klass->get_mdversion (self);
}


void
modulemd_module_stream_set_module_name (ModulemdModuleStream *self,
                                        const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM (self));

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  g_clear_pointer (&priv->module_name, g_free);
  priv->module_name = g_strdup (module_name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_NAME]);
}


const gchar *
modulemd_module_stream_get_module_name (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), NULL);

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  return priv->module_name;
}


void
modulemd_module_stream_set_stream_name (ModulemdModuleStream *self,
                                        const gchar *stream_name)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM (self));

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  g_clear_pointer (&priv->stream_name, g_free);
  priv->stream_name = g_strdup (stream_name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_NAME]);
}


const gchar *
modulemd_module_stream_get_stream_name (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), NULL);

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  return priv->stream_name;
}


void
modulemd_module_stream_set_version (ModulemdModuleStream *self,
                                    guint64 version)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM (self));

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  priv->version = version;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VERSION]);
}


guint64
modulemd_module_stream_get_version (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), 0);

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  return priv->version;
}


void
modulemd_module_stream_set_context (ModulemdModuleStream *self,
                                    const gchar *context)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM (self));

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  g_clear_pointer (&priv->context, g_free);
  priv->context = g_strdup (context);
  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CONTEXT]);
}


const gchar *
modulemd_module_stream_get_context (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), 0);

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  return priv->context;
}


static void
modulemd_module_stream_get_property (GObject *object,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdModuleStream *self = MODULEMD_MODULE_STREAM (object);

  switch (prop_id)
    {
    case PROP_MDVERSION:
      g_value_set_uint64 (value, modulemd_module_stream_get_mdversion (self));
      break;

    case PROP_MODULE_NAME:
      g_value_set_string (value,
                          modulemd_module_stream_get_module_name (self));
      break;

    case PROP_STREAM_NAME:
      g_value_set_string (value,
                          modulemd_module_stream_get_stream_name (self));
      break;

    case PROP_VERSION:
      g_value_set_uint64 (value, modulemd_module_stream_get_version (self));
      break;

    case PROP_CONTEXT:
      g_value_set_string (value, modulemd_module_stream_get_context (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_module_stream_set_property (GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdModuleStream *self = MODULEMD_MODULE_STREAM (object);

  switch (prop_id)
    {
    case PROP_MODULE_NAME:
      modulemd_module_stream_set_module_name (self,
                                              g_value_get_string (value));
      break;

    case PROP_STREAM_NAME:
      modulemd_module_stream_set_stream_name (self,
                                              g_value_get_string (value));
      break;

    case PROP_VERSION:
      modulemd_module_stream_set_version (self, g_value_get_uint64 (value));
      break;

    case PROP_CONTEXT:
      modulemd_module_stream_set_context (self, g_value_get_string (value));
      break;

    case PROP_MDVERSION:
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_module_stream_class_init (ModulemdModuleStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_module_stream_finalize;
  object_class->get_property = modulemd_module_stream_get_property;
  object_class->set_property = modulemd_module_stream_set_property;

  klass->copy = modulemd_module_stream_default_copy;
  klass->validate = modulemd_module_stream_default_validate;

  /* get_mdversion() must be implemented by the child class */
  klass->get_mdversion = NULL;

  properties[PROP_MDVERSION] = g_param_spec_uint64 (
    "mdversion",
    "Metadata Version",
    "The metadata version of this ModuleStream object. Read-only.",
    0,
    MD_MODULESTREAM_VERSION_LATEST,
    0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  properties[PROP_MODULE_NAME] = g_param_spec_string (
    "module-name",
    "Module Name",
    "The name of the module providing this stream.",
    MODULESTREAM_PLACEHOLDER,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_STREAM_NAME] = g_param_spec_string (
    "stream-name",
    "Stream Name",
    "The name of this module stream.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_VERSION] = g_param_spec_uint64 (
    "version",
    "Module Stream Version",
    "The version of this module stream",
    0,
    G_MAXUINT64,
    0,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_CONTEXT] = g_param_spec_string (
    "context",
    "Module Stream Context",
    "The context of this module stream. Distinguishes between streams with "
    "the same version but different dependencies due to module stream "
    "expansion.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_module_stream_init (ModulemdModuleStream *self)
{
}
