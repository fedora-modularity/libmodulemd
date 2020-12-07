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
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-component.h"
#include "modulemd-errors.h"
#include "modulemd-module-stream-v2.h"
#include "modulemd-module-stream.h"
#include "modulemd-profile.h"
#include "modulemd-rpm-map-entry.h"
#include "modulemd-service-level.h"
#include "modulemd-translation-entry.h"
#include "private/modulemd-buildopts-private.h"
#include "private/modulemd-component-module-private.h"
#include "private/modulemd-component-private.h"
#include "private/modulemd-component-rpm-private.h"
#include "private/modulemd-dependencies-private.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-profile-private.h"
#include "private/modulemd-rpm-map-entry-private.h"
#include "private/modulemd-service-level-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"


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
  PROP_STATIC_CONTEXT,
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

  g_clear_pointer (&self->rpm_artifact_map, g_hash_table_unref);

  g_clear_pointer (&self->rpm_filters, g_hash_table_unref);

  g_clear_pointer (&self->servicelevels, g_hash_table_unref);

  g_clear_pointer (&self->dependencies, g_ptr_array_unref);

  g_clear_pointer (&self->obsoletes, g_object_unref);

  g_clear_pointer (&self->xmd, g_variant_unref);

  G_OBJECT_CLASS (modulemd_module_stream_v2_parent_class)->finalize (object);
}


static gboolean
modulemd_module_stream_v2_equals (ModulemdModuleStream *self_1,
                                  ModulemdModuleStream *self_2)
{
  ModulemdModuleStreamV2 *v2_self_1 = NULL;
  ModulemdModuleStreamV2 *v2_self_2 = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self_1), FALSE);
  v2_self_1 = MODULEMD_MODULE_STREAM_V2 (self_1);
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self_2), FALSE);
  v2_self_2 = MODULEMD_MODULE_STREAM_V2 (self_2);

  if (!MODULEMD_MODULE_STREAM_CLASS (modulemd_module_stream_v2_parent_class)
         ->equals (self_1, self_2))
    {
      return FALSE;
    }

  /*Check property equality*/
  if (g_strcmp0 (v2_self_1->community, v2_self_2->community) != 0)
    {
      return FALSE;
    }

  if (g_strcmp0 (v2_self_1->description, v2_self_2->description) != 0)
    {
      return FALSE;
    }

  if (g_strcmp0 (v2_self_1->documentation, v2_self_2->documentation) != 0)
    {
      return FALSE;
    }

  if (g_strcmp0 (v2_self_1->summary, v2_self_2->summary) != 0)
    {
      return FALSE;
    }

  if (g_strcmp0 (v2_self_1->tracker, v2_self_2->tracker) != 0)
    {
      return FALSE;
    }

  /* Test the negations of static_context just in case somehow they are
   * different non-zero values
   */
  if (!v2_self_1->static_context != !v2_self_2->static_context)
    {
      return FALSE;
    }

  if (!modulemd_buildopts_equals (v2_self_1->buildopts, v2_self_2->buildopts))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (v2_self_1->rpm_components,
                                   v2_self_2->rpm_components,
                                   modulemd_component_equals_wrapper))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (v2_self_1->module_components,
                                   v2_self_2->module_components,
                                   modulemd_component_equals_wrapper))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v2_self_1->module_licenses,
                                           v2_self_2->module_licenses))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v2_self_1->content_licenses,
                                           v2_self_2->content_licenses))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (v2_self_1->profiles,
                                   v2_self_2->profiles,
                                   modulemd_profile_equals_wrapper))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v2_self_1->rpm_api,
                                           v2_self_2->rpm_api))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v2_self_1->rpm_artifacts,
                                           v2_self_2->rpm_artifacts))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_sets_are_equal (v2_self_1->rpm_filters,
                                           v2_self_2->rpm_filters))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (v2_self_1->servicelevels,
                                   v2_self_2->servicelevels,
                                   modulemd_service_level_equals_wrapper))
    {
      return FALSE;
    }


  /* < string, GHashTable <string, Modulemd.RpmMapEntry> > */
  if (!modulemd_hash_table_equals (
        v2_self_1->rpm_artifact_map,
        v2_self_2->rpm_artifact_map,
        modulemd_RpmMapEntry_hash_table_equals_wrapper))
    {
      return FALSE;
    }


  if (v2_self_1->dependencies->len != v2_self_2->dependencies->len)
    {
      return FALSE;
    }

  for (guint i = 0; i < v2_self_1->dependencies->len; i++)
    {
      /*Ordering is important for the dependencies, 
       so that each array index must be the same.*/
      if (!modulemd_dependencies_equals (
            g_ptr_array_index (v2_self_1->dependencies, i),
            g_ptr_array_index (v2_self_2->dependencies, i)))
        {
          return FALSE;
        }
    }

  if (v2_self_1->xmd == NULL && v2_self_2->xmd == NULL)
    {
      return TRUE;
    }

  if (v2_self_1->xmd == NULL || v2_self_2->xmd == NULL)
    {
      return FALSE;
    }

  if (!g_variant_equal (v2_self_1->xmd, v2_self_2->xmd))
    {
      return FALSE;
    }

  return TRUE;
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

  modulemd_module_stream_set_arch (MODULEMD_MODULE_STREAM (self), arch);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ARCH]);
}


