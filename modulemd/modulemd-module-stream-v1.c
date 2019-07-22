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

#include "modulemd-buildopts.h"
#include "modulemd-component.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-errors.h"
#include "modulemd-module-stream.h"
#include "modulemd-module-stream-v1.h"
#include "modulemd-translation-entry.h"
#include "modulemd-profile.h"
#include "modulemd-service-level.h"
#include "private/glib-extensions.h"
#include "private/modulemd-buildopts-private.h"
#include "private/modulemd-component-private.h"
#include "private/modulemd-component-rpm-private.h"
#include "private/modulemd-component-module-private.h"
#include "private/modulemd-dependencies-private.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-profile-private.h"
#include "private/modulemd-service-level-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"


G_DEFINE_TYPE (ModulemdModuleStreamV1,
               modulemd_module_stream_v1,
               MODULEMD_TYPE_MODULE_STREAM)

enum
{
  PROP_0,
  PROP_ARCH,
  PROP_BUILDOPTS,
  PROP_COMMUNITY,
  PROP_DOCUMENTATION,
  PROP_TRACKER,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdModuleStreamV1 *
modulemd_module_stream_v1_new (const gchar *module_name,
                               const gchar *module_stream)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_MODULE_STREAM_V1,
                       "module-name", module_name,
                       "stream-name", module_stream,
                       NULL);
  // clang-format on
}


static void
modulemd_module_stream_v1_finalize (GObject *object)
{
  ModulemdModuleStreamV1 *self = MODULEMD_MODULE_STREAM_V1 (object);

  /* Properties */
  g_clear_object (&self->buildopts);
  g_clear_pointer (&self->community, g_free);
  g_clear_pointer (&self->description, g_free);
  g_clear_pointer (&self->documentation, g_free);
  g_clear_pointer (&self->summary, g_free);
  g_clear_pointer (&self->tracker, g_free);

  /* Internal Data Structures */
  g_clear_pointer (&self->module_components, g_hash_table_unref);
  g_clear_pointer (&self->rpm_components, g_hash_table_unref);

  g_clear_pointer (&self->content_licenses, g_hash_table_unref);
  g_clear_pointer (&self->module_licenses, g_hash_table_unref);

  g_clear_pointer (&self->profiles, g_hash_table_unref);

  g_clear_pointer (&self->rpm_api, g_hash_table_unref);

  g_clear_pointer (&self->rpm_artifacts, g_hash_table_unref);

  g_clear_pointer (&self->rpm_filters, g_hash_table_unref);

  g_clear_pointer (&self->servicelevels, g_hash_table_unref);

  g_clear_pointer (&self->buildtime_deps, g_hash_table_unref);
  g_clear_pointer (&self->runtime_deps, g_hash_table_unref);

  g_clear_pointer (&self->xmd, g_variant_unref);

  G_OBJECT_CLASS (modulemd_module_stream_v1_parent_class)->finalize (object);
}


/* ===== Properties ====== */


static guint64
modulemd_module_stream_v1_get_mdversion (ModulemdModuleStream *self)
{
  return MD_MODULESTREAM_VERSION_ONE;
}

void
modulemd_module_stream_v1_set_arch (ModulemdModuleStreamV1 *self,
                                    const gchar *arch)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  modulemd_module_stream_set_arch (MODULEMD_MODULE_STREAM (self), arch);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ARCH]);
}


const gchar *
modulemd_module_stream_v1_get_arch (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_module_stream_get_arch (MODULEMD_MODULE_STREAM (self));
}


void
modulemd_module_stream_v1_set_buildopts (ModulemdModuleStreamV1 *self,
                                         ModulemdBuildopts *buildopts)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_clear_object (&self->buildopts);
  self->buildopts = modulemd_buildopts_copy (buildopts);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BUILDOPTS]);
}


ModulemdBuildopts *
modulemd_module_stream_v1_get_buildopts (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return self->buildopts;
}


void
modulemd_module_stream_v1_set_community (ModulemdModuleStreamV1 *self,
                                         const gchar *community)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_clear_pointer (&self->community, g_free);
  self->community = g_strdup (community);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COMMUNITY]);
}


const gchar *
modulemd_module_stream_v1_get_community (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return self->community;
}


void
modulemd_module_stream_v1_set_description (ModulemdModuleStreamV1 *self,
                                           const gchar *description)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_clear_pointer (&self->description, g_free);
  self->description = g_strdup (description);
}


const gchar *
modulemd_module_stream_v1_get_description (ModulemdModuleStreamV1 *self,
                                           const gchar *locale)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  ModulemdTranslationEntry *entry =
    modulemd_module_stream_get_translation_entry (
      MODULEMD_MODULE_STREAM (self), locale);
  if (entry != NULL &&
      modulemd_translation_entry_get_description (entry) != NULL)
    return modulemd_translation_entry_get_description (entry);

  return self->description;
}


void
modulemd_module_stream_v1_set_documentation (ModulemdModuleStreamV1 *self,
                                             const gchar *documentation)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_clear_pointer (&self->documentation, g_free);
  self->documentation = g_strdup (documentation);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COMMUNITY]);
}


const gchar *
modulemd_module_stream_v1_get_documentation (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return self->documentation;
}


void
modulemd_module_stream_v1_set_summary (ModulemdModuleStreamV1 *self,
                                       const gchar *summary)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_clear_pointer (&self->summary, g_free);
  self->summary = g_strdup (summary);
}


const gchar *
modulemd_module_stream_v1_get_summary (ModulemdModuleStreamV1 *self,
                                       const gchar *locale)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  ModulemdTranslationEntry *entry =
    modulemd_module_stream_get_translation_entry (
      MODULEMD_MODULE_STREAM (self), locale);
  if (entry != NULL && modulemd_translation_entry_get_summary (entry) != NULL)
    return modulemd_translation_entry_get_summary (entry);

  return self->summary;
}


