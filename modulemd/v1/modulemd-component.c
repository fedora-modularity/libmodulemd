/*
 * This file is part of libmodulemd
 * Copyright (C) 2017-2018 Stephen Gallagher
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#include "modulemd-component.h"

enum
{
  COMPONENT_PROP_0,

  COMPONENT_PROP_BUILDORDER,
  COMPONENT_PROP_NAME,
  COMPONENT_PROP_RATIONALE,

  COMPONENT_N_PROPS
};

GParamSpec *component_properties[COMPONENT_N_PROPS] = {
  NULL,
};

/* Private structure definition */
typedef struct
{
  guint64 buildorder;
  gchar *name;
  gchar *rationale;
} ModulemdComponentPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (ModulemdComponent,
                            modulemd_component,
                            G_TYPE_OBJECT)

static void
modulemd_component_default_set_buildorder (ModulemdComponent *self,
                                           guint64 buildorder)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT (self));
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);


  priv->buildorder = buildorder;
}

static guint64
modulemd_component_default_peek_buildorder (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), 0);
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return priv->buildorder;
}

static void
modulemd_component_default_set_name (ModulemdComponent *self,
                                     const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT (self));
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  priv->name = g_strdup (name);
}

static const gchar *
modulemd_component_default_peek_name (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), 0);
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return priv->name;
}

static gchar *
modulemd_component_default_dup_name (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), 0);
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return g_strdup (priv->name);
}

static void
modulemd_component_default_set_rationale (ModulemdComponent *self,
                                          const gchar *rationale)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT (self));
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  priv->rationale = g_strdup (rationale);
}

static const gchar *
modulemd_component_default_peek_rationale (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), 0);
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return priv->rationale;
}

static gchar *
modulemd_component_default_dup_rationale (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), 0);
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return g_strdup (priv->rationale);
}

/**
 * modulemd_component_set_buildorder:
 * @buildorder: The order to build this component
 *
 * Sets the 'buildorder' property.
 *
 * Since: 1.0
 */
void
modulemd_component_set_buildorder (ModulemdComponent *self, guint64 buildorder)
{
  ModulemdComponentClass *klass;

  g_return_if_fail (MODULEMD_IS_COMPONENT (self));

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_if_fail (klass->set_buildorder);

  klass->set_buildorder (self, buildorder);
}

/**
 * modulemd_component_get_buildorder:
 *
 * Returns the 'buildorder' property
 *
 * Deprecated: 1.1
 * Use peek_buildorder() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_component_peek_buildorder)
guint64
modulemd_component_get_buildorder (ModulemdComponent *self)
{
  return modulemd_component_peek_buildorder (self);
}


/**
 * modulemd_component_peek_buildorder:
 *
 * Returns the 'buildorder' property
 *
 * Since: 1.1
 */
guint64
modulemd_component_peek_buildorder (ModulemdComponent *self)
{
  ModulemdComponentClass *klass;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), 0);

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->peek_buildorder, 0);

  return klass->peek_buildorder (self);
}


/**
 * modulemd_component_set_name:
 * @name: The name of the component
 *
 * Sets the 'name' property.
 *
 * Since: 1.0
 */
void
modulemd_component_set_name (ModulemdComponent *self, const gchar *name)
{
  ModulemdComponentClass *klass;

  g_return_if_fail (MODULEMD_IS_COMPONENT (self));

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_if_fail (klass->set_name);

  klass->set_name (self, name);
}


/**
 * modulemd_component_get_name:
 *
 * Returns the 'name' property;
 *
 * Deprecated: 1.1
 * Use peek_name() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_component_peek_name)
const gchar *
modulemd_component_get_name (ModulemdComponent *self)
{
  return modulemd_component_peek_name (self);
}


/**
 * modulemd_component_peek_name:
 *
 * Returns the 'name' property;
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_peek_name (ModulemdComponent *self)
{
  ModulemdComponentClass *klass;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->peek_name, NULL);


  return klass->peek_name (self);
}

/**
 * modulemd_component_dup_name:
 *
 * Returns a copy of the 'name' property;
 *
 * Since: 1.1
 */
gchar *
modulemd_component_dup_name (ModulemdComponent *self)
{
  ModulemdComponentClass *klass;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->dup_name, NULL);


  return klass->dup_name (self);
}


/**
 * modulemd_component_set_rationale:
 * @rationale: The rationale for including this component
 *
 * Sets the 'rationale' property.
 *
 * Since: 1.0
 */
