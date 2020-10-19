/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat, Inc.
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#include "modulemd-2.0/modulemd-buildopts.h"
#include "modulemd-2.0/modulemd-errors.h"
#include "modulemd-2.0/modulemd-module-stream.h"

#include "private/modulemd-build-config.h"
#include "private/modulemd-buildopts-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdBuildConfig
{
  GObject parent_instance;

  gchar *context;
  gchar *platform;
  GHashTable *requires; /* hashtable<string, string> */
  GHashTable *buildrequires; /* hashtable<string, string> */
  ModulemdBuildopts *buildopts;
};

G_DEFINE_TYPE (ModulemdBuildConfig, modulemd_build_config, G_TYPE_OBJECT)

ModulemdBuildConfig *
modulemd_build_config_new (void)
{
  return g_object_new (MODULEMD_TYPE_BUILD_CONFIG, NULL);
}

static void
modulemd_build_config_finalize (GObject *object)
{
  ModulemdBuildConfig *self = (ModulemdBuildConfig *)object;

  g_clear_pointer (&self->context, g_free);
  g_clear_pointer (&self->platform, g_free);
  g_clear_pointer (&self->requires, g_hash_table_unref);
  g_clear_pointer (&self->buildrequires, g_hash_table_unref);
  g_clear_object (&self->buildopts);

  G_OBJECT_CLASS (modulemd_build_config_parent_class)->finalize (object);
}

static void
modulemd_build_config_class_init (ModulemdBuildConfigClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_build_config_finalize;
}

static void
modulemd_build_config_init (ModulemdBuildConfig *self)
{
  self->requires =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  self->buildrequires =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}


void
modulemd_build_config_set_context (ModulemdBuildConfig *self,
                                   const gchar *context)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));

  g_clear_pointer (&self->context, g_free);

  if (context)
    {
      self->context = g_strdup (context);
    }
}


const gchar *
modulemd_build_config_get_context (ModulemdBuildConfig *self)
{
  g_return_val_if_fail (MODULEMD_IS_BUILD_CONFIG (self), NULL);

  return self->context;
}


void
modulemd_build_config_set_platform (ModulemdBuildConfig *self,
                                    const gchar *platform)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));

  g_clear_pointer (&self->platform, g_free);

  if (platform)
    {
      self->platform = g_strdup (platform);
    }
}


const gchar *
modulemd_build_config_get_platform (ModulemdBuildConfig *self)
{
  g_return_val_if_fail (MODULEMD_IS_BUILD_CONFIG (self), NULL);

  return self->platform;
}


void
modulemd_build_config_add_runtime_requirement (ModulemdBuildConfig *self,
                                               const gchar *module_name,
                                               const gchar *stream_name)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));
  g_return_if_fail (module_name && stream_name);

  g_hash_table_replace (
    self->requires, g_strdup (module_name), g_strdup (stream_name));
}


void
modulemd_build_config_remove_runtime_requirement (ModulemdBuildConfig *self,
                                                  const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));
  g_return_if_fail (module_name);

  g_hash_table_remove (self->requires, module_name);
}


void
modulemd_build_config_clear_runtime_requirements (ModulemdBuildConfig *self)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));

  g_hash_table_remove_all (self->requires);
}


const gchar *
modulemd_build_config_get_runtime_requirement_stream (
  ModulemdBuildConfig *self, const gchar *module_name)
{
  g_return_val_if_fail (MODULEMD_IS_BUILD_CONFIG (self), NULL);

  return g_hash_table_lookup (self->requires, module_name);
}


GStrv
modulemd_build_config_get_runtime_modules_as_strv (ModulemdBuildConfig *self)
{
  g_return_val_if_fail (MODULEMD_IS_BUILD_CONFIG (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->requires);
}


void
modulemd_build_config_add_buildtime_requirement (ModulemdBuildConfig *self,
                                                 const gchar *module_name,
                                                 const gchar *stream_name)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));
  g_return_if_fail (module_name && stream_name);

  g_hash_table_replace (
    self->buildrequires, g_strdup (module_name), g_strdup (stream_name));
}


void
modulemd_build_config_remove_buildtime_requirement (ModulemdBuildConfig *self,
                                                    const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));
  g_return_if_fail (module_name);

  g_hash_table_remove (self->buildrequires, module_name);
}


void
modulemd_build_config_clear_buildtime_requirements (ModulemdBuildConfig *self)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));

  g_hash_table_remove_all (self->buildrequires);
}


