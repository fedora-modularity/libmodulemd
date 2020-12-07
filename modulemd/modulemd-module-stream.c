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

#include "modulemd.h"
#include "modulemd-errors.h"
#include "modulemd-module-index.h"
#include "modulemd-module-stream-v1.h"
#include "modulemd-module-stream-v2.h"
#include "modulemd-module-stream.h"
#include "modulemd-packager-v3.h"
#include "private/glib-extensions.h"
#include "private/modulemd-component-private.h"
#include "private/modulemd-module-private.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include <errno.h>
#include <inttypes.h>

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
  yaml_stream = g_fopen (path, "rbe");
  err = errno;

  if (!yaml_stream)
    {
      g_set_error (
        error, MODULEMD_ERROR, MMD_ERROR_FILE_ACCESS, "%s", g_strerror (err));
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
  g_autoptr (ModulemdPackagerV3) packager_v3 = NULL;
  ModulemdYamlDocumentTypeEnum doctype;
  const GError *gerror = NULL;

  /* The first event must be the stream start */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return NULL;
    }
  if (event.type != YAML_STREAM_START_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_PARSE,
                           "YAML didn't begin with STREAM_START.");
      return NULL;
    }
  yaml_event_delete (&event);

  /* The second event must be the document start */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return NULL;
    }
  if (event.type != YAML_DOCUMENT_START_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_PARSE,
                           "YAML didn't begin with STREAM_START.");
      return NULL;
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
      return NULL;
    }


  doctype = modulemd_subdocument_info_get_doctype (subdoc);

  if (doctype != MODULEMD_YAML_DOC_MODULESTREAM &&
      doctype != MODULEMD_YAML_DOC_PACKAGER)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_PARSE,
                   "Expected `document: modulemd[-packager]`, got %d",
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
      stream = MODULEMD_MODULE_STREAM (modulemd_module_stream_v2_parse_yaml (
        subdoc, strict, doctype == MODULEMD_YAML_DOC_PACKAGER, &nested_error));
      if (!stream)
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return NULL;
        }
      break;

    default:
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_PARSE,
                   "Unknown ModuleStream version: %" PRIu64,
                   modulemd_subdocument_info_get_mdversion (subdoc));
      return NULL;
      break;
    }

  /* The last event must be the stream end */
  if (!yaml_parser_parse (parser, &event))
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_UNPARSEABLE,
                           "Parser error");
      return NULL;
    }

  if (event.type != YAML_STREAM_END_EVENT)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_PARSE,
                           "YAML contained more than a single subdocument");
      return NULL;
    }
  yaml_event_delete (&event);

  if (module_name)
    {
      modulemd_module_stream_set_module_name (stream, module_name);
    }

  if (module_stream)
    {
      modulemd_module_stream_set_stream_name (stream, module_stream);
    }

  if (!modulemd_module_stream_validate (stream, &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

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
  g_clear_object (&priv->translation);

  G_OBJECT_CLASS (modulemd_module_stream_parent_class)->finalize (object);
}


static gboolean
modulemd_module_stream_default_equals (ModulemdModuleStream *self_1,
                                       ModulemdModuleStream *self_2)
{
  if (modulemd_module_stream_get_version (self_1) !=
      modulemd_module_stream_get_version (self_2))
    {
      return FALSE;
    }

  if (g_strcmp0 (modulemd_module_stream_get_module_name (self_1),
                 modulemd_module_stream_get_module_name (self_2)) != 0)
    {
      return FALSE;
    }

  if (g_strcmp0 (modulemd_module_stream_get_stream_name (self_1),
                 modulemd_module_stream_get_stream_name (self_2)) != 0)
    {
      return FALSE;
    }

  if (g_strcmp0 (modulemd_module_stream_get_context (self_1),
                 modulemd_module_stream_get_context (self_2)) != 0)
    {
      return FALSE;
    }

  if (g_strcmp0 (modulemd_module_stream_get_arch (self_1),
                 modulemd_module_stream_get_arch (self_2)) != 0)
    {
      return FALSE;
    }

  return TRUE;
}


gboolean
modulemd_module_stream_equals (ModulemdModuleStream *self_1,
                               ModulemdModuleStream *self_2)
{
  ModulemdModuleStreamClass *klass;

  if (!self_1 && !self_2)
    {
      return TRUE;
    }

  if (!self_1 || !self_2)
    {
      return FALSE;
    }

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self_1), FALSE);
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self_2), FALSE);

  klass = MODULEMD_MODULE_STREAM_GET_CLASS (self_1);
  g_return_val_if_fail (klass->equals, FALSE);

  return klass->equals (self_1, self_2);
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
    {
      return NULL;
    }

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
    {
      return NULL;
    }

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), NULL);

  klass = MODULEMD_MODULE_STREAM_GET_CLASS (self);
  g_return_val_if_fail (klass->copy, NULL);

  return klass->copy (self, module_name, module_stream);
}


