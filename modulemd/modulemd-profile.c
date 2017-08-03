/* modulemd-profile.c
 *
 * Copyright (C) 2017 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <glib.h>
#include "modulemd.h"

enum
{
  PROFILE_PROP_0,

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

  ModulemdSimpleSet *rpms;
};

G_DEFINE_TYPE (ModulemdProfile, modulemd_profile, G_TYPE_OBJECT)

/**
 * modulemd_profile_set_description:
 * @description: the profile description.
 *
 * Sets the "description" property.
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
 */
const gchar *
modulemd_profile_get_description (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  return self->description;
}

/**
 * modulemd_profile_set_rpms:
 * @rpms: A #ModuleSimpleSet: The rpms to be installed by this profile.
 *
 * Assigns the set of RPMs that will be installed when this profile is
 * activated.
 */
void
modulemd_profile_set_rpms (ModulemdProfile *self, ModulemdSimpleSet *rpms)
{
  g_return_if_fail (MODULEMD_IS_PROFILE (self));
  g_return_if_fail (MODULEMD_IS_SIMPLESET (rpms));

  /* TODO: Test for differences before replacing */
  g_object_unref (self->rpms);
  self->rpms = g_object_ref (rpms);

  g_object_notify_by_pspec (G_OBJECT (self),
                            profile_properties[PROFILE_PROP_RPMS]);
}

/**
 * modulemd_profile_get_rpms:
 *
 * Retrieves the "rpms" for this profile
 *
 * Returns: (transfer full): a #SimpleSet containing the set of RPMs in the
 * "rpms" property.
 */
ModulemdSimpleSet *
modulemd_profile_get_rpms (ModulemdProfile *self)
{
  g_return_val_if_fail (MODULEMD_IS_PROFILE (self), NULL);

  return g_object_ref (self->rpms);
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
      g_value_set_string (value, modulemd_profile_get_description (self));
      break;

    case PROFILE_PROP_RPMS:
      g_value_set_object (value, modulemd_profile_get_rpms (self));
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