void
modulemd_module_stream_v1_set_tracker (ModulemdModuleStreamV1 *self,
                                       const gchar *tracker)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_clear_pointer (&self->tracker, g_free);
  self->tracker = g_strdup (tracker);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TRACKER]);
}


const gchar *
modulemd_module_stream_v1_get_tracker (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return self->tracker;
}


/* ===== Non-property Methods ===== */


void
modulemd_module_stream_v1_add_component (ModulemdModuleStreamV1 *self,
                                         ModulemdComponent *component)
{
  GHashTable *table = NULL;

  /* Do nothing if we were passed a NULL component */
  if (!component)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));
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
modulemd_module_stream_v1_remove_module_component (
  ModulemdModuleStreamV1 *self, const gchar *component_name)
{
  /* Do nothing if we were passed a NULL component_name */
  if (!component_name)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove (self->module_components, component_name);
}


void
modulemd_module_stream_v1_clear_module_components (
  ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->module_components);
}


void
modulemd_module_stream_v1_remove_rpm_component (ModulemdModuleStreamV1 *self,
                                                const gchar *component_name)
{
  /* Do nothing if we were passed a NULL component_name */
  if (!component_name)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove (self->rpm_components, component_name);
}


void
modulemd_module_stream_v1_clear_rpm_components (ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->rpm_components);
}


GStrv
modulemd_module_stream_v1_get_module_component_names_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->module_components);
}


GStrv
modulemd_module_stream_v1_get_rpm_component_names_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_components);
}


ModulemdComponentModule *
modulemd_module_stream_v1_get_module_component (ModulemdModuleStreamV1 *self,
                                                const gchar *component_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return g_hash_table_lookup (self->module_components, component_name);
}


ModulemdComponentRpm *
modulemd_module_stream_v1_get_rpm_component (ModulemdModuleStreamV1 *self,
                                             const gchar *component_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return g_hash_table_lookup (self->rpm_components, component_name);
}


void
modulemd_module_stream_v1_add_content_license (ModulemdModuleStreamV1 *self,
                                               const gchar *license)
{
  if (!license)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_add (self->content_licenses, g_strdup (license));
}


static void
modulemd_module_stream_v1_replace_content_licenses (
  ModulemdModuleStreamV1 *self, GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  MODULEMD_REPLACE_SET (self->content_licenses, set);
}


void
modulemd_module_stream_v1_clear_content_licenses (ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->content_licenses);
}


void
modulemd_module_stream_v1_add_module_license (ModulemdModuleStreamV1 *self,
                                              const gchar *license)
{
  if (!license)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_add (self->module_licenses, g_strdup (license));
}


static void
modulemd_module_stream_v1_replace_module_licenses (
  ModulemdModuleStreamV1 *self, GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  MODULEMD_REPLACE_SET (self->module_licenses, set);
}


void
modulemd_module_stream_v1_clear_module_licenses (ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->module_licenses);
}


void
modulemd_module_stream_v1_remove_content_license (ModulemdModuleStreamV1 *self,
                                                  const gchar *license)
{
  if (!license)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove (self->content_licenses, license);
}


void
modulemd_module_stream_v1_remove_module_license (ModulemdModuleStreamV1 *self,
                                                 const gchar *license)
{
  if (!license)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove (self->module_licenses, license);
}


GStrv
modulemd_module_stream_v1_get_content_licenses_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->content_licenses);
}


GStrv
modulemd_module_stream_v1_get_module_licenses_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->module_licenses);
}


void
modulemd_module_stream_v1_add_profile (ModulemdModuleStreamV1 *self,
                                       ModulemdProfile *profile)
{
  if (!profile)
    return;
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));
  g_return_if_fail (MODULEMD_IS_PROFILE (profile));

  ModulemdProfile *copied_profile = modulemd_profile_copy (profile);
  modulemd_profile_set_owner (copied_profile, MODULEMD_MODULE_STREAM (self));

  g_hash_table_replace (self->profiles,
                        g_strdup (modulemd_profile_get_name (profile)),
                        copied_profile);
}


void
modulemd_module_stream_v1_clear_profiles (ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->profiles);
}


GStrv
modulemd_module_stream_v1_get_profile_names_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->profiles);
}


ModulemdProfile *
modulemd_module_stream_v1_get_profile (ModulemdModuleStreamV1 *self,
                                       const gchar *profile_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return g_hash_table_lookup (self->profiles, profile_name);
}


void
modulemd_module_stream_v1_add_rpm_api (ModulemdModuleStreamV1 *self,
                                       const gchar *rpm)
{
  if (!rpm)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_add (self->rpm_api, g_strdup (rpm));
}


static void
modulemd_module_stream_v1_replace_rpm_api (ModulemdModuleStreamV1 *self,
                                           GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  MODULEMD_REPLACE_SET (self->rpm_api, set);
}


void
modulemd_module_stream_v1_remove_rpm_api (ModulemdModuleStreamV1 *self,
                                          const gchar *rpm)
{
  if (!rpm)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove (self->rpm_api, rpm);
}


void
modulemd_module_stream_v1_clear_rpm_api (ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->rpm_api);
}


GStrv
modulemd_module_stream_v1_get_rpm_api_as_strv (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_api);
}


void
modulemd_module_stream_v1_add_rpm_artifact (ModulemdModuleStreamV1 *self,
                                            const gchar *nevr)
{
  if (!nevr)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_add (self->rpm_artifacts, g_strdup (nevr));
}


static void
modulemd_module_stream_v1_replace_rpm_artifacts (ModulemdModuleStreamV1 *self,
                                                 GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  MODULEMD_REPLACE_SET (self->rpm_artifacts, set);
}


void
modulemd_module_stream_v1_remove_rpm_artifact (ModulemdModuleStreamV1 *self,
                                               const gchar *nevr)
{
  if (!nevr)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove (self->rpm_artifacts, nevr);
}


void
modulemd_module_stream_v1_clear_rpm_artifacts (ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->rpm_artifacts);
}


