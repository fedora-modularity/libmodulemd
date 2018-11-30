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
#include "modulemd-module-stream.h"
#include "modulemd-module-stream-v1.h"
#include "modulemd-profile.h"
#include "modulemd-service-level.h"
#include "private/glib-extensions.h"
#include "private/modulemd-buildopts-private.h"
#include "private/modulemd-component-rpm-private.h"
#include "private/modulemd-component-module-private.h"
#include "private/modulemd-dependencies-private.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-profile-private.h"
#include "private/modulemd-service-level-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdModuleStreamV1
{
  GObject parent_instance;

  /* Properties */
  gchar *arch;
  ModulemdBuildopts *buildopts;
  gchar *community;
  gchar *description;
  gchar *documentation;
  gchar *summary;
  gchar *tracker;

  /* Internal Data Structures */
  GHashTable *rpm_components; /* <string, Modulemd.ComponentRpm> */
  GHashTable *module_components; /* <string, Modulemd.ComponentModule */

  GHashTable *content_licenses; /* string set */
  GHashTable *module_licenses; /* string set */

  GHashTable *profiles; /* <string, Modulemd.Profile> */

  GHashTable *rpm_api; /* string set */

  GHashTable *rpm_artifacts; /* string set */

  GHashTable *rpm_filters; /* string set */

  GHashTable *servicelevels; /* <string, Modulemd.ServiceLevel */

  GHashTable *buildtime_deps; /* <string, string> */
  GHashTable *runtime_deps; /* <string, string> */

  GVariant *xmd;
};

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
  g_clear_pointer (&self->arch, g_free);
  g_clear_object (&self->buildopts);
  g_clear_pointer (&self->community, g_free);
  g_clear_pointer (&self->description, g_free);
  g_clear_pointer (&self->documentation, g_free);
  g_clear_pointer (&self->summary, g_free);

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

  g_clear_pointer (&self->arch, g_free);
  self->arch = g_strdup (arch);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ARCH]);
}


const gchar *
modulemd_module_stream_v1_get_arch (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return self->arch;
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

  /* TODO: retrieve translated strings */

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

  /* TODO: retrieve translated strings */

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
                        g_strdup (modulemd_component_get_name (component)),
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
modulemd_module_stream_v1_remove_rpm_component (ModulemdModuleStreamV1 *self,
                                                const gchar *component_name)
{
  /* Do nothing if we were passed a NULL component_name */
  if (!component_name)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_remove (self->rpm_components, component_name);
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

  g_hash_table_replace (self->profiles,
                        g_strdup (modulemd_profile_get_name (profile)),
                        modulemd_profile_copy (profile));
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

  g_clear_pointer (&self->xmd, g_variant_unref);
  self->xmd = xmd;
}

GVariant *
modulemd_module_stream_v1_get_xmd (ModulemdModuleStreamV1 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self), NULL);

  return self->xmd;
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
                                          GError **error);

static gboolean
modulemd_module_stream_v1_parse_servicelevels (
  yaml_parser_t *parser, ModulemdModuleStreamV1 *modulestream, GError **error);

static gboolean
modulemd_module_stream_v1_parse_deps (yaml_parser_t *parser,
                                      ModulemdModuleStreamV1 *modulestream,
                                      GError **error);

static gboolean
modulemd_module_stream_v1_parse_refs (yaml_parser_t *parser,
                                      ModulemdModuleStreamV1 *modulestream,
                                      GError **error);

static gboolean
modulemd_module_stream_v1_parse_profiles (yaml_parser_t *parser,
                                          ModulemdModuleStreamV1 *modulestream,
                                          GError **error);

static gboolean
modulemd_module_stream_v1_parse_components (
  yaml_parser_t *parser, ModulemdModuleStreamV1 *modulestream, GError **error);

static GVariant *
modulemd_module_stream_v1_parse_raw (yaml_parser_t *parser,
                                     ModulemdModuleStreamV1 *modulestream,
                                     GError **error);


