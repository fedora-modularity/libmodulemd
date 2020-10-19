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

#include "private/modulemd-build-config.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-packager-v3.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdPackagerV3
{
  GObject parent_instance;

  gchar *summary;
  gchar *description;
  GHashTable *module_licenses; /* string set */
  GVariant *xmd;
  GHashTable *build_configs; /* <string, Modulemd.BuildConfig> */
  gchar *community;
  gchar *documentation;
  gchar *tracker;
  GHashTable *profiles; /* <string, Modulemd.Profile> */
  GHashTable *rpm_api; /* string set */
  GHashTable *rpm_filters; /* string set */
  GHashTable *rpm_components; /* <string, Modulemd.ComponentRpm> */
  GHashTable *module_components; /* <string, Modulemd.ComponentModule */
};

G_DEFINE_TYPE (ModulemdPackagerV3, modulemd_packager_v3, G_TYPE_OBJECT)

ModulemdPackagerV3 *
modulemd_packager_v3_new (void)
{
  return g_object_new (MODULEMD_TYPE_PACKAGER_V3, NULL);
}

static void
modulemd_packager_v3_finalize (GObject *object)
{
  ModulemdPackagerV3 *self = (ModulemdPackagerV3 *)object;

  g_clear_pointer (&self->summary, g_free);
  g_clear_pointer (&self->description, g_free);
  g_clear_pointer (&self->module_licenses, g_hash_table_unref);
  g_clear_pointer (&self->xmd, g_variant_unref);
  g_clear_pointer (&self->build_configs, g_hash_table_unref);
  g_clear_pointer (&self->community, g_free);
  g_clear_pointer (&self->documentation, g_free);
  g_clear_pointer (&self->tracker, g_free);
  g_clear_pointer (&self->profiles, g_hash_table_unref);
  g_clear_pointer (&self->rpm_api, g_hash_table_unref);
  g_clear_pointer (&self->rpm_filters, g_hash_table_unref);
  g_clear_pointer (&self->rpm_components, g_hash_table_unref);
  g_clear_pointer (&self->module_components, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_packager_v3_parent_class)->finalize (object);
}

static void
modulemd_packager_v3_class_init (ModulemdPackagerV3Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_packager_v3_finalize;
}

static void
modulemd_packager_v3_init (ModulemdPackagerV3 *self)
{
  self->module_licenses =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  self->build_configs =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->profiles =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->rpm_api =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  self->rpm_filters =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  self->rpm_components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->module_components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}


/**
 * modulemd_packager_v3_copy:
 *
 * Returns: (transfer full): A newly-allocated deep-copy of this
 * #ModulemdPackagerV3 object. This object must be freed with g_object_unref().
 *
 * Since: 2.10
 */
ModulemdPackagerV3 *
modulemd_packager_v3_copy (ModulemdPackagerV3 *self)
{
  g_autoptr (ModulemdPackagerV3) copy = modulemd_packager_v3_new ();

  modulemd_packager_v3_set_summary (copy,
                                    modulemd_packager_v3_get_summary (self));

  modulemd_packager_v3_set_description (
    copy, modulemd_packager_v3_get_description (self));

  MODULEMD_REPLACE_SET (copy->module_licenses, self->module_licenses);

  modulemd_packager_v3_set_xmd (copy, modulemd_packager_v3_get_xmd (self));

  /* TODO: BuildConfigs */

  modulemd_packager_v3_set_community (
    copy, modulemd_packager_v3_get_community (self));

  modulemd_packager_v3_set_documentation (
    copy, modulemd_packager_v3_get_documentation (self));

  modulemd_packager_v3_set_tracker (copy,
                                    modulemd_packager_v3_get_tracker (self));

  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, self, profiles, modulemd_packager_v3_add_profile);

  return g_steal_pointer (&copy);
}


void
modulemd_packager_v3_set_summary (ModulemdPackagerV3 *self,
                                  const gchar *summary)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_clear_pointer (&self->summary, g_free);

  if (summary)
    {
      self->summary = g_strdup (summary);
    }
}


