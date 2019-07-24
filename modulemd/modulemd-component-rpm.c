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

  gchar *override_name;
  gchar *ref;
  gchar *repository;
  gchar *cache;
  gboolean buildroot;
  gboolean srpm_buildroot;

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
  PROP_BUILDROOT,
  PROP_SRPM_BUILDROOT,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdComponentRpm *
modulemd_component_rpm_new (const gchar *key)
{
  return g_object_new (MODULEMD_TYPE_COMPONENT_RPM, "name", key, NULL);
}


static void
modulemd_component_rpm_finalize (GObject *object)
{
  ModulemdComponentRpm *self = (ModulemdComponentRpm *)object;

  g_clear_pointer (&self->override_name, g_free);
  g_clear_pointer (&self->ref, g_free);
  g_clear_pointer (&self->repository, g_free);
  g_clear_pointer (&self->cache, g_free);
  g_clear_pointer (&self->arches, g_hash_table_unref);
  g_clear_pointer (&self->multilib, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_component_rpm_parent_class)->finalize (object);
}


static gboolean
modulemd_component_rpm_equals (ModulemdComponent *self_1,
                               ModulemdComponent *self_2)
{
  ModulemdComponentRpm *rpm_self_1 = NULL;
  ModulemdComponentRpm *rpm_self_2 = NULL;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self_1), FALSE);
  rpm_self_1 = MODULEMD_COMPONENT_RPM (self_1);
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self_2), FALSE);
  rpm_self_2 = MODULEMD_COMPONENT_RPM (self_2);

  if (!MODULEMD_COMPONENT_CLASS (modulemd_component_rpm_parent_class)
         ->equals (self_1, self_2))
    return FALSE;

  if (g_strcmp0 (rpm_self_1->override_name, rpm_self_2->override_name) != 0)
    return FALSE;

  if (g_strcmp0 (rpm_self_1->ref, rpm_self_2->ref) != 0)
    return FALSE;

  if (g_strcmp0 (rpm_self_1->repository, rpm_self_2->repository) != 0)
    return FALSE;

  if (g_strcmp0 (rpm_self_1->cache, rpm_self_2->cache) != 0)
    return FALSE;

  if (!modulemd_boolean_equals (rpm_self_1->buildroot, rpm_self_2->buildroot))
    return FALSE;

  if (!modulemd_boolean_equals (rpm_self_1->srpm_buildroot,
                                rpm_self_2->srpm_buildroot))
    return FALSE;

  if (!modulemd_hash_table_sets_are_equal (rpm_self_1->arches,
                                           rpm_self_2->arches))
    return FALSE;

  if (!modulemd_hash_table_sets_are_equal (rpm_self_1->multilib,
                                           rpm_self_2->multilib))
    return FALSE;

  return TRUE;
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


static void
modulemd_component_rpm_set_name (ModulemdComponent *self, const gchar *name);
static const gchar *
modulemd_component_rpm_get_name (ModulemdComponent *self);

static ModulemdComponent *
modulemd_component_rpm_copy (ModulemdComponent *self, const gchar *key)
{
  ModulemdComponentRpm *rpm_self = NULL;
  g_autoptr (ModulemdComponentRpm) copy = NULL;
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);
  rpm_self = MODULEMD_COMPONENT_RPM (self);

  copy = MODULEMD_COMPONENT_RPM (
    MODULEMD_COMPONENT_CLASS (modulemd_component_rpm_parent_class)
      ->copy (self, key));

  modulemd_component_rpm_set_ref (copy,
                                  modulemd_component_rpm_get_ref (rpm_self));
  modulemd_component_rpm_set_name (MODULEMD_COMPONENT (copy),
                                   rpm_self->override_name);
  modulemd_component_rpm_set_repository (
    copy, modulemd_component_rpm_get_repository (rpm_self));
  modulemd_component_rpm_set_cache (
    copy, modulemd_component_rpm_get_cache (rpm_self));
  modulemd_component_rpm_set_buildroot (copy, rpm_self->buildroot);
  modulemd_component_rpm_set_srpm_buildroot (copy, rpm_self->srpm_buildroot);

  g_clear_pointer (&copy->arches, g_hash_table_unref);
  copy->arches = hash_table_str_set_copy (rpm_self->arches);
  g_clear_pointer (&copy->multilib, g_hash_table_unref);
  copy->multilib = hash_table_str_set_copy (rpm_self->multilib);

  return MODULEMD_COMPONENT (g_steal_pointer (&copy));
}


static void
modulemd_component_rpm_set_name (ModulemdComponent *self, const gchar *name)
{
  const gchar *key = NULL;
  ModulemdComponentRpm *rpm_self = NULL;

  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));
  rpm_self = MODULEMD_COMPONENT_RPM (self);

  if (g_strcmp0 (rpm_self->override_name, name) == 0)
    {
      /* No change, so just return.
       */
      return;
    }

  /* We're changing the value, so clear the existing version */
  g_clear_pointer (&rpm_self->override_name, g_free);

  key = MODULEMD_COMPONENT_CLASS (modulemd_component_rpm_parent_class)
          ->get_name (self);

  /* If we were passed non-NULL and the passed name differs from the hash table
   * key, save it. Otherwise, just leave it set NULL.
   */
  if (name && g_strcmp0 (key, name) != 0)
    {
      rpm_self->override_name = g_strdup (name);
    }
}


