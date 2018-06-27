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

#pragma once

#include "modulemd.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-defaults.h"
#include "modulemd-dependencies.h"
#include "modulemd-profile.h"
#include "modulemd-servicelevel.h"
#include "modulemd-simpleset.h"

G_BEGIN_DECLS

#define MODULEMD_MODULESTREAM_ERROR modulemd_modulestream_error_quark ()
GQuark
modulemd_modulestream_error_quark (void);

enum ModulemdModuleStreamError
{
  MODULEMD_MODULESTREAM_ERROR_MISSING_CONTENT,
};

#define MODULEMD_TYPE_MODULESTREAM modulemd_modulestream_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdModuleStream, modulemd_modulestream, MODULEMD, MODULESTREAM, GObject)

ModulemdModuleStream *
modulemd_modulestream_new (void);

ModulemdModuleStream *
modulemd_modulestream_copy (ModulemdModuleStream *self);

gboolean
modulemd_modulestream_import_from_file (ModulemdModuleStream *self,
                                        const gchar *yaml_file,
                                        GPtrArray **failures,
                                        GError **error);

gboolean
modulemd_modulestream_dump (ModulemdModuleStream *self,
                            const gchar *yaml_file,
                            GError **error);


gboolean
modulemd_modulestream_import_from_string (ModulemdModuleStream *self,
                                          const gchar *yaml_string,
                                          GPtrArray **failures,
                                          GError **error);

gchar *
modulemd_modulestream_dumps (ModulemdModuleStream *self, GError **error);

gboolean
modulemd_modulestream_import_from_stream (ModulemdModuleStream *self,
                                          FILE *stream,
                                          GPtrArray **failures,
                                          GError **error);

gboolean
modulemd_modulestream_upgrade (ModulemdModuleStream *self);

void
modulemd_modulestream_set_arch (ModulemdModuleStream *self, const gchar *arch);

gchar *
modulemd_modulestream_get_arch (ModulemdModuleStream *self);

const gchar *
modulemd_modulestream_peek_arch (ModulemdModuleStream *self);

void
modulemd_modulestream_set_buildopts (ModulemdModuleStream *self,
                                     ModulemdBuildopts *buildopts);

ModulemdBuildopts *
modulemd_modulestream_get_buildopts (ModulemdModuleStream *self);

ModulemdBuildopts *
modulemd_modulestream_peek_buildopts (ModulemdModuleStream *self);

void
modulemd_modulestream_set_buildrequires (ModulemdModuleStream *self,
                                         GHashTable *buildrequires);

GHashTable *
modulemd_modulestream_get_buildrequires (ModulemdModuleStream *self);

void
modulemd_modulestream_set_community (ModulemdModuleStream *self,
                                     const gchar *community);

gchar *
modulemd_modulestream_get_community (ModulemdModuleStream *self);

const gchar *
modulemd_modulestream_peek_community (ModulemdModuleStream *self);

void
modulemd_modulestream_set_content_licenses (ModulemdModuleStream *self,
                                            ModulemdSimpleSet *licenses);

ModulemdSimpleSet *
modulemd_modulestream_get_content_licenses (ModulemdModuleStream *self);

const ModulemdSimpleSet *
modulemd_modulestream_peek_content_licenses (ModulemdModuleStream *self);

void
modulemd_modulestream_set_context (ModulemdModuleStream *self,
                                   const gchar *context);

gchar *
modulemd_modulestream_get_context (ModulemdModuleStream *self);

const gchar *
modulemd_modulestream_peek_context (ModulemdModuleStream *self);

void
modulemd_modulestream_set_dependencies (ModulemdModuleStream *self,
                                        GPtrArray *deps);

void
modulemd_modulestream_add_dependencies (ModulemdModuleStream *self,
                                        ModulemdDependencies *dep);
GPtrArray *
modulemd_modulestream_get_dependencies (ModulemdModuleStream *self);

const GPtrArray *
modulemd_modulestream_peek_dependencies (ModulemdModuleStream *self);

void
modulemd_modulestream_set_description (ModulemdModuleStream *self,
                                       const gchar *description);

gchar *
modulemd_modulestream_get_description (ModulemdModuleStream *self);

const gchar *
modulemd_modulestream_peek_description (ModulemdModuleStream *self);

void
modulemd_modulestream_set_documentation (ModulemdModuleStream *self,
                                         const gchar *documentation);

gchar *
modulemd_modulestream_get_documentation (ModulemdModuleStream *self);

const gchar *
modulemd_modulestream_peek_documentation (ModulemdModuleStream *self);

void
modulemd_modulestream_set_eol (ModulemdModuleStream *self, const GDate *date);

GDate *
modulemd_modulestream_get_eol (ModulemdModuleStream *self);

const GDate *
modulemd_modulestream_peek_eol (ModulemdModuleStream *self);

void
modulemd_modulestream_set_mdversion (ModulemdModuleStream *self,
                                     const guint64 mdversion);

guint64
modulemd_modulestream_get_mdversion (ModulemdModuleStream *self);