ModulemdModuleStreamV1 *
modulemd_module_stream_v1_parse_yaml (yaml_parser_t *parser, GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdModuleStreamV1) modulestream = NULL;
  g_autoptr (GHashTable) set = NULL;
  g_autoptr (ModulemdBuildopts) buildopts = NULL;
  g_autoptr (GVariant) xmd = NULL;
  g_autoptr (GDate) eol = NULL;
  g_autoptr (ModulemdServiceLevel) sl = NULL;

  guint64 version;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  modulestream = modulemd_module_stream_v1_new (NULL, NULL);

  /* Read the MAPPING_START */
  YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (
        error, event, "Data section did not begin with a map.");
    }

  /* Process through the mapping */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          /* Mapping Keys */

          /* Module Name */
          if (g_str_equal ((const gchar *)event.data.scalar.value, "name"))
            {
              MMD_SET_PARSED_YAML_STRING (
                parser,
                error,
                modulemd_module_stream_set_module_name,
                MODULEMD_MODULE_STREAM (modulestream));
            }

          /* Module Stream Name */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "stream"))
            {
              MMD_SET_PARSED_YAML_STRING (
                parser,
                error,
                modulemd_module_stream_set_stream_name,
                MODULEMD_MODULE_STREAM (modulestream));
            }

          /* Module Version */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "version"))
            {
              version = modulemd_yaml_parse_uint64 (parser, &nested_error);
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
                parser,
                error,
                modulemd_module_stream_set_context,
                MODULEMD_MODULE_STREAM (modulestream));
            }

          /* Module Artifact Architecture */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "arch"))
            {
              MMD_SET_PARSED_YAML_STRING (parser,
                                          error,
                                          modulemd_module_stream_v1_set_arch,
                                          modulestream);
            }

          /* Module Summary */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "summary"))
            {
              MMD_SET_PARSED_YAML_STRING (
                parser,
                error,
                modulemd_module_stream_v1_set_summary,
                modulestream);
            }

          /* Module Description */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "description"))
            {
              MMD_SET_PARSED_YAML_STRING (
                parser,
                error,
                modulemd_module_stream_v1_set_description,
                modulestream);
            }

          /* Service Levels */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "servicelevels"))
            {
              if (!modulemd_module_stream_v1_parse_servicelevels (
                    parser, modulestream, &nested_error))
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
                    parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Extensible Metadata */
          else if (g_str_equal ((const gchar *)event.data.scalar.value, "xmd"))
            {
              xmd = modulemd_module_stream_v1_parse_raw (
                parser, modulestream, &nested_error);
              if (!xmd)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
              modulemd_module_stream_v1_set_xmd (modulestream, xmd);
              xmd = NULL;
            }

          /* Dependencies */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "dependencies"))
            {
              if (!modulemd_module_stream_v1_parse_deps (
                    parser, modulestream, &nested_error))
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
                    parser, modulestream, &nested_error))
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
                    parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* API */
          else if (g_str_equal ((const gchar *)event.data.scalar.value, "api"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                parser, "rpms", &nested_error);
              modulemd_module_stream_v1_replace_rpm_api (modulestream, set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          /* Filter */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "filter"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                parser, "rpms", &nested_error);
              modulemd_module_stream_v1_replace_rpm_filters (modulestream,
                                                             set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          /* Build Options */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "buildopts"))
            {
              buildopts =
                modulemd_buildopts_parse_yaml (parser, &nested_error);
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
                    parser, modulestream, &nested_error))
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
                parser, "rpms", &nested_error);
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
              eol = modulemd_yaml_parse_date (parser, &nested_error);
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
              MMD_YAML_ERROR_EVENT_EXIT (
                error,
                event,
                "Unexpected key in data: %s",
                (const gchar *)event.data.scalar.value);
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


  /* Make sure that mandatory fields are present */
  if (!modulemd_module_stream_v1_get_summary (modulestream, "C"))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "Summary is missing");
      return NULL;
    }

  if (!modulemd_module_stream_v1_get_description (modulestream, "C"))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "Description is missing");
      return NULL;
    }

  if (!g_hash_table_size (modulestream->module_licenses))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "Module license is missing");
      return NULL;
    }


  return g_steal_pointer (&modulestream);
}


static gboolean
modulemd_module_stream_v1_parse_licenses (yaml_parser_t *parser,
                                          ModulemdModuleStreamV1 *modulestream,
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
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error,
                event,
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
  yaml_parser_t *parser, ModulemdModuleStreamV1 *modulestream, GError **error)
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

          sl = modulemd_service_level_parse_yaml (parser, name, &nested_error);
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
                                      GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (ModulemdDependencies) deps = NULL;
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
            }

          else
            {
              MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                error,
                event,
                "Unknown key in references: %s",
                (const gchar *)event.data.scalar.value);
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
            parser, (const gchar *)event.data.scalar.value, &nested_error);
          if (!profile)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }

          modulemd_module_stream_v1_add_profile (modulestream, profile);
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
  yaml_parser_t *parser, ModulemdModuleStreamV1 *modulestream, GError **error);
static gboolean
modulemd_module_stream_v1_parse_module_components (
  yaml_parser_t *parser, ModulemdModuleStreamV1 *modulestream, GError **error);


