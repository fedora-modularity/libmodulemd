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

#include "modulemd-component-rpm.h"
#include "private/modulemd-component-private.h"
#include "private/modulemd-component-rpm-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define CR_DEFAULT_STRING "__UNSET_COMPONENT_RPM__"

struct _ModulemdComponentRpm
{
  GObject parent_instance;

  gchar *ref;
  gchar *repository;
  gchar *cache;

  GHashTable *arches;
  GHashTable *multilib;
};

G_DEFINE_TYPE (ModulemdComponentRpm,
               modulemd_component_rpm,
               MODULEMD_TYPE_COMPONENT)

enum
{
  PROP_0,

  PROP_REF,
  PROP_REPOSITORY,
  PROP_CACHE,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdComponentRpm *
modulemd_component_rpm_new (const gchar *name)
{
  return g_object_new (MODULEMD_TYPE_COMPONENT_RPM, "name", name, NULL);
}


static void
modulemd_component_rpm_finalize (GObject *object)
{
  ModulemdComponentRpm *self = (ModulemdComponentRpm *)object;

  g_clear_pointer (&self->ref, g_free);
  g_clear_pointer (&self->repository, g_free);
  g_clear_pointer (&self->cache, g_free);
  g_clear_pointer (&self->arches, g_hash_table_unref);
  g_clear_pointer (&self->multilib, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_component_rpm_parent_class)->finalize (object);
}


static GHashTable *
hash_table_str_set_copy (GHashTable *orig)
{
  GHashTable *new;
  GHashTableIter iter;
  gpointer key;

  g_return_val_if_fail (orig, NULL);

  new = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  g_hash_table_iter_init (&iter, orig);
  while (g_hash_table_iter_next (&iter, &key, NULL))
    {
      g_hash_table_add (new, g_strdup ((const gchar *)key));
    }

  return new;
}


static ModulemdComponent *
modulemd_component_rpm_copy (ModulemdComponent *self, const gchar *name)
{
  ModulemdComponentRpm *rpm_self = NULL;
  g_autoptr (ModulemdComponentRpm) copy = NULL;
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);
  rpm_self = MODULEMD_COMPONENT_RPM (self);

  copy = MODULEMD_COMPONENT_RPM (
    MODULEMD_COMPONENT_CLASS (modulemd_component_rpm_parent_class)
      ->copy (self, name));

  modulemd_component_rpm_set_ref (copy,
                                  modulemd_component_rpm_get_ref (rpm_self));
  modulemd_component_rpm_set_repository (
    copy, modulemd_component_rpm_get_repository (rpm_self));
  modulemd_component_rpm_set_cache (
    copy, modulemd_component_rpm_get_cache (rpm_self));

  g_clear_pointer (&copy->arches, g_hash_table_unref);
  copy->arches = hash_table_str_set_copy (rpm_self->arches);
  g_clear_pointer (&copy->multilib, g_hash_table_unref);
  copy->multilib = hash_table_str_set_copy (rpm_self->multilib);

  return MODULEMD_COMPONENT (g_steal_pointer (&copy));
}


void
modulemd_component_rpm_set_ref (ModulemdComponentRpm *self, const gchar *ref)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  g_clear_pointer (&self->ref, g_free);
  self->ref = g_strdup (ref);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REF]);
}


const gchar *
modulemd_component_rpm_get_ref (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->ref;
}


void
modulemd_component_rpm_set_cache (ModulemdComponentRpm *self,
                                  const gchar *cache)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  g_clear_pointer (&self->cache, g_free);
  self->cache = g_strdup (cache);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CACHE]);
}


const gchar *
modulemd_component_rpm_get_cache (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->cache;
}


void
modulemd_component_rpm_set_repository (ModulemdComponentRpm *self,
                                       const gchar *repository)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  g_clear_pointer (&self->repository, g_free);
  self->repository = g_strdup (repository);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REPOSITORY]);
}


const gchar *
modulemd_component_rpm_get_repository (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->repository;
}


void
modulemd_component_rpm_add_restricted_arch (ModulemdComponentRpm *self,
                                            const gchar *arch)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  g_hash_table_add (self->arches, g_strdup (arch));
}


void
modulemd_component_rpm_reset_arches (ModulemdComponentRpm *self)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  g_hash_table_remove_all (self->arches);
}


GStrv
modulemd_component_rpm_get_arches_as_strv (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->arches);
  ;
}


void
modulemd_component_rpm_add_multilib_arch (ModulemdComponentRpm *self,
                                          const gchar *arch)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  g_hash_table_add (self->multilib, g_strdup (arch));
}


void
modulemd_component_rpm_reset_multilib_arches (ModulemdComponentRpm *self)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  g_hash_table_remove_all (self->multilib);
}


GStrv
modulemd_component_rpm_get_multilib_arches_as_strv (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->multilib);
  ;
}