void
modulemd_modulestream_add_module_component (
  ModulemdModuleStream *self, ModulemdComponentModule *component);

void
modulemd_modulestream_clear_module_components (ModulemdModuleStream *self);

void
modulemd_modulestream_set_module_components (ModulemdModuleStream *self,
                                             GHashTable *components);

GHashTable *
modulemd_modulestream_get_module_components (ModulemdModuleStream *self);

void
modulemd_modulestream_set_module_licenses (ModulemdModuleStream *self,
                                           ModulemdSimpleSet *licenses);

ModulemdSimpleSet *
modulemd_modulestream_get_module_licenses (ModulemdModuleStream *self);

const ModulemdSimpleSet *
modulemd_modulestream_peek_module_licenses (ModulemdModuleStream *self);

void
modulemd_modulestream_set_name (ModulemdModuleStream *self, const gchar *name);

gchar *
modulemd_modulestream_get_name (ModulemdModuleStream *self);

const gchar *
modulemd_modulestream_peek_name (ModulemdModuleStream *self);

void
modulemd_modulestream_add_profile (ModulemdModuleStream *self,
                                   ModulemdProfile *profile);

void
modulemd_modulestream_clear_profiles (ModulemdModuleStream *self);

void
modulemd_modulestream_set_profiles (ModulemdModuleStream *self,
                                    GHashTable *profiles);

GHashTable *
modulemd_modulestream_get_profiles (ModulemdModuleStream *self);

void
modulemd_modulestream_set_requires (ModulemdModuleStream *self,
                                    GHashTable *requires);

GHashTable *
modulemd_modulestream_get_requires (ModulemdModuleStream *self);

void
modulemd_modulestream_set_rpm_api (ModulemdModuleStream *self,
                                   ModulemdSimpleSet *apis);

ModulemdSimpleSet *
modulemd_modulestream_get_rpm_api (ModulemdModuleStream *self);

const ModulemdSimpleSet *
modulemd_modulestream_peek_rpm_api (ModulemdModuleStream *self);

void
modulemd_modulestream_set_rpm_artifacts (ModulemdModuleStream *self,
                                         ModulemdSimpleSet *artifacts);

ModulemdSimpleSet *
modulemd_modulestream_get_rpm_artifacts (ModulemdModuleStream *self);

const ModulemdSimpleSet *
modulemd_modulestream_peek_rpm_artifacts (ModulemdModuleStream *self);

void
modulemd_modulestream_add_rpm_component (ModulemdModuleStream *self,
                                         ModulemdComponentRpm *component);

void
modulemd_modulestream_clear_rpm_components (ModulemdModuleStream *self);

void
modulemd_modulestream_set_rpm_components (ModulemdModuleStream *self,
                                          GHashTable *components);

GHashTable *
modulemd_modulestream_get_rpm_components (ModulemdModuleStream *self);

void
modulemd_modulestream_set_rpm_filter (ModulemdModuleStream *self,
                                      ModulemdSimpleSet *filter);

ModulemdSimpleSet *
modulemd_modulestream_get_rpm_filter (ModulemdModuleStream *self);

const ModulemdSimpleSet *
modulemd_modulestream_peek_rpm_filter (ModulemdModuleStream *self);

void
modulemd_modulestream_clear_servicelevels (ModulemdModuleStream *self);

void
modulemd_modulestream_set_servicelevels (ModulemdModuleStream *self,
                                         GHashTable *servicelevels);

void
modulemd_modulestream_add_servicelevel (ModulemdModuleStream *self,
                                        ModulemdServiceLevel *servicelevel);

GHashTable *
modulemd_modulestream_get_servicelevels (ModulemdModuleStream *self);

void
modulemd_modulestream_set_stream (ModulemdModuleStream *self,
                                  const gchar *stream);

gchar *
modulemd_modulestream_get_stream (ModulemdModuleStream *self);

const gchar *
modulemd_modulestream_peek_stream (ModulemdModuleStream *self);

void
modulemd_modulestream_set_summary (ModulemdModuleStream *self,
                                   const gchar *summary);

gchar *
modulemd_modulestream_get_summary (ModulemdModuleStream *self);

const gchar *
modulemd_modulestream_peek_summary (ModulemdModuleStream *self);

void
modulemd_modulestream_set_tracker (ModulemdModuleStream *self,
                                   const gchar *tracker);

gchar *
modulemd_modulestream_get_tracker (ModulemdModuleStream *self);

const gchar *
modulemd_modulestream_peek_tracker (ModulemdModuleStream *self);

void
modulemd_modulestream_set_version (ModulemdModuleStream *self,
                                   const guint64 version);

guint64
modulemd_modulestream_get_version (ModulemdModuleStream *self);

void
modulemd_modulestream_set_xmd (ModulemdModuleStream *self, GHashTable *xmd);

GHashTable *
modulemd_modulestream_get_xmd (ModulemdModuleStream *self);

gchar *
modulemd_modulestream_get_nsvc (ModulemdModuleStream *self);

G_END_DECLS
