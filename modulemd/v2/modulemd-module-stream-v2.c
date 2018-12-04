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
#include "modulemd-module-stream-v2.h"
#include "modulemd-translation-entry.h"
#include "modulemd-profile.h"
#include "modulemd-service-level.h"
#include "private/modulemd-buildopts-private.h"
#include "private/modulemd-component-rpm-private.h"
#include "private/modulemd-component-module-private.h"
#include "private/modulemd-dependencies-private.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-profile-private.h"
#include "private/modulemd-service-level-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdModuleStreamV2
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
  GHashTable *module_components; /* <string, Modulemd.ComponentModule */
  GHashTable *rpm_components; /* <string, Modulemd.ComponentRpm> */

  GHashTable *content_licenses; /* string set */
  GHashTable *module_licenses; /* string set */

  GHashTable *profiles; /* <string, Modulemd.Profile> */

  GHashTable *rpm_api; /* string set */

  GHashTable *rpm_artifacts; /* string set */

  GHashTable *rpm_filters; /* string set */

  GHashTable *servicelevels; /* <string, Modulemd.ServiceLevel */

  GPtrArray *dependencies; /* <Modulemd.Dependencies> */

  GVariant *xmd;
};

G_DEFINE_TYPE (ModulemdModuleStreamV2,
               modulemd_module_stream_v2,
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

ModulemdModuleStreamV2 *
modulemd_module_stream_v2_new (const gchar *module_name,
                               const gchar *module_stream)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_MODULE_STREAM_V2,
                       "module-name", module_name,
                       "stream-name", module_stream,
                       NULL);
  // clang-format on
}


static void
modulemd_module_stream_v2_finalize (GObject *object)
{
  ModulemdModuleStreamV2 *self = MODULEMD_MODULE_STREAM_V2 (object);

  /* Properties */
  g_clear_pointer (&self->arch, g_free);
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

  g_clear_pointer (&self->dependencies, g_ptr_array_unref);

  g_clear_pointer (&self->xmd, g_variant_unref);

  G_OBJECT_CLASS (modulemd_module_stream_v2_parent_class)->finalize (object);
}


static guint64
modulemd_module_stream_v2_get_mdversion (ModulemdModuleStream *self)
{
  return MD_MODULESTREAM_VERSION_TWO;
}


void
modulemd_module_stream_v2_set_arch (ModulemdModuleStreamV2 *self,
                                    const gchar *arch)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_clear_pointer (&self->arch, g_free);
  self->arch = g_strdup (arch);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ARCH]);
}


const gchar *
modulemd_module_stream_v2_get_arch (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return self->arch;
}


void
modulemd_module_stream_v2_set_buildopts (ModulemdModuleStreamV2 *self,
                                         ModulemdBuildopts *buildopts)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_clear_object (&self->buildopts);
  self->buildopts = modulemd_buildopts_copy (buildopts);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BUILDOPTS]);
}


ModulemdBuildopts *
modulemd_module_stream_v2_get_buildopts (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return self->buildopts;
}


void
modulemd_module_stream_v2_set_community (ModulemdModuleStreamV2 *self,
                                         const gchar *community)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_clear_pointer (&self->community, g_free);
  self->community = g_strdup (community);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COMMUNITY]);
}


const gchar *
modulemd_module_stream_v2_get_community (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return self->community;
}


void
modulemd_module_stream_v2_set_description (ModulemdModuleStreamV2 *self,
                                           const gchar *description)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_clear_pointer (&self->description, g_free);
  self->description = g_strdup (description);
}


const gchar *
modulemd_module_stream_v2_get_description (ModulemdModuleStreamV2 *self,
                                           const gchar *locale)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  ModulemdTranslationEntry *entry =
    modulemd_module_stream_get_translation_entry (
      MODULEMD_MODULE_STREAM (self), locale);
  if (entry != NULL &&
      modulemd_translation_entry_get_description (entry) != NULL)
    return modulemd_translation_entry_get_description (entry);

  return self->description;
}