gchar *
modulemd_packager_v3_get_summary (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return self->summary;
}


void
modulemd_packager_v3_set_description (ModulemdPackagerV3 *self,
                                      const gchar *description)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_clear_pointer (&self->description, g_free);

  if (description)
    {
      self->description = g_strdup (description);
    }
}


gchar *
modulemd_packager_v3_get_description (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return self->description;
}


void
modulemd_packager_v3_add_module_license (ModulemdPackagerV3 *self,
                                         const gchar *license)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  if (!license)
    {
      return;
    }

  g_hash_table_add (self->module_licenses, g_strdup (license));
}


void
modulemd_packager_v3_remove_module_license (ModulemdPackagerV3 *self,
                                            const gchar *license)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  if (!license)
    {
      return;
    }

  g_hash_table_remove (self->module_licenses, license);
}


void
modulemd_packager_v3_set_xmd (ModulemdPackagerV3 *self, GVariant *xmd)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  /* Do nothing if we were passed the same pointer */
  if (self->xmd == xmd)
    {
      return;
    }

  g_clear_pointer (&self->xmd, g_variant_unref);
  self->xmd = modulemd_variant_deep_copy (xmd);
}

GVariant *
modulemd_packager_v3_get_xmd (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);
  return self->xmd;
}


void
modulemd_packager_v3_add_build_config (ModulemdPackagerV3 *self,
                                       ModulemdBuildConfig *buildconfig)
{
  if (!buildconfig)
    return;

  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));
  g_return_if_fail (MODULEMD_IS_BUILD_CONFIG (buildconfig));

  g_hash_table_replace (
    self->build_configs,
    g_strdup (modulemd_build_config_get_context (buildconfig)),
    modulemd_build_config_copy (buildconfig));
}


void
modulemd_packager_v3_clear_build_configs (ModulemdPackagerV3 *self)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));
  g_hash_table_remove_all (self->build_configs);
}


GStrv
modulemd_packager_v3_get_build_config_contexts (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->build_configs);
}


ModulemdBuildConfig *
modulemd_packager_v3_get_build_config (ModulemdPackagerV3 *self, const gchar *context)
{

  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return g_hash_table_lookup (self->build_configs, context);
}

void
modulemd_packager_v3_set_community (ModulemdPackagerV3 *self,
                                    const gchar *community)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_clear_pointer (&self->community, g_free);

  if (community)
    {
      self->community = g_strdup (community);
    }
}


const gchar *
modulemd_packager_v3_get_community (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return self->community;
}


void
modulemd_packager_v3_set_documentation (ModulemdPackagerV3 *self,
                                        const gchar *documentation)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_clear_pointer (&self->documentation, g_free);

  if (documentation)
    {
      self->documentation = g_strdup (documentation);
    }
}


const gchar *
modulemd_packager_v3_get_documentation (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return self->documentation;
}


void
modulemd_packager_v3_set_tracker (ModulemdPackagerV3 *self,
                                  const gchar *tracker)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_clear_pointer (&self->tracker, g_free);

  if (tracker)
    {
      self->tracker = g_strdup (tracker);
    }
}


const gchar *
modulemd_packager_v3_get_tracker (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return self->tracker;
}


void
modulemd_packager_v3_add_profile (ModulemdPackagerV3 *self,
                                  ModulemdProfile *profile)
{
  if (!profile)
    {
      return;
    }
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));
  g_return_if_fail (MODULEMD_IS_PROFILE (profile));

  ModulemdProfile *copied_profile = modulemd_profile_copy (profile);

  g_hash_table_replace (self->profiles,
                        g_strdup (modulemd_profile_get_name (profile)),
                        copied_profile);
}


void
modulemd_packager_v3_clear_profiles (ModulemdPackagerV3 *self)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove_all (self->profiles);
}


GStrv
modulemd_packager_v3_get_profile_names_as_strv (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->profiles);
}


ModulemdProfile *
modulemd_packager_v3_get_profile (ModulemdPackagerV3 *self,
                                  const gchar *profile_name)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return g_hash_table_lookup (self->profiles, profile_name);
}


