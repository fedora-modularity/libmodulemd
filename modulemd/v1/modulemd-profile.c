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

#include <glib.h>
#include "modulemd.h"

enum
{
  PROFILE_PROP_0,

  PROFILE_PROP_NAME,
  PROFILE_PROP_DESC,
  PROFILE_PROP_RPMS,

  PROFILE_N_PROPERTIES
};

static GParamSpec *profile_properties[PROFILE_N_PROPERTIES] = {
  NULL,
};

struct _ModulemdProfile
{
  GObject parent_instance;

  gchar *description;
  gchar *name;

  ModulemdSimpleSet *rpms;
};

G_DEFINE_TYPE (ModulemdProfile, modulemd_profile, G_TYPE_OBJECT)

/**
 * modulemd_profile_set_description:
 * @description: (nullable): the profile description.
 *
 * Sets the "description" property.
 *
 * Since: 1.0
 */
void
modulemd_profile_set_description (ModulemdProfile *self,
                                  const gchar *description)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));

  if (g_strcmp0 (self->description, description) != 0)
    {
      g_free (self->description);
      self->description = g_strdup (description);
      g_object_notify_by_pspec (G_OBJECT (self),
                                profile_properties[PROFILE_PROP_DESC]);
    }
}


/**
 * modulemd_profile_get_description:
 *
 * Retrieves the profile description.
 *
 * Returns: A string containing the "description" property.
 *
 * Deprecated: 1.1
 * Use peek_description() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_profile_peek_description)
const gchar *
modulemd_profile_get_description (ModulemdProfile *self)
{
  return modulemd_profile_peek_description (self);
}


/**
 * modulemd_profile_peek_description:
 *
 * Retrieves the profile description.
 *
 * Returns: A string containing the "description" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_profile_peek_description (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  return self->description;
}


/**
 * modulemd_profile_dup_description:
 *
 * Retrieves a copy of the profile description.
 *
 * Returns: A copy of the string containing the "description" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_profile_dup_description (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  return g_strdup (self->description);
}


/**
 * modulemd_profile_set_name:
 * @name: (nullable): the profile name.
 *
 * Sets the "name" property.
 *
 * Since: 1.0
 */
void
modulemd_profile_set_name (ModulemdProfile *self, const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));

  if (g_strcmp0 (self->name, name) != 0)
    {
      g_free (self->name);
      self->name = g_strdup (name);
      g_object_notify_by_pspec (G_OBJECT (self),
                                profile_properties[PROFILE_PROP_NAME]);
    }
}


/**
 * modulemd_profile_get_name:
 *
 * Retrieves the profile name.
 *
 * Returns: A string containing the "name" property.
 *
 * Deprecated: 1.1
 * Use peek_name() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_profile_peek_name)
const gchar *
modulemd_profile_get_name (ModulemdProfile *self)
{
  return modulemd_profile_peek_name (self);
}


/**
 * modulemd_profile_peek_name:
 *
 * Retrieves the profile name.
 *
 * Returns: A string containing the "name" property.
 *
 * Since: 1.1
 */
const gchar *
modulemd_profile_peek_name (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  return self->name;
}


/**
 * modulemd_profile_dup_name:
 *
 * Retrieves a copy of the profile name.
 *
 * Returns: A copy of string containing the "name" property.
 *
 * Since: 1.1
 */
gchar *
modulemd_profile_dup_name (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  return g_strdup (self->name);
}


/**
 * modulemd_profile_set_rpms:
 * @rpms: (nullable): A #ModuleSimpleSet: The rpms to be installed by this profile.
 *
 * Assigns the set of RPMs that will be installed when this profile is
 * activated.
 *
 * Since: 1.0
 */
void
modulemd_profile_set_rpms (ModulemdProfile *self, ModulemdSimpleSet *rpms)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));
  g_return_if_fail (!rpms || MODULEMD_IS_SIMPLESET (rpms));

  modulemd_simpleset_copy (rpms, &self->rpms);

  g_object_notify_by_pspec (G_OBJECT (self),
                            profile_properties[PROFILE_PROP_RPMS]);
}


