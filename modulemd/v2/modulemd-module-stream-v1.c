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
#include "private/modulemd-util.h"

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


void
modulemd_module_stream_v1_add_module_license (ModulemdModuleStreamV1 *self,
                                              const gchar *license)
{
  if (!license)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V1 (self));

  g_hash_table_add (self->module_licenses, g_strdup (license));
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