GStrv
modulemd_module_stream_v1_get_rpm_artifacts_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_artifacts);
}


void
modulemd_module_stream_v1_add_rpm_filter (ModulemdModuleStreamV1 *self,
                                          const gchar *rpm)
{
  if (!rpm)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_add (self->rpm_filters, g_strdup (rpm));
}


static void
modulemd_module_stream_v1_replace_rpm_filters (ModulemdModuleStreamV1 *self,
                                               GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  MODULEMD_REPLACE_SET (self->rpm_filters, set);
}


void
modulemd_module_stream_v1_remove_rpm_filter (ModulemdModuleStreamV1 *self,
                                             const gchar *rpm)
{
  if (!rpm)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove (self->rpm_filters, rpm);
}


void
modulemd_module_stream_v1_clear_rpm_filters (ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->rpm_filters);
}


GStrv
modulemd_module_stream_v1_get_rpm_filters_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_filters);
}


void
modulemd_module_stream_v1_add_servicelevel (ModulemdModuleStreamV1 *self,
                                            ModulemdServiceLevel *servicelevel)
{
  if (!servicelevel)
    return;
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));
  g_return_if_fail (MODULEMD_IS_SERVICE_LEVEL (servicelevel));

  g_hash_table_replace (
    self->servicelevels,
    g_strdup (modulemd_service_level_get_name (servicelevel)),
    modulemd_service_level_copy (servicelevel));
}


void
modulemd_module_stream_v1_clear_servicelevels (ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->servicelevels);
}


GStrv
modulemd_module_stream_v1_get_servicelevel_names_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->servicelevels);
}


ModulemdServiceLevel *
modulemd_module_stream_v1_get_servicelevel (ModulemdModuleStreamV1 *self,
                                            const gchar *servicelevel_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return g_hash_table_lookup (self->servicelevels, servicelevel_name);
}


void
modulemd_module_stream_v1_set_eol (ModulemdModuleStreamV1 *self, GDate *eol)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  /* The "eol" field in the YAML is a relic of an early iteration and has been
   * entirely replaced by the ServiceLevel concept. If we encounter it, we just
   * treat it as if it was the EOL value for a service level named "rawhide".
   */
  g_autoptr (ModulemdServiceLevel) sl = modulemd_service_level_new ("rawhide");
  modulemd_service_level_set_eol (sl, eol);

  return modulemd_module_stream_v1_add_servicelevel (self, sl);
}


GDate *
modulemd_module_stream_v1_get_eol (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  ModulemdServiceLevel *sl =
    modulemd_module_stream_v1_get_servicelevel (self, "rawhide");

  return modulemd_service_level_get_eol (sl);
}


void
modulemd_module_stream_v1_add_buildtime_requirement (
  ModulemdModuleStreamV1 *self,
  const gchar *module_name,
  const gchar *module_stream)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));
  g_return_if_fail (module_name && module_stream);

  g_hash_table_replace (
    self->buildtime_deps, g_strdup (module_name), g_strdup (module_stream));
}


static void
modulemd_module_stream_v1_replace_buildtime_deps (ModulemdModuleStreamV1 *self,
                                                  GHashTable *deps)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  if (deps)
    {
      g_hash_table_unref (self->buildtime_deps);
      self->buildtime_deps = modulemd_hash_table_deep_str_copy (deps);
    }
  else
    {
      g_hash_table_remove_all (self->buildtime_deps);
    }
}


void
modulemd_module_stream_v1_add_runtime_requirement (
  ModulemdModuleStreamV1 *self,
  const gchar *module_name,
  const gchar *module_stream)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));
  g_return_if_fail (module_name && module_stream);

  g_hash_table_replace (
    self->runtime_deps, g_strdup (module_name), g_strdup (module_stream));
}


static void
modulemd_module_stream_v1_replace_runtime_deps (ModulemdModuleStreamV1 *self,
                                                GHashTable *deps)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  if (deps)
    {
      g_hash_table_unref (self->runtime_deps);
      self->runtime_deps = modulemd_hash_table_deep_str_copy (deps);
    }
  else
    {
      g_hash_table_remove_all (self->runtime_deps);
    }
}


void
modulemd_module_stream_v1_remove_buildtime_requirement (
  ModulemdModuleStreamV1 *self, const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));
  g_return_if_fail (module_name);

  g_hash_table_remove (self->buildtime_deps, module_name);
}


void
modulemd_module_stream_v1_remove_runtime_requirement (
  ModulemdModuleStreamV1 *self, const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));
  g_return_if_fail (module_name);

  g_hash_table_remove (self->runtime_deps, module_name);
}


void
modulemd_module_stream_v1_clear_buildtime_requirements (
  ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->buildtime_deps);
}


void
modulemd_module_stream_v1_clear_runtime_requirements (
  ModulemdModuleStreamV1 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove_all (self->runtime_deps);
}


GStrv
modulemd_module_stream_v1_get_buildtime_modules_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->buildtime_deps);
}


GStrv
modulemd_module_stream_v1_get_runtime_modules_as_strv (
  ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->runtime_deps);
}


const gchar *
modulemd_module_stream_v1_get_buildtime_requirement_stream (
  ModulemdModuleStreamV1 *self, const gchar *module_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return g_hash_table_lookup (self->buildtime_deps, module_name);
}


const gchar *
modulemd_module_stream_v1_get_runtime_requirement_stream (
  ModulemdModuleStreamV1 *self, const gchar *module_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return g_hash_table_lookup (self->runtime_deps, module_name);
}


void
modulemd_module_stream_v1_set_xmd (ModulemdModuleStreamV1 *self, GVariant *xmd)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  /* Do nothing if we were passed the same pointer */
  if (self->xmd == xmd)
    return;

  g_clear_pointer (&self->xmd, g_variant_unref);
  self->xmd = modulemd_variant_deep_copy (xmd);
}

GVariant *
modulemd_module_stream_v1_get_xmd (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return self->xmd;
}