const gchar *
modulemd_module_stream_v2_get_arch (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_module_stream_get_arch (MODULEMD_MODULE_STREAM (self));
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
    {
      return modulemd_translation_entry_get_description (entry);
    }

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
    {
      return modulemd_translation_entry_get_summary (entry);
    }

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


ModulemdObsoletes *
modulemd_module_stream_v2_get_obsoletes_resolved (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  ModulemdObsoletes *o = self->obsoletes;
  if (o && modulemd_obsoletes_get_reset (o))
    {
      return NULL;
    }

  return o;
}

void
modulemd_module_stream_v2_associate_obsoletes (ModulemdModuleStreamV2 *self,
                                               ModulemdObsoletes *obsoletes)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_clear_pointer (&self->obsoletes, g_object_unref);
  if (obsoletes != NULL)
    {
      self->obsoletes = g_object_ref (obsoletes);
    }
}

ModulemdObsoletes *
modulemd_module_stream_v2_get_obsoletes (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return self->obsoletes;
}


/* ===== Non-property Methods ===== */

void
modulemd_module_stream_v2_add_component (ModulemdModuleStreamV2 *self,
                                         ModulemdComponent *component)
{
  GHashTable *table = NULL;

  /* Do nothing if we were passed a NULL component */
  if (!component)
    {
      return;
    }

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
                        g_strdup (modulemd_component_get_key (component)),
                        modulemd_component_copy (component, NULL));
}


void
modulemd_module_stream_v2_remove_module_component (
  ModulemdModuleStreamV2 *self, const gchar *component_name)
{
  /* Do nothing if we were passed a NULL component_name */
  if (!component_name)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->module_components, component_name);
}


void
modulemd_module_stream_v2_clear_module_components (
  ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove_all (self->module_components);
}


void
modulemd_module_stream_v2_remove_rpm_component (ModulemdModuleStreamV2 *self,
                                                const gchar *component_name)
{
  /* Do nothing if we were passed a NULL component_name */
  if (!component_name)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->rpm_components, component_name);
}


void
modulemd_module_stream_v2_clear_rpm_components (ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove_all (self->rpm_components);
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
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->content_licenses, g_strdup (license));
}


void
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
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->module_licenses, g_strdup (license));
}


void
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
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->content_licenses, license);
}


void
modulemd_module_stream_v2_remove_module_license (ModulemdModuleStreamV2 *self,
                                                 const gchar *license)
{
  if (!license)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->module_licenses, license);
}


void
modulemd_module_stream_v2_clear_content_licenses (ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove_all (self->content_licenses);
}


void
modulemd_module_stream_v2_clear_module_licenses (ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove_all (self->module_licenses);
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
    {
      return;
    }
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));
  g_return_if_fail (MODULEMD_IS_PROFILE (profile));

  ModulemdProfile *copied_profile = modulemd_profile_copy (profile);
  modulemd_profile_set_owner (copied_profile, MODULEMD_MODULE_STREAM (self));

  g_hash_table_replace (self->profiles,
                        g_strdup (modulemd_profile_get_name (profile)),
                        copied_profile);
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


struct profile_match_ctx
{
  GHashTable *profiles;
  GPtrArray *found;
  const gchar *pattern;
};