const gchar *
modulemd_build_config_get_buildtime_requirement_stream (
  ModulemdBuildConfig *self, const gchar *module_name)
{
  g_return_val_if_fail (MODULEMD_IS_BUILD_CONFIG (self), NULL);

  return g_hash_table_lookup (self->buildrequires, module_name);
}


GStrv
modulemd_build_config_get_buildtime_modules_as_strv (ModulemdBuildConfig *self)
{
  g_return_val_if_fail (MODULEMD_IS_BUILD_CONFIG (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->buildrequires);
}


void
modulemd_build_config_set_buildopts (ModulemdBuildConfig *self,
                                     ModulemdBuildopts *buildopts)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));

  g_clear_object (&self->buildopts);
  if (buildopts)
    {
      self->buildopts = modulemd_buildopts_copy (buildopts);
    }
}


ModulemdBuildopts *
modulemd_build_config_get_buildopts (ModulemdBuildConfig *self)
{
  g_return_val_if_fail (MODULEMD_IS_BUILD_CONFIG (self), NULL);

  return self->buildopts;
}


static void
modulemd_build_config_replace_runtime_deps (ModulemdBuildConfig *self,
                                            GHashTable *deps);
static void
modulemd_build_config_replace_buildtime_deps (ModulemdBuildConfig *self,
                                              GHashTable *deps);


ModulemdBuildConfig *
modulemd_build_config_parse_yaml (yaml_parser_t *parser,
                                  gboolean strict,
                                  GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (ModulemdBuildConfig) buildconfig = NULL;
  g_autoptr (ModulemdBuildopts) buildopts = NULL;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (GHashTable) deptable = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  buildconfig = modulemd_build_config_new ();

  /* Read in attributes of this config */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          if (g_str_equal (event.data.scalar.value, "context"))
            MMD_SET_PARSED_YAML_STRING (
              parser, error, modulemd_build_config_set_context, buildconfig);

          else if (g_str_equal (event.data.scalar.value, "platform"))
            MMD_SET_PARSED_YAML_STRING (
              parser, error, modulemd_build_config_set_platform, buildconfig);

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "buildrequires"))
            {
              deptable =
                modulemd_yaml_parse_string_string_map (parser, &nested_error);
              if (!deptable)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
              modulemd_build_config_replace_buildtime_deps (buildconfig,
                                                            deptable);
              g_clear_pointer (&deptable, g_hash_table_unref);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "requires"))
            {
              deptable =
                modulemd_yaml_parse_string_string_map (parser, &nested_error);
              if (!deptable)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
              modulemd_build_config_replace_runtime_deps (buildconfig,
                                                          deptable);
              g_clear_pointer (&deptable, g_hash_table_unref);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "buildopts"))
            {
              buildopts =
                modulemd_buildopts_parse_yaml (parser, strict, &nested_error);
              if (!buildopts)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }

              modulemd_build_config_set_buildopts (buildconfig, buildopts);
              g_clear_object (&buildopts);
            }

          else
            {
              SKIP_UNKNOWN (parser,
                            FALSE,
                            "Unexpected key in build config: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }

          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error, event, "Unexpected YAML event in profile");
          break;
        }
      yaml_event_delete (&event);
    }

  /* Validate the input */
  if (!modulemd_build_config_validate (buildconfig, &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

  return g_steal_pointer (&buildconfig);
}


gboolean
modulemd_build_config_emit_yaml (ModulemdBuildConfig *self,
                                 yaml_emitter_t *emitter,
                                 GError **error)
{
  MODULEMD_INIT_TRACE ();
  int ret;
  g_autoptr (GError) nested_error = NULL;
  MMD_INIT_YAML_EVENT (event);

  ret = mmd_emitter_start_mapping (
    emitter, YAML_BLOCK_MAPPING_STYLE, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Failed to start BuildConfig mapping: ");
      return FALSE;
    }

  EMIT_KEY_VALUE_IF_SET (emitter, error, "context", self->context);
  EMIT_KEY_VALUE_IF_SET (emitter, error, "platform", self->platform);
  EMIT_HASHTABLE_KEY_VALUES_IF_NON_EMPTY (
    emitter, error, "buildrequires", self->buildrequires);
  EMIT_HASHTABLE_KEY_VALUES_IF_NON_EMPTY (
    emitter, error, "requires", self->requires);

  if (self->buildopts != NULL)
    {
      EMIT_SCALAR (emitter, error, "buildopts");
      EMIT_MAPPING_START (emitter, error);
      if (!modulemd_buildopts_emit_yaml (
            self->buildopts, emitter, &nested_error))
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Failed to emit BuildConfig buildopts: ");
          return FALSE;
        }
      EMIT_MAPPING_END (emitter, error);
    }

  ret = mmd_emitter_end_mapping (emitter, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Failed to end BuildConfig mapping");
      return FALSE;
    }
  return TRUE;
}