/**
 * modulemd_profile_get_rpms:
 *
 * Retrieves the "rpms" for this profile
 *
 * Returns: (transfer none): a #SimpleSet containing the set of RPMs in the
 * "rpms" property.
 *
 * Deprecated: 1.1
 * Use peek_rpms() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_profile_peek_rpms)
ModulemdSimpleSet *
modulemd_profile_get_rpms (ModulemdProfile *self)
{
  return modulemd_profile_peek_rpms (self);
}


/**
 * modulemd_profile_peek_rpms:
 *
 * Retrieves the "rpms" for this profile
 *
 * Returns: (transfer none): a #SimpleSet containing the set of RPMs in the
 * "rpms" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_profile_peek_rpms (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  return self->rpms;
}


/**
 * modulemd_profile_dup_rpms:
 *
 * Retrieves a copy of the "rpms" for this profile
 *
 * Returns: (transfer full): a #SimpleSet containing the set of RPMs in the
 * "rpms" property.
 *
 * Since: 1.1
 */
ModulemdSimpleSet *
modulemd_profile_dup_rpms (ModulemdProfile *self)
{
  ModulemdSimpleSet *rpms = NULL;
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  modulemd_simpleset_copy (self->rpms, &rpms);

  return rpms;
}


void
modulemd_profile_add_rpm (ModulemdProfile *self, const gchar *rpm)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));

  modulemd_simpleset_add (self->rpms, rpm);

  g_object_notify_by_pspec (G_OBJECT (self),
                            profile_properties[PROFILE_PROP_RPMS]);
}

void
modulemd_profile_remove_rpm (ModulemdProfile *self, const gchar *rpm)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));

  modulemd_simpleset_remove (self->rpms, rpm);

  g_object_notify_by_pspec (G_OBJECT (self),
                            profile_properties[PROFILE_PROP_RPMS]);
}


/**
 * modulemd_profile_copy:
 *
 * Creates a copy of this profile
 *
 * Returns: (transfer full): a copy of this #ModulemdProfile
 *
 * Since: 1.1
 */
ModulemdProfile *
modulemd_profile_copy (ModulemdProfile *self)
{
  ModulemdProfile *new_profile = NULL;

  if (!self)
    return NULL;

  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  new_profile = modulemd_profile_new ();

  modulemd_profile_set_description (new_profile,
                                    modulemd_profile_peek_description (self));
  modulemd_profile_set_name (new_profile, modulemd_profile_peek_name (self));
  modulemd_profile_set_rpms (new_profile, modulemd_profile_peek_rpms (self));

  return new_profile;
}


static void
modulemd_profile_set_property (GObject *gobject,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
  ModulemdProfile *self = MODULEMD_PROFILE (gobject);

  switch (property_id)
    {
    case PROFILE_PROP_DESC:
      modulemd_profile_set_description (self, g_value_get_string (value));
      break;

    case PROFILE_PROP_NAME:
      modulemd_profile_set_name (self, g_value_get_string (value));
      break;

    case PROFILE_PROP_RPMS:
      modulemd_profile_set_rpms (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_profile_get_property (GObject *gobject,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec)
{
  ModulemdProfile *self = MODULEMD_PROFILE (gobject);

  switch (property_id)
    {
    case PROFILE_PROP_DESC:
      g_value_set_string (value, modulemd_profile_peek_description (self));
      break;

    case PROFILE_PROP_NAME:
      g_value_set_string (value, modulemd_profile_peek_name (self));
      break;

    case PROFILE_PROP_RPMS:
      g_value_set_object (value, modulemd_profile_peek_rpms (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_profile_finalize (GObject *gobject)
{
  ModulemdProfile *self = (ModulemdProfile *)gobject;

  g_clear_pointer (&self->description, g_free);
  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->rpms, g_object_unref);

  G_OBJECT_CLASS (modulemd_profile_parent_class)->finalize (gobject);
}

static void
modulemd_profile_class_init (ModulemdProfileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = modulemd_profile_set_property;
  object_class->get_property = modulemd_profile_get_property;
  object_class->finalize = modulemd_profile_finalize;

  profile_properties[PROFILE_PROP_DESC] =
    g_param_spec_string ("description",
                         "Profile description",
                         "A string property representing a detailed "
                         "description of the profile.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  profile_properties[PROFILE_PROP_NAME] =
    g_param_spec_string ("name",
                         "Profile name",
                         "A string property representing a short name of the "
                         "profile.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  profile_properties[PROFILE_PROP_RPMS] =
    g_param_spec_object ("rpms",
                         "Set of RPMs",
                         "A set of RPMs that will be installed when this "
                         "profile is activated.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (
    object_class, PROFILE_N_PROPERTIES, profile_properties);
}

static void
modulemd_profile_init (ModulemdProfile *self)
{
  self->rpms = modulemd_simpleset_new ();
}

ModulemdProfile *
modulemd_profile_new (void)
{
  return g_object_new (MODULEMD_TYPE_PROFILE, NULL);
}