static const gchar *
modulemd_component_rpm_get_name (ModulemdComponent *self)
{
  ModulemdComponentRpm *rpm_self = NULL;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);
  rpm_self = MODULEMD_COMPONENT_RPM (self);

  /* If an override name was set, return it */
  if (rpm_self->override_name)
    return rpm_self->override_name;

  /* Otherwise, return the hash table key as the name */
  return MODULEMD_COMPONENT_CLASS (modulemd_component_rpm_parent_class)
    ->get_name (self);
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
modulemd_component_rpm_set_buildroot (ModulemdComponentRpm *self,
                                      gboolean buildroot)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  self->buildroot = buildroot;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BUILDROOT]);
}


gboolean
modulemd_component_rpm_get_buildroot (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), FALSE);

  return self->buildroot;
}


void
modulemd_component_rpm_set_srpm_buildroot (ModulemdComponentRpm *self,
                                           gboolean srpm_buildroot)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  self->srpm_buildroot = srpm_buildroot;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SRPM_BUILDROOT]);
}


gboolean
modulemd_component_rpm_get_srpm_buildroot (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), FALSE);

  return self->srpm_buildroot;
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
    case PROP_BUILDROOT:
      g_value_set_boolean (value, modulemd_component_rpm_get_buildroot (self));
      break;
    case PROP_SRPM_BUILDROOT:
      g_value_set_boolean (value,
                           modulemd_component_rpm_get_srpm_buildroot (self));
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
    case PROP_BUILDROOT:
      modulemd_component_rpm_set_buildroot (self, g_value_get_boolean (value));
      break;
    case PROP_SRPM_BUILDROOT:
      modulemd_component_rpm_set_srpm_buildroot (self,
                                                 g_value_get_boolean (value));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
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
  component_class->equals = modulemd_component_rpm_equals;
  component_class->set_name = modulemd_component_rpm_set_name;
  component_class->get_name = modulemd_component_rpm_get_name;

  // clang-format off
  properties[PROP_CACHE] = g_param_spec_string (
    "cache",
    "Cache",
    "The lookaside cache URL.",
    CR_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_REF] = g_param_spec_string (
    "ref",
    "Ref",
    "The commit ID in the SCM repository.",
    CR_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_REPOSITORY] = g_param_spec_string (
    "repository",
    "Repository",
    "The URI of the SCM repository.",
    CR_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_BUILDROOT] = g_param_spec_boolean (
    "buildroot",
    "Buildroot",
    "Whether the packages listed in this module's buildroot profile will be "
    "installed into the buildroot of any component built in subsequent "
    "buildorder/buildafter batches.",
    FALSE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_SRPM_BUILDROOT] = g_param_spec_boolean (
    "srpm-buildroot",
    "SrpmBuildroot",
    "Whether the packages listed in this module's srpm-buildroot profile will "
    "be installed into the buildroot when performing the buildSRPMfromSCM "
    "step in subsequent buildorder/buildafter batches.",
    FALSE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  // clang-format on
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

  EMIT_KEY_VALUE_IF_SET (emitter, error, "name", self->override_name);

  EMIT_KEY_VALUE_IF_SET (emitter, error, "repository", self->repository);

  EMIT_KEY_VALUE_IF_SET (emitter, error, "cache", self->cache);

  EMIT_KEY_VALUE_IF_SET (emitter, error, "ref", self->ref);

  /* Only output buildroot if it's TRUE */
  if (modulemd_component_rpm_get_buildroot (self))
    EMIT_KEY_VALUE (emitter, error, "buildroot", "true");

  /* Only output srpm-buildroot if it's TRUE */
  if (modulemd_component_rpm_get_srpm_buildroot (self))
    EMIT_KEY_VALUE (emitter, error, "srpm-buildroot", "true");

  if (!modulemd_component_emit_yaml_build_common (
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
  gboolean truth_value;
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
                                "name"))
            {
              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                MMD_YAML_ERROR_EVENT_EXIT (
                  error,
                  event,
                  "Failed to parse override name in component: %s",
                  nested_error->message);

              modulemd_component_set_name (MODULEMD_COMPONENT (r), value);
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
                                "buildroot"))
            {
              truth_value = modulemd_yaml_parse_bool (parser, &nested_error);
              if (nested_error)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse buildroot in component: %s",
                    nested_error->message);
                }

              modulemd_component_rpm_set_buildroot (r, truth_value);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "srpm-buildroot"))
            {
              truth_value = modulemd_yaml_parse_bool (parser, &nested_error);
              if (nested_error)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse srpm-buildroot in component: %s",
                    nested_error->message);
                }

              modulemd_component_rpm_set_srpm_buildroot (r, truth_value);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "buildafter"))
            {
              if (!modulemd_component_parse_buildafter (
                    MODULEMD_COMPONENT (r), parser, &nested_error))
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse buildafter in component: %s",
                    nested_error->message);
                }
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "buildonly"))
            {
              if (!modulemd_component_parse_buildonly (
                    MODULEMD_COMPONENT (r), parser, &nested_error))
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse buildonly in component: %s",
                    nested_error->message);
                }
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
