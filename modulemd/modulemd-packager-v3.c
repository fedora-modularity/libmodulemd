/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat, Inc.
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#include "private/modulemd-build-config-private.h"
#include "private/modulemd-component-private.h"
#include "private/modulemd-component-module-private.h"
#include "private/modulemd-component-rpm-private.h"
#include "private/modulemd-defaults-v1-private.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-packager-v3-private.h"
#include "private/modulemd-profile-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdPackagerV3
{
  GObject parent_instance;

  gchar *module_name;
  gchar *stream_name;
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

  g_clear_pointer (&self->module_name, g_free);
  g_clear_pointer (&self->stream_name, g_free);
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


ModulemdPackagerV3 *
modulemd_packager_v3_copy (ModulemdPackagerV3 *self)
{
  g_autoptr (ModulemdPackagerV3) copy = modulemd_packager_v3_new ();

  modulemd_packager_v3_set_module_name (
    copy, modulemd_packager_v3_get_module_name (self));

  modulemd_packager_v3_set_stream_name (
    copy, modulemd_packager_v3_get_stream_name (self));

  modulemd_packager_v3_set_summary (copy,
                                    modulemd_packager_v3_get_summary (self));

  modulemd_packager_v3_set_description (
    copy, modulemd_packager_v3_get_description (self));

  MODULEMD_REPLACE_SET (copy->module_licenses, self->module_licenses);

  modulemd_packager_v3_set_xmd (copy, modulemd_packager_v3_get_xmd (self));

  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, self, build_configs, modulemd_packager_v3_add_build_config);

  modulemd_packager_v3_set_community (
    copy, modulemd_packager_v3_get_community (self));

  modulemd_packager_v3_set_documentation (
    copy, modulemd_packager_v3_get_documentation (self));

  modulemd_packager_v3_set_tracker (copy,
                                    modulemd_packager_v3_get_tracker (self));

  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, self, profiles, modulemd_packager_v3_add_profile);

  modulemd_packager_v3_replace_rpm_api (copy, self->rpm_api);

  modulemd_packager_v3_replace_rpm_filters (copy, self->rpm_filters);

  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, self, rpm_components, modulemd_packager_v3_add_component);

  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, self, module_components, modulemd_packager_v3_add_component);

  return g_steal_pointer (&copy);
}


void
modulemd_packager_v3_set_module_name (ModulemdPackagerV3 *self,
                                      const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_clear_pointer (&self->module_name, g_free);

  if (module_name)
    {
      self->module_name = g_strdup (module_name);
    }
}


const gchar *
modulemd_packager_v3_get_module_name (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return self->module_name;
}


void
modulemd_packager_v3_set_stream_name (ModulemdPackagerV3 *self,
                                      const gchar *stream_name)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_clear_pointer (&self->stream_name, g_free);

  if (stream_name)
    {
      self->stream_name = g_strdup (stream_name);
    }
}


const gchar *
modulemd_packager_v3_get_stream_name (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return self->stream_name;
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


const gchar *
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


const gchar *
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
modulemd_packager_v3_clear_module_licenses (ModulemdPackagerV3 *self)
{
  g_return_if_fail (MODULEMD_IS_PACKAGER_V3 (self));

  g_hash_table_remove_all (self->module_licenses);
}


GStrv
modulemd_packager_v3_get_module_licenses_as_strv (ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->module_licenses);
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
modulemd_packager_v3_get_build_config_contexts_as_strv (
  ModulemdPackagerV3 *self)
{
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->build_configs);
}


ModulemdBuildConfig *
modulemd_packager_v3_get_build_config (ModulemdPackagerV3 *self,
                                       const gchar *context)
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