void
modulemd_component_set_rationale (ModulemdComponent *self,
                                  const gchar *rationale)
{
  ModulemdComponentClass *klass;

  g_return_if_fail (MODULEMD_IS_COMPONENT (self));

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_if_fail (klass->set_rationale);

  klass->set_rationale (self, rationale);
}


/**
 * modulemd_component_get_rationale:
 *
 * Returns the 'rationale' property;
 *
 * Deprecated: 1.1
 * Use peek_rationale() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_component_peek_rationale)
const gchar *
modulemd_component_get_rationale (ModulemdComponent *self)
{
  return modulemd_component_peek_rationale (self);
}


/**
 * modulemd_component_peek_rationale:
 *
 * Returns the 'rationale' property;
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_peek_rationale (ModulemdComponent *self)
{
  ModulemdComponentClass *klass;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->peek_rationale, NULL);


  return klass->peek_rationale (self);
}

/**
 * modulemd_component_dup_rationale:
 *
 * Returns a copy of the 'rationale' property;
 *
 * Since: 1.1
 */
gchar *
modulemd_component_dup_rationale (ModulemdComponent *self)
{
  ModulemdComponentClass *klass;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->peek_rationale, NULL);


  return klass->dup_rationale (self);
}


/**
 * modulemd_component_copy:
 *
 * Returns a complete copy of this Component.
 *
 * Returns: (transfer full): A copy of this Component.
 *
 * Since: 1.1
 */
ModulemdComponent *
modulemd_component_copy (ModulemdComponent *self)
{
  ModulemdComponentClass *klass;

  if (!self)
    return NULL;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->copy, NULL);


  return klass->copy (self);
}


static void
modulemd_component_set_property (GObject *gobject,
                                 guint property_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdComponent *self = MODULEMD_COMPONENT (gobject);

  switch (property_id)
    {
    case COMPONENT_PROP_BUILDORDER:
      modulemd_component_set_buildorder (self, g_value_get_uint64 (value));
      break;

    case COMPONENT_PROP_NAME:
      modulemd_component_set_name (self, g_value_get_string (value));
      break;

    case COMPONENT_PROP_RATIONALE:
      modulemd_component_set_rationale (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_component_get_property (GObject *gobject,
                                 guint property_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdComponent *self = MODULEMD_COMPONENT (gobject);

  switch (property_id)
    {
    case COMPONENT_PROP_BUILDORDER:
      g_value_set_uint64 (value, modulemd_component_peek_buildorder (self));
      break;

    case COMPONENT_PROP_NAME:
      g_value_set_string (value, modulemd_component_peek_name (self));
      break;

    case COMPONENT_PROP_RATIONALE:
      g_value_set_string (value, modulemd_component_peek_rationale (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_component_finalize (GObject *gobject)
{
  ModulemdComponent *self = (ModulemdComponent *)gobject;
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  g_clear_pointer (&priv->name, g_free);
  g_clear_pointer (&priv->rationale, g_free);
}

static void
modulemd_component_class_init (ModulemdComponentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = modulemd_component_set_property;
  object_class->get_property = modulemd_component_get_property;

  object_class->finalize = modulemd_component_finalize;

  klass->set_buildorder = modulemd_component_default_set_buildorder;
  klass->peek_buildorder = modulemd_component_default_peek_buildorder;

  klass->set_name = modulemd_component_default_set_name;
  klass->peek_name = modulemd_component_default_peek_name;
  klass->dup_name = modulemd_component_default_dup_name;

  klass->set_rationale = modulemd_component_default_set_rationale;
  klass->peek_rationale = modulemd_component_default_peek_rationale;
  klass->dup_rationale = modulemd_component_default_dup_rationale;

  /* copy() is pure-virtual */
  klass->copy = NULL;

  component_properties[COMPONENT_PROP_BUILDORDER] =
    g_param_spec_uint64 ("buildorder",
                         "Build order",
                         "The buildorder index for this component.",
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  component_properties[COMPONENT_PROP_NAME] =
    g_param_spec_string ("name",
                         "Component name",
                         "The component name.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  component_properties[COMPONENT_PROP_RATIONALE] =
    g_param_spec_string ("rationale",
                         "Component rationale",
                         "The rationale for including this component in "
                         "the module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (
    object_class, COMPONENT_N_PROPS, component_properties);
}

static void
modulemd_component_init (ModulemdComponent *self)
{
}

/**
 * modulemd_component_new:
 *
 * Allocates a new #ModulemdComponent
 *
 * Return value: a new #ModulemdComponent
 *
 * Since: 1.0
 */
ModulemdComponent *
modulemd_component_new (void)
{
  return g_object_new (MODULEMD_TYPE_COMPONENT, NULL);
}