void
modulemd_module_stream_v2_set_documentation (ModulemdModuleStreamV2 *self,
                                             const gchar *documentation)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_clear_pointer (&self->documentation, g_free);
  self->documentation = g_strdup (documentation);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COMMUNITY]);
}


const gchar *
modulemd_module_stream_v2_get_documentation (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return self->documentation;
}


void
modulemd_module_stream_v2_set_summary (ModulemdModuleStreamV2 *self,
                                       const gchar *summary)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_clear_pointer (&self->summary, g_free);
  self->summary = g_strdup (summary);
}


const gchar *
modulemd_module_stream_v2_get_summary (ModulemdModuleStreamV2 *self,
                                       const gchar *locale)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  ModulemdTranslationEntry *entry =
    modulemd_module_stream_get_translation_entry (
      MODULEMD_MODULE_STREAM (self), locale);
  if (entry != NULL && modulemd_translation_entry_get_summary (entry) != NULL)
    return modulemd_translation_entry_get_summary (entry);

  return self->summary;
}


void
modulemd_module_stream_v2_set_tracker (ModulemdModuleStreamV2 *self,
                                       const gchar *tracker)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_clear_pointer (&self->tracker, g_free);
  self->tracker = g_strdup (tracker);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TRACKER]);
}


const gchar *
modulemd_module_stream_v2_get_tracker (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return self->tracker;
}


/* ===== Non-property Methods ===== */

void
modulemd_module_stream_v2_add_component (ModulemdModuleStreamV2 *self,
                                         ModulemdComponent *component)
{
  GHashTable *table = NULL;

  /* Do nothing if we were passed a NULL component */
  if (!component)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));
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
modulemd_module_stream_v2_remove_module_component (
  ModulemdModuleStreamV2 *self, const gchar *component_name)
{
  /* Do nothing if we were passed a NULL component_name */
  if (!component_name)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->module_components, component_name);
}


void
modulemd_module_stream_v2_remove_rpm_component (ModulemdModuleStreamV2 *self,
                                                const gchar *component_name)
{
  /* Do nothing if we were passed a NULL component_name */
  if (!component_name)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->rpm_components, component_name);
}


GStrv
modulemd_module_stream_v2_get_module_component_names_as_strv (
  ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->module_components);
}


GStrv
modulemd_module_stream_v2_get_rpm_component_names_as_strv (
  ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_components);
}


ModulemdComponentModule *
modulemd_module_stream_v2_get_module_component (ModulemdModuleStreamV2 *self,
                                                const gchar *component_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return g_hash_table_lookup (self->module_components, component_name);
}


ModulemdComponentRpm *
modulemd_module_stream_v2_get_rpm_component (ModulemdModuleStreamV2 *self,
                                             const gchar *component_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return g_hash_table_lookup (self->rpm_components, component_name);
}


void
modulemd_module_stream_v2_add_content_license (ModulemdModuleStreamV2 *self,
                                               const gchar *license)
{
  if (!license)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->content_licenses, g_strdup (license));
}


static void
modulemd_module_stream_v2_replace_content_licenses (
  ModulemdModuleStreamV2 *self, GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  MODULEMD_REPLACE_SET (self->content_licenses, set);
}


void
modulemd_module_stream_v2_add_module_license (ModulemdModuleStreamV2 *self,
                                              const gchar *license)
{
  if (!license)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->module_licenses, g_strdup (license));
}


static void
modulemd_module_stream_v2_replace_module_licenses (
  ModulemdModuleStreamV2 *self, GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  MODULEMD_REPLACE_SET (self->module_licenses, set);
}


void
modulemd_module_stream_v2_remove_content_license (ModulemdModuleStreamV2 *self,
                                                  const gchar *license)
{
  if (!license)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->content_licenses, license);
}


void
modulemd_module_stream_v2_remove_module_license (ModulemdModuleStreamV2 *self,
                                                 const gchar *license)
{
  if (!license)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->module_licenses, license);
}