static void
profile_match (gpointer data, gpointer user_data)
{
  struct profile_match_ctx *match_ctx = (struct profile_match_ctx *)user_data;

  /* Add it to the found list if it matches the pattern */
  if (modulemd_fnmatch (match_ctx->pattern, (const gchar *)data))
    {
      g_ptr_array_add (match_ctx->found,
                       g_object_ref (g_hash_table_lookup (
                         match_ctx->profiles, (const gchar *)data)));
    }
}

GPtrArray *
modulemd_module_stream_v2_search_profiles (ModulemdModuleStreamV2 *self,
                                           const gchar *profile_pattern)
{
  /* The list of profiles will probably never be large, so we'll optimize for
   * the worst-case and preallocate the array to the number of profiles.
   */
  GPtrArray *found =
    g_ptr_array_new_full (g_hash_table_size (self->profiles), g_object_unref);

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), found);

  g_autoptr (GPtrArray) profile_names =
    modulemd_ordered_str_keys (self->profiles, modulemd_strcmp_sort);

  struct profile_match_ctx match_ctx = { .profiles = self->profiles,
                                         .found = found,
                                         .pattern = profile_pattern };

  g_ptr_array_foreach (profile_names, profile_match, &match_ctx);

  return found;
}


void
modulemd_module_stream_v2_add_rpm_api (ModulemdModuleStreamV2 *self,
                                       const gchar *rpm)
{
  if (!rpm)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->rpm_api, g_strdup (rpm));
}


void
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
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->rpm_api, rpm);
}


void
modulemd_module_stream_v2_clear_rpm_api (ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove_all (self->rpm_api);
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
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->rpm_artifacts, g_strdup (nevr));
}


void
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
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->rpm_artifacts, nevr);
}


void
modulemd_module_stream_v2_clear_rpm_artifacts (ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove_all (self->rpm_artifacts);
}


GStrv
modulemd_module_stream_v2_get_rpm_artifacts_as_strv (
  ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->rpm_artifacts);
}


static GHashTable *
get_or_create_digest_table (ModulemdModuleStreamV2 *self, const gchar *digest)
{
  GHashTable *digest_table =
    g_hash_table_lookup (self->rpm_artifact_map, digest);
  if (digest_table == NULL)
    {
      digest_table = g_hash_table_new_full (
        g_str_hash, g_str_equal, g_free, g_object_unref);
      g_hash_table_insert (
        self->rpm_artifact_map, g_strdup (digest), digest_table);
    }

  return digest_table;
}


void
modulemd_module_stream_v2_set_rpm_artifact_map_entry (
  ModulemdModuleStreamV2 *self,
  ModulemdRpmMapEntry *entry,
  const gchar *digest,
  const gchar *checksum)
{
  GHashTable *digest_table = NULL;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));
  g_return_if_fail (entry && digest && checksum);

  digest_table = get_or_create_digest_table (self, digest);

  g_hash_table_insert (
    digest_table, g_strdup (checksum), modulemd_rpm_map_entry_copy (entry));
}


ModulemdRpmMapEntry *
modulemd_module_stream_v2_get_rpm_artifact_map_entry (
  ModulemdModuleStreamV2 *self, const gchar *digest, const gchar *checksum)
{
  GHashTable *digest_table = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);
  g_return_val_if_fail (digest && checksum, NULL);

  digest_table = g_hash_table_lookup (self->rpm_artifact_map, digest);
  if (!digest_table)
    {
      return NULL;
    }

  return g_hash_table_lookup (digest_table, checksum);
}


void
modulemd_module_stream_v2_add_rpm_filter (ModulemdModuleStreamV2 *self,
                                          const gchar *rpm)
{
  if (!rpm)
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_add (self->rpm_filters, g_strdup (rpm));
}


void
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
    {
      return;
    }

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove (self->rpm_filters, rpm);
}


void
modulemd_module_stream_v2_clear_rpm_filters (ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_hash_table_remove_all (self->rpm_filters);
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
    {
      return;
    }
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


void
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


void
modulemd_module_stream_v2_clear_dependencies (ModulemdModuleStreamV2 *self)
{
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  g_ptr_array_set_size (self->dependencies, 0);
}


static gboolean
dep_equal_wrapper (gconstpointer a, gconstpointer b)
{
  return modulemd_dependencies_equals ((ModulemdDependencies *)a,
                                       (ModulemdDependencies *)b);
}


void
modulemd_module_stream_v2_remove_dependencies (ModulemdModuleStreamV2 *self,
                                               ModulemdDependencies *deps)
{
  guint index;
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self));

  while (g_ptr_array_find_with_equal_func (
    self->dependencies, deps, dep_equal_wrapper, &index))
    {
      g_ptr_array_remove_index (self->dependencies, index);
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

  /* Do nothing if we were passed the same pointer */
  if (self->xmd == xmd)
    {
      return;
    }

  g_clear_pointer (&self->xmd, g_variant_unref);
  self->xmd = modulemd_variant_deep_copy (xmd);
}

