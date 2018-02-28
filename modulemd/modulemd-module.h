/* modulemd-module.h
 *
 * Copyright (C) 2017 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MODULEMD_MODULE_H
#define MODULEMD_MODULE_H

#include <glib-object.h>
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-dependencies.h"
#include "modulemd-profile.h"
#include "modulemd-servicelevel.h"
#include "modulemd-simpleset.h"

enum
{
  MD_VERSION_UNSET = 0,

  MD_VERSION_1 = 1,
  MD_VERSION_2 = 2,

  MD_VERSION_MAX = G_MAXUINT64
};

#define MD_VERSION_LATEST MD_VERSION_2

G_BEGIN_DECLS

#define MODULEMD_TYPE_MODULE modulemd_module_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdModule, modulemd_module, MODULEMD, MODULE, GObject)

ModulemdModule *
modulemd_module_new (void);

ModulemdModule *
modulemd_module_new_from_file (const gchar *yaml_file);

void
modulemd_module_new_all_from_file (const gchar *yaml_file,
                                   ModulemdModule ***_modules);

ModulemdModule *
modulemd_module_new_from_string (const gchar *yaml_string);

void
modulemd_module_new_all_from_string (const gchar *yaml_string,
                                     ModulemdModule ***_modules);

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

void
modulemd_module_set_buildrequires (ModulemdModule *self,
                                   GHashTable *buildrequires);

GHashTable *
modulemd_module_get_buildrequires (ModulemdModule *self);

GHashTable *
modulemd_module_peek_buildrequires (ModulemdModule *self);

void
modulemd_module_set_community (ModulemdModule *self, const gchar *community);

const gchar *
modulemd_module_get_community (ModulemdModule *self);

const gchar *
modulemd_module_peek_community (ModulemdModule *self);

void
modulemd_module_set_content_licenses (ModulemdModule *self,
                                      ModulemdSimpleSet *licenses);

ModulemdSimpleSet *
modulemd_module_get_content_licenses (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_content_licenses (ModulemdModule *self);

void
modulemd_module_set_context (ModulemdModule *self, const gchar *context);

const gchar *
modulemd_module_get_context (ModulemdModule *self);

const gchar *
modulemd_module_peek_context (ModulemdModule *self);

void
modulemd_module_set_description (ModulemdModule *self,
                                 const gchar *description);

const gchar *
modulemd_module_get_description (ModulemdModule *self);

const gchar *
modulemd_module_peek_description (ModulemdModule *self);

void
modulemd_module_set_dependencies (ModulemdModule *self, GPtrArray *deps);

void
modulemd_module_add_dependencies (ModulemdModule *self,
                                  ModulemdDependencies *dep);
GPtrArray *
modulemd_module_get_dependencies (ModulemdModule *self);

GPtrArray *
modulemd_module_peek_dependencies (ModulemdModule *self);

void
modulemd_module_set_documentation (ModulemdModule *self,
                                   const gchar *documentation);

const gchar *
modulemd_module_get_documentation (ModulemdModule *self);

const gchar *
modulemd_module_peek_documentation (ModulemdModule *self);

void
modulemd_module_set_eol (ModulemdModule *self, const GDate *date);

const GDate *
modulemd_module_get_eol (ModulemdModule *self);

const GDate *
modulemd_module_peek_eol (ModulemdModule *self);

void
modulemd_module_set_mdversion (ModulemdModule *self, const guint64 mdversion);

const guint64
modulemd_module_get_mdversion (ModulemdModule *self);

const guint64
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

void
modulemd_module_set_module_licenses (ModulemdModule *self,
                                     ModulemdSimpleSet *licenses);

ModulemdSimpleSet *
modulemd_module_get_module_licenses (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_module_licenses (ModulemdModule *self);

void
modulemd_module_set_name (ModulemdModule *self, const gchar *name);

const gchar *
modulemd_module_get_name (ModulemdModule *self);

const gchar *
modulemd_module_peek_name (ModulemdModule *self);

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

void
modulemd_module_set_requires (ModulemdModule *self, GHashTable *requires);

GHashTable *
modulemd_module_get_requires (ModulemdModule *self);

GHashTable *
modulemd_module_peek_requires (ModulemdModule *self);

void
modulemd_module_set_rpm_api (ModulemdModule *self, ModulemdSimpleSet *apis);

ModulemdSimpleSet *
modulemd_module_get_rpm_api (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_rpm_api (ModulemdModule *self);

void
modulemd_module_set_rpm_artifacts (ModulemdModule *self,
                                   ModulemdSimpleSet *artifacts);

ModulemdSimpleSet *
modulemd_module_get_rpm_artifacts (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_rpm_artifacts (ModulemdModule *self);

void
modulemd_module_set_rpm_buildopts (ModulemdModule *self,
                                   GHashTable *buildopts);

GHashTable *
modulemd_module_get_rpm_buildopts (ModulemdModule *self);

GHashTable *
modulemd_module_peek_rpm_buildopts (ModulemdModule *self);

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

void
modulemd_module_set_rpm_filter (ModulemdModule *self,
                                ModulemdSimpleSet *filter);

ModulemdSimpleSet *
modulemd_module_get_rpm_filter (ModulemdModule *self);

ModulemdSimpleSet *
modulemd_module_peek_rpm_filter (ModulemdModule *self);

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

void
modulemd_module_set_stream (ModulemdModule *self, const gchar *stream);

const gchar *
modulemd_module_get_stream (ModulemdModule *self);

const gchar *
modulemd_module_peek_stream (ModulemdModule *self);

void
modulemd_module_set_summary (ModulemdModule *self, const gchar *summary);

const gchar *
modulemd_module_get_summary (ModulemdModule *self);

const gchar *
modulemd_module_peek_summary (ModulemdModule *self);

void
modulemd_module_set_tracker (ModulemdModule *self, const gchar *tracker);

const gchar *
modulemd_module_get_tracker (ModulemdModule *self);

const gchar *
modulemd_module_peek_tracker (ModulemdModule *self);

void
modulemd_module_set_version (ModulemdModule *self, const guint64 version);

const guint64
modulemd_module_get_version (ModulemdModule *self);

const guint64
modulemd_module_peek_version (ModulemdModule *self);

void
modulemd_module_set_xmd (ModulemdModule *self, GHashTable *xmd);

GHashTable *
modulemd_module_get_xmd (ModulemdModule *self);

GHashTable *
modulemd_module_peek_xmd (ModulemdModule *self);

G_END_DECLS

#endif /* MODULEMD_MODULE_H */