static ModulemdModuleStream *
modulemd_module_stream_upgrade_v1_to_v2 (ModulemdModuleStream *from);

ModulemdModuleStream *
modulemd_module_stream_upgrade (ModulemdModuleStream *self,
                                guint64 mdversion,
                                GError **error)
{
  g_autoptr (ModulemdModuleStream) current_stream = NULL;
  g_autoptr (ModulemdModuleStream) updated_stream = NULL;
  g_autoptr (GError) nested_error = NULL;
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
                           MMD_ERROR_UPGRADE,
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
          /* Upgrade from ModuleStreamV1 to ModuleStreamV2 */
          updated_stream =
            modulemd_module_stream_upgrade_v1_to_v2 (current_stream);
          if (!updated_stream)
            {
              /* This should be impossible, since there are no failure returns
               * from modulemd_module_stream_upgrade_v1_to_v2()
               */
              g_set_error (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_UPGRADE,
                           "Upgrading to v2 failed for an unknown reason");
              return NULL;
            }
          break;

        default:
          /* If we get here, it means we failed to address an upgrade. */
          g_set_error (
            error,
            MODULEMD_ERROR,
            MMD_ERROR_UPGRADE,
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

ModulemdModule *
modulemd_module_stream_upgrade_ext (ModulemdModuleStream *self,
                                    guint64 mdversion,
                                    GError **error)
{
  g_autoptr (ModulemdModuleStream) current_stream = NULL;
  g_autoptr (ModulemdModuleStream) updated_stream = NULL;
  g_autoptr (ModulemdModule) current_module = NULL;
  g_autoptr (ModulemdModule) updated_module = NULL;
  g_autoptr (GError) nested_error = NULL;
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
                           MMD_ERROR_UPGRADE,
                           "ModuleStream downgrades are not supported.");
      return NULL;
    }

  if (current_mdversion == mdversion)
    {
      /* If we're already at the requested version, just wrap it in a Module
       * and return that.
       */
      current_module =
        modulemd_module_new (modulemd_module_stream_get_module_name (self));

      if (modulemd_module_add_stream (
            current_module, self, current_mdversion, &nested_error) ==
          MD_MODULESTREAM_VERSION_ERROR)
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return NULL;
        }

      return g_steal_pointer (&current_module);
    }

  current_stream = g_object_ref (self);

  while (current_mdversion != mdversion)
    {
      switch (current_mdversion)
        {
        case 1:
          /* Upgrade from ModuleStreamV1 to ModuleStreamV2 */
          updated_stream =
            modulemd_module_stream_upgrade_v1_to_v2 (current_stream);
          if (!updated_stream)
            {
              /* This should be impossible, since there are no failure returns
               * from modulemd_module_stream_upgrade_v1_to_v2()
               */
              g_set_error (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_UPGRADE,
                           "Upgrading to v2 failed for an unknown reason");
              return NULL;
            }
          g_object_unref (current_stream);
          current_stream = g_steal_pointer (&updated_stream);
          current_mdversion = MD_MODULESTREAM_VERSION_TWO;
          break;

        default:
          /* If we get here, it means we failed to address an upgrade. */
          g_set_error (
            error,
            MODULEMD_ERROR,
            MMD_ERROR_UPGRADE,
            "Cannot upgrade beyond metadata version %" G_GUINT64_FORMAT,
            current_mdversion);
          return NULL;
        }
    }

  /* if that latest upgrade was still a Stream, wrap it in a Module */
  if (current_stream)
    {
      current_module = modulemd_module_new (
        modulemd_module_stream_get_module_name (current_stream));

      if (modulemd_module_add_stream (current_module,
                                      current_stream,
                                      current_mdversion,
                                      &nested_error) ==
          MD_MODULESTREAM_VERSION_ERROR)
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return NULL;
        }

      g_clear_object (&current_stream);
    }

  return g_steal_pointer (&current_module);
}


static ModulemdModuleStream *
modulemd_module_stream_upgrade_v1_to_v2 (ModulemdModuleStream *from)
{
  ModulemdModuleStreamV1 *v1_stream = NULL;
  g_autoptr (ModulemdModuleStreamV2) copy = NULL;
  g_autoptr (ModulemdDependencies) deps = NULL;
  GHashTableIter iter;
  gpointer key;
  gpointer value;

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
    {
      modulemd_module_stream_v2_set_xmd (copy, v1_stream->xmd);
    }


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
                           MMD_ERROR_VALIDATE,
                           "Metadata version is unset.");
      return FALSE;
    }
  if (mdversion > MD_MODULESTREAM_VERSION_LATEST)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Unknown metadata version.");
      return FALSE;
    }

  return TRUE;
}


