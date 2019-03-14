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

#include "modulemd-component-module.h"
#include "private/modulemd-component-private.h"
#include "private/modulemd-component-module-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define CM_DEFAULT_STRING "__UNSET_COMPONENT_MODULE__"

struct _ModulemdComponentModule
{
  GObject parent_instance;

  gchar *ref;
  gchar *repository;
};

G_DEFINE_TYPE (ModulemdComponentModule,
               modulemd_component_module,
               MODULEMD_TYPE_COMPONENT)

enum
{
  PROP_0,

  PROP_REF,
  PROP_REPOSITORY,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdComponentModule *
modulemd_component_module_new (const gchar *key)
{
  return g_object_new (MODULEMD_TYPE_COMPONENT_MODULE, "name", key, NULL);
}


static void
modulemd_component_module_finalize (GObject *object)
{
  ModulemdComponentModule *self = (ModulemdComponentModule *)object;

  g_clear_pointer (&self->ref, g_free);
  g_clear_pointer (&self->repository, g_free);

  G_OBJECT_CLASS (modulemd_component_module_parent_class)->finalize (object);
}


static ModulemdComponent *
modulemd_component_module_copy (ModulemdComponent *self, const gchar *key)
{
  ModulemdComponentModule *module_self = NULL;
  g_autoptr (ModulemdComponentModule) copy = NULL;
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_MODULE (self), NULL);
  module_self = MODULEMD_COMPONENT_MODULE (self);

  copy = MODULEMD_COMPONENT_MODULE (
    MODULEMD_COMPONENT_CLASS (modulemd_component_module_parent_class)
      ->copy (self, key));

  modulemd_component_module_set_ref (
    copy, modulemd_component_module_get_ref (module_self));
  modulemd_component_module_set_repository (
    copy, modulemd_component_module_get_repository (module_self));

  return MODULEMD_COMPONENT (g_steal_pointer (&copy));
}


void
modulemd_component_module_set_ref (ModulemdComponentModule *self,
                                   const gchar *ref)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_MODULE (self));

  g_clear_pointer (&self->ref, g_free);
  self->ref = g_strdup (ref);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REF]);
}


const gchar *
modulemd_component_module_get_ref (ModulemdComponentModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_MODULE (self), NULL);

  return self->ref;
}


void
modulemd_component_module_set_repository (ModulemdComponentModule *self,
                                          const gchar *repository)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_MODULE (self));

  g_clear_pointer (&self->repository, g_free);
  self->repository = g_strdup (repository);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REPOSITORY]);
}


const gchar *
modulemd_component_module_get_repository (ModulemdComponentModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_MODULE (self), NULL);

  return self->repository;
}


static void
modulemd_component_module_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
  ModulemdComponentModule *self = MODULEMD_COMPONENT_MODULE (object);

  switch (prop_id)
    {
    case PROP_REF:
      g_value_set_string (value, modulemd_component_module_get_ref (self));
      break;
    case PROP_REPOSITORY:
      g_value_set_string (value,
                          modulemd_component_module_get_repository (self));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_component_module_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
  ModulemdComponentModule *self = MODULEMD_COMPONENT_MODULE (object);

  switch (prop_id)
    {
    case PROP_REF:
      modulemd_component_module_set_ref (self, g_value_get_string (value));
      break;
    case PROP_REPOSITORY:
      modulemd_component_module_set_repository (self,
                                                g_value_get_string (value));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_component_module_class_init (ModulemdComponentModuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdComponentClass *component_class =
    MODULEMD_COMPONENT_CLASS (object_class);

  object_class->finalize = modulemd_component_module_finalize;
  object_class->get_property = modulemd_component_module_get_property;
  object_class->set_property = modulemd_component_module_set_property;

  component_class->copy = modulemd_component_module_copy;

  properties[PROP_REF] =
    g_param_spec_string ("ref",
                         "Ref",
                         "The commit ID in the SCM repository.",
                         CM_DEFAULT_STRING,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_REPOSITORY] =
    g_param_spec_string ("repository",
                         "Repository",
                         "The URI of the SCM repository.",
                         CM_DEFAULT_STRING,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_component_module_init (ModulemdComponentModule *self)
{
  /* Nothing to init */
}


gboolean
modulemd_component_module_emit_yaml (ModulemdComponentModule *self,
                                     yaml_emitter_t *emitter,
                                     GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);

  if (!modulemd_component_emit_yaml_start (
        MODULEMD_COMPONENT (self), emitter, error))
    return FALSE;

  if (modulemd_component_module_get_repository (self) != NULL)
    {
      if (!mmd_emitter_scalar (
            emitter, "repository", YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;

      if (!mmd_emitter_scalar (emitter,
                               modulemd_component_module_get_repository (self),
                               YAML_PLAIN_SCALAR_STYLE,
                               error))
        return FALSE;
    }

  if (modulemd_component_module_get_ref (self) != NULL)
    {
      if (!mmd_emitter_scalar (emitter, "ref", YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;

      if (!mmd_emitter_scalar (emitter,
                               modulemd_component_module_get_ref (self),
                               YAML_PLAIN_SCALAR_STYLE,
                               error))
        return FALSE;
    }

  if (!modulemd_component_emit_yaml_buildorder (
        MODULEMD_COMPONENT (self), emitter, error))
    return FALSE;

  if (!mmd_emitter_end_mapping (emitter, error))
    return FALSE;

  return TRUE;
}


ModulemdComponentModule *
modulemd_component_module_parse_yaml (yaml_parser_t *parser,
                                      const gchar *name,
                                      gboolean strict,
                                      GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  g_autofree gchar *value = NULL;
  gint buildorder = 0;
  g_autoptr (ModulemdComponentModule) m = NULL;

  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  m = modulemd_component_module_new (name);

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
                error, event, "Missing mapping in module component entry");
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

              modulemd_component_set_rationale (MODULEMD_COMPONENT (m), value);
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

              modulemd_component_module_set_repository (m, value);
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

              modulemd_component_module_set_ref (m, value);
              g_clear_pointer (&value, g_free);
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

              modulemd_component_set_buildorder (MODULEMD_COMPONENT (m),
                                                 buildorder);
            }
          else
            {
              SKIP_UNKNOWN (parser,
                            NULL,
                            "Unexpected key in module component body: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;
        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_EVENT_EXIT (
            error, event, "Unexpected YAML event in module component");
          break;
        }
      yaml_event_delete (&event);
    }

  return g_steal_pointer (&m);
}