GVariant *
modulemd_module_stream_v2_get_xmd (ModulemdModuleStreamV2 *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), NULL);
  return self->xmd;
}


gboolean
modulemd_module_stream_v2_includes_nevra (ModulemdModuleStreamV2 *self,
                                          const gchar *nevra_pattern)
{
  /* If g_hash_table_find() returns non-NULL, the nevra was found in this
   * module stream, so return TRUE
   */
  return !!g_hash_table_find (
    self->rpm_artifacts, modulemd_rpm_match, (void *)nevra_pattern);
}


void
modulemd_module_stream_v2_set_static_context (ModulemdModuleStreamV2 *self)
{
  self->static_context = TRUE;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STATIC_CONTEXT]);
}


void
modulemd_module_stream_v2_unset_static_context (ModulemdModuleStreamV2 *self)
{
  self->static_context = FALSE;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STATIC_CONTEXT]);
}


gboolean
modulemd_module_stream_v2_is_static_context (ModulemdModuleStreamV2 *self)
{
  return self->static_context;
}


static gboolean
modulemd_module_stream_v2_validate_context (const gchar *context,
                                            GError **error)
{
  /* must be string of up to MMD_MAXCONTEXTLEN [a-zA-Z0-9] */

  if (context == NULL || *context == '\0')
    {
      g_set_error (
        error, MODULEMD_ERROR, MMD_ERROR_VALIDATE, "Empty stream context");
      return FALSE;
    }

  if (strlen (context) > MMD_MAXCONTEXTLEN)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_VALIDATE,
                   "Stream context '%s' exceeds maximum length (%d)",
                   context,
                   MMD_MAXCONTEXTLEN);
      return FALSE;
    }

  for (const gchar *i = context; *i != '\0'; i++)
    {
      if (!g_ascii_isalnum (*i))
        {
          g_set_error (error,
                       MODULEMD_ERROR,
                       MMD_ERROR_VALIDATE,
                       "Non-alphanumeric character in stream context '%s'",
                       context);
          return FALSE;
        }
    }

  return TRUE;
}