gboolean
modulemd_module_stream_validate_components (GHashTable *components,
                                            GError **error)
{
  GHashTableIter iter;
  GHashTableIter buildafter_iter;
  gpointer key;
  gpointer value;
  gpointer ba_key;
  gpointer ba_value;
  gboolean has_buildorder = FALSE;
  gboolean has_buildafter = FALSE;

  /* Iterate through components and verify that they don't mix buildorder and
   * buildafter
   */
  g_hash_table_iter_init (&iter, components);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      /* First, ensure that the component validates in general */
      if (!modulemd_component_validate (MODULEMD_COMPONENT (value), error))
        {
          return FALSE;
        }

      /* Record whether we've seen buildorder at least once */
      if (modulemd_component_get_buildorder (MODULEMD_COMPONENT (value)))
        {
          has_buildorder = TRUE;
        }

      /* Record whether we've seen buildafter at least once */
      if (modulemd_component_has_buildafter (MODULEMD_COMPONENT (value)))
        {
          has_buildafter = TRUE;

          /* Verify the all the items listed in buildafter actually appear
           * in this stream
           */
          if (!has_buildorder)
            {
              g_hash_table_iter_init (
                &buildafter_iter,
                modulemd_component_get_buildafter_internal (
                  MODULEMD_COMPONENT (value)));

              while (
                g_hash_table_iter_next (&buildafter_iter, &ba_key, &ba_value))
                {
                  /* Check each entry in the list and confirm that it appears
                   * in the table of components
                   */
                  if (!g_hash_table_contains (components, ba_key))
                    {
                      g_set_error (
                        error,
                        MODULEMD_ERROR,
                        MMD_ERROR_VALIDATE,
                        "Buildafter '%s' not found in components list",
                        (const gchar *)ba_key);
                      return FALSE;
                    }
                }
            }
        }

      /* If both buildorder and buildafter have been seen in this stream, it
       * is invalid.
       */
      if (has_buildafter && has_buildorder)
        {
          g_set_error_literal (
            error,
            MODULEMD_ERROR,
            MMD_ERROR_VALIDATE,
            "Cannot mix buildorder and buildafter in the same stream");
          return FALSE;
        }
    }

  return TRUE;
}


gboolean
modulemd_module_stream_validate_component_rpm_arches (GHashTable *components,
                                                      GStrv module_arches,
                                                      GError **error)
{
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  g_auto (GStrv) rpm_arches = NULL;
  int num_arches;

  /* If no module level arches are provided, there's nothing to check.
   */
  if (g_strv_length (module_arches) == 0)
    {
      return TRUE;
    }

  /* Iterate through rpm components and verify that any arches specified are a
   * subset of the module level arches.
   */
  g_hash_table_iter_init (&iter, components);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      if (!MODULEMD_IS_COMPONENT_RPM (value))
        {
          continue;
        }

      rpm_arches = modulemd_component_rpm_get_arches_as_strv (
        MODULEMD_COMPONENT_RPM (value));

      num_arches = g_strv_length (rpm_arches);
      for (int i = 0; i < num_arches; i++)
        {
          if (!g_strv_contains ((const gchar *const *)module_arches,
                                rpm_arches[i]))
            {
              g_set_error (
                error,
                MODULEMD_ERROR,
                MMD_ERROR_VALIDATE,
                "Component rpm '%s' arch '%s' not in module buildopts.arches",
                modulemd_component_get_name (MODULEMD_COMPONENT (value)),
                rpm_arches[i]);
              return FALSE;
            }
        }

      g_clear_pointer (&rpm_arches, g_strfreev);
    }

  return TRUE;
}


