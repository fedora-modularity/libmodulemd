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

#include "modulemd-buildopts.h"
#include "private/glib-extensions.h"
#include "private/modulemd-buildopts-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define B_DEFAULT_STRING "__BUILDOPTS_RPM_MACROS_UNSET__"

struct _ModulemdBuildopts
{
  GObject parent_instance;

  gchar *rpm_macros;

  GHashTable *whitelist;
};

G_DEFINE_TYPE (ModulemdBuildopts, modulemd_buildopts, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_RPM_MACROS,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdBuildopts *
modulemd_buildopts_new (void)
{
  return g_object_new (MODULEMD_TYPE_BUILDOPTS, NULL);
}


ModulemdBuildopts *
modulemd_buildopts_copy (ModulemdBuildopts *self)
{
  g_autoptr (ModulemdBuildopts) b = NULL;
  g_return_val_if_fail (MODULEMD_IS_BUILDOPTS (self), NULL);

  b = modulemd_buildopts_new ();

  modulemd_buildopts_set_rpm_macros (b,
                                     modulemd_buildopts_get_rpm_macros (self));

  g_hash_table_unref (b->whitelist);
  b->whitelist = g_hash_table_ref (self->whitelist);

  return g_steal_pointer (&b);
}


static void
modulemd_buildopts_finalize (GObject *object)
{
  ModulemdBuildopts *self = (ModulemdBuildopts *)object;

  g_clear_pointer (&self->rpm_macros, g_free);
  g_clear_pointer (&self->whitelist, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_buildopts_parent_class)->finalize (object);
}


void
modulemd_buildopts_set_rpm_macros (ModulemdBuildopts *self,
                                   const gchar *rpm_macros)
{
  g_return_if_fail (MODULEMD_IS_BUILDOPTS (self));

  g_clear_pointer (&self->rpm_macros, g_free);
  self->rpm_macros = g_strdup (rpm_macros);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_MACROS]);
}


const gchar *
modulemd_buildopts_get_rpm_macros (ModulemdBuildopts *self)
{
  g_return_val_if_fail (MODULEMD_IS_BUILDOPTS (self), NULL);

  return self->rpm_macros;
}


void
modulemd_buildopts_add_rpm_to_whitelist (ModulemdBuildopts *self,
                                         const gchar *rpm)
{
  g_return_if_fail (MODULEMD_IS_BUILDOPTS (self));
  g_hash_table_add (self->whitelist, g_strdup (rpm));
}


void
modulemd_buildopts_remove_rpm_from_whitelist (ModulemdBuildopts *self,
                                              const gchar *rpm)
{
  g_return_if_fail (MODULEMD_IS_BUILDOPTS (self));
  g_hash_table_remove (self->whitelist, rpm);
}