static void
modulemd_build_config_replace_runtime_deps (ModulemdBuildConfig *self,
                                            GHashTable *deps)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));

  if (deps)
    {
      g_clear_pointer (&self->requires, g_hash_table_unref);
      self->requires = modulemd_hash_table_deep_str_copy (deps);
    }
  else
    {
      g_hash_table_remove_all (self->requires);
    }
}


static void
modulemd_build_config_replace_buildtime_deps (ModulemdBuildConfig *self,
                                              GHashTable *deps)
{
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (self));

  if (deps)
    {
      g_clear_pointer (&self->buildrequires, g_hash_table_unref);
      self->buildrequires = modulemd_hash_table_deep_str_copy (deps);
    }
  else
    {
      g_hash_table_remove_all (self->buildrequires);
    }
}


gboolean
modulemd_build_config_validate (ModulemdBuildConfig *buildconfig,
                                GError **error)
{
  gsize i;

  /* Context must be present and be between 1 and MMD_MAXCONTEXTLEN
   * alphanumeric characters
   */
  if (buildconfig->context == NULL || buildconfig->context[0] == '\0')
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_VALIDATE,
                   "Empty context in BuildConfig");
      return FALSE;
    }

  for (i = 0; i < MMD_MAXCONTEXTLEN; i++)
    {
      if (buildconfig->context[i] == '\0')
        break;

      if (!(g_ascii_isalnum (buildconfig->context[i])))
        {
          g_set_error (error,
                       MODULEMD_ERROR,
                       MMD_ERROR_VALIDATE,
                       "Non-alphanumeric character in BuildConfig context");
          return FALSE;
        }
    }

  if (buildconfig->context[i] != '\0')
    {
      /* We passed the maximum length without encountering a
       * NULL-terminator
       */
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_VALIDATE,
                   "BuildConfig context exceeds maximum characters");
      return FALSE;
    }

  /* The platform value must be set.
   * In the future, we should probably validate its contents, but this is
   * not currently defined in the specification.
   */

  if (!buildconfig->platform)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_VALIDATE,
                   "Unset platform in BuildConfig");
      return FALSE;
    }

  return TRUE;
}


ModulemdBuildConfig *
modulemd_build_config_copy (ModulemdBuildConfig *self)
{
  g_autoptr (ModulemdBuildConfig) copy = modulemd_build_config_new ();

  modulemd_build_config_set_context (copy,
                                     modulemd_build_config_get_context (self));
  modulemd_build_config_set_platform (
    copy, modulemd_build_config_get_platform (self));

  if (self->requires)
    {
      modulemd_build_config_replace_runtime_deps (copy, self->requires);
    }

  if (self->buildrequires)
    {
      modulemd_build_config_replace_buildtime_deps (copy, self->buildrequires);
    }

  modulemd_build_config_set_buildopts (
    copy, modulemd_build_config_get_buildopts (self));

  return g_steal_pointer (&copy);
}


gboolean
modulemd_build_config_equals (ModulemdBuildConfig *self_1,
                              ModulemdBuildConfig *self_2)
{
  if (!self_1 && !self_2)
    {
      return TRUE;
    }

  if (!self_1 || !self_2)
    {
      return FALSE;
    }

  g_return_val_if_fail (MODULEMD_IS_BUILD_CONFIG (self_1), FALSE);
  g_return_val_if_fail (MODULEMD_IS_BUILD_CONFIG (self_2), FALSE);

  if (g_strcmp0 (self_1->context, self_2->context) != 0)
    {
      return FALSE;
    }

  if (g_strcmp0 (self_1->platform, self_2->platform) != 0)
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (
        self_1->requires, self_2->requires, g_str_equal))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (
        self_1->buildrequires, self_2->buildrequires, g_str_equal))
    {
      return FALSE;
    }

  if (!modulemd_buildopts_equals (self_1->buildopts, self_2->buildopts))
    {
      return FALSE;
    }

  return TRUE;
}
