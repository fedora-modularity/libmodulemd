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

#ifndef MODULEMD_MODULE_H
#define MODULEMD_MODULE_H

#include "modulemd.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-defaults.h"
#include "modulemd-dependencies.h"
#include "modulemd-profile.h"
#include "modulemd-servicelevel.h"
#include "modulemd-simpleset.h"


G_BEGIN_DECLS

#define MODULEMD_MODULE_ERROR modulemd_module_error_quark ()
GQuark
modulemd_module_error_quark (void);

enum ModulemdModuleError
{
  MODULEMD_MODULE_ERROR_MISSING_CONTENT,
};

#define MODULEMD_TYPE_MODULE modulemd_module_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdModule, modulemd_module, MODULEMD, MODULE, GObject)

ModulemdModule *
modulemd_module_new (void);

ModulemdModule *
modulemd_module_new_from_file (const gchar *yaml_file);

ModulemdModule *
modulemd_module_new_from_file_ext (const gchar *yaml_file,
                                   GPtrArray **failures,
                                   GError **error);

void
modulemd_module_new_all_from_file (const gchar *yaml_file,
                                   ModulemdModule ***_modules);

void
modulemd_module_new_all_from_file_ext (const gchar *yaml_file,
                                       GPtrArray **data);

ModulemdModule *
modulemd_module_new_from_string (const gchar *yaml_string);

ModulemdModule *
modulemd_module_new_from_string_ext (const gchar *yaml_string,
                                     GPtrArray **failures,
                                     GError **error);

void
modulemd_module_new_all_from_string (const gchar *yaml_string,
                                     ModulemdModule ***_modules);

void
modulemd_module_new_all_from_string_ext (const gchar *yaml_string,
                                         GPtrArray **data);


ModulemdModule *
modulemd_module_new_from_stream (FILE *stream, GError **error);

ModulemdModule *
modulemd_module_new_from_stream_ext (FILE *stream,
                                     GPtrArray **failures,
                                     GError **error);

void
modulemd_module_dump (ModulemdModule *self, const gchar *yaml_file);

gchar *
modulemd_module_dumps (ModulemdModule *self);

void
modulemd_module_dump_all (GPtrArray *module_array, const gchar *yaml_file);

gchar *
modulemd_module_dumps_all (GPtrArray *module_array);

gboolean
modulemd_module_upgrade (ModulemdModule *self);

void
modulemd_module_set_arch (ModulemdModule *self, const gchar *arch);

const gchar *
modulemd_module_get_arch (ModulemdModule *self);

const gchar *
modulemd_module_peek_arch (ModulemdModule *self);

gchar *
modulemd_module_dup_arch (ModulemdModule *self);

void
modulemd_module_set_buildopts (ModulemdModule *self,
                               ModulemdBuildopts *buildopts);

ModulemdBuildopts *
modulemd_module_get_buildopts (ModulemdModule *self);

ModulemdBuildopts *
modulemd_module_peek_buildopts (ModulemdModule *self);

void
modulemd_module_set_buildrequires (ModulemdModule *self,
                                   GHashTable *buildrequires);

GHashTable *
modulemd_module_get_buildrequires (ModulemdModule *self);

GHashTable *
modulemd_module_peek_buildrequires (ModulemdModule *self);

GHashTable *
modulemd_module_dup_buildrequires (ModulemdModule *self);

void
modulemd_module_set_community (ModulemdModule *self, const gchar *community);

const gchar *
modulemd_module_get_community (ModulemdModule *self);

const gchar *
modulemd_module_peek_community (ModulemdModule *self);

gchar *
modulemd_module_dup_community (ModulemdModule *self);

void
modulemd_module_set_content_licenses (ModulemdModule *self,
                                      ModulemdSimpleSet *licenses);