gboolean
modulemd_packager_v3_to_defaults (ModulemdPackagerV3 *self,
                                  ModulemdDefaults **defaults_ptr,
                                  GError **error)
{
  g_autoptr (ModulemdDefaultsV1) defaults = NULL;
  ModulemdProfile *profile;
  g_autoptr (GError) nested_error = NULL;
  GHashTableIter iter;
  gpointer value;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (defaults_ptr == NULL || *defaults_ptr == NULL, FALSE);
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), FALSE);

  g_hash_table_iter_init (&iter, self->profiles);
  while (g_hash_table_iter_next (&iter, NULL, &value))
    {
      profile = MODULEMD_PROFILE (value);
      if (modulemd_profile_is_default (profile))
        {
          if (!defaults)
            {
              defaults = modulemd_defaults_v1_new (self->module_name);
            }
          modulemd_defaults_v1_add_default_profile_for_stream (
            defaults,
            self->stream_name,
            modulemd_profile_get_name (profile),
            NULL);
        }
    }

  if (!defaults)
    {
      return TRUE;
    }

  if (!modulemd_defaults_validate (MODULEMD_DEFAULTS (defaults),
                                   &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }

  *defaults_ptr = MODULEMD_DEFAULTS (g_steal_pointer (&defaults));
  return TRUE;
}