void
modulemd_packager_v3_add_rpm_api (ModulemdPackagerV3 *self, const gchar *rpm)
{
  if (!rpm)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_add (self->rpm_api, g_strdup (rpm));
}


void
modulemd_packager_v3_replace_rpm_api (ModulemdPackagerV3 *self,
                                      GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  MODULEMD_REPLACE_SET (self->rpm_api, set);
}


void
modulemd_packager_v3_remove_rpm_api (ModulemdPackagerV3 *self,
                                     const gchar *rpm)
{
  if (!rpm)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove (self->rpm_api, rpm);
}


void
modulemd_packager_v3_clear_rpm_api (ModulemdPackagerV3 *self)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove_all (self->rpm_api);
}


GStrv
modulemd_packager_v3_get_rpm_api_as_strv (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_api);
}

void
modulemd_packager_v3_add_rpm_filter (ModulemdPackagerV3 *self,
                                     const gchar *rpm)
{
  if (!rpm)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_add (self->rpm_filters, g_strdup (rpm));
}


void
modulemd_packager_v3_replace_rpm_filters (ModulemdPackagerV3 *self,
                                          GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  MODULEMD_REPLACE_SET (self->rpm_filters, set);
}


void
modulemd_packager_v3_remove_rpm_filter (ModulemdPackagerV3 *self,
                                        const gchar *rpm)
{
  if (!rpm)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove (self->rpm_filters, rpm);
}


void
modulemd_packager_v3_clear_rpm_filters (ModulemdPackagerV3 *self)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove_all (self->rpm_filters);
}


GStrv
modulemd_packager_v3_get_rpm_filters_as_strv (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_filters);
}


void
modulemd_packager_v3_add_component (ModulemdPackagerV3 *self,
                                    ModulemdComponent *component)
{
  GHashTable *table = NULL;

  /* Do nothing if we were passed a NULL component */
  if (!component)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));
  g_return_if_fail (MODULEMD_IS_COMPONENT (component));

  if (MODULEMD_IS_COMPONENT_RPM (component))
    {
      table = self->rpm_components;
    }
  else if (MODULEMD_IS_COMPONENT_MODULE (component))
    {
      table = self->module_components;
    }
  else
    {
      /* Unknown component. Raise a warning and return */
      g_return_if_reached ();
    }

  /* Add the component to the table. This will replace an existing component
   * with the same name
   */
  g_hash_table_replace (table,
                        g_strdup (modulemd_component_get_key (component)),
                        modulemd_component_copy (component, NULL));
}


void
modulemd_packager_v3_remove_module_component (ModulemdPackagerV3 *self,
                                              const gchar *component_name)
{
  /* Do nothing if we were passed a NULL component_name */
  if (!component_name)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove (self->module_components, component_name);
}


void
modulemd_packager_v3_clear_module_components (ModulemdPackagerV3 *self)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove_all (self->module_components);
}


void
modulemd_packager_v3_remove_rpm_component (ModulemdPackagerV3 *self,
                                           const gchar *component_name)
{
  /* Do nothing if we were passed a NULL component_name */
  if (!component_name)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove (self->rpm_components, component_name);
}


void
modulemd_packager_v3_clear_rpm_components (ModulemdPackagerV3 *self)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove_all (self->rpm_components);
}


GStrv
modulemd_packager_v3_get_module_component_names_as_strv (
  ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->module_components);
}


GStrv
modulemd_packager_v3_get_rpm_component_names_as_strv (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_components);
}


ModulemdComponentModule *
modulemd_packager_v3_get_module_component (ModulemdPackagerV3 *self,
                                           const gchar *component_name)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return g_hash_table_lookup (self->module_components, component_name);
}


ModulemdComponentRpm *
modulemd_packager_v3_get_rpm_component (ModulemdPackagerV3 *self,
                                        const gchar *component_name)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return g_hash_table_lookup (self->rpm_components, component_name);
}