ModulemdSimpleSet *
modulemd_module_get_content_licenses (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_content_licenses (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_dup_content_licenses (ModulemdModule *self);

void
modulemd_module_set_context (ModulemdModule *self, const gchar *context);

const gchar *
modulemd_module_get_context (ModulemdModule *self);

const gchar *
modulemd_module_peek_context (ModulemdModule *self);

gchar *
modulemd_module_dup_context (ModulemdModule *self);

void
modulemd_module_set_dependencies (ModulemdModule *self, GPtrArray *deps);

void
modulemd_module_add_dependencies (ModulemdModule *self,
                                  ModulemdDependencies *dep);
GPtrArray *
modulemd_module_get_dependencies (ModulemdModule *self);

GPtrArray *
modulemd_module_peek_dependencies (ModulemdModule *self);

GPtrArray *
modulemd_module_dup_dependencies (ModulemdModule *self);

void
modulemd_module_set_description (ModulemdModule *self,
                                 const gchar *description);

const gchar *
modulemd_module_get_description (ModulemdModule *self);

const gchar *
modulemd_module_peek_description (ModulemdModule *self);

gchar *
modulemd_module_dup_description (ModulemdModule *self);

void
modulemd_module_set_documentation (ModulemdModule *self,
                                   const gchar *documentation);

const gchar *
modulemd_module_get_documentation (ModulemdModule *self);

const gchar *
modulemd_module_peek_documentation (ModulemdModule *self);

gchar *
modulemd_module_dup_documentation (ModulemdModule *self);

void
modulemd_module_set_eol (ModulemdModule *self, const GDate *date);

const GDate *
modulemd_module_get_eol (ModulemdModule *self);

const GDate *
modulemd_module_peek_eol (ModulemdModule *self);

GDate *
modulemd_module_dup_eol (ModulemdModule *self);

void
modulemd_module_set_mdversion (ModulemdModule *self, const guint64 mdversion);

const guint64
modulemd_module_get_mdversion (ModulemdModule *self);

guint64
modulemd_module_peek_mdversion (ModulemdModule *self);

void
modulemd_module_add_module_component (ModulemdModule *self,
                                      ModulemdComponentModule *component);

void
modulemd_module_clear_module_components (ModulemdModule *self);

void
modulemd_module_set_module_components (ModulemdModule *self,
                                       GHashTable *components);

GHashTable *
modulemd_module_get_module_components (ModulemdModule *self);

GHashTable *
modulemd_module_peek_module_components (ModulemdModule *self);

GHashTable *
modulemd_module_dup_module_components (ModulemdModule *self);

void
modulemd_module_set_module_licenses (ModulemdModule *self,
                                     ModulemdSimpleSet *licenses);

ModulemdSimpleSet *
modulemd_module_get_module_licenses (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_module_licenses (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_dup_module_licenses (ModulemdModule *self);

void
modulemd_module_set_name (ModulemdModule *self, const gchar *name);

const gchar *
modulemd_module_get_name (ModulemdModule *self);

const gchar *
modulemd_module_peek_name (ModulemdModule *self);

gchar *
modulemd_module_dup_name (ModulemdModule *self);

void
modulemd_module_add_profile (ModulemdModule *self, ModulemdProfile *profile);

void
modulemd_module_clear_profiles (ModulemdModule *self);

void
modulemd_module_set_profiles (ModulemdModule *self, GHashTable *profiles);

GHashTable *
modulemd_module_get_profiles (ModulemdModule *self);

GHashTable *
modulemd_module_peek_profiles (ModulemdModule *self);

GHashTable *
modulemd_module_dup_profiles (ModulemdModule *self);

void
modulemd_module_set_requires (ModulemdModule *self, GHashTable *requires);

GHashTable *
modulemd_module_get_requires (ModulemdModule *self);

GHashTable *
modulemd_module_peek_requires (ModulemdModule *self);

GHashTable *
modulemd_module_dup_requires (ModulemdModule *self);

void
modulemd_module_set_rpm_api (ModulemdModule *self, ModulemdSimpleSet *apis);

ModulemdSimpleSet *
modulemd_module_get_rpm_api (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_rpm_api (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_dup_rpm_api (ModulemdModule *self);

void
modulemd_module_set_rpm_artifacts (ModulemdModule *self,
                                   ModulemdSimpleSet *artifacts);

ModulemdSimpleSet *
modulemd_module_get_rpm_artifacts (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_rpm_artifacts (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_dup_rpm_artifacts (ModulemdModule *self);

void
modulemd_module_set_rpm_buildopts (ModulemdModule *self,
                                   GHashTable *buildopts);

GHashTable *
modulemd_module_get_rpm_buildopts (ModulemdModule *self);

GHashTable *
modulemd_module_peek_rpm_buildopts (ModulemdModule *self);

GHashTable *
modulemd_module_dup_rpm_buildopts (ModulemdModule *self);

void
modulemd_module_add_rpm_component (ModulemdModule *self,
                                   ModulemdComponentRpm *component);

void
modulemd_module_clear_rpm_components (ModulemdModule *self);

void
modulemd_module_set_rpm_components (ModulemdModule *self,
                                    GHashTable *components);

GHashTable *
modulemd_module_get_rpm_components (ModulemdModule *self);

GHashTable *
modulemd_module_peek_rpm_components (ModulemdModule *self);

GHashTable *
modulemd_module_dup_rpm_components (ModulemdModule *self);

void
modulemd_module_set_rpm_filter (ModulemdModule *self,
                                ModulemdSimpleSet *filter);

ModulemdSimpleSet *
modulemd_module_get_rpm_filter (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_rpm_filter (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_dup_rpm_filter (ModulemdModule *self);

void
modulemd_module_clear_servicelevels (ModulemdModule *self);

void
modulemd_module_set_servicelevels (ModulemdModule *self,
                                   GHashTable *servicelevels);

void
modulemd_module_add_servicelevel (ModulemdModule *self,
                                  ModulemdServiceLevel *servicelevel);

GHashTable *
modulemd_module_get_servicelevels (ModulemdModule *self);

GHashTable *
modulemd_module_peek_servicelevels (ModulemdModule *self);

GHashTable *
modulemd_module_dup_servicelevels (ModulemdModule *self);

void
modulemd_module_set_stream (ModulemdModule *self, const gchar *stream);

const gchar *
modulemd_module_get_stream (ModulemdModule *self);

const gchar *
modulemd_module_peek_stream (ModulemdModule *self);

gchar *
modulemd_module_dup_stream (ModulemdModule *self);

void
modulemd_module_set_summary (ModulemdModule *self, const gchar *summary);

const gchar *
modulemd_module_get_summary (ModulemdModule *self);

const gchar *
modulemd_module_peek_summary (ModulemdModule *self);

gchar *
modulemd_module_dup_summary (ModulemdModule *self);

void
modulemd_module_set_tracker (ModulemdModule *self, const gchar *tracker);

const gchar *
modulemd_module_get_tracker (ModulemdModule *self);

const gchar *
modulemd_module_peek_tracker (ModulemdModule *self);

gchar *
modulemd_module_dup_tracker (ModulemdModule *self);

void
modulemd_module_set_version (ModulemdModule *self, const guint64 version);

const guint64
modulemd_module_get_version (ModulemdModule *self);

guint64
modulemd_module_peek_version (ModulemdModule *self);

void
modulemd_module_set_xmd (ModulemdModule *self, GHashTable *xmd);

GHashTable *
modulemd_module_get_xmd (ModulemdModule *self);

GHashTable *
modulemd_module_peek_xmd (ModulemdModule *self);

GHashTable *
modulemd_module_dup_xmd (ModulemdModule *self);

ModulemdModule *
modulemd_module_copy (ModulemdModule *self);

gchar *
modulemd_module_dup_nsvc (ModulemdModule *self);

G_END_DECLS

#endif /* MODULEMD_MODULE_H */