gchar **
modulemd_buildopts_get_rpm_whitelist_as_strv (ModulemdBuildopts *self)
{
  g_return_val_if_fail (MODULEMD_IS_BUILDOPTS (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->whitelist);
}


static void
modulemd_buildopts_get_property (GObject *object,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdBuildopts *self = MODULEMD_BUILDOPTS (object);

  switch (prop_id)
    {
    case PROP_RPM_MACROS:
      g_value_set_string (value, modulemd_buildopts_get_rpm_macros (self));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_buildopts_set_property (GObject *object,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdBuildopts *self = MODULEMD_BUILDOPTS (object);

  switch (prop_id)
    {
    case PROP_RPM_MACROS:
      modulemd_buildopts_set_rpm_macros (self, g_value_get_string (value));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_buildopts_class_init (ModulemdBuildoptsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_buildopts_finalize;
  object_class->get_property = modulemd_buildopts_get_property;
  object_class->set_property = modulemd_buildopts_set_property;

  properties[PROP_RPM_MACROS] = g_param_spec_string (
    "rpm-macros",
    "Rpm Macros",
    "A string containing RPM build macros in the form that they would appear "
    "in an RPM macros file on-disk.",
    B_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_buildopts_init (ModulemdBuildopts *self)
{
  self->whitelist =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}


/* === YAML Functions === */

static gboolean
modulemd_buildopts_parse_rpm_buildopts (yaml_parser_t *parser,
                                        ModulemdBuildopts *buildopts,
                                        gboolean strict,
                                        GError **error);

ModulemdBuildopts *
modulemd_buildopts_parse_yaml (yaml_parser_t *parser,
                               gboolean strict,
                               GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  g_autoptr (ModulemdBuildopts) buildopts = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  buildopts = modulemd_buildopts_new ();

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT: in_map = TRUE; break;

        case YAML_MAPPING_END_EVENT:
          in_map = FALSE;
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            MMD_YAML_ERROR_EVENT_EXIT_BOOL (
              error, event, "Missing mapping in buildopts");

          if (g_str_equal ((const gchar *)event.data.scalar.value, "rpms"))
            {
              if (!modulemd_buildopts_parse_rpm_buildopts (
                    parser, buildopts, strict, &nested_error))
                {
                  g_propagate_error (error, nested_error);
                  return NULL;
                }
            }
          else
            {
              SKIP_UNKNOWN (parser,
                            NULL,
                            "Unexpected key in buildopts: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (error,
                                     event,
                                     "Unexpected YAML event in buildopts: %s",
                                     mmd_yaml_get_event_name (event.type));
          break;
        }

      yaml_event_delete (&event);
    }
  return g_steal_pointer (&buildopts);
}

static gboolean
modulemd_buildopts_parse_rpm_buildopts (yaml_parser_t *parser,
                                        ModulemdBuildopts *buildopts,
                                        gboolean strict,
                                        GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  g_autofree gchar *value = NULL;
  g_autoptr (GError) nested_error = NULL;

  /* Read in RPM attributes */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT: in_map = TRUE; break;

        case YAML_MAPPING_END_EVENT:
          in_map = FALSE;
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            MMD_YAML_ERROR_EVENT_EXIT_BOOL (
              error, event, "Missing mapping in buildopts rpms entry");

          if (g_str_equal (event.data.scalar.value, "whitelist"))
            {
              g_hash_table_unref (buildopts->whitelist);
              buildopts->whitelist =
                modulemd_yaml_parse_string_set (parser, &nested_error);
              if (buildopts->whitelist == NULL)
                {
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error,
                    event,
                    "Failed to parse whitelist list in buildopts rpms: %s",
                    nested_error->message);
                }
            }
          else if (g_str_equal (event.data.scalar.value, "macros"))
            {
              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                  error,
                  event,
                  "Failed to parse rpm_macros in buildopts: %s",
                  nested_error->message);
              modulemd_buildopts_set_rpm_macros (buildopts, value);
              g_clear_pointer (&value, g_free);
            }
          else
            {
              SKIP_UNKNOWN (parser,
                            FALSE,
                            "Unexpected key in buildopts body: %s",
                            (const gchar *)event.data.scalar.value);
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in rpm buildopts: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }

      yaml_event_delete (&event);
    }
  return TRUE;
}


gboolean
modulemd_buildopts_emit_yaml (ModulemdBuildopts *self,
                              yaml_emitter_t *emitter,
                              GError **error)
{
  MODULEMD_INIT_TRACE ();
  int ret;
  g_auto (GStrv) whitelist = NULL;
  g_autoptr (GError) nested_error = NULL;
  MMD_INIT_YAML_EVENT (event);

  ret = mmd_emitter_scalar (
    emitter, "rpms", YAML_PLAIN_SCALAR_STYLE, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error,
        g_steal_pointer (&nested_error),
        "Failed to emit buildopts 'rpms' constant: ");
      return FALSE;
    }

  ret = mmd_emitter_start_mapping (
    emitter, YAML_BLOCK_MAPPING_STYLE, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Failed to start buildopts mapping: ");
      return FALSE;
    }

  if (modulemd_buildopts_get_rpm_macros (self) != NULL)
    {
      ret = mmd_emitter_scalar (
        emitter, "macros", YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (error,
                                      g_steal_pointer (&nested_error),
                                      "Failed to emit buildopts macros key: ");
          return FALSE;
        }

      ret = mmd_emitter_scalar (emitter,
                                modulemd_buildopts_get_rpm_macros (self),
                                YAML_FOLDED_SCALAR_STYLE,
                                &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Failed to emit buildopts macros value: ");
          return FALSE;
        }
    }

  if (g_hash_table_size (self->whitelist) != 0)
    {
      ret = mmd_emitter_scalar (
        emitter, "whitelist", YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Failed to emit buildopts whitelist key: ");
          return FALSE;
        }

      whitelist = modulemd_buildopts_get_rpm_whitelist_as_strv (self);

      ret = mmd_emitter_strv (
        emitter, YAML_BLOCK_SEQUENCE_STYLE, whitelist, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (error,
                                      g_steal_pointer (&nested_error),
                                      "Failed to emit buildopts whitelist: ");
          return FALSE;
        }
    }

  ret = mmd_emitter_end_mapping (emitter, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Failed to end buildopts mapping");
      return FALSE;
    }
  return TRUE;
}


gboolean
modulemd_buildopts_equals (ModulemdBuildopts *self, ModulemdBuildopts *other)
{
  if (self == other) return TRUE;

  const gchar *rpm_macros_self = modulemd_buildopts_get_rpm_macros (self);
  const gchar *rpm_macros_other = modulemd_buildopts_get_rpm_macros (other);

  if (g_strcmp0 (rpm_macros_self, rpm_macros_other) != 0) return FALSE;

  gchar **rpm_whitelist_as_strv_self = modulemd_buildopts_get_rpm_whitelist_as_strv (self);
  gchar **rpm_whitelist_as_strv_other = modulemd_buildopts_get_rpm_whitelist_as_strv (other);


  return TRUE;
}