static gboolean
modulemd_module_stream_v1_equals (ModulemdModuleStream *self_1,
                                  ModulemdModuleStream *self_2)
{
  ModulemdModuleStreamV1 *v1_self_1 = NULL;
  ModulemdModuleStreamV1 *v1_self_2 = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self_1), FALSE);
  v1_self_1 = MODULEMD_MODULE_STREAM_V1 (self_1);
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self_2), FALSE);
  v1_self_2 = MODULEMD_MODULE_STREAM_V1 (self_2);

  if (!MODULEMD_MODULE_STREAM_CLASS (modulemd_module_stream_v1_parent_class)
         ->equals (self_1, self_2))
    {
      return FALSE;
    }

  /*Check property equality*/
  if (g_strcmp0 (v1_self_1->community, v1_self_2->community) != 0)
    return FALSE;

  if (g_strcmp0 (v1_self_1->description, v1_self_2->description) != 0)
    return FALSE;

  if (g_strcmp0 (v1_self_1->documentation, v1_self_2->documentation) != 0)
    return FALSE;

  if (g_strcmp0 (v1_self_1->summary, v1_self_2->summary) != 0)
    return FALSE;

  if (g_strcmp0 (v1_self_1->tracker, v1_self_2->tracker) != 0)
    return FALSE;

  if (!modulemd_buildopts_equals (v1_self_1->buildopts, v1_self_2->buildopts))
    return FALSE;

  if (!modulemd_hash_table_equals (v1_self_1->rpm_components,
                                   v1_self_2->rpm_components,
                                   modulemd_component_equals_wrapper))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (v1_self_1->module_components,
                                   v1_self_2->module_components,
                                   modulemd_component_equals_wrapper))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v1_self_1->module_licenses,
                                           v1_self_2->module_licenses))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v1_self_1->content_licenses,
                                           v1_self_2->content_licenses))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (v1_self_1->profiles,
                                   v1_self_2->profiles,
                                   modulemd_profile_equals_wrapper))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v1_self_1->rpm_api,
                                           v1_self_2->rpm_api))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v1_self_1->rpm_artifacts,
                                           v1_self_2->rpm_artifacts))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v1_self_1->rpm_filters,
                                           v1_self_2->rpm_filters))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (v1_self_1->servicelevels,
                                   v1_self_2->servicelevels,
                                   modulemd_service_level_equals_wrapper))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (
        v1_self_1->buildtime_deps, v1_self_2->buildtime_deps, g_str_equal))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (
        v1_self_1->runtime_deps, v1_self_2->runtime_deps, g_str_equal))
    {
      return FALSE;
    }

  if (v1_self_1->xmd == NULL && v1_self_2->xmd == NULL)
    return TRUE;

  if (v1_self_1->xmd == NULL || v1_self_2->xmd == NULL)
    return FALSE;

  if (!g_variant_equal (v1_self_1->xmd, v1_self_2->xmd))
    return FALSE;

  return TRUE;
}


static gboolean
modulemd_module_stream_v1_validate (ModulemdModuleStream *self, GError **error)
{
  GHashTableIter iter;
  gpointer key, value;
  gchar *nevra = NULL;
  ModulemdModuleStreamV1 *v1_self = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), FALSE);
  v1_self = MODULEMD_MODULE_STREAM_V1 (self);

  if (!MODULEMD_MODULE_STREAM_CLASS (modulemd_module_stream_v1_parent_class)
         ->validate (self, error))
    {
      return FALSE;
    }

  /* Make sure that mandatory fields are present */
  if (!modulemd_module_stream_v1_get_summary (v1_self, "C"))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "Summary is missing");
      return FALSE;
    }

  if (!modulemd_module_stream_v1_get_description (v1_self, "C"))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "Description is missing");
      return FALSE;
    }

  if (!g_hash_table_size (v1_self->module_licenses))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "Module license is missing");
      return FALSE;
    }

  /* Verify that the components are consistent with regards to buildorder and
   * buildafter values.
   */
  if (!modulemd_module_stream_validate_components (v1_self->rpm_components,
                                                   &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }

  /* Iterate through the artifacts and validate that they are in the proper
   * NEVRA format
   */
  g_hash_table_iter_init (&iter, v1_self->rpm_artifacts);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      nevra = (gchar *)key;
      if (!modulemd_validate_nevra (nevra))
        {
          g_set_error (error,
                       MODULEMD_ERROR,
                       MODULEMD_ERROR_VALIDATE,
                       "Artifact '%s' was not in valid N-E:V-R.A format.",
                       nevra);
          return FALSE;
        }
    }

  return TRUE;
}


