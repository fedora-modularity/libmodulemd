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
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

typedef struct
{
  gchar *module_name;
  gchar *stream_name;
  guint64 version;
  gchar *context;
  gchar *arch;
  ModulemdTranslation *translation;
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
  PROP_ARCH,
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
                                  gboolean strict,
                                  GError **error);


ModulemdModuleStream *
modulemd_module_stream_read_file (const gchar *path,
                                  gboolean strict,
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
    &parser, module_name, module_stream, strict, error);
}


ModulemdModuleStream *
modulemd_module_stream_read_string (const gchar *yaml_string,
                                    gboolean strict,
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
    &parser, module_name, module_stream, strict, error);
}


ModulemdModuleStream *
modulemd_module_stream_read_stream (FILE *stream,
                                    gboolean strict,
                                    const gchar *module_name,
                                    const gchar *module_stream,
                                    GError **error)
{
  MMD_INIT_YAML_PARSER (parser);
  yaml_parser_set_input_file (&parser, stream);

  return modulemd_module_stream_read_yaml (
    &parser, module_name, module_stream, strict, error);
}


static ModulemdModuleStream *
modulemd_module_stream_read_yaml (yaml_parser_t *parser,
                                  const gchar *module_name,
                                  const gchar *module_stream,
                                  gboolean strict,
                                  GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_PARSER (stream_parser);
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;

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

  /* The second event must be the document start */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return NULL;
    }
  if (event.type != YAML_DOCUMENT_START_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_PARSE,
                           "YAML didn't begin with STREAM_START.");
      return NULL;
    }
  yaml_event_delete (&event);

  subdoc = modulemd_yaml_parse_document_type (parser);
  if (subdoc == NULL)
    {
      g_propagate_prefixed_error (
        error,
        g_steal_pointer (&nested_error),
        "Parse error identifying document type and version: ");
      return NULL;
    }

  if (modulemd_subdocument_info_get_doctype (subdoc) !=
      MODULEMD_YAML_DOC_MODULESTREAM)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_PARSE,
                   "Expected `document: modulemd`, got %d",
                   modulemd_subdocument_info_get_doctype (subdoc));
      return NULL;
    }

  /* Read mdversion and parse 'data' with the appropriate subclass */
  switch (modulemd_subdocument_info_get_mdversion (subdoc))
    {
    case MD_MODULESTREAM_VERSION_ONE:
      stream = MODULEMD_MODULE_STREAM (
        modulemd_module_stream_v1_parse_yaml (subdoc, strict, &nested_error));
      if (!stream)
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return NULL;
        }
      break;

    case MD_MODULESTREAM_VERSION_TWO:
      stream = MODULEMD_MODULE_STREAM (
        modulemd_module_stream_v2_parse_yaml (subdoc, strict, &nested_error));
      if (!stream)
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return NULL;
        }
      break;

    default:
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_PARSE,
                   "Unknown ModuleStream version: %" PRIu64,
                   modulemd_subdocument_info_get_mdversion (subdoc));
      break;
    }

  /* The last event must be the stream end */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return NULL;
    }

  if (event.type != YAML_STREAM_END_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MODULEMD_YAML_ERROR_PARSE,
                           "YAML contained more than a single subdocument");
      return NULL;
    }
  yaml_event_delete (&event);

  return g_steal_pointer (&stream);
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
  g_clear_pointer (&priv->arch, g_free);
  g_clear_pointer (&priv->translation, g_object_unref);

  G_OBJECT_CLASS (modulemd_module_stream_parent_class)->finalize (object);
}


static ModulemdModuleStream *
modulemd_module_stream_default_copy (ModulemdModuleStream *self,
                                     const gchar *module_name,
                                     const gchar *module_stream)
{
  g_autoptr (ModulemdModuleStream) copy = NULL;
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
  modulemd_module_stream_associate_translation (
    copy, modulemd_module_stream_get_translation (self));

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


static ModulemdModuleStream *
modulemd_module_stream_upgrade_to_v2 (ModulemdModuleStream *from,
                                      GError **error);

ModulemdModuleStream *
modulemd_module_stream_upgrade (ModulemdModuleStream *self,
                                guint64 mdversion,
                                GError **error)
{
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdModuleStream) current_stream = NULL;
  g_autoptr (ModulemdModuleStream) updated_stream = NULL;
  guint64 current_mdversion = modulemd_module_stream_get_mdversion (self);

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), NULL);

  if (!mdversion)
    {
      /* If target mdversion is unspecified, set it to the latest */
      mdversion = MD_MODULESTREAM_VERSION_LATEST;
    }

  if (mdversion < current_mdversion)
    {
      /* Downgrades are not supported */
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MODULEMD_ERROR_UPGRADE,
                           "ModuleStream downgrades are not supported.");
      return NULL;
    }

  if (current_mdversion == mdversion)
    {
      /* If we're already on the requested version, just make a copy */
      return modulemd_module_stream_copy (self, NULL, NULL);
    }

  current_stream = g_object_ref (self);

  while (current_mdversion != mdversion)
    {
      switch (current_mdversion)
        {
        case 1:
          /* Upgrade to ModuleStreamV2 */
          updated_stream = modulemd_module_stream_upgrade_to_v2 (
            current_stream, &nested_error);
          if (!updated_stream)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return NULL;
            }
          break;

        default:
          /* If we get here, it means we failed to address an upgrade. */
          g_set_error (
            error,
            MODULEMD_ERROR,
            MODULEMD_ERROR_UPGRADE,
            "Cannot upgrade beyond metadata version %" G_GUINT64_FORMAT,
            current_mdversion);
          return NULL;
        }

      g_object_unref (current_stream);
      current_stream = g_steal_pointer (&updated_stream);
      current_mdversion =
        modulemd_module_stream_get_mdversion (current_stream);
    }

  return g_steal_pointer (&current_stream);
}