gboolean
modulemd_module_stream_validate (ModulemdModuleStream *self, GError **error)
{
  ModulemdModuleStreamClass *klass;

  if (!self)
    {
      return FALSE;
    }

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
    {
      return FALSE;
    }

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


gchar *
modulemd_module_stream_get_NSVCA_as_string (ModulemdModuleStream *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), 0);

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);
  gchar *nsvca = NULL;
  gchar *endptr = NULL;

  g_autofree gchar *stream = NULL;
  g_autofree gchar *version = NULL;
  g_autofree gchar *context = NULL;
  g_autofree gchar *arch = NULL;

  if (!priv->module_name)
    {
      /* Mandatory field is missing */
      return NULL;
    }

  if (priv->stream_name)
    {
      stream = g_strdup (priv->stream_name);
    }
  else
    {
      stream = g_strdup ("");
    }

  if (priv->version)
    {
      version = g_strdup_printf ("%" PRIu64 "", priv->version);
    }
  else
    {
      version = g_strdup ("");
    }

  if (priv->context)
    {
      context = g_strdup (priv->context);
    }
  else
    {
      context = g_strdup ("");
    }

  if (priv->arch)
    {
      arch = g_strdup (priv->arch);
    }
  else
    {
      arch = g_strdup ("");
    }


  nsvca =
    g_strjoin (":", priv->module_name, stream, version, context, arch, NULL);

  /* Remove any trailing colons */
  endptr = nsvca + strlen (nsvca) - 1;
  while (*endptr == ':' && endptr > nsvca)
    {
      *endptr = '\0';
      endptr--;
    }

  return nsvca;
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

  klass->equals = modulemd_module_stream_default_equals;
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
    "Module Stream Architecture",
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
    {
      priv->translation = g_object_ref (translation);
    }
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
    {
      return NULL;
    }

  if (g_str_equal (locale, "C"))
    {
      return NULL;
    }

  ModulemdModuleStreamPrivate *priv =
    modulemd_module_stream_get_instance_private (self);

  if (priv->translation == NULL)
    {
      return NULL;
    }

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
    {
      return FALSE;
    }

  /* Start data: */
  EMIT_MAPPING_START (emitter, error);

  if (modulemd_module_stream_get_module_name (self) != NULL &&
      !modulemd_module_stream_is_autogen_module_name (self))
    {
      EMIT_KEY_VALUE (
        emitter, error, "name", modulemd_module_stream_get_module_name (self));
    }

  /* Always emit the stream quoted, since a purely numeric-looking stream such
   * as 5.30 might otherwise be interpreted by parsers like pyyaml as a number
   * and result in being read (and written) as '5.3'.
   */
  if (modulemd_module_stream_get_stream_name (self) != NULL &&
      !modulemd_module_stream_is_autogen_stream_name (self))
    {
      EMIT_KEY_VALUE_FULL (emitter,
                           error,
                           "stream",
                           modulemd_module_stream_get_stream_name (self),
                           YAML_DOUBLE_QUOTED_SCALAR_STYLE);
    }

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


gboolean
modulemd_module_stream_includes_nevra (ModulemdModuleStream *self,
                                       const gchar *nevra_pattern)
{
  ModulemdModuleStreamVersionEnum version;
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (self), FALSE);

  version = modulemd_module_stream_get_mdversion (self);
  switch (version)
    {
    case MD_MODULESTREAM_VERSION_ONE:
      return modulemd_module_stream_v1_includes_nevra (
        MODULEMD_MODULE_STREAM_V1 (self), nevra_pattern);
      break;

    case MD_MODULESTREAM_VERSION_TWO:
      return modulemd_module_stream_v2_includes_nevra (
        MODULEMD_MODULE_STREAM_V2 (self), nevra_pattern);
      break;

    default:
      /* We should never reach here */
      g_return_val_if_reached (FALSE);
      break;
    }

  return FALSE;
}

void
modulemd_module_stream_set_autogen_module_name (ModulemdModuleStream *self,
                                                guint id)
{
  g_autofree gchar *name = NULL;

  if (!modulemd_module_stream_get_module_name (self))
    {
      name = g_strdup_printf ("__unnamed_module_%u", id);
      modulemd_module_stream_set_module_name (self, name);
      g_clear_pointer (&name, g_free);
    }
}

void
modulemd_module_stream_set_autogen_stream_name (ModulemdModuleStream *self,
                                                guint id)
{
  g_autofree gchar *name = NULL;

  if (!modulemd_module_stream_get_stream_name (self))
    {
      name = g_strdup_printf ("__unnamed_stream_%u", id);
      modulemd_module_stream_set_stream_name (self, name);
      g_clear_pointer (&name, g_free);
    }
}

gboolean
modulemd_module_stream_is_autogen_module_name (ModulemdModuleStream *self)
{
  const gchar *name = NULL;

  name = modulemd_module_stream_get_module_name (self);
  if (name && g_str_has_prefix (name, "__unnamed_module_"))
    {
      return TRUE;
    }

  return FALSE;
}

gboolean
modulemd_module_stream_is_autogen_stream_name (ModulemdModuleStream *self)
{
  const gchar *name = NULL;

  name = modulemd_module_stream_get_stream_name (self);
  if (name && g_str_has_prefix (name, "__unnamed_stream_"))
    {
      return TRUE;
    }

  return FALSE;
}

void
modulemd_module_stream_clear_autogen_module_name (ModulemdModuleStream *self)
{
  if (modulemd_module_stream_is_autogen_module_name (self))
    {
      modulemd_module_stream_set_module_name (self, NULL);
    }
}

void
modulemd_module_stream_clear_autogen_stream_name (ModulemdModuleStream *self)
{
  if (modulemd_module_stream_is_autogen_stream_name (self))
    {
      modulemd_module_stream_set_stream_name (self, NULL);
    }
}