static void
modulemd_module_stream_v1_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
  ModulemdModuleStreamV1 *self = MODULEMD_MODULE_STREAM_V1 (object);

  switch (prop_id)
    {
    case PROP_ARCH:
      g_value_set_string (value, modulemd_module_stream_v1_get_arch (self));
      break;

    case PROP_BUILDOPTS:
      g_value_set_object (value,
                          modulemd_module_stream_v1_get_buildopts (self));
      break;

    case PROP_COMMUNITY:
      g_value_set_string (value,
                          modulemd_module_stream_v1_get_community (self));
      break;

    case PROP_DOCUMENTATION:
      g_value_set_string (value,
                          modulemd_module_stream_v1_get_documentation (self));
      break;

    case PROP_TRACKER:
      g_value_set_string (value, modulemd_module_stream_v1_get_tracker (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_module_stream_v1_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
  ModulemdModuleStreamV1 *self = MODULEMD_MODULE_STREAM_V1 (object);

  switch (prop_id)
    {
    case PROP_ARCH:
      modulemd_module_stream_v1_set_arch (self, g_value_get_string (value));
      break;

    case PROP_BUILDOPTS:
      modulemd_module_stream_v1_set_buildopts (self,
                                               g_value_get_object (value));
      break;

    case PROP_COMMUNITY:
      modulemd_module_stream_v1_set_community (self,
                                               g_value_get_string (value));
      break;

    case PROP_DOCUMENTATION:
      modulemd_module_stream_v1_set_documentation (self,
                                                   g_value_get_string (value));
      break;

    case PROP_TRACKER:
      modulemd_module_stream_v1_set_tracker (self, g_value_get_string (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static ModulemdModuleStream *
modulemd_module_stream_v1_copy (ModulemdModuleStream *self,
                                const gchar *module_name,
                                const gchar *module_stream)
{
  ModulemdModuleStreamV1 *v1_self = NULL;
  g_autoptr (ModulemdModuleStreamV1) copy = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);
  v1_self = MODULEMD_MODULE_STREAM_V1 (self);

  copy = MODULEMD_MODULE_STREAM_V1 (
    MODULEMD_MODULE_STREAM_CLASS (modulemd_module_stream_v1_parent_class)
      ->copy (self, module_name, module_stream));

  /* Properties */
  STREAM_COPY_IF_SET (v1, copy, v1_self, arch);
  STREAM_COPY_IF_SET (v1, copy, v1_self, buildopts);
  STREAM_COPY_IF_SET (v1, copy, v1_self, community);
  STREAM_COPY_IF_SET_WITH_LOCALE (v1, copy, v1_self, description);
  STREAM_COPY_IF_SET (v1, copy, v1_self, documentation);
  STREAM_COPY_IF_SET_WITH_LOCALE (v1, copy, v1_self, summary);
  STREAM_COPY_IF_SET (v1, copy, v1_self, tracker);

  /* Internal Data Structures: With replace function */
  STREAM_REPLACE_HASHTABLE (v1, copy, v1_self, content_licenses);
  STREAM_REPLACE_HASHTABLE (v1, copy, v1_self, module_licenses);
  STREAM_REPLACE_HASHTABLE (v1, copy, v1_self, rpm_api);
  STREAM_REPLACE_HASHTABLE (v1, copy, v1_self, rpm_artifacts);
  STREAM_REPLACE_HASHTABLE (v1, copy, v1_self, rpm_filters);
  STREAM_REPLACE_HASHTABLE (v1, copy, v1_self, buildtime_deps);
  STREAM_REPLACE_HASHTABLE (v1, copy, v1_self, runtime_deps);

  /* Internal Data Structures: With add on value */
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v1_self, rpm_components, modulemd_module_stream_v1_add_component);
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v1_self, module_components, modulemd_module_stream_v1_add_component);
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v1_self, profiles, modulemd_module_stream_v1_add_profile);
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v1_self, servicelevels, modulemd_module_stream_v1_add_servicelevel);

  STREAM_COPY_IF_SET (v1, copy, v1_self, xmd);

  return MODULEMD_MODULE_STREAM (g_steal_pointer (&copy));
}


static gboolean
modulemd_module_stream_v1_depends_on_stream (ModulemdModuleStream *self,
                                             const gchar *module_name,
                                             const gchar *stream_name)
{
  const gchar *stream = NULL;
  ModulemdModuleStreamV1 *v1_self = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), FALSE);
  g_return_val_if_fail (module_name && stream_name, FALSE);

  v1_self = MODULEMD_MODULE_STREAM_V1 (self);

  stream = g_hash_table_lookup (v1_self->runtime_deps, module_name);
  if (!stream)
    {
      return FALSE;
    }

  return g_str_equal (stream, stream_name);
}


static gboolean
modulemd_module_stream_v1_build_depends_on_stream (ModulemdModuleStream *self,
                                                   const gchar *module_name,
                                                   const gchar *stream_name)
{
  const gchar *stream = NULL;
  ModulemdModuleStreamV1 *v1_self = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), FALSE);
  g_return_val_if_fail (module_name && stream_name, FALSE);

  v1_self = MODULEMD_MODULE_STREAM_V1 (self);

  stream = g_hash_table_lookup (v1_self->buildtime_deps, module_name);
  if (!stream)
    {
      return FALSE;
    }

  return g_str_equal (stream, stream_name);
}