static ModulemdModuleStream *
modulemd_module_stream_upgrade_to_v2 (ModulemdModuleStream *from,
                                      GError **error)
{
  ModulemdModuleStreamV1 *v1_stream = NULL;
  g_autoptr (ModulemdModuleStreamV2) copy = NULL;
  g_autoptr (ModulemdDependencies) deps = NULL;
  GHashTableIter iter;
  gpointer key, value;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (from), NULL);
  v1_stream = MODULEMD_MODULE_STREAM_V1 (from);

  copy = modulemd_module_stream_v2_new (
    modulemd_module_stream_get_module_name (from),
    modulemd_module_stream_get_stream_name (from));


  /* Parent class copy */
  modulemd_module_stream_set_version (
    MODULEMD_MODULE_STREAM (copy), modulemd_module_stream_get_version (from));
  modulemd_module_stream_set_context (
    MODULEMD_MODULE_STREAM (copy), modulemd_module_stream_get_context (from));
  modulemd_module_stream_associate_translation (
    MODULEMD_MODULE_STREAM (copy),
    modulemd_module_stream_get_translation (from));

  /* Copy all attributes that are the same as V1 */

  /* Properties */
  STREAM_UPGRADE_IF_SET (v1, v2, copy, v1_stream, arch);
  STREAM_UPGRADE_IF_SET (v1, v2, copy, v1_stream, buildopts);
  STREAM_UPGRADE_IF_SET (v1, v2, copy, v1_stream, community);
  STREAM_UPGRADE_IF_SET_WITH_LOCALE (v1, v2, copy, v1_stream, description);
  STREAM_UPGRADE_IF_SET (v1, v2, copy, v1_stream, documentation);
  STREAM_UPGRADE_IF_SET_WITH_LOCALE (v1, v2, copy, v1_stream, summary);
  STREAM_UPGRADE_IF_SET (v1, v2, copy, v1_stream, tracker);

  /* Internal Data Structures: With replace function */
  STREAM_REPLACE_HASHTABLE (v2, copy, v1_stream, content_licenses);
  STREAM_REPLACE_HASHTABLE (v2, copy, v1_stream, module_licenses);
  STREAM_REPLACE_HASHTABLE (v2, copy, v1_stream, rpm_api);
  STREAM_REPLACE_HASHTABLE (v2, copy, v1_stream, rpm_artifacts);
  STREAM_REPLACE_HASHTABLE (v2, copy, v1_stream, rpm_filters);

  /* Internal Data Structures: With add on value */
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v1_stream, rpm_components, modulemd_module_stream_v2_add_component);
  COPY_HASHTABLE_BY_VALUE_ADDER (copy,
                                 v1_stream,
                                 module_components,
                                 modulemd_module_stream_v2_add_component);
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v1_stream, profiles, modulemd_module_stream_v2_add_profile);
  COPY_HASHTABLE_BY_VALUE_ADDER (copy,
                                 v1_stream,
                                 servicelevels,
                                 modulemd_module_stream_v2_add_servicelevel);


  if (v1_stream->xmd != NULL)
    modulemd_module_stream_v2_set_xmd (copy, g_variant_ref (v1_stream->xmd));


  /* Upgrade the Dependencies */
  if (g_hash_table_size (v1_stream->buildtime_deps) > 0 ||
      g_hash_table_size (v1_stream->runtime_deps) > 0)
    {
      deps = modulemd_dependencies_new ();

      /* Add the build-time deps */
      g_hash_table_iter_init (&iter, v1_stream->buildtime_deps);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          modulemd_dependencies_add_buildtime_stream (deps, key, value);
        }

      /* Add the run-time deps */
      g_hash_table_iter_init (&iter, v1_stream->runtime_deps);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          modulemd_dependencies_add_runtime_stream (deps, key, value);
        }

      /* Add the Dependencies to this ModuleStreamV2 */
      modulemd_module_stream_v2_add_dependencies (copy, deps);
    }

  return MODULEMD_MODULE_STREAM (g_steal_pointer (&copy));
}


