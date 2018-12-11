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


#include <inttypes.h>
#include "modulemd-defaults.h"
#include "modulemd-defaults-v1.h"
#include "private/modulemd-defaults-private.h"
#include "private/modulemd-defaults-v1-private.h"
#include "private/modulemd-util.h"

#define DEF_DEFAULT_NAME_STRING "__NAME_UNSET__"

typedef struct
{
  gchar *module_name;
  guint64 modified;
} ModulemdDefaultsPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ModulemdDefaults,
                                     modulemd_defaults,
                                     G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_MDVERSION,
  PROP_MODULE_NAME,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdDefaults *
modulemd_defaults_new (guint64 mdversion, const gchar *module_name)
{
  g_return_val_if_fail (mdversion && mdversion <= MD_DEFAULTS_VERSION_LATEST,
                        NULL);

  if (mdversion == MD_DEFAULTS_VERSION_ONE)
    {
      return MODULEMD_DEFAULTS (modulemd_defaults_v1_new (module_name));
    }

  /* This should be unreachable */
  g_return_val_if_reached (NULL);
}


static void
modulemd_defaults_finalize (GObject *object)
{
  ModulemdDefaults *self = (ModulemdDefaults *)object;
  ModulemdDefaultsPrivate *priv =
    modulemd_defaults_get_instance_private (self);

  g_clear_pointer (&priv->module_name, g_free);

  G_OBJECT_CLASS (modulemd_defaults_parent_class)->finalize (object);
}


ModulemdDefaults *
modulemd_defaults_copy (ModulemdDefaults *self)
{
  ModulemdDefaultsClass *klass;

  if (!self)
    return NULL;

  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), NULL);

  klass = MODULEMD_DEFAULTS_GET_CLASS (self);
  g_return_val_if_fail (klass->copy, NULL);

  return klass->copy (self);
}


static ModulemdDefaults *
modulemd_defaults_default_copy (ModulemdDefaults *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), NULL);

  return modulemd_defaults_new (modulemd_defaults_get_mdversion (self),
                                modulemd_defaults_get_module_name (self));
}


gboolean
modulemd_defaults_validate (ModulemdDefaults *self, GError **error)
{
  ModulemdDefaultsClass *klass;

  if (!self)
    return FALSE;

  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), FALSE);

  klass = MODULEMD_DEFAULTS_GET_CLASS (self);
  g_return_val_if_fail (klass->validate, FALSE);

  return klass->validate (self, error);
}


static gboolean
modulemd_defaults_default_validate (ModulemdDefaults *self, GError **error)
{
  ModulemdDefaultsPrivate *priv =
    modulemd_defaults_get_instance_private (self);
  guint64 mdversion = modulemd_defaults_get_mdversion (self);

  if (!mdversion)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MODULEMD_ERROR_VALIDATE,
                           "Metadata version is unset.");
      return FALSE;
    }
  else if (mdversion > MD_DEFAULTS_VERSION_LATEST)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_VALIDATE,
                   "Metadata version unknown: %" PRIu64 ".",
                   mdversion);
      return FALSE;
    }

  if (!priv->module_name)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MODULEMD_ERROR_VALIDATE,
                           "Module name is unset.");
      return FALSE;
    }

  /* Make sure we have a real module name set */
  if (g_str_equal (modulemd_defaults_get_module_name (self),
                   DEFAULT_PLACEHOLDER))
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MODULEMD_ERROR_VALIDATE,
                           "Defaults did not specify a module name.");
      return FALSE;
    }

  return TRUE;
}


ModulemdDefaults *
modulemd_defaults_upgrade (ModulemdDefaults *self,
                           guint64 mdversion,
                           GError **error)
{
  g_assert_true (MODULEMD_IS_DEFAULTS (self));

  if (!mdversion)
    mdversion = MD_DEFAULTS_VERSION_LATEST;

  if (mdversion > MD_DEFAULTS_VERSION_LATEST)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_UPGRADE,
                   "Unknown metadata version for upgrade: %" PRIu64 ".",
                   mdversion);
      return NULL;
    }

  if (modulemd_defaults_get_mdversion (self) == mdversion)
    {
      /* Already at this version, just copy it and return that */
      return modulemd_defaults_copy (self);
    }

  return NULL;
}


guint64
modulemd_defaults_get_mdversion (ModulemdDefaults *self)
{
  ModulemdDefaultsClass *klass;

  g_return_val_if_fail (self && MODULEMD_IS_DEFAULTS (self), 0);

  klass = MODULEMD_DEFAULTS_GET_CLASS (self);
  g_return_val_if_fail (klass->get_mdversion, 0);

  return klass->get_mdversion (self);
}