static gboolean
modulemd_module_stream_v2_validate (ModulemdModuleStream *self, GError **error)
{
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  gchar *nevra = NULL;
  ModulemdModuleStreamV2 *v2_self = NULL;
  ModulemdDependencies *deps = NULL;
  g_autoptr (GError) nested_error = NULL;
  g_auto (GStrv) buildopts_arches = NULL;
  const gchar *context = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), FALSE);
  v2_self = MODULEMD_MODULE_STREAM_V2 (self);

  if (!MODULEMD_MODULE_STREAM_CLASS (modulemd_module_stream_v2_parent_class)
         ->validate (self, error))
    {
      return FALSE;
    }

  /* Validate static context if present */
  if (v2_self->static_context)
    {
      context =
        modulemd_module_stream_get_context (MODULEMD_MODULE_STREAM (self));
      if (context)
        {
          if (!modulemd_module_stream_v2_validate_context (context,
                                                           &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
        }
    }

  /* Make sure that mandatory fields are present */
  if (!modulemd_module_stream_v2_get_summary (v2_self, "C"))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_MISSING_REQUIRED,
                   "Summary is missing");
      return FALSE;
    }

  if (!modulemd_module_stream_v2_get_description (v2_self, "C"))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_MISSING_REQUIRED,
                   "Description is missing");
      return FALSE;
    }

  if (!g_hash_table_size (v2_self->module_licenses))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_MISSING_REQUIRED,
                   "Module license is missing");
      return FALSE;
    }

  /* Verify that the components are consistent with regards to buildorder and
   * buildafter values.
   */
  if (!modulemd_module_stream_validate_components (v2_self->rpm_components,
                                                   &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return FALSE;
    }

  if (v2_self->buildopts != NULL)
    {
      /* Verify that the component rpm arches are consistent with any module
       * level arches.
       */
      buildopts_arches =
        modulemd_buildopts_get_arches_as_strv (v2_self->buildopts);
      if (!modulemd_module_stream_validate_component_rpm_arches (
            v2_self->rpm_components, buildopts_arches, &nested_error))
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return FALSE;
        }
    }

  /* Iterate through the artifacts and validate that they are in the proper
   * NEVRA format
   */
  g_hash_table_iter_init (&iter, v2_self->rpm_artifacts);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      nevra = (gchar *)key;
      if (!modulemd_validate_nevra (nevra))
        {
          g_set_error (error,
                       MODULEMD_ERROR,
                       MMD_ERROR_VALIDATE,
                       "Artifact '%s' was not in valid N-E:V-R.A format.",
                       nevra);
          return FALSE;
        }
    }

  /* Iterate through the Dependencies and validate them */
  for (guint i = 0; i < v2_self->dependencies->len; i++)
    {
      deps =
        MODULEMD_DEPENDENCIES (g_ptr_array_index (v2_self->dependencies, i));
      if (!modulemd_dependencies_validate (deps, &nested_error))
        {
          g_propagate_prefixed_error (error,
                                      g_steal_pointer (&nested_error),
                                      "Dependency failed to validate: ");
          return FALSE;
        }
    }

  return TRUE;
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

    case PROP_STATIC_CONTEXT:
      g_value_set_boolean (value,
                           modulemd_module_stream_v2_is_static_context (self));
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

    case PROP_STATIC_CONTEXT:
      if (g_value_get_boolean (value))
        {
          modulemd_module_stream_v2_set_static_context (self);
        }
      else
        {
          modulemd_module_stream_v2_unset_static_context (self);
        }
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
copy_rpm_artifact_map (ModulemdModuleStreamV2 *from,
                       ModulemdModuleStreamV2 *to)
{
  GHashTableIter outer;
  GHashTableIter inner;
  gpointer outer_key;
  gpointer outer_value;
  gpointer inner_key;
  gpointer inner_value;
  GHashTable *to_digest_table = NULL;

  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (from));
  g_return_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (to));

  g_hash_table_iter_init (&outer, from->rpm_artifact_map);
  while (g_hash_table_iter_next (&outer, &outer_key, &outer_value))
    {
      to_digest_table = get_or_create_digest_table (to, outer_key);

      g_hash_table_iter_init (&inner, (GHashTable *)outer_value);
      while (g_hash_table_iter_next (&inner, &inner_key, &inner_value))
        {
          g_hash_table_insert (to_digest_table,
                               g_strdup (inner_key),
                               modulemd_rpm_map_entry_copy (inner_value));
        }
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
  copy->static_context = v2_self->static_context;

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

  copy_rpm_artifact_map (v2_self, copy);

  STREAM_COPY_IF_SET (v2, copy, v2_self, xmd);

  modulemd_module_stream_v2_associate_obsoletes (
    copy, modulemd_module_stream_v2_get_obsoletes (v2_self));

  return MODULEMD_MODULE_STREAM (g_steal_pointer (&copy));
}


static gboolean
depends_on_stream (ModulemdModuleStreamV2 *self,
                   const gchar *module_name,
                   const gchar *stream_name,
                   gboolean is_builddep)
{
  ModulemdDependencies *dep = NULL;

  /* Iterate through all of the dependency objects */
  for (gint i = 0; i < self->dependencies->len; i++)
    {
      dep = g_ptr_array_index (self->dependencies, i);
      if (is_builddep)
        {
          if (modulemd_dependencies_buildrequires_module_and_stream (
                dep, module_name, stream_name))
            {
              return TRUE;
            }
        }
      else
        {
          if (modulemd_dependencies_requires_module_and_stream (
                dep, module_name, stream_name))
            {
              return TRUE;
            }
        }
    }

  return FALSE;
}

static gboolean
modulemd_module_stream_v2_depends_on_stream (ModulemdModuleStream *self,
                                             const gchar *module_name,
                                             const gchar *stream_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), FALSE);
  g_return_val_if_fail (module_name && stream_name, FALSE);

  return depends_on_stream (
    MODULEMD_MODULE_STREAM_V2 (self), module_name, stream_name, FALSE);
}