GStrv
modulemd_module_stream_v2_get_content_licenses_as_strv (
  ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->content_licenses);
}


GStrv
modulemd_module_stream_v2_get_module_licenses_as_strv (
  ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->module_licenses);
}


void
modulemd_module_stream_v2_add_profile (ModulemdModuleStreamV2 *self,
                                       ModulemdProfile *profile)
{
  if (!profile)
    return;
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));
  g_return_if_fail (MODULEMD_IS_PROFILE (profile));

  g_hash_table_replace (self->profiles,
                        g_strdup (modulemd_profile_get_name (profile)),
                        modulemd_profile_copy (profile));
}


void
modulemd_module_stream_v2_clear_profiles (ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove_all (self->profiles);
}


GStrv
modulemd_module_stream_v2_get_profile_names_as_strv (
  ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->profiles);
}


ModulemdProfile *
modulemd_module_stream_v2_get_profile (ModulemdModuleStreamV2 *self,
                                       const gchar *profile_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return g_hash_table_lookup (self->profiles, profile_name);
}


void
modulemd_module_stream_v2_add_rpm_api (ModulemdModuleStreamV2 *self,
                                       const gchar *rpm)
{
  if (!rpm)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->rpm_api, g_strdup (rpm));
}


static void
modulemd_module_stream_v2_replace_rpm_api (ModulemdModuleStreamV2 *self,
                                           GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  MODULEMD_REPLACE_SET (self->rpm_api, set);
}


void
modulemd_module_stream_v2_remove_rpm_api (ModulemdModuleStreamV2 *self,
                                          const gchar *rpm)
{
  if (!rpm)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->rpm_api, rpm);
}

GStrv
modulemd_module_stream_v2_get_rpm_api_as_strv (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_api);
}


void
modulemd_module_stream_v2_add_rpm_artifact (ModulemdModuleStreamV2 *self,
                                            const gchar *nevr)
{
  if (!nevr)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->rpm_artifacts, g_strdup (nevr));
}


static void
modulemd_module_stream_v2_replace_rpm_artifacts (ModulemdModuleStreamV2 *self,
                                                 GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  MODULEMD_REPLACE_SET (self->rpm_artifacts, set);
}


void
modulemd_module_stream_v2_remove_rpm_artifact (ModulemdModuleStreamV2 *self,
                                               const gchar *nevr)
{
  if (!nevr)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->rpm_artifacts, nevr);
}


GStrv
modulemd_module_stream_v2_get_rpm_artifacts_as_strv (
  ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_artifacts);
}


void
modulemd_module_stream_v2_add_rpm_filter (ModulemdModuleStreamV2 *self,
                                          const gchar *rpm)
{
  if (!rpm)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->rpm_filters, g_strdup (rpm));
}


static void
modulemd_module_stream_v2_replace_rpm_filters (ModulemdModuleStreamV2 *self,
                                               GHashTable *set)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  MODULEMD_REPLACE_SET (self->rpm_filters, set);
}


void
modulemd_module_stream_v2_remove_rpm_filter (ModulemdModuleStreamV2 *self,
                                             const gchar *rpm)
{
  if (!rpm)
    return;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->rpm_filters, rpm);
}


GStrv
modulemd_module_stream_v2_get_rpm_filters_as_strv (
  ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_filters);
}


void
modulemd_module_stream_v2_add_servicelevel (ModulemdModuleStreamV2 *self,
                                            ModulemdServiceLevel *servicelevel)
{
  if (!servicelevel)
    return;
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));
  g_return_if_fail (MODULEMD_IS_SERVICE_LEVEL (servicelevel));

  g_hash_table_replace (
    self->servicelevels,
    g_strdup (modulemd_service_level_get_name (servicelevel)),
    modulemd_service_level_copy (servicelevel));
}


void
modulemd_module_stream_v2_clear_servicelevels (ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove_all (self->servicelevels);
}


