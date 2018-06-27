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

#include "modulemd.h"
#include "modulemd-buildopts.h"

struct _ModulemdBuildopts
{
  GObject parent_instance;

  gchar *rpm_macros;

  ModulemdSimpleSet *rpm_whitelist;
};

G_DEFINE_TYPE (ModulemdBuildopts, modulemd_buildopts, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_RPM_MACROS,
  PROP_RPM_WHITELIST,

  N_PROPS
};


static GParamSpec *properties[N_PROPS];

ModulemdBuildopts *
modulemd_buildopts_new (void)
{
  return g_object_new (MODULEMD_TYPE_BUILDOPTS, NULL);
}


static void
modulemd_buildopts_finalize (GObject *object)
{
  ModulemdBuildopts *self = (ModulemdBuildopts *)object;

  g_clear_pointer (&self->rpm_macros, g_free);
  g_clear_pointer (&self->rpm_whitelist, g_object_unref);

  G_OBJECT_CLASS (modulemd_buildopts_parent_class)->finalize (object);
}


/**
 * modulemd_buildopts_set_rpm_macros:
 * @macros: (nullable): A string containing RPM build macros in the form that
 * they would appear in an RPM macros file on-disk.
 *
 * Assigns RPM macros for the build-system.
 *
 * Since: 1.5
 */
void
modulemd_buildopts_set_rpm_macros (ModulemdBuildopts *self,
                                   const gchar *macros)
{
  g_return_if_fail (MODULEMD_IS_BUILDOPTS (self));

  if (g_strcmp0 (self->rpm_macros, macros))
    {
      g_free (self->rpm_macros);
      self->rpm_macros = g_strdup (macros);

      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_MACROS]);
    }
}


/**
 * modulemd_buildopts_get_rpm_macros:
 *
 * Retrieves a copy of the string containing RPM build macros in the form that
 * they would appear in an RPM macros file on-disk.
 *
 * Returns: a copy of the string containing RPM build macros in the form that
 * they would appear in an RPM macros file on-disk. The caller must free the
 * returned string with g_free() once finished with it. This function may
 * return NULL if no RPM macros have been set.
 *
 * Since: 1.5
 */
gchar *
modulemd_buildopts_get_rpm_macros (ModulemdBuildopts *self)
{
  g_return_val_if_fail (MODULEMD_IS_BUILDOPTS (self), NULL);
  return g_strdup (self->rpm_macros);
}


/**
 * modulemd_buildopts_set_rpm_whitelist:
 * @whitelist: (array zero-terminated=1) (transfer none): The set of RPM names
 * for the whitelist.
 *
 * This will make a copy of all of the unique items in @whitelist.
 *
 * Since: 1.5
 */
void
modulemd_buildopts_set_rpm_whitelist (ModulemdBuildopts *self, GStrv whitelist)
{
  g_return_if_fail (MODULEMD_IS_BUILDOPTS (self));

  g_clear_pointer (&self->rpm_whitelist, g_object_unref);

  if (!whitelist)
    return;

  self->rpm_whitelist = modulemd_simpleset_new ();
  for (gsize i = 0; whitelist[i]; i++)
    {
      modulemd_simpleset_add (self->rpm_whitelist, whitelist[i]);
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_WHITELIST]);
}


/**
 * modulemd_buildopts_set_rpm_whitelist_simpleset:
 * @whitelist: (transfer none): The #ModulemdSimpleSet set of RPM names
 * for the whitelist.
 *
 * This will make a copy of all of the unique items in @whitelist.
 *
 * Since: 1.5
 */
void
modulemd_buildopts_set_rpm_whitelist_simpleset (ModulemdBuildopts *self,
                                                ModulemdSimpleSet *whitelist)
{
  g_return_if_fail (MODULEMD_IS_BUILDOPTS (self));

  g_clear_pointer (&self->rpm_whitelist, g_object_unref);

  if (!whitelist)
    return;

  modulemd_simpleset_copy (whitelist, &self->rpm_whitelist);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RPM_WHITELIST]);
}


/**
 * modulemd_buildopts_get_rpm_whitelist:
 *
 * Returns a copy of the whitelist.
 *
 * Returns: (array zero-terminated=1) (transfer full): The set of RPM names
 * for the whitelist. May return NULL if no whitelist is stored.
 *
 * Since: 1.5
 */
GStrv
modulemd_buildopts_get_rpm_whitelist (ModulemdBuildopts *self)
{
  g_return_val_if_fail (MODULEMD_IS_BUILDOPTS (self), NULL);

  if (!self->rpm_whitelist)
    {
      return NULL;
    }

  return modulemd_simpleset_dup (self->rpm_whitelist);
}


/**
 * modulemd_buildopts_get_rpm_whitelist_simpleset:
 *
 * Returns a copy of the whitelist as a #ModulemdSimpleset
 *
 * Returns: (transfer full): The #ModulemdSimpleSet of RPM names
 * for the whitelist. May return NULL if no whitelist is stored.
 *
 * Since: 1.5
 */
ModulemdSimpleSet *
modulemd_buildopts_get_rpm_whitelist_simpleset (ModulemdBuildopts *self)
{
  g_autoptr (ModulemdSimpleSet) set = NULL;
  g_return_val_if_fail (MODULEMD_IS_BUILDOPTS (self), NULL);

  if (!self->rpm_whitelist)
    {
      return NULL;
    }

  modulemd_simpleset_copy (self->rpm_whitelist, &set);

  return g_object_ref (set);
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
      g_value_take_string (value, modulemd_buildopts_get_rpm_macros (self));
      break;
    case PROP_RPM_WHITELIST:
      g_value_take_boxed (value, modulemd_buildopts_get_rpm_whitelist (self));
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
    case PROP_RPM_WHITELIST:
      modulemd_buildopts_set_rpm_whitelist (self, g_value_get_boxed (value));
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
    "RPM Macros",
    "RPM Macros to be set in the buildsystem for this module.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_RPM_WHITELIST] =
    g_param_spec_boxed ("rpm-whitelist",
                        "The RPM whitelist.",
                        "A whitelist of RPM names that can be built for this "
                        "module stream. May differ from the components list.",
                        G_TYPE_STRV,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_buildopts_init (ModulemdBuildopts *self)
{
}


/**
 * modulemd_buildopts_copy:
 *
 * Make a deep copy of this #ModulemdBuildopts object.
 *
 * Returns: (transfer full): A deep copy of this #ModulemdBuildopts object.
 * This value must be freed with g_object_unref().
 *
 * Since: 1.5
 */
ModulemdBuildopts *
modulemd_buildopts_copy (ModulemdBuildopts *self)
{
  g_autoptr (ModulemdBuildopts) new = NULL;
  g_auto (GStrv) whitelist = NULL;

  if (!self)
    return NULL;

  new = modulemd_buildopts_new ();

  modulemd_buildopts_set_rpm_macros (new, self->rpm_macros);
  whitelist = modulemd_buildopts_get_rpm_whitelist (self);
  modulemd_buildopts_set_rpm_whitelist (new, whitelist);

  return g_object_ref (new);
}