static gboolean
modulemd_module_stream_v2_build_depends_on_stream (ModulemdModuleStream *self,
                                                   const gchar *module_name,
                                                   const gchar *stream_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM_V2 (self), FALSE);
  g_return_val_if_fail (module_name && stream_name, FALSE);

  return depends_on_stream (
    MODULEMD_MODULE_STREAM_V2 (self), module_name, stream_name, TRUE);
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
  stream_class->equals = modulemd_module_stream_v2_equals;
  stream_class->validate = modulemd_module_stream_v2_validate;
  stream_class->depends_on_stream =
    modulemd_module_stream_v2_depends_on_stream;
  stream_class->build_depends_on_stream =
    modulemd_module_stream_v2_build_depends_on_stream;


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

  properties[PROP_STATIC_CONTEXT] =
    g_param_spec_boolean ("static-context",
                          "Static Context",
                          "Whether the context is static",
                          0,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

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

  self->rpm_artifact_map = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_hash_table_unref);

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
                                          gboolean strict,
                                          gboolean only_packager,
                                          GError **error);

static gboolean
modulemd_module_stream_v2_parse_servicelevels (
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  GError **error);

static gboolean
modulemd_module_stream_v2_parse_deps (yaml_parser_t *parser,
                                      ModulemdModuleStreamV2 *modulestream,
                                      gboolean strict,
                                      GError **error);

static gboolean
modulemd_module_stream_v2_parse_refs (yaml_parser_t *parser,
                                      ModulemdModuleStreamV2 *modulestream,
                                      gboolean strict,
                                      GError **error);

static gboolean
modulemd_module_stream_v2_parse_profiles (yaml_parser_t *parser,
                                          ModulemdModuleStreamV2 *modulestream,
                                          gboolean strict,
                                          GError **error);

static gboolean
modulemd_module_stream_v2_parse_components (
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  gboolean only_packager,
  GError **error);

static gboolean
modulemd_module_stream_v2_parse_artifacts (
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  GError **error);