GStrv
modulemd_module_stream_v2_get_servicelevel_names_as_strv (
  ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->servicelevels);
}


ModulemdServiceLevel *
modulemd_module_stream_v2_get_servicelevel (ModulemdModuleStreamV2 *self,
                                            const gchar *servicelevel_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return g_hash_table_lookup (self->servicelevels, servicelevel_name);
}


void
modulemd_module_stream_v2_add_dependencies (ModulemdModuleStreamV2 *self,
                                            ModulemdDependencies *deps)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_ptr_array_add (self->dependencies, modulemd_dependencies_copy (deps));
}


static void
modulemd_module_stream_v2_replace_dependencies (ModulemdModuleStreamV2 *self,
                                                GPtrArray *array)
{
  gsize i;
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  for (i = 0; i < array->len; i++)
    {
      modulemd_module_stream_v2_add_dependencies (
        self, g_ptr_array_index (array, i));
    }
}


GPtrArray *
modulemd_module_stream_v2_get_dependencies (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return self->dependencies;
}


void
modulemd_module_stream_v2_set_xmd (ModulemdModuleStreamV2 *self, GVariant *xmd)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_clear_pointer (&self->xmd, g_variant_unref);
  self->xmd = xmd;
}

GVariant *
modulemd_module_stream_v2_get_xmd (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);
  return self->xmd;
}