static void
modulemd_module_stream_v1_class_init (ModulemdModuleStreamV1Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdModuleStreamClass *stream_class =
    MODULEMD_MODULE_STREAM_CLASS (object_class);

  object_class->finalize = modulemd_module_stream_v1_finalize;
  object_class->get_property = modulemd_module_stream_v1_get_property;
  object_class->set_property = modulemd_module_stream_v1_set_property;

  stream_class->get_mdversion = modulemd_module_stream_v1_get_mdversion;
  stream_class->copy = modulemd_module_stream_v1_copy;
  stream_class->equals = modulemd_module_stream_v1_equals;
  stream_class->validate = modulemd_module_stream_v1_validate;
  stream_class->depends_on_stream =
    modulemd_module_stream_v1_depends_on_stream;
  stream_class->build_depends_on_stream =
    modulemd_module_stream_v1_build_depends_on_stream;

  properties[PROP_ARCH] = g_param_spec_string (
    "arch",
    "Module Artifact Architecture",
    "The architecture of the produced artifacts",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_BUILDOPTS] =
    g_param_spec_object ("buildopts",
                         "Module Build Options",
                         "Build options for module components",
                         MODULEMD_TYPE_BUILDOPTS,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_COMMUNITY] = g_param_spec_string (
    "community",
    "Module Community Website",
    "The website address of the upstream community for this module",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_DOCUMENTATION] = g_param_spec_string (
    "documentation",
    "Module Documentation Website",
    "The website address of the upstream documentation for this module",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_TRACKER] = g_param_spec_string (
    "tracker",
    "Module Bug Tracker Website",
    "The website address of the upstream bug tracker for this module",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_module_stream_v1_init (ModulemdModuleStreamV1 *self)
{
  /* Properties */

  /* Internal Data Structures */
  self->module_components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  self->rpm_components =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->content_licenses =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  self->module_licenses =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  self->profiles =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->rpm_api =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  self->rpm_artifacts =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  self->rpm_filters =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  self->servicelevels =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  self->buildtime_deps =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  self->runtime_deps =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}


static gboolean
modulemd_module_stream_v1_parse_licenses (yaml_parser_t *parser,
                                          ModulemdModuleStreamV1 *modulestream,
                                          gboolean strict,
                                          GError **error);

static gboolean
modulemd_module_stream_v1_parse_servicelevels (
  yaml_parser_t *parser,
  ModulemdModuleStreamV1 *modulestream,
  gboolean strict,
  GError **error);

static gboolean
modulemd_module_stream_v1_parse_deps (yaml_parser_t *parser,
                                      ModulemdModuleStreamV1 *modulestream,
                                      gboolean strict,
                                      GError **error);

static gboolean
modulemd_module_stream_v1_parse_refs (yaml_parser_t *parser,
                                      ModulemdModuleStreamV1 *modulestream,
                                      gboolean strict,
                                      GError **error);

static gboolean
modulemd_module_stream_v1_parse_profiles (yaml_parser_t *parser,
                                          ModulemdModuleStreamV1 *modulestream,
                                          gboolean strict,
                                          GError **error);

static gboolean
modulemd_module_stream_v1_parse_components (
  yaml_parser_t *parser,
  ModulemdModuleStreamV1 *modulestream,
  gboolean strict,
  GError **error);

static GVariant *
modulemd_module_stream_v1_parse_raw (yaml_parser_t *parser,
                                     ModulemdModuleStreamV1 *modulestream,
                                     GError **error);


ModulemdModuleStreamV1 *
modulemd_module_stream_v1_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                      gboolean strict,
                                      GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdModuleStreamV1) modulestream = NULL;
  g_autoptr (GHashTable) set = NULL;
  g_autoptr (ModulemdBuildopts) buildopts = NULL;
  g_autoptr (GVariant) xmd = NULL;
  g_autoptr (GDate) eol = NULL;
  g_autoptr (ModulemdServiceLevel) sl = NULL;

  if (!modulemd_subdocument_info_get_data_parser (
        subdoc, &parser, strict, error))
    return NULL;

  guint64 version;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  modulestream = modulemd_module_stream_v1_new (NULL, NULL);

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

          /* Module Name */
          if (g_str_equal ((const gchar *)event.data.scalar.value, "name"))
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_set_module_name,
                MODULEMD_MODULE_STREAM (modulestream));
            }

          /* Module Stream Name */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "stream"))
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_set_stream_name,
                MODULEMD_MODULE_STREAM (modulestream));
            }

          /* Module Version */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "version"))
            {
              version = modulemd_yaml_parse_uint64 (&parser, &nested_error);
              if (nested_error)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }

              modulemd_module_stream_set_version (
                MODULEMD_MODULE_STREAM (modulestream), version);
            }

          /* Module Context */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "context"))
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_set_context,
                MODULEMD_MODULE_STREAM (modulestream));
            }

          /* Module Artifact Architecture */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "arch"))
            {
              MMD_SET_PARSED_YAML_STRING (&parser,
                                          error,
                                          modulemd_module_stream_v1_set_arch,
                                          modulestream);
            }

          /* Module Summary */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "summary"))
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_v1_set_summary,
                modulestream);
            }

          /* Module Description */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "description"))
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_v1_set_description,
                modulestream);
            }

          /* Service Levels */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "servicelevels"))
            {
              if (!modulemd_module_stream_v1_parse_servicelevels (
                    &parser, modulestream, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Licences */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "license"))
            {
              if (!modulemd_module_stream_v1_parse_licenses (
                    &parser, modulestream, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Extensible Metadata */
          else if (g_str_equal ((const gchar *)event.data.scalar.value, "xmd"))
            {
              xmd = modulemd_module_stream_v1_parse_raw (
                &parser, modulestream, &nested_error);
              if (!xmd)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
              modulemd_module_stream_v1_set_xmd (modulestream, xmd);
              g_clear_pointer (&xmd, g_variant_unref);
            }

          /* Dependencies */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "dependencies"))
            {
              if (!modulemd_module_stream_v1_parse_deps (
                    &parser, modulestream, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* References */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "references"))
            {
              if (!modulemd_module_stream_v1_parse_refs (
                    &parser, modulestream, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Profiles */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "profiles"))
            {
              if (!modulemd_module_stream_v1_parse_profiles (
                    &parser, modulestream, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* API */
          else if (g_str_equal ((const gchar *)event.data.scalar.value, "api"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                &parser, "rpms", strict, &nested_error);
              modulemd_module_stream_v1_replace_rpm_api (modulestream, set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          /* Filter */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "filter"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                &parser, "rpms", strict, &nested_error);
              modulemd_module_stream_v1_replace_rpm_filters (modulestream,
                                                             set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          /* Build Options */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "buildopts"))
            {
              buildopts =
                modulemd_buildopts_parse_yaml (&parser, strict, &nested_error);
              if (!buildopts)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }

              modulemd_module_stream_v1_set_buildopts (modulestream,
                                                       buildopts);
              g_clear_object (&buildopts);
            }

          /* Components */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "components"))
            {
              if (!modulemd_module_stream_v1_parse_components (
                    &parser, modulestream, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Artifacts */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "artifacts"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                &parser, "rpms", strict, &nested_error);
              if (!set)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
              modulemd_module_stream_v1_replace_rpm_artifacts (modulestream,
                                                               set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          /* EOL (Deprecated) */
          else if (g_str_equal ((const gchar *)event.data.scalar.value, "eol"))
            {
              eol = modulemd_yaml_parse_date (&parser, &nested_error);
              if (!eol)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse EOL date in data: %s",
                    nested_error->message);
                }

              /* We store EOL as the "rawhide" service level, according to the
               * spec.
               */
              sl = modulemd_service_level_new ("rawhide");
              modulemd_service_level_set_eol (sl, eol);
              modulemd_module_stream_v1_add_servicelevel (modulestream, sl);

              g_clear_object (&sl);
              g_clear_pointer (&eol, g_date_free);
            }


          /* Unknown key */
          else
            {
              SKIP_UNKNOWN (&parser,
                            NULL,
                            "Unexpected key in data: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;


        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event in ModuleStreamV1: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }


  if (!modulemd_module_stream_validate (MODULEMD_MODULE_STREAM (modulestream),
                                        &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }


  return g_steal_pointer (&modulestream);
}


static gboolean
modulemd_module_stream_v1_parse_licenses (yaml_parser_t *parser,
                                          ModulemdModuleStreamV1 *modulestream,
                                          gboolean strict,
                                          GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (GHashTable) set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Process through the mapping */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          if (in_map)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error,
                event,
                "Unexpected extra MAPPING_START event in licenses");
            }
          in_map = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          if (!in_map)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error, event, "Unexpected MAPPING_END event in licenses");
            }
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error, event, "Received scalar outside of mapping");
            }

          if (g_str_equal ((const gchar *)event.data.scalar.value, "module"))
            {
              set = modulemd_yaml_parse_string_set (parser, &nested_error);
              if (!set)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
              modulemd_module_stream_v1_replace_module_licenses (modulestream,
                                                                 set);
              g_clear_pointer (&set, g_hash_table_unref);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "content"))
            {
              set = modulemd_yaml_parse_string_set (parser, &nested_error);
              modulemd_module_stream_v1_replace_content_licenses (modulestream,
                                                                  set);
            }
          else
            {
              SKIP_UNKNOWN (parser,
                            FALSE,
                            "Unexpected key in licenses: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }

          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in licenses: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_module_stream_v1_parse_servicelevels (
  yaml_parser_t *parser,
  ModulemdModuleStreamV1 *modulestream,
  gboolean strict,
  GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  const gchar *name = NULL;
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Process through the mapping */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          if (in_map)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error,
                event,
                "Unexpected extra MAPPING_START event in servicelevels");
            }
          in_map = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          if (!in_map)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error, event, "Unexpected MAPPING_END event in servicelevels");
            }
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error, event, "Received scalar outside of mapping");
            }

          name = (const gchar *)event.data.scalar.value;

          sl = modulemd_service_level_parse_yaml (
            parser, name, strict, &nested_error);
          if (!sl)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }

          modulemd_module_stream_v1_add_servicelevel (modulestream, sl);
          g_clear_object (&sl);

          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in servicelevels: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}

