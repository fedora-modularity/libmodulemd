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

#include <inttypes.h>

#include "modulemd-component.h"
#include "private/modulemd-component-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define C_DEFAULT_STRING "__UNSET__"

typedef struct
{
  gint64 buildorder;
  gchar *name;
  gchar *rationale;
} ModulemdComponentPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ModulemdComponent,
                                     modulemd_component,
                                     G_TYPE_OBJECT);

enum
{
  PROP_0,

  PROP_BUILDORDER,
  PROP_NAME,
  PROP_RATIONALE,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


static void
modulemd_component_finalize (GObject *object)
{
  ModulemdComponent *self = (ModulemdComponent *)object;
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  g_clear_pointer (&priv->name, g_free);
  g_clear_pointer (&priv->rationale, g_free);

  G_OBJECT_CLASS (modulemd_component_parent_class)->finalize (object);
}


ModulemdComponent *
modulemd_component_copy (ModulemdComponent *self, const gchar *name)
{
  g_return_val_if_fail (self, NULL);

  ModulemdComponentClass *klass;

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->copy, NULL);

  return klass->copy (self, name);
}


static ModulemdComponent *
modulemd_component_copy_component (ModulemdComponent *self, const gchar *name)
{
  g_autoptr (ModulemdComponent) m = NULL;
  if (name == NULL)
    name = modulemd_component_get_name (self);

  m = g_object_new (G_OBJECT_TYPE (self), "name", name, NULL);

  modulemd_component_set_buildorder (m,
                                     modulemd_component_get_buildorder (self));
  modulemd_component_set_rationale (m,
                                    modulemd_component_get_rationale (self));

  return g_steal_pointer (&m);
}


void
modulemd_component_set_buildorder (ModulemdComponent *self, gint64 buildorder)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT (self));

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  priv->buildorder = buildorder;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BUILDORDER]);
}


gint64
modulemd_component_get_buildorder (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), 0);

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return priv->buildorder;
}


static void
modulemd_component_set_name (ModulemdComponent *self, const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT (self));
  g_return_if_fail (name);
  g_return_if_fail (!g_str_equal (name, C_DEFAULT_STRING));

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);
  g_clear_pointer (&priv->name, g_free);
  priv->name = g_strdup (name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}


const gchar *
modulemd_component_get_name (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return priv->name;
}


void
modulemd_component_set_rationale (ModulemdComponent *self,
                                  const gchar *rationale)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT (self));

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);
  g_clear_pointer (&priv->rationale, g_free);
  priv->rationale = g_strdup (rationale);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RATIONALE]);
}


const gchar *
modulemd_component_get_rationale (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return priv->rationale;
}


static void
modulemd_component_get_property (GObject *object,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdComponent *self = MODULEMD_COMPONENT (object);

  switch (prop_id)
    {
    case PROP_BUILDORDER:
      g_value_set_int64 (value, modulemd_component_get_buildorder (self));
      break;

    case PROP_NAME:
      g_value_set_string (value, modulemd_component_get_name (self));
      break;

    case PROP_RATIONALE:
      g_value_set_string (value, modulemd_component_get_rationale (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_component_set_property (GObject *object,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdComponent *self = MODULEMD_COMPONENT (object);

  switch (prop_id)
    {
    case PROP_BUILDORDER:
      modulemd_component_set_buildorder (self, g_value_get_int64 (value));
      break;

    case PROP_NAME:
      modulemd_component_set_name (self, g_value_get_string (value));
      break;

    case PROP_RATIONALE:
      modulemd_component_set_rationale (self, g_value_get_string (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static gboolean
modulemd_component_defualt_equals (ModulemdComponent *self, ModulemdComponent *other) 
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), FALSE);
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (other), FALSE);

  if (self == other) return TRUE;

  gint64 buildorder_self = modulemd_component_get_buildorder (self);
  gint64 buildorder_other = modulemd_component_get_buildorder (other);

  if (buildorder_self != buildorder_other) return FALSE;

  const gchar *name_self = modulemd_component_get_name (self);
  const gchar *name_other = modulemd_component_get_name (other);
  
  if (g_strcmp0 (name_self, name_other) != 0) return FALSE;

  const gchar *rationale_self = modulemd_component_get_rationale (self);
  const gchar *rationale_other = modulemd_component_get_rationale (other);
  
  if (g_strcmp0 (rationale_self, rationale_other) != 0) return FALSE;
    
  return TRUE;
}


gboolean modulemd_component_equals (ModulemdComponent *self, ModulemdComponent *other) 
{
  ModulemdComponentClass *klass;

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), FALSE);
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (other), FALSE);

  return klass->equals (self, other);
}


static void
modulemd_component_class_init (ModulemdComponentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_component_finalize;
  object_class->get_property = modulemd_component_get_property;
  object_class->set_property = modulemd_component_set_property;

  klass->copy = modulemd_component_copy_component;
  klass->equals = modulemd_component_equals;

  properties[PROP_BUILDORDER] = g_param_spec_int64 (
    "buildorder",
    "Buildorder",
    "The order this component should be built relative to others.",
    G_MININT64,
    G_MAXINT64,
    0,
    G_PARAM_READWRITE);
  properties[PROP_NAME] = g_param_spec_string (
    "name",
    "Name",
    "The name of the component.",
    C_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_RATIONALE] =
    g_param_spec_string ("rationale",
                         "Rationale",
                         "A description of the reason this component is "
                         "part of the module stream.",
                         C_DEFAULT_STRING,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_component_init (ModulemdComponent *self)
{
  /* Nothing to init */
}


gboolean
modulemd_component_emit_yaml_start (ModulemdComponent *self,
                                    yaml_emitter_t *emitter,
                                    GError **error)
{
  MODULEMD_INIT_TRACE ();

  if (!mmd_emitter_scalar (emitter,
                           modulemd_component_get_name (self),
                           YAML_PLAIN_SCALAR_STYLE,
                           error))
    return FALSE;

  if (!mmd_emitter_start_mapping (emitter, YAML_BLOCK_MAPPING_STYLE, error))
    return FALSE;

  if (modulemd_component_get_rationale (self) != NULL)
    {
      if (!mmd_emitter_scalar (
            emitter, "rationale", YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;

      if (!mmd_emitter_scalar (emitter,
                               modulemd_component_get_rationale (self),
                               YAML_PLAIN_SCALAR_STYLE,
                               error))
        return FALSE;
    }

  /* The rest of the fields are emitted by childs, after which they need to call
emit_yaml_end to end the mapping. */
  return TRUE;
}


gboolean
modulemd_component_emit_yaml_buildorder (ModulemdComponent *self,
                                         yaml_emitter_t *emitter,
                                         GError **error)
{
  g_autofree gchar *buildorder = NULL;

  if (modulemd_component_get_buildorder (self) != 0)
    {
      buildorder =
        g_strdup_printf ("%" PRIu64, modulemd_component_get_buildorder (self));
      ;
      if (!mmd_emitter_scalar (
            emitter, "buildorder", YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;

      if (!mmd_emitter_scalar (
            emitter, buildorder, YAML_PLAIN_SCALAR_STYLE, error))
        return FALSE;
    }

  return TRUE;
}