ModulemdModuleStreamV2 *
modulemd_module_stream_v2_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                      gboolean strict,
                                      gboolean only_packager,
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
  gboolean static_context;

  if (!modulemd_subdocument_info_get_data_parser (
        subdoc, &parser, strict, error))
    {
      return FALSE;
    }

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
          if (g_str_equal ((const gchar *)event.data.scalar.value, "name") &&
              !only_packager)
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_set_module_name,
                MODULEMD_MODULE_STREAM (modulestream));
            }

          /* Module Stream Name */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "stream") &&
                   !only_packager)
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_set_stream_name,
                MODULEMD_MODULE_STREAM (modulestream));
            }

          /* Module Version */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "version") &&
                   !only_packager)
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
                                "context") &&
                   !only_packager)
            {
              MMD_SET_PARSED_YAML_STRING (
                &parser,
                error,
                modulemd_module_stream_set_context,
                MODULEMD_MODULE_STREAM (modulestream));
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "static_context"))
            {
              static_context =
                modulemd_yaml_parse_bool (&parser, &nested_error);
              if (nested_error)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }

              if (static_context)
                {
                  modulemd_module_stream_v2_set_static_context (modulestream);
                }
              else
                {
                  modulemd_module_stream_v2_unset_static_context (
                    modulestream);
                }
            }

          /* Module Artifact Architecture */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "arch") &&
                   !only_packager)
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
                                "servicelevels") &&
                   !only_packager)
            {
              if (!modulemd_module_stream_v2_parse_servicelevels (
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
              if (!modulemd_module_stream_v2_parse_licenses (&parser,
                                                             modulestream,
                                                             strict,
                                                             only_packager,
                                                             &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Extensible Metadata */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "xmd") &&
                   !only_packager)
            {
              xmd = mmd_parse_xmd (&parser, &nested_error);
              if (!xmd)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
              modulemd_module_stream_v2_set_xmd (modulestream, xmd);
              g_clear_pointer (&xmd, g_variant_unref);
            }

          /* Dependencies */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "dependencies"))
            {
              if (!modulemd_module_stream_v2_parse_deps (
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
              if (!modulemd_module_stream_v2_parse_refs (
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
              if (!modulemd_module_stream_v2_parse_profiles (
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
              modulemd_module_stream_v2_replace_rpm_api (modulestream, set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          /* Filter */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "filter"))
            {
              set = modulemd_yaml_parse_string_set_from_map (
                &parser, "rpms", strict, &nested_error);
              modulemd_module_stream_v2_replace_rpm_filters (modulestream,
                                                             set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          /* Build Options */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "buildopts") &&
                   !only_packager)
            {
              buildopts =
                modulemd_buildopts_parse_yaml (&parser, strict, &nested_error);
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
              if (!modulemd_module_stream_v2_parse_components (&parser,
                                                               modulestream,
                                                               strict,
                                                               only_packager,
                                                               &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }

          /* Artifacts */
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "artifacts") &&
                   !only_packager)
            {
              if (!modulemd_module_stream_v2_parse_artifacts (
                    &parser, modulestream, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }

              g_clear_pointer (&set, g_hash_table_unref);
            }

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
            "Unexpected YAML event in ModuleStreamV2: %s",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  return g_steal_pointer (&modulestream);
}


static gboolean
modulemd_module_stream_v2_parse_licenses (yaml_parser_t *parser,
                                          ModulemdModuleStreamV2 *modulestream,
                                          gboolean strict,
                                          gboolean only_packager,
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
                                "content") &&
                   !only_packager)
            {
              set = modulemd_yaml_parse_string_set (parser, &nested_error);
              modulemd_module_stream_v2_replace_content_licenses (modulestream,
                                                                  set);
              g_clear_pointer (&set, g_hash_table_unref);
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
modulemd_module_stream_v2_parse_servicelevels (
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
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
                                      gboolean strict,
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
          deps =
            modulemd_dependencies_parse_yaml (parser, strict, &nested_error);
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
modulemd_module_stream_v2_parse_profiles (yaml_parser_t *parser,
                                          ModulemdModuleStreamV2 *modulestream,
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
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  gboolean only_packager,
  GError **error);
static gboolean
modulemd_module_stream_v2_parse_module_components (
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  GError **error);


static gboolean
modulemd_module_stream_v2_parse_components (
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  gboolean only_packager,
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
              if (!modulemd_module_stream_v2_parse_rpm_components (
                    parser,
                    modulestream,
                    strict,
                    only_packager,
                    &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "modules"))
            {
              if (!modulemd_module_stream_v2_parse_module_components (
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
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  gboolean only_packager,
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
            only_packager,
            &nested_error);
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
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
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


static gboolean
modulemd_module_stream_v2_parse_rpm_map (yaml_parser_t *parser,
                                         ModulemdModuleStreamV2 *modulestream,
                                         gboolean strict,
                                         GError **error);


static gboolean
modulemd_module_stream_v2_parse_artifacts (
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GHashTable) set = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* We *must* get a MAPPING_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Got %s instead of MAPPING_START in artifacts.",
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
              set = modulemd_yaml_parse_string_set (parser, &nested_error);
              if (!set)
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }

              modulemd_module_stream_v2_replace_rpm_artifacts (modulestream,
                                                               set);
              g_clear_pointer (&set, g_hash_table_unref);
            }

          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "rpm-map"))
            {
              if (!modulemd_module_stream_v2_parse_rpm_map (
                    parser, modulestream, strict, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return FALSE;
                }
            }

          else
            {
              /* Encountered a key other than the expected ones. */
              SKIP_UNKNOWN (parser,
                            FALSE,
                            "Unexpected key in map: %s",
                            (const gchar *)event.data.scalar.value);
            }

          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in map");
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_module_stream_v2_parse_rpm_map_digest (
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  const gchar *digest,
  GError **error);

static gboolean
modulemd_module_stream_v2_parse_rpm_map (yaml_parser_t *parser,
                                         ModulemdModuleStreamV2 *modulestream,
                                         gboolean strict,
                                         GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* We *must* get a MAPPING_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Got %s instead of MAPPING_START in rpm-map.",
        mmd_yaml_get_event_name (event.type));
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          /* Each entry in the map here represents a digest name */
          if (!modulemd_module_stream_v2_parse_rpm_map_digest (
                parser,
                modulestream,
                strict,
                (const gchar *)event.data.scalar.value,
                &nested_error))
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in map");
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


static gboolean
modulemd_module_stream_v2_parse_rpm_map_digest (
  yaml_parser_t *parser,
  ModulemdModuleStreamV2 *modulestream,
  gboolean strict,
  const gchar *digest,
  GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (GError) nested_error = NULL;
  const gchar *checksum = NULL;
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* We *must* get a MAPPING_START here */
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Got %s instead of MAPPING_START in rpm-map.",
        mmd_yaml_get_event_name (event.type));
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          /* Each key in this map is a checksum with the value being a
           * ModulemdRpmMapEntry
           */
          checksum = (const gchar *)event.data.scalar.value;

          entry =
            modulemd_rpm_map_entry_parse_yaml (parser, strict, &nested_error);
          if (!entry)
            {
              g_propagate_error (error, g_steal_pointer (&nested_error));
              return FALSE;
            }

          modulemd_module_stream_v2_set_rpm_artifact_map_entry (
            modulestream, entry, digest, checksum);

          g_clear_object (&entry);
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in map");
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}


gboolean
modulemd_module_stream_v2_emit_rpm_map (ModulemdModuleStreamV2 *self,
                                        yaml_emitter_t *emitter,
                                        GError **error);

gboolean
modulemd_module_stream_v2_emit_yaml (ModulemdModuleStreamV2 *self,
                                     yaml_emitter_t *emitter,
                                     GError **error)
{
  MODULEMD_INIT_TRACE ();

  if (!modulemd_module_stream_emit_yaml_base (
        MODULEMD_MODULE_STREAM (self), emitter, error))
    {
      return FALSE;
    }

  if (modulemd_module_stream_v2_is_static_context (self))
    {
      EMIT_KEY_VALUE (emitter, error, "static_context", "true");
    }

  EMIT_KEY_VALUE_IF_SET (
    emitter, error, "arch", modulemd_module_stream_v2_get_arch (self));
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
                   MMD_YAML_ERROR_EMIT,
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
        {
          return FALSE;
        }
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
        {
          return FALSE;
        }
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

  if (NON_EMPTY_TABLE (self->rpm_artifacts) ||
      NON_EMPTY_TABLE (self->rpm_artifact_map))
    {
      EMIT_SCALAR (emitter, error, "artifacts");
      EMIT_MAPPING_START (emitter, error);

      /* Emit the rpm artifacts */
      EMIT_STRING_SET_IF_NON_EMPTY (
        emitter, error, "rpms", self->rpm_artifacts);

      /* Emit the rpm-map */
      if (!modulemd_module_stream_v2_emit_rpm_map (self, emitter, error))
        {
          return FALSE;
        }

      EMIT_MAPPING_END (emitter, error);
    }

  /* The "data" mapping */
  EMIT_MAPPING_END (emitter, error);
  /* The overall document mapping */
  EMIT_MAPPING_END (emitter, error);
  if (!mmd_emitter_end_document (emitter, error))
    {
      return FALSE;
    }

  return TRUE;
}


gboolean
modulemd_module_stream_v2_emit_rpm_map (ModulemdModuleStreamV2 *self,
                                        yaml_emitter_t *emitter,
                                        GError **error)
{
  GHashTable *digest_table = NULL;
  g_autoptr (GPtrArray) digests = NULL;
  const gchar *digest = NULL;
  g_autoptr (GPtrArray) checksums = NULL;
  const gchar *checksum = NULL;
  ModulemdRpmMapEntry *entry = NULL;

  if (!NON_EMPTY_TABLE (self->rpm_artifact_map))
    {
      /* Nothing to output here */
      return TRUE;
    }

  digests =
    modulemd_ordered_str_keys (self->rpm_artifact_map, modulemd_strcmp_sort);

  EMIT_SCALAR (emitter, error, "rpm-map");
  EMIT_MAPPING_START (emitter, error);

  for (guint i = 0; i < digests->len; i++)
    {
      digest = g_ptr_array_index (digests, i);
      EMIT_SCALAR (emitter, error, digest);

      digest_table = g_hash_table_lookup (self->rpm_artifact_map, digest);

      EMIT_MAPPING_START (emitter, error);

      checksums =
        modulemd_ordered_str_keys (digest_table, modulemd_strcmp_sort);

      for (guint j = 0; j < digests->len; j++)
        {
          checksum = g_ptr_array_index (checksums, j);
          EMIT_SCALAR (emitter, error, checksum);

          entry = g_hash_table_lookup (digest_table, checksum);

          if (!modulemd_rpm_map_entry_emit_yaml (entry, emitter, error))
            {
              return FALSE;
            }
        }

      EMIT_MAPPING_END (emitter, error);

      g_clear_pointer (&checksums, g_ptr_array_unref);
    }

  EMIT_MAPPING_END (emitter, error);

  return TRUE;
}