ModulemdModuleStreamV2 *
modulemd_packager_v3_to_stream_v2 (ModulemdPackagerV3 *self, GError **error)
{
  g_autoptr (ModulemdModuleStreamV2) v2_stream = NULL;
  g_autoptr (ModulemdDependencies) deps = NULL;
  g_autoptr (ModulemdProfile) profile = NULL;
  g_auto (GStrv) modules = NULL;
  g_auto (GStrv) contexts = NULL;
  g_autoptr (GError) nested_error = NULL;
  ModulemdBuildopts *buildopts = NULL;
  ModulemdBuildConfig *bc;
  GHashTableIter iter;
  gpointer value;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  v2_stream = modulemd_module_stream_v2_new (
    modulemd_packager_v3_get_module_name (self),
    modulemd_packager_v3_get_stream_name (self));

  modulemd_module_stream_v2_set_summary (
    v2_stream, modulemd_packager_v3_get_summary (self));

  modulemd_module_stream_v2_set_description (
    v2_stream, modulemd_packager_v3_get_description (self));

  /* Packager v3 "license" is optional. Stream v2 "license" is required
   * Fill in the default Packager v3 license if none has been specified.
   */
  if (g_hash_table_size (self->module_licenses) == 0)
    {
      modulemd_module_stream_v2_add_module_license (
        v2_stream, MMD_PACKAGER_DEFAULT_MODULE_LICENSE);
    }
  else
    {
      MODULEMD_REPLACE_SET (v2_stream->module_licenses, self->module_licenses);
    }

  modulemd_module_stream_v2_set_xmd (v2_stream,
                                     modulemd_packager_v3_get_xmd (self));

  modulemd_module_stream_v2_set_community (
    v2_stream, modulemd_packager_v3_get_community (self));

  modulemd_module_stream_v2_set_documentation (
    v2_stream, modulemd_packager_v3_get_documentation (self));

  modulemd_module_stream_v2_set_tracker (
    v2_stream, modulemd_packager_v3_get_tracker (self));

  g_hash_table_iter_init (&iter, self->profiles);
  while (g_hash_table_iter_next (&iter, NULL, &value))
    {
      profile = modulemd_profile_copy (MODULEMD_PROFILE (value));
      modulemd_profile_unset_default (profile);
      modulemd_module_stream_v2_add_profile (v2_stream, profile);
      g_clear_object (&profile);
    }

  modulemd_module_stream_v2_replace_rpm_api (v2_stream, self->rpm_api);

  modulemd_module_stream_v2_replace_rpm_filters (v2_stream, self->rpm_filters);

  COPY_HASHTABLE_BY_VALUE_ADDER (
    v2_stream, self, rpm_components, modulemd_module_stream_v2_add_component);

  COPY_HASHTABLE_BY_VALUE_ADDER (v2_stream,
                                 self,
                                 module_components,
                                 modulemd_module_stream_v2_add_component);


  /* get the list of packager build configuration contexts */
  contexts = modulemd_packager_v3_get_build_config_contexts_as_strv (self);

  /* If there is exactly one build configuration, use it for the stream */
  /* context. Otherwise, leave the stream context unset. */
  if (g_strv_length (contexts) == 1)
    {
      modulemd_module_stream_set_context (MODULEMD_MODULE_STREAM (v2_stream),
                                          contexts[0]);
    }

  /* map each BuildConfig object to a Dependencies object */
  for (guint i = 0; i < g_strv_length (contexts); i++)
    {
      bc = modulemd_packager_v3_get_build_config (self, contexts[i]);

      if (i == 0)
        {
          /* Use the buildopts from the first build configuration to */
          /* set the stream buildopts. */
          buildopts = modulemd_build_config_get_buildopts (bc);
        }

      deps = modulemd_dependencies_new ();

      modulemd_dependencies_add_buildtime_stream (
        deps, "platform", modulemd_build_config_get_platform (bc));
      modulemd_dependencies_add_runtime_stream (
        deps, "platform", modulemd_build_config_get_platform (bc));

      modules = modulemd_build_config_get_buildtime_modules_as_strv (bc);
      for (guint j = 0; j < g_strv_length (modules); j++)
        {
          modulemd_dependencies_add_buildtime_stream (
            deps,
            modules[j],
            modulemd_build_config_get_buildtime_requirement_stream (
              bc, modules[j]));
        }
      g_clear_pointer (&modules, g_strfreev);

      modules = modulemd_build_config_get_runtime_modules_as_strv (bc);
      for (guint j = 0; j < g_strv_length (modules); j++)
        {
          modulemd_dependencies_add_runtime_stream (
            deps,
            modules[j],
            modulemd_build_config_get_runtime_requirement_stream (bc,
                                                                  modules[j]));
        }
      g_clear_pointer (&modules, g_strfreev);

      modulemd_module_stream_v2_add_dependencies (v2_stream, deps);
      g_clear_object (&deps);
    }
  g_clear_pointer (&contexts, g_strfreev);

  if (buildopts)
    {
      modulemd_module_stream_v2_set_buildopts (v2_stream, buildopts);
    }

  if (!modulemd_module_stream_validate (MODULEMD_MODULE_STREAM (v2_stream),
                                        &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

  return g_steal_pointer (&v2_stream);
}

ModulemdModuleIndex *
modulemd_packager_v3_to_stream_v2_ext (ModulemdPackagerV3 *self,
                                       GError **error)
{
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (ModulemdModuleStreamV2) v2_stream = NULL;
  g_autoptr (ModulemdDefaults) defaults = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  g_return_val_if_fail (MODULEMD_IS_PACKAGER_V3 (self), NULL);

  v2_stream = modulemd_packager_v3_to_stream_v2 (self, &nested_error);
  if (!v2_stream)
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

  /* autogen module/stream names if necessary for adding to index */

  modulemd_module_stream_set_autogen_module_name (
    MODULEMD_MODULE_STREAM (v2_stream), 0);
  modulemd_module_stream_set_autogen_stream_name (
    MODULEMD_MODULE_STREAM (v2_stream), 0);

  index = modulemd_module_index_new ();
  if (!modulemd_module_index_add_module_stream (
        index, MODULEMD_MODULE_STREAM (v2_stream), &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

  g_clear_object (&v2_stream);

  if (!modulemd_packager_v3_to_defaults (self, &defaults, &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

  if (defaults)
    {
      if (!modulemd_module_index_add_defaults (index, defaults, &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return NULL;
        }

      g_clear_object (&defaults);
    }

  return g_steal_pointer (&index);
}


static gboolean
modulemd_packager_v3_parse_build_configs (yaml_parser_t *parser,
                                          ModulemdPackagerV3 *packager,
                                          gboolean strict,
                                          GError **error);

static gboolean
modulemd_packager_v3_parse_refs (yaml_parser_t *parser,
                                 ModulemdPackagerV3 *packager,
                                 gboolean strict,
                                 GError **error);

static gboolean
modulemd_packager_v3_parse_profiles (yaml_parser_t *parser,
                                     ModulemdPackagerV3 *packager,
                                     gboolean strict,
                                     GError **error);

static gboolean
modulemd_packager_v3_parse_components (yaml_parser_t *parser,
                                       ModulemdPackagerV3 *packager,
                                       gboolean strict,
                                       GError **error);


ModulemdPackagerV3 *
modulemd_packager_v3_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                 GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  const gboolean strict = TRUE; /* PackagerV3 should always parse strictly */
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdPackagerV3) packager = NULL;
  g_autoptr (GHashTable) set = NULL;
  g_autoptr (GVariant) xmd = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (!modulemd_subdocument_info_get_data_parser (
        subdoc, &parser, strict, error))
    {
      return FALSE;
    }

  packager = modulemd_packager_v3_new ();

  /* Read the MAPPING_START */
  YAML_PARSER_PARSE_WITH_EXIT (&parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (
        error, event, "Data section did not begin with a map.");
    }

  /* Process through the mapping */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (&parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          /* Mapping Keys */
          if (g_str_equal ((const gchar *)event.data.scalar.value, "name"))
            {
              MMD_SET_PARSED_YAML_STRING (&parser,
                                          error,
                                          modulemd_packager_v3_set_module_name,
                                          packager);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "stream"))
            {
              MMD_SET_PARSED_YAML_STRING (&parser,
                                          error,
                                          modulemd_packager_v3_set_stream_name,
                                          packager);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "summary"))
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser, error, modulemd_packager_v3_set_summary, packager);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "description"))
            {
              MMD_SET_PARSED_YAML_STRING (&parser,
                                          error,
                                          modulemd_packager_v3_set_description,
                                          packager);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "license"))
            {
              set = modulemd_yaml_parse_string_set (&parser, &nested_error);
              MODULEMD_REPLACE_SET (packager->module_licenses, set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value, "xmd"))
            {
              xmd = mmd_parse_xmd (&parser, &nested_error);
              if (!xmd)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
              modulemd_packager_v3_set_xmd (packager, xmd);
              g_clear_pointer (&xmd, g_variant_unref);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "configurations"))
            {
              if (!modulemd_packager_v3_parse_build_configs (
                    &parser, packager, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "references"))
            {
              if (!modulemd_packager_v3_parse_refs (
                    &parser, packager, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "profiles"))
            {
              if (!modulemd_packager_v3_parse_profiles (
                    &parser, packager, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value, "api"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                &parser, "rpms", strict, &nested_error);
              modulemd_packager_v3_replace_rpm_api (packager, set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "filter"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                &parser, "rpms", strict, &nested_error);
              modulemd_packager_v3_replace_rpm_filters (packager, set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "components"))
            {
              if (!modulemd_packager_v3_parse_components (
                    &parser, packager, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          else
            {
              SKIP_UNKNOWN (&parser,
                            NULL,
                            "Unexpected key in packager v3 document: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }

          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event in ModuleStreamV2: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return g_steal_pointer (&packager);
}


static gboolean
modulemd_packager_v3_parse_build_configs (yaml_parser_t *parser,
                                          ModulemdPackagerV3 *packager,
                                          gboolean strict,
                                          GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (ModulemdBuildConfig) buildconfig = NULL;
  g_autoptr (GError) nested_error = NULL;

  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_SEQUENCE_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Unexpected YAML event in build_configs");
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_SEQUENCE_END_EVENT: done = TRUE; break;

        case YAML_MAPPING_START_EVENT:
          buildconfig =
            modulemd_build_config_parse_yaml (parser, strict, &nested_error);
          if (!buildconfig)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
          modulemd_packager_v3_add_build_config (packager, buildconfig);
          g_clear_object (&buildconfig);
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in build_config list");
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_packager_v3_parse_refs (yaml_parser_t *parser,
                                 ModulemdPackagerV3 *packager,
                                 gboolean strict,
                                 GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autofree gchar *scalar = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Process through the map */
  /* We *must* get a MAPPING_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Got %s instead of MAPPING_START in references.",
        mmd_yaml_get_event_name (event.type));
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          if (g_str_equal ((const gchar *)event.data.scalar.value,
                           "community"))
            {
              scalar = modulemd_yaml_parse_string (parser, &nested_error);
              if (!scalar)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }

              modulemd_packager_v3_set_community (packager, scalar);
              g_clear_pointer (&scalar, g_free);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "documentation"))
            {
              scalar = modulemd_yaml_parse_string (parser, &nested_error);
              if (!scalar)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }

              modulemd_packager_v3_set_documentation (packager, scalar);
              g_clear_pointer (&scalar, g_free);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "tracker"))
            {
              scalar = modulemd_yaml_parse_string (parser, &nested_error);
              if (!scalar)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }

              modulemd_packager_v3_set_tracker (packager, scalar);
              g_clear_pointer (&scalar, g_free);
            }

          else
            {
              SKIP_UNKNOWN (parser,
                            FALSE,
                            "Unexpected key in references: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in references: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_packager_v3_parse_profiles (yaml_parser_t *parser,
                                     ModulemdPackagerV3 *packager,
                                     gboolean strict,
                                     GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdProfile) profile = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Process through the map */
  /* We *must* get a MAPPING_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Got %s instead of MAPPING_START in profiles.",
        mmd_yaml_get_event_name (event.type));
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          profile = modulemd_profile_parse_yaml (
            parser,
            (const gchar *)event.data.scalar.value,
            strict,
            &nested_error);
          if (!profile)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }

          modulemd_packager_v3_add_profile (packager, profile);
          g_clear_pointer (&profile, g_object_unref);
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in profiles: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_packager_v3_parse_rpm_components (yaml_parser_t *parser,
                                           ModulemdPackagerV3 *packager,
                                           gboolean strict,
                                           GError **error);
static gboolean
modulemd_packager_v3_parse_module_components (yaml_parser_t *parser,
                                              ModulemdPackagerV3 *packager,
                                              gboolean strict,
                                              GError **error);


static gboolean
modulemd_packager_v3_parse_components (yaml_parser_t *parser,
                                       ModulemdPackagerV3 *packager,
                                       gboolean strict,
                                       GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Process through the sequence */
  /* We *must* get a MAPPING_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Got %s instead of MAPPING_START in components.",
        mmd_yaml_get_event_name (event.type));
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          if (g_str_equal ((const gchar *)event.data.scalar.value, "rpms"))
            {
              if (!modulemd_packager_v3_parse_rpm_components (
                    parser, packager, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "modules"))
            {
              if (!modulemd_packager_v3_parse_module_components (
                    parser, packager, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }

          else
            {
              SKIP_UNKNOWN (parser,
                            FALSE,
                            "Unexpected key in components: %s",
                            (const gchar *)event.data.scalar.value);
            }

          break;


        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in components: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_packager_v3_parse_rpm_components (yaml_parser_t *parser,
                                           ModulemdPackagerV3 *packager,
                                           gboolean strict,
                                           GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdComponentRpm) component = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* We *must* get a MAPPING_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Got %s instead of MAPPING_START in rpm components.",
        mmd_yaml_get_event_name (event.type));
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          component = modulemd_component_rpm_parse_yaml (
            parser,
            (const gchar *)event.data.scalar.value,
            strict,
            FALSE,
            &nested_error);
          if (!component)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
          modulemd_packager_v3_add_component (packager,
                                              (ModulemdComponent *)component);
          g_clear_pointer (&component, g_object_unref);
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in RPM component: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_packager_v3_parse_module_components (yaml_parser_t *parser,
                                              ModulemdPackagerV3 *packager,
                                              gboolean strict,
                                              GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdComponentModule) component = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* We *must* get a MAPPING_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Got %s instead of MAPPING_START in module components.",
        mmd_yaml_get_event_name (event.type));
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          component = modulemd_component_module_parse_yaml (
            parser,
            (const gchar *)event.data.scalar.value,
            strict,
            &nested_error);
          if (!component)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
          modulemd_packager_v3_add_component (packager,
                                              (ModulemdComponent *)component);
          g_clear_pointer (&component, g_object_unref);
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in module component: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}