void
modulemd_defaults_set_modified (ModulemdDefaults *self, guint64 modified)
{
  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  ModulemdDefaultsPrivate *priv =
    modulemd_defaults_get_instance_private (self);

  priv->modified = modified;
}


guint64
modulemd_defaults_get_modified (ModulemdDefaults *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), 0);

  ModulemdDefaultsPrivate *priv =
    modulemd_defaults_get_instance_private (self);

  return priv->modified;
}


void
modulemd_defaults_set_module_name (ModulemdDefaults *self,
                                   const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_DEFAULTS (self));

  /* It is a coding error if we ever get a NULL name here */
  g_return_if_fail (module_name);

  /* It is a coding error if we ever get the default name here */
  g_return_if_fail (g_strcmp0 (module_name, DEF_DEFAULT_NAME_STRING));

  ModulemdDefaultsPrivate *priv =
    modulemd_defaults_get_instance_private (self);

  g_clear_pointer (&priv->module_name, g_free);
  priv->module_name = g_strdup (module_name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_NAME]);
}


const gchar *
modulemd_defaults_get_module_name (ModulemdDefaults *self)
{
  ModulemdDefaultsPrivate *priv =
    modulemd_defaults_get_instance_private (self);

  return priv->module_name;
}


static void
modulemd_defaults_get_property (GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
  ModulemdDefaults *self = MODULEMD_DEFAULTS (object);

  switch (prop_id)
    {
    case PROP_MODULE_NAME:
      g_value_set_string (value, modulemd_defaults_get_module_name (self));
      break;

    case PROP_MDVERSION:
      g_value_set_uint64 (value, modulemd_defaults_get_mdversion (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_defaults_set_property (GObject *object,
                                guint prop_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
  ModulemdDefaults *self = MODULEMD_DEFAULTS (object);

  switch (prop_id)
    {
    case PROP_MODULE_NAME:
      modulemd_defaults_set_module_name (self, g_value_get_string (value));
      break;

    case PROP_MDVERSION:
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_defaults_class_init (ModulemdDefaultsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_defaults_finalize;
  object_class->get_property = modulemd_defaults_get_property;
  object_class->set_property = modulemd_defaults_set_property;

  klass->copy = modulemd_defaults_default_copy;
  klass->validate = modulemd_defaults_default_validate;

  properties[PROP_MDVERSION] = g_param_spec_uint64 (
    "mdversion",
    "Metadata Version",
    "The metadata version of this Defaults object. Read-only.",
    0,
    MD_DEFAULTS_VERSION_LATEST,
    0,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  properties[PROP_MODULE_NAME] = g_param_spec_string (
    "module-name",
    "Module Name",
    "The name of the module to which these defaults apply.",
    DEF_DEFAULT_NAME_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_defaults_init (ModulemdDefaults *self)
{
}


ModulemdDefaults *
modulemd_defaults_merge (ModulemdDefaults *from,
                         ModulemdDefaults *into,
                         GError **error)
{
  g_autoptr (ModulemdDefaults) merged_defaults = NULL;
  guint64 mdversion;
  const gchar *module_name = NULL;
  guint64 from_modified;
  guint64 into_modified;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (from), NULL);
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (into), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  /* TODO: Upgrade defaults if either is a different mdversion. Right now, we
   * only have a single version of this document, so there's no need to worry
   * about it. For now, treat it as a failure so we don't forget to update this
   * location if we add a new version.
   */

  mdversion = modulemd_defaults_get_mdversion (into);
  g_return_val_if_fail (modulemd_defaults_get_mdversion (from) == mdversion,
                        NULL);
  g_return_val_if_fail (mdversion == MD_DEFAULTS_VERSION_ONE, NULL);

  from_modified = modulemd_defaults_get_modified (from);
  into_modified = modulemd_defaults_get_modified (into);

  if (from_modified > into_modified)
    {
      /* Just return 'from' if it has a higher modified value */
      return modulemd_defaults_copy (from);
    }
  else if (into_modified > from_modified)
    {
      /* Just return 'into' if it has a higher modified value */
      return modulemd_defaults_copy (into);
    }

  /* Modified value is the same, so we need to merge */

  module_name = modulemd_defaults_get_module_name (into);
  if (!g_str_equal (module_name, modulemd_defaults_get_module_name (from)))
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_VALIDATE,
                   "Module name mismatch in merge: %s != %s",
                   module_name,
                   modulemd_defaults_get_module_name (from));
      return NULL;
    }

  merged_defaults = modulemd_defaults_v1_merge (module_name,
                                                MODULEMD_DEFAULTS_V1 (from),
                                                MODULEMD_DEFAULTS_V1 (into),
                                                &nested_error);
  if (!merged_defaults)
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

  return g_steal_pointer (&merged_defaults);
}