static gboolean
modulemd_module_stream_v1_parse_components (
  yaml_parser_t *parser, ModulemdModuleStreamV1 *modulestream, GError **error)
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
                    parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "modules"))
            {
              if (!modulemd_module_stream_v1_parse_module_components (
                    parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
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
  yaml_parser_t *parser, ModulemdModuleStreamV1 *modulestream, GError **error)
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
            parser, (const gchar *)event.data.scalar.value, &nested_error);
          if (!component)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
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
  yaml_parser_t *parser, ModulemdModuleStreamV1 *modulestream, GError **error)
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
            parser, (const gchar *)event.data.scalar.value, &nested_error);
          if (!component)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
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
mmd_variant_from_scalar (const gchar *scalar);

static GVariant *
mmd_variant_from_mapping (yaml_parser_t *parser, GError **error);

static GVariant *
mmd_variant_from_sequence (yaml_parser_t *parser, GError **error);

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


static GVariant *
mmd_variant_from_scalar (const gchar *scalar)
{
  MODULEMD_INIT_TRACE ();
  GVariant *variant = NULL;

  g_debug ("Variant from scalar: %s", scalar);

  g_return_val_if_fail (scalar, NULL);

  /* Treat "TRUE" and "FALSE" as boolean values */
  if (g_str_equal (scalar, "TRUE"))
    {
      variant = g_variant_new_boolean (TRUE);
    }
  else if (g_str_equal (scalar, "FALSE"))
    {
      variant = g_variant_new_boolean (FALSE);
    }

  else if (scalar)
    {
      /* Any value we don't handle specifically becomes a string */
      variant = g_variant_new_string (scalar);
    }

  return g_variant_ref_sink (variant);
}


static GVariant *
mmd_variant_from_mapping (yaml_parser_t *parser, GError **error)
{
  MODULEMD_INIT_TRACE ();
  gboolean done = FALSE;
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);

  g_autoptr (GVariantDict) dict = NULL;
  g_autoptr (GVariant) value = NULL;
  g_autofree gchar *key = NULL;
  g_autoptr (GError) nested_error = NULL;

  dict = g_variant_dict_new (NULL);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT:
          /* We've processed the whole dictionary */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* All mapping keys must be scalars */
          key = g_strdup ((const gchar *)event.data.scalar.value);

          YAML_PARSER_PARSE_WITH_EXIT (parser, &value_event, error);

          switch (value_event.type)
            {
            case YAML_SCALAR_EVENT:
              value = mmd_variant_from_scalar (
                (const gchar *)value_event.data.scalar.value);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error, event, "Error parsing scalar");
                }
              break;

            case YAML_MAPPING_START_EVENT:
              value = mmd_variant_from_mapping (parser, &nested_error);
              if (!value)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                }
              break;

            case YAML_SEQUENCE_START_EVENT:
              value = mmd_variant_from_sequence (parser, &nested_error);
              if (!value)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                }
              break;

            default:
              /* We received a YAML event we shouldn't expect at this level */
              MMD_YAML_ERROR_EVENT_EXIT (
                error,
                event,
                "Unexpected YAML event in inner raw mapping: %s",
                mmd_yaml_get_event_name (value_event.type));
              break;
            }

          yaml_event_delete (&value_event);
          g_variant_dict_insert_value (dict, key, value);
          g_clear_pointer (&key, g_free);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event in raw mapping: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }

      yaml_event_delete (&event);
    }

  return g_variant_ref_sink (g_variant_dict_end (dict));
}


static GVariant *
mmd_variant_from_sequence (yaml_parser_t *parser, GError **error)
{
  MODULEMD_INIT_TRACE ();
  gboolean done = FALSE;
  MMD_INIT_YAML_EVENT (event);

  g_auto (GVariantBuilder) builder;
  g_autoptr (GVariant) value = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_variant_builder_init (&builder, G_VARIANT_TYPE_ARRAY);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_SEQUENCE_END_EVENT:
          /* We've processed the whole sequence */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          value =
            mmd_variant_from_scalar ((const gchar *)event.data.scalar.value);
          if (!value)
            {
              MMD_YAML_ERROR_EVENT_EXIT (error, event, "Error parsing scalar");
            }
          break;

        case YAML_MAPPING_START_EVENT:
          value = mmd_variant_from_mapping (parser, &nested_error);
          if (!value)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return NULL;
            }
          break;

        case YAML_SEQUENCE_START_EVENT:
          value = mmd_variant_from_sequence (parser, &nested_error);
          if (!value)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return NULL;
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event in raw sequence: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }

      g_variant_builder_add_value (&builder, g_variant_ref (value));

      yaml_event_delete (&event);
    }

  return g_variant_ref_sink (g_variant_builder_end (&builder));
}
