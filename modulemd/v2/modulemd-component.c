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
  GHashTable *buildafter;
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
  g_clear_pointer (&priv->buildafter, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_component_parent_class)->finalize (object);
}


ModulemdComponent *
modulemd_component_copy (ModulemdComponent *self, const gchar *key)
{
  g_return_val_if_fail (self, NULL);

  ModulemdComponentClass *klass;

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->copy, NULL);

  return klass->copy (self, key);
}


static gboolean
modulemd_component_default_validate (ModulemdComponent *self, GError **error)
{
  if (!self)
    return FALSE;

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), FALSE);

  if (priv->buildorder && modulemd_component_has_buildafter (self))
    {
      /* It is invalid to have both buildorder and buildafter set on a
       * component
       */
      g_set_error (
        error,
        MODULEMD_ERROR,
        MODULEMD_ERROR_VALIDATE,
        "Cannot mix buildorder and buildafter in the same component");
      return FALSE;
    }

  return TRUE;
}


gboolean
modulemd_component_validate (ModulemdComponent *self, GError **error)
{
  ModulemdComponentClass *klass;

  if (!self)
    return FALSE;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), FALSE);

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->validate, FALSE);

  return klass->validate (self, error);
}


static ModulemdComponent *
modulemd_component_copy_component (ModulemdComponent *self, const gchar *key)
{
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);
  ModulemdComponentPrivate *m_priv = NULL;
  g_autoptr (ModulemdComponent) m = NULL;
  if (key == NULL)
    key = priv->name;

  m = g_object_new (G_OBJECT_TYPE (self), "name", key, NULL);

  modulemd_component_set_buildorder (m,
                                     modulemd_component_get_buildorder (self));
  modulemd_component_set_rationale (m,
                                    modulemd_component_get_rationale (self));


  m_priv = modulemd_component_get_instance_private (m);
  g_clear_pointer (&m_priv->buildafter, g_hash_table_unref);
  m_priv->buildafter = modulemd_hash_table_deep_set_copy (priv->buildafter);

  return g_steal_pointer (&m);
}


void
modulemd_component_add_buildafter (ModulemdComponent *self, const gchar *key)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT (self));

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  g_hash_table_add (priv->buildafter, g_strdup (key));
}


GStrv
modulemd_component_get_buildafter_as_strv (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return modulemd_ordered_str_keys_as_strv (priv->buildafter);
}


GHashTable *
modulemd_component_get_buildafter_internal (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return priv->buildafter;
}


gboolean
modulemd_component_has_buildafter (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), FALSE);

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return !!g_hash_table_size (priv->buildafter);
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
modulemd_component_set_key (ModulemdComponent *self, const gchar *name)
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

void
modulemd_component_set_name (ModulemdComponent *self, const gchar *name)
{
  ModulemdComponentClass *klass;

  klass = MODULEMD_COMPONENT_GET_CLASS (self);

  /* Do nothing if the child class has not implemented this */
  if (!klass->set_name)
    return;

  klass->set_name (self, name);
}

const gchar *
modulemd_component_get_key (ModulemdComponent *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT (self), NULL);

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  return priv->name;
}


const gchar *
modulemd_component_get_name (ModulemdComponent *self)
{
  ModulemdComponentClass *klass;

  klass = MODULEMD_COMPONENT_GET_CLASS (self);
  g_return_val_if_fail (klass->get_name, NULL);

  return klass->get_name (self);
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
      /* On lookups, return the real name value, not the key. This differs from
       * the set function, which always assigns the key.
       */
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
      /* On object creation, we set the key. */
      modulemd_component_set_key (self, g_value_get_string (value));
      break;

    case PROP_RATIONALE:
      modulemd_component_set_rationale (self, g_value_get_string (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_component_class_init (ModulemdComponentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_component_finalize;
  object_class->get_property = modulemd_component_get_property;
  object_class->set_property = modulemd_component_set_property;

  klass->copy = modulemd_component_copy_component;
  klass->set_name = NULL;
  klass->get_name = modulemd_component_get_key;
  klass->validate = modulemd_component_default_validate;

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
    "The name of the component. This is the real name of the component and "
    "may differ from the key used to associate this component with the "
    "ModuleStream.",
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
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  priv->buildafter =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}


gboolean
modulemd_component_parse_buildafter (ModulemdComponent *self,
                                     yaml_parser_t *parser,
                                     GError **error)
{
  g_autoptr (GHashTable) buildafter = NULL;
  g_autoptr (GError) nested_error = NULL;

  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  MODULEMD_INIT_TRACE ();

  buildafter = modulemd_yaml_parse_string_set (parser, &nested_error);
  if (!buildafter)
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }

  g_clear_pointer (&priv->buildafter, g_hash_table_unref);
  priv->buildafter = g_steal_pointer (&buildafter);

  return TRUE;
}


gboolean
modulemd_component_emit_yaml_start (ModulemdComponent *self,
                                    yaml_emitter_t *emitter,
                                    GError **error)
{
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  g_autoptr (GError) nested_error = NULL;

  MODULEMD_INIT_TRACE ();

  if (!modulemd_component_validate (self, &nested_error))
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Component failed to validate: ");
      return FALSE;
    }

  if (!mmd_emitter_scalar (
        emitter, priv->name, YAML_PLAIN_SCALAR_STYLE, error))
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
  ModulemdComponentPrivate *priv =
    modulemd_component_get_instance_private (self);

  g_autofree gchar *buildorder = NULL;
  g_autoptr (GPtrArray) buildafter = NULL;

  MODULEMD_INIT_TRACE ();

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

  else if (g_hash_table_size (priv->buildafter))
    {
      buildafter =
        modulemd_ordered_str_keys (priv->buildafter, modulemd_strcmp_sort);

      EMIT_SCALAR (emitter, error, "buildafter");

      EMIT_SEQUENCE_START (emitter, error);

      for (gint i = 0; i < buildafter->len; i++)
        {
          EMIT_SCALAR (emitter, error, g_ptr_array_index (buildafter, i));
        }

      EMIT_SEQUENCE_END (emitter, error);
    }

  return TRUE;
}