static gboolean
modulemd_module_stream_v1_parse_deps (yaml_parser_t *parser,
                                      ModulemdModuleStreamV1 *modulestream,
                                      gboolean strict,
                                      GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (GHashTable) deptable = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Process through the sequence */
  /* We *must* get a MAPPING_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Got %s instead of SEQUENCE_START in dependencies.",
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
                           "buildrequires"))
            {
              deptable =
                modulemd_yaml_parse_string_string_map (parser, &nested_error);
              if (!deptable)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
              modulemd_module_stream_v1_replace_buildtime_deps (modulestream,
                                                                deptable);
              g_clear_pointer (&deptable, g_hash_table_unref);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "requires"))
            {
              deptable =
                modulemd_yaml_parse_string_string_map (parser, &nested_error);
              if (!deptable)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
              modulemd_module_stream_v1_replace_runtime_deps (modulestream,
                                                              deptable);
              g_clear_pointer (&deptable, g_hash_table_unref);
            }

          else
            {
              SKIP_UNKNOWN (parser,
                            FALSE,
                            "Unexpected key in dependencies: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }

          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in dependencies: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_module_stream_v1_parse_refs (yaml_parser_t *parser,
                                      ModulemdModuleStreamV1 *modulestream,
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
        "Got %s instead of MAPPING_START in dependencies.",
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

              modulemd_module_stream_v1_set_community (modulestream, scalar);
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

              modulemd_module_stream_v1_set_documentation (modulestream,
                                                           scalar);
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

              modulemd_module_stream_v1_set_tracker (modulestream, scalar);
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
            "Unexpected YAML event in dependencies: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_module_stream_v1_parse_profiles (yaml_parser_t *parser,
                                          ModulemdModuleStreamV1 *modulestream,
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

          modulemd_module_stream_v1_add_profile (modulestream, profile);
          g_clear_pointer (&profile, g_object_unref);
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error,
            event,
            "Unexpected YAML event in dependencies: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_module_stream_v1_parse_rpm_components (
  yaml_parser_t *parser,
  ModulemdModuleStreamV1 *modulestream,
  gboolean strict,
  GError **error);
static gboolean
modulemd_module_stream_v1_parse_module_components (
  yaml_parser_t *parser,
  ModulemdModuleStreamV1 *modulestream,
  gboolean strict,
  GError **error);


static gboolean
modulemd_module_stream_v1_parse_components (
  yaml_parser_t *parser,
  ModulemdModuleStreamV1 *modulestream,
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
              if (!modulemd_module_stream_v1_parse_rpm_components (
                    parser, modulestream, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "modules"))
            {
              if (!modulemd_module_stream_v1_parse_module_components (
                    parser, modulestream, strict, &nested_error))
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
              break;
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
modulemd_module_stream_v1_parse_rpm_components (
  yaml_parser_t *parser,
  ModulemdModuleStreamV1 *modulestream,
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
            &nested_error);
          if (!component)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
          modulemd_module_stream_v1_add_component (
            modulestream, (ModulemdComponent *)component);
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
modulemd_module_stream_v1_parse_module_components (
  yaml_parser_t *parser,
  ModulemdModuleStreamV1 *modulestream,
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
          modulemd_module_stream_v1_add_component (
            modulestream, (ModulemdComponent *)component);
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


static GVariant *
modulemd_module_stream_v1_parse_raw (yaml_parser_t *parser,
                                     ModulemdModuleStreamV1 *modulestream,
                                     GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (GVariant) variant = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

  switch (event.type)
    {
    case YAML_SCALAR_EVENT:
      variant =
        mmd_variant_from_scalar ((const gchar *)event.data.scalar.value);
      if (!variant)
        {
          MMD_YAML_ERROR_EVENT_EXIT (error, event, "Error parsing scalar");
        }
      break;

    case YAML_MAPPING_START_EVENT:
      variant = mmd_variant_from_mapping (parser, &nested_error);
      break;

    default:
      MMD_YAML_ERROR_EVENT_EXIT (error,
                                 event,
                                 "Unexpected YAML event in raw parsing: %s",
                                 mmd_yaml_get_event_name (event.type));
      break;
    }

  return g_steal_pointer (&variant);
}


gboolean
modulemd_module_stream_v1_emit_yaml (ModulemdModuleStreamV1 *self,
                                     yaml_emitter_t *emitter,
                                     GError **error)
{
  MODULEMD_INIT_TRACE ();
  if (!modulemd_module_stream_emit_yaml_base (
        MODULEMD_MODULE_STREAM (self), emitter, error))
    return FALSE;

  EMIT_KEY_VALUE_IF_SET (
    emitter, error, "arch", modulemd_module_stream_v1_get_arch (self));
  EMIT_KEY_VALUE (emitter, error, "summary", self->summary);
  EMIT_KEY_VALUE_FULL (emitter,
                       error,
                       "description",
                       self->description,
                       YAML_FOLDED_SCALAR_STYLE);

  EMIT_HASHTABLE_VALUES_IF_NON_EMPTY (emitter,
                                      error,
                                      "servicelevels",
                                      self->servicelevels,
                                      modulemd_service_level_emit_yaml);

  if (!NON_EMPTY_TABLE (self->module_licenses))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_EMIT,
                   "Module licenses is not allowed to be empty");
      return FALSE;
    }

  EMIT_SCALAR (emitter, error, "license");
  EMIT_MAPPING_START (emitter, error);
  EMIT_STRING_SET (emitter, error, "module", self->module_licenses);
  EMIT_STRING_SET_IF_NON_EMPTY (
    emitter, error, "content", self->content_licenses);
  EMIT_MAPPING_END (emitter, error);

  if (self->xmd != NULL)
    {
      EMIT_SCALAR (emitter, error, "xmd");
      if (!modulemd_yaml_emit_variant (emitter, self->xmd, error))
        return FALSE;
    }

  if (NON_EMPTY_TABLE (self->buildtime_deps) ||
      NON_EMPTY_TABLE (self->runtime_deps))
    {
      EMIT_SCALAR (emitter, error, "dependencies");
      EMIT_MAPPING_START (emitter, error);
      EMIT_HASHTABLE_KEY_VALUES_IF_NON_EMPTY (
        emitter, error, "buildrequires", self->buildtime_deps);
      EMIT_HASHTABLE_KEY_VALUES_IF_NON_EMPTY (
        emitter, error, "requires", self->runtime_deps);
      EMIT_MAPPING_END (emitter, error);
    }

  if (self->community || self->documentation || self->tracker)
    {
      EMIT_SCALAR (emitter, error, "references");
      EMIT_MAPPING_START (emitter, error);
      EMIT_KEY_VALUE_IF_SET (emitter, error, "community", self->community);
      EMIT_KEY_VALUE_IF_SET (
        emitter, error, "documentation", self->documentation);
      EMIT_KEY_VALUE_IF_SET (emitter, error, "tracker", self->tracker);
      EMIT_MAPPING_END (emitter, error);
    }

  EMIT_HASHTABLE_VALUES_IF_NON_EMPTY (
    emitter, error, "profiles", self->profiles, modulemd_profile_emit_yaml);

  if (NON_EMPTY_TABLE (self->rpm_api))
    {
      EMIT_SCALAR (emitter, error, "api");
      EMIT_MAPPING_START (emitter, error);
      EMIT_STRING_SET (emitter, error, "rpms", self->rpm_api);
      EMIT_MAPPING_END (emitter, error);
    }

  if (NON_EMPTY_TABLE (self->rpm_filters))
    {
      EMIT_SCALAR (emitter, error, "filter");
      EMIT_MAPPING_START (emitter, error);
      EMIT_STRING_SET (emitter, error, "rpms", self->rpm_filters);
      EMIT_MAPPING_END (emitter, error);
    }

  if (self->buildopts != NULL)
    {
      EMIT_SCALAR (emitter, error, "buildopts");
      EMIT_MAPPING_START (emitter, error);
      if (!modulemd_buildopts_emit_yaml (self->buildopts, emitter, error))
        return FALSE;
      EMIT_MAPPING_END (emitter, error);
    }

  if (NON_EMPTY_TABLE (self->rpm_components) ||
      NON_EMPTY_TABLE (self->module_components))
    {
      EMIT_SCALAR (emitter, error, "components");
      EMIT_MAPPING_START (emitter, error);
      EMIT_HASHTABLE_VALUES_IF_NON_EMPTY (emitter,
                                          error,
                                          "rpms",
                                          self->rpm_components,
                                          modulemd_component_rpm_emit_yaml);
      EMIT_HASHTABLE_VALUES_IF_NON_EMPTY (emitter,
                                          error,
                                          "modules",
                                          self->module_components,
                                          modulemd_component_module_emit_yaml);
      EMIT_MAPPING_END (emitter, error);
    }

  if (NON_EMPTY_TABLE (self->rpm_artifacts))
    {
      EMIT_SCALAR (emitter, error, "artifacts");
      EMIT_MAPPING_START (emitter, error);
      EMIT_STRING_SET (emitter, error, "rpms", self->rpm_artifacts);
      EMIT_MAPPING_END (emitter, error);
    }

  /* The "data" mapping */
  EMIT_MAPPING_END (emitter, error);
  /* The overall document mapping */
  EMIT_MAPPING_END (emitter, error);
  if (!mmd_emitter_end_document (emitter, error))
    return FALSE;

  return TRUE;
}