static gboolean
modulemd_module_stream_default_validate (ModulemdModuleStream *self,
                                         GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), FALSE);

  guint64 mdversion = modulemd_module_stream_get_mdversion (self);

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


void
modulemd_module_stream_set_arch (ModulemdModuleStream *self, const gchar *arch)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM (self));

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  g_clear_pointer (&priv->arch, g_free);
  priv->arch = g_strdup (arch);
  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CONTEXT]);
}


const gchar *
modulemd_module_stream_get_arch (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), 0);

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  return priv->arch;
}


gchar *
modulemd_module_stream_get_nsvc_as_string (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), 0);

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  gchar *nsvc = NULL;

  if (!priv->module_name || !priv->stream_name)
    {
      /* Mandatory field is missing */
      return NULL;
    }

  if (priv->context)
    {
      nsvc = g_strdup_printf ("%s:%s:%" PRIu64 ":%s",
                              priv->module_name,
                              priv->stream_name,
                              priv->version,
                              priv->context);
    }
  else
    {
      nsvc = g_strdup_printf (
        "%s:%s:%" PRIu64, priv->module_name, priv->stream_name, priv->version);
    }

  return nsvc;
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

    case PROP_ARCH:
      g_value_set_string (value, modulemd_module_stream_get_arch (self));
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

    case PROP_ARCH:
      modulemd_module_stream_set_arch (self, g_value_get_string (value));
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
    NULL,
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

  properties[PROP_ARCH] = g_param_spec_string (
    "arch",
    "Module Stream Architetcture",
    "The processor architecture of this module stream.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_module_stream_init (ModulemdModuleStream *self)
{
}


void
modulemd_module_stream_associate_translation (ModulemdModuleStream *self,
                                              ModulemdTranslation *translation)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM (self));

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  g_clear_pointer (&priv->translation, g_object_unref);
  if (translation != NULL)
    priv->translation = g_object_ref (translation);
}


ModulemdTranslation *
modulemd_module_stream_get_translation (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), NULL);

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  return priv->translation;
}


ModulemdTranslationEntry *
modulemd_module_stream_get_translation_entry (ModulemdModuleStream *self,
                                              const gchar *locale)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), NULL);

  if (locale == NULL)
    return NULL;

  if (g_str_equal (locale, "C"))
    return NULL;

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  if (priv->translation == NULL)
    return NULL;

  return modulemd_translation_get_translation_entry (priv->translation,
                                                     locale);
}


gboolean
modulemd_module_stream_emit_yaml_base (ModulemdModuleStream *self,
                                       yaml_emitter_t *emitter,
                                       GError **error)
{
  MODULEMD_INIT_TRACE ();
  g_autofree gchar *version_string = NULL;

  if (modulemd_module_stream_get_version (self) != 0)
    {
      version_string = g_strdup_printf (
        "%" PRIu64, modulemd_module_stream_get_version (self));
    }

  /* Emit document headers */
  if (!modulemd_yaml_emit_document_headers (
        emitter,
        MODULEMD_YAML_DOC_MODULESTREAM,
        modulemd_module_stream_get_mdversion (self),
        error))
    return FALSE;

  /* Start data: */
  EMIT_MAPPING_START (emitter, error);

  EMIT_KEY_VALUE_IF_SET (
    emitter, error, "name", modulemd_module_stream_get_module_name (self));
  EMIT_KEY_VALUE_IF_SET (
    emitter, error, "stream", modulemd_module_stream_get_stream_name (self));
  EMIT_KEY_VALUE_IF_SET (emitter, error, "version", version_string);
  EMIT_KEY_VALUE_IF_SET (
    emitter, error, "context", modulemd_module_stream_get_context (self));

  /* The rest of the fields will be emitted by the version-specific emitters */
  return TRUE;
}


gboolean
modulemd_module_stream_depends_on_stream (ModulemdModuleStream *self,
                                          const gchar *module_name,
                                          const gchar *stream_name)
{
  ModulemdModuleStreamClass *klass;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), FALSE);

  klass = MODULEMD_MODULE_STREAM_GET_CLASS (self);
  g_return_val_if_fail (klass->depends_on_stream, FALSE);

  return klass->depends_on_stream (self, module_name, stream_name);
}


gboolean
modulemd_module_stream_build_depends_on_stream (ModulemdModuleStream *self,
                                                const gchar *module_name,
                                                const gchar *stream_name)
{
  ModulemdModuleStreamClass *klass;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), FALSE);

  klass = MODULEMD_MODULE_STREAM_GET_CLASS (self);
  g_return_val_if_fail (klass->build_depends_on_stream, FALSE);

  return klass->build_depends_on_stream (self, module_name, stream_name);
}