static void
modulemd_component_rpm_get_property (GObject *object,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdComponentRpm *self = MODULEMD_COMPONENT_RPM (object);

  switch (prop_id)
    {
    case PROP_REF:
      g_value_set_string (value, modulemd_component_rpm_get_ref (self));
      break;
    case PROP_REPOSITORY:
      g_value_set_string (value, modulemd_component_rpm_get_repository (self));
      break;
    case PROP_CACHE:
      g_value_set_string (value, modulemd_component_rpm_get_cache (self));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_component_rpm_set_property (GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdComponentRpm *self = MODULEMD_COMPONENT_RPM (object);

  switch (prop_id)
    {
    case PROP_REF:
      modulemd_component_rpm_set_ref (self, g_value_get_string (value));
      break;
    case PROP_REPOSITORY:
      modulemd_component_rpm_set_repository (self, g_value_get_string (value));
      break;
    case PROP_CACHE:
      modulemd_component_rpm_set_cache (self, g_value_get_string (value));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


gboolean
modulemd_component_rpm_equals (ModulemdComponentRpm *self,
                               ModulemdComponentRpm *other)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), FALSE);
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (other), FALSE);


  if (self == other)
    return FALSE;


  GStrv arches_as_strv_self = modulemd_component_rpm_get_arches_as_strv (self);
  GStrv arches_as_strv_other =
    modulemd_component_rpm_get_arches_as_strv (other);

  // check number of keys
  if (g_strv_length (arches_as_strv_self) !=
      g_strv_length (arches_as_strv_other))
    return FALSE;


  // check each element of strv
  int i = 0;
  while (i < g_strv_length (arches_as_strv_self))
    {
      if (g_strcmp0 (*(arches_as_strv_self + i), *(arches_as_strv_other + i)))
        return FALSE;
      i++;
    }

  GStrv multilib_arches_as_strv_self =
    modulemd_component_rpm_get_multilib_arches_as_strv (self);
  GStrv multilib_arches_as_strv_other =
    modulemd_component_rpm_get_multilib_arches_as_strv (other);

  // check number of keys
  if (g_strv_length (multilib_arches_as_strv_self) !=
      g_strv_length (multilib_arches_as_strv_other))
    return FALSE;

  // check each element of strv
  i = 0;
  while (i < g_strv_length (multilib_arches_as_strv_self))
    {
      if (g_strcmp0 (*(multilib_arches_as_strv_self + i),
                     *(multilib_arches_as_strv_other + i)))
        return FALSE;
      i++;
    }

  const gchar *cache_self = modulemd_component_rpm_get_cache (self);
  const gchar *cache_other = modulemd_component_rpm_get_cache (other);

  if (g_strcmp0 (cache_self, cache_other) != 0)
    return FALSE;

  const gchar *ref_self = modulemd_component_rpm_get_ref (self);
  const gchar *ref_other = modulemd_component_rpm_get_ref (other);

  if (g_strcmp0 (ref_self, ref_other) != 0)
    return FALSE;

  const gchar *repository_self = modulemd_component_rpm_get_repository (self);
  const gchar *repository_other =
    modulemd_component_rpm_get_repository (other);

  if (g_strcmp0 (repository_self, repository_other) != 0)
    return FALSE;

  return TRUE;
}