static void
modulemd_module_stream_v2_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
  ModulemdModuleStreamV2 *self = MODULEMD_MODULE_STREAM_V2 (object);

  switch (prop_id)
    {
    case PROP_ARCH:
      g_value_set_string (value, modulemd_module_stream_v2_get_arch (self));
      break;

    case PROP_BUILDOPTS:
      g_value_set_object (value,
                          modulemd_module_stream_v2_get_buildopts (self));
      break;

    case PROP_COMMUNITY:
      g_value_set_string (value,
                          modulemd_module_stream_v2_get_community (self));
      break;

    case PROP_DOCUMENTATION:
      g_value_set_string (value,
                          modulemd_module_stream_v2_get_documentation (self));
      break;

    case PROP_TRACKER:
      g_value_set_string (value, modulemd_module_stream_v2_get_tracker (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_module_stream_v2_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
  ModulemdModuleStreamV2 *self = MODULEMD_MODULE_STREAM_V2 (object);
  switch (prop_id)
    {
    case PROP_ARCH:
      modulemd_module_stream_v2_set_arch (self, g_value_get_string (value));
      break;

    case PROP_BUILDOPTS:
      modulemd_module_stream_v2_set_buildopts (self,
                                               g_value_get_object (value));
      break;

    case PROP_COMMUNITY:
      modulemd_module_stream_v2_set_community (self,
                                               g_value_get_string (value));
      break;

    case PROP_DOCUMENTATION:
      modulemd_module_stream_v2_set_documentation (self,
                                                   g_value_get_string (value));
      break;

    case PROP_TRACKER:
      modulemd_module_stream_v2_set_tracker (self, g_value_get_string (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static ModulemdModuleStream *
modulemd_module_stream_v2_copy (ModulemdModuleStream *self,
                                const gchar *module_name,
                                const gchar *module_stream)
{
  ModulemdModuleStreamV2 *v2_self = NULL;
  g_autoptr (ModulemdModuleStreamV2) copy = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);
  v2_self = MODULEMD_MODULE_STREAM_V2 (self);

  copy = MODULEMD_MODULE_STREAM_V2 (
    MODULEMD_MODULE_STREAM_CLASS (modulemd_module_stream_v2_parent_class)
      ->copy (self, module_name, module_stream));

  /* Properties */
  STREAM_COPY_IF_SET (v2, copy, v2_self, arch);
  STREAM_COPY_IF_SET (v2, copy, v2_self, buildopts);
  STREAM_COPY_IF_SET (v2, copy, v2_self, community);
  STREAM_COPY_IF_SET_WITH_LOCALE (v2, copy, v2_self, description);
  STREAM_COPY_IF_SET (v2, copy, v2_self, documentation);
  STREAM_COPY_IF_SET_WITH_LOCALE (v2, copy, v2_self, summary);
  STREAM_COPY_IF_SET (v2, copy, v2_self, tracker);

  /* Internal Data Structures: With replace function */
  STREAM_REPLACE_HASHTABLE (v2, copy, v2_self, content_licenses);
  STREAM_REPLACE_HASHTABLE (v2, copy, v2_self, module_licenses);
  STREAM_REPLACE_HASHTABLE (v2, copy, v2_self, rpm_api);
  STREAM_REPLACE_HASHTABLE (v2, copy, v2_self, rpm_artifacts);
  STREAM_REPLACE_HASHTABLE (v2, copy, v2_self, rpm_filters);

  /* Internal Data Structures: With add on value */
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v2_self, rpm_components, modulemd_module_stream_v2_add_component);
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v2_self, module_components, modulemd_module_stream_v2_add_component);
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v2_self, profiles, modulemd_module_stream_v2_add_profile);
  COPY_HASHTABLE_BY_VALUE_ADDER (
    copy, v2_self, servicelevels, modulemd_module_stream_v2_add_servicelevel);

  STREAM_REPLACE_HASHTABLE (v2, copy, v2_self, dependencies);

  if (v2_self->xmd != NULL)
    modulemd_module_stream_v2_set_xmd (copy, g_variant_ref (v2_self->xmd));

  return MODULEMD_MODULE_STREAM (g_steal_pointer (&copy));
}

static void
modulemd_module_stream_v2_class_init (ModulemdModuleStreamV2Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdModuleStreamClass *stream_class =
    MODULEMD_MODULE_STREAM_CLASS (object_class);

  object_class->finalize = modulemd_module_stream_v2_finalize;
  object_class->get_property = modulemd_module_stream_v2_get_property;
  object_class->set_property = modulemd_module_stream_v2_set_property;

  stream_class->get_mdversion = modulemd_module_stream_v2_get_mdversion;
  stream_class->copy = modulemd_module_stream_v2_copy;

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
modulemd_module_stream_v2_init (ModulemdModuleStreamV2 *self)
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

  /* The common case is for a single entry, so we'll optimize for that when
   * preallocating
   */
  self->dependencies = g_ptr_array_new_full (1, g_object_unref);
}


static gboolean
modulemd_module_stream_v2_parse_licenses (yaml_parser_t *parser,
                                          ModulemdModuleStreamV2 *modulestream,
                                          GError **error);

static gboolean
modulemd_module_stream_v2_parse_servicelevels (
  yaml_parser_t *parser, ModulemdModuleStreamV2 *modulestream, GError **error);

static gboolean
modulemd_module_stream_v2_parse_deps (yaml_parser_t *parser,
                                      ModulemdModuleStreamV2 *modulestream,
                                      GError **error);

static gboolean
modulemd_module_stream_v2_parse_refs (yaml_parser_t *parser,
                                      ModulemdModuleStreamV2 *modulestream,
                                      GError **error);

static gboolean
modulemd_module_stream_v2_parse_profiles (yaml_parser_t *parser,
                                          ModulemdModuleStreamV2 *modulestream,
                                          GError **error);

static gboolean
modulemd_module_stream_v2_parse_components (
  yaml_parser_t *parser, ModulemdModuleStreamV2 *modulestream, GError **error);

static GVariant *
modulemd_module_stream_v2_parse_raw (yaml_parser_t *parser,
                                     ModulemdModuleStreamV2 *modulestream,
                                     GError **error);


ModulemdModuleStreamV2 *
modulemd_module_stream_v2_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                      GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (ModulemdModuleStreamV2) modulestream = NULL;
  g_autoptr (GHashTable) set = NULL;
  g_autoptr (ModulemdBuildopts) buildopts = NULL;
  g_autoptr (GVariant) xmd = NULL;
  guint64 version;

  if (!modulemd_subdocument_info_get_data_parser (subdoc, &parser, error))
    return FALSE;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  modulestream = modulemd_module_stream_v2_new (NULL, NULL);

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
                                          modulemd_module_stream_v2_set_arch,
                                          modulestream);
            }

          /* Module Summary */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "summary"))
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_v2_set_summary,
                modulestream);
            }

          /* Module Description */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "description"))
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_v2_set_description,
                modulestream);
            }

          /* Service Levels */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "servicelevels"))
            {
              if (!modulemd_module_stream_v2_parse_servicelevels (
                    &parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Licences */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "license"))
            {
              if (!modulemd_module_stream_v2_parse_licenses (
                    &parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Extensible Metadata */
          else if (g_str_equal ((const gchar *)event.data.scalar.value, "xmd"))
            {
              xmd = modulemd_module_stream_v2_parse_raw (
                &parser, modulestream, &nested_error);
              if (!xmd)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
              modulemd_module_stream_v2_set_xmd (modulestream, xmd);
              xmd = NULL;
            }

          /* Dependencies */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "dependencies"))
            {
              if (!modulemd_module_stream_v2_parse_deps (
                    &parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* References */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "references"))
            {
              if (!modulemd_module_stream_v2_parse_refs (
                    &parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Profiles */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "profiles"))
            {
              if (!modulemd_module_stream_v2_parse_profiles (
                    &parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* API */
          else if (g_str_equal ((const gchar *)event.data.scalar.value, "api"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                &parser, "rpms", &nested_error);
              modulemd_module_stream_v2_replace_rpm_api (modulestream, set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          /* Filter */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "filter"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                &parser, "rpms", &nested_error);
              modulemd_module_stream_v2_replace_rpm_filters (modulestream,
                                                             set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          /* Build Options */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "buildopts"))
            {
              buildopts =
                modulemd_buildopts_parse_yaml (&parser, &nested_error);
              if (!buildopts)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }

              modulemd_module_stream_v2_set_buildopts (modulestream,
                                                       buildopts);
              g_clear_object (&buildopts);
            }

          /* Components */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "components"))
            {
              if (!modulemd_module_stream_v2_parse_components (
                    &parser, modulestream, &nested_error))
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
                &parser, "rpms", &nested_error);
              if (!set)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
              modulemd_module_stream_v2_replace_rpm_artifacts (modulestream,
                                                               set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

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
            "Unexpected YAML event in ModuleStreamV2: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }


  /* Make sure that mandatory fields are present */
  if (!modulemd_module_stream_v2_get_summary (modulestream, "C"))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_MISSING_REQUIRED,
                   "Summary is missing");
      return NULL;
    }

  if (!modulemd_module_stream_v2_get_description (modulestream, "C"))
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
modulemd_module_stream_v2_parse_licenses (yaml_parser_t *parser,
                                          ModulemdModuleStreamV2 *modulestream,
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
              modulemd_module_stream_v2_replace_module_licenses (modulestream,
                                                                 set);
              g_clear_pointer (&set, g_hash_table_unref);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "content"))
            {
              set = modulemd_yaml_parse_string_set (parser, &nested_error);
              modulemd_module_stream_v2_replace_content_licenses (modulestream,
                                                                  set);
              g_clear_pointer (&set, g_hash_table_unref);
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
modulemd_module_stream_v2_parse_servicelevels (
  yaml_parser_t *parser, ModulemdModuleStreamV2 *modulestream, GError **error)
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

          modulemd_module_stream_v2_add_servicelevel (modulestream, sl);
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
modulemd_module_stream_v2_parse_deps (yaml_parser_t *parser,
                                      ModulemdModuleStreamV2 *modulestream,
                                      GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (ModulemdDependencies) deps = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Process through the sequence */
  /* We *must* get a SEQUENCE_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_SEQUENCE_START_EVENT)
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
        case YAML_SEQUENCE_END_EVENT: done = TRUE; break;

        case YAML_MAPPING_START_EVENT:
          deps = modulemd_dependencies_parse_yaml (parser, &nested_error);
          if (!deps)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }

          modulemd_module_stream_v2_add_dependencies (modulestream, deps);
          g_clear_object (&deps);
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
modulemd_module_stream_v2_parse_refs (yaml_parser_t *parser,
                                      ModulemdModuleStreamV2 *modulestream,
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

              modulemd_module_stream_v2_set_community (modulestream, scalar);
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

              modulemd_module_stream_v2_set_documentation (modulestream,
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

              modulemd_module_stream_v2_set_tracker (modulestream, scalar);
              g_clear_pointer (&scalar, g_free);
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
modulemd_module_stream_v2_parse_profiles (yaml_parser_t *parser,
                                          ModulemdModuleStreamV2 *modulestream,
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

          modulemd_module_stream_v2_add_profile (modulestream, profile);
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
modulemd_module_stream_v2_parse_rpm_components (
  yaml_parser_t *parser, ModulemdModuleStreamV2 *modulestream, GError **error);
static gboolean
modulemd_module_stream_v2_parse_module_components (
  yaml_parser_t *parser, ModulemdModuleStreamV2 *modulestream, GError **error);


static gboolean
modulemd_module_stream_v2_parse_components (
  yaml_parser_t *parser, ModulemdModuleStreamV2 *modulestream, GError **error)
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
              if (!modulemd_module_stream_v2_parse_rpm_components (
                    parser, modulestream, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "modules"))
            {
              if (!modulemd_module_stream_v2_parse_module_components (
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
modulemd_module_stream_v2_parse_rpm_components (
  yaml_parser_t *parser, ModulemdModuleStreamV2 *modulestream, GError **error)
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
          modulemd_module_stream_v2_add_component (
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
modulemd_module_stream_v2_parse_module_components (
  yaml_parser_t *parser, ModulemdModuleStreamV2 *modulestream, GError **error)
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
          modulemd_module_stream_v2_add_component (
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
mmd_variant_from_scalar (const gchar *scalar);

static GVariant *
mmd_variant_from_mapping (yaml_parser_t *parser, GError **error);

static GVariant *
mmd_variant_from_sequence (yaml_parser_t *parser, GError **error);

static GVariant *
modulemd_module_stream_v2_parse_raw (yaml_parser_t *parser,
                                     ModulemdModuleStreamV2 *modulestream,
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
  gboolean empty_array = TRUE;
  GVariant *result = NULL;

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

      if (value)
        {
          g_variant_builder_add_value (&builder, g_variant_ref (value));
          empty_array = FALSE;
        }

      yaml_event_delete (&event);
    }

  if (empty_array)
    {
      /* If we got an empty array, treat it as a zero-length array of
       * GVariants
       */
      result = g_variant_new ("av", NULL);
    }
  else
    {
      /* Otherwise, finish it up */
      result = g_variant_builder_end (&builder);
    }

  return g_variant_ref_sink (result);
}

gboolean
modulemd_module_stream_v2_emit_yaml (ModulemdModuleStreamV2 *self,
                                     yaml_emitter_t *emitter,
                                     GError **error)
{
  MODULEMD_INIT_TRACE ();
  if (!modulemd_module_stream_emit_yaml_base (
        MODULEMD_MODULE_STREAM (self), emitter, error))
    return FALSE;

  EMIT_KEY_VALUE_IF_SET (
    emitter, error, "arch", modulemd_module_stream_v2_get_arch (self));
  EMIT_KEY_VALUE (emitter, error, "summary", self->summary);
  EMIT_KEY_VALUE (emitter, error, "description", self->description);

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

  EMIT_ARRAY_VALUES_IF_NON_EMPTY (emitter,
                                  error,
                                  "dependencies",
                                  self->dependencies,
                                  modulemd_dependencies_emit_yaml);

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