static void
modulemd_component_rpm_class_init (ModulemdComponentRpmClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdComponentClass *component_class =
    MODULEMD_COMPONENT_CLASS (object_class);

  object_class->finalize = modulemd_component_rpm_finalize;
  object_class->get_property = modulemd_component_rpm_get_property;
  object_class->set_property = modulemd_component_rpm_set_property;

  component_class->copy = modulemd_component_rpm_copy;

  properties[PROP_CACHE] =
    g_param_spec_string ("cache",
                         "Cache",
                         "The lookaside cache URL.",
                         CR_DEFAULT_STRING,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_REF] =
    g_param_spec_string ("ref",
                         "Ref",
                         "The commit ID in the SCM repository.",
                         CR_DEFAULT_STRING,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_REPOSITORY] =
    g_param_spec_string ("repository",
                         "Repository",
                         "The URI of the SCM repository.",
                         CR_DEFAULT_STRING,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_component_rpm_init (ModulemdComponentRpm *self)
{
  self->arches = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  self->multilib =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}


gboolean
modulemd_component_rpm_emit_yaml (ModulemdComponentRpm *self,
                                  yaml_emitter_t *emitter,
                                  GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  g_auto (GStrv) list = NULL;

  if (!modulemd_component_emit_yaml_start (
        MODULEMD_COMPONENT (self), emitter, error))
    return FALSE;

  if (modulemd_component_rpm_get_repository (self) != NULL)
    {
      if (!mmd_emitter_scalar (
            emitter, "repository", YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;

      if (!mmd_emitter_scalar (emitter,
                               modulemd_component_rpm_get_repository (self),
                               YAML_PLAIN_SCALAR_STYLE,
                               error))
        return FALSE;
    }

  if (modulemd_component_rpm_get_cache (self) != NULL)
    {
      if (!mmd_emitter_scalar (
            emitter, "cache", YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;

      if (!mmd_emitter_scalar (emitter,
                               modulemd_component_rpm_get_cache (self),
                               YAML_PLAIN_SCALAR_STYLE,
                               error))
        return FALSE;
    }

  if (modulemd_component_rpm_get_ref (self) != NULL)
    {
      if (!mmd_emitter_scalar (emitter, "ref", YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;

      if (!mmd_emitter_scalar (emitter,
                               modulemd_component_rpm_get_ref (self),
                               YAML_PLAIN_SCALAR_STYLE,
                               error))
        return FALSE;
    }

  if (!modulemd_component_emit_yaml_buildorder (
        MODULEMD_COMPONENT (self), emitter, error))
    return FALSE;

  if (g_hash_table_size (self->arches) != 0)
    {
      if (!mmd_emitter_scalar (
            emitter, "arches", YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;

      list = modulemd_component_rpm_get_arches_as_strv (self);

      if (!mmd_emitter_strv (emitter, YAML_FLOW_SEQUENCE_STYLE, list, error))
        return FALSE;

      g_clear_pointer (&list, g_strfreev);
    }

  if (g_hash_table_size (self->multilib) != 0)
    {
      if (!mmd_emitter_scalar (
            emitter, "multilib", YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;

      list = modulemd_component_rpm_get_multilib_arches_as_strv (self);

      if (!mmd_emitter_strv (emitter, YAML_FLOW_SEQUENCE_STYLE, list, error))
        return FALSE;

      g_clear_pointer (&list, g_strfreev);
    }

  if (!mmd_emitter_end_mapping (emitter, error))
    return FALSE;

  return TRUE;
}


ModulemdComponentRpm *
modulemd_component_rpm_parse_yaml (yaml_parser_t *parser,
                                   const gchar *name,
                                   gboolean strict,
                                   GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  g_autofree gchar *value = NULL;
  g_autoptr (GHashTable) list = NULL;
  gint buildorder = 0;
  g_autoptr (ModulemdComponentRpm) r = NULL;

  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  r = modulemd_component_rpm_new (name);

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
            {
              MMD_YAML_ERROR_EVENT_EXIT (
                error, event, "Missing mapping in rpm component entry");
              break;
            }
          if (g_str_equal ((const gchar *)event.data.scalar.value,
                           "rationale"))
            {
              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                MMD_YAML_ERROR_EVENT_EXIT (
                  error,
                  event,
                  "Failed to parse rationale in component: %s",
                  nested_error->message);

              modulemd_component_set_rationale (MODULEMD_COMPONENT (r), value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "repository"))
            {
              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                MMD_YAML_ERROR_EVENT_EXIT (
                  error,
                  event,
                  "Failed to parse repository in component: %s",
                  nested_error->message);

              modulemd_component_rpm_set_repository (r, value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value, "ref"))
            {
              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                MMD_YAML_ERROR_EVENT_EXIT (
                  error,
                  event,
                  "Failed to parse ref in component: %s",
                  nested_error->message);

              modulemd_component_rpm_set_ref (r, value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "cache"))
            {
              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                MMD_YAML_ERROR_EVENT_EXIT (
                  error,
                  event,
                  "Failed to parse cache in component: %s",
                  nested_error->message);

              modulemd_component_rpm_set_cache (r, value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "arches"))
            {
              list = modulemd_yaml_parse_string_set (parser, &nested_error);
              if (!list)
                MMD_YAML_ERROR_EVENT_EXIT (
                  error,
                  event,
                  "Failed to parse arches in component: %s",
                  nested_error->message);

              g_clear_pointer (&r->arches, g_hash_table_unref);
              r->arches = g_steal_pointer (&list);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "multilib"))
            {
              list = modulemd_yaml_parse_string_set (parser, &nested_error);
              if (!list)
                MMD_YAML_ERROR_EVENT_EXIT (
                  error,
                  event,
                  "Failed to parse arches in component: %s",
                  nested_error->message);

              g_clear_pointer (&r->multilib, g_hash_table_unref);
              r->multilib = g_steal_pointer (&list);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "buildorder"))
            {
              buildorder = modulemd_yaml_parse_int64 (parser, &nested_error);
              if (buildorder == 0 && nested_error != NULL)
                MMD_YAML_ERROR_EVENT_EXIT (
                  error,
                  event,
                  "Failed to parse buildorder in component: %s",
                  nested_error->message);

              modulemd_component_set_buildorder (MODULEMD_COMPONENT (r),
                                                 buildorder);
            }
          else
            {
              SKIP_UNKNOWN (parser,
                            NULL,
                            "Unexpected key in rpm component body: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;
        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_EVENT_EXIT (
            error, event, "Unexpected YAML event in rpm component");
          break;
        }
      yaml_event_delete (&event);
    }

  return g_steal_pointer (&r);
}
