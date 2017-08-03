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
#include "modulemd-simpleset.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_MODULE modulemd_module_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdModule, modulemd_module, MODULEMD, MODULE, GObject)

ModulemdModule *
modulemd_module_new (void);

void
modulemd_module_set_buildrequires (ModulemdModule *self,
                                   GHashTable *buildrequires);

GHashTable *
modulemd_module_get_buildrequires (ModulemdModule *self);

void
modulemd_module_set_community (ModulemdModule *self, const gchar *community);

const gchar *
modulemd_module_get_community (ModulemdModule *self);

void
modulemd_module_set_content_licenses (ModulemdModule *self,
                                      ModulemdSimpleSet *licenses);

ModulemdSimpleSet *
modulemd_module_get_content_licenses (ModulemdModule *self);

void
modulemd_module_set_description (ModulemdModule *self,
                                 const gchar *description);

const gchar *
modulemd_module_get_description (ModulemdModule *self);

void
modulemd_module_set_documentation (ModulemdModule *self,
                                   const gchar *documentation);

const gchar *
modulemd_module_get_documentation (ModulemdModule *self);

void
modulemd_module_set_mdversion (ModulemdModule *self, const guint64 mdversion);

const guint64
modulemd_module_get_mdversion (ModulemdModule *self);

void
modulemd_module_set_module_components (ModulemdModule *self,
                                       GHashTable *components);

GHashTable *
modulemd_module_get_module_components (ModulemdModule *self);

void
modulemd_module_set_module_licenses (ModulemdModule *self,
                                     ModulemdSimpleSet *licenses);

ModulemdSimpleSet *
modulemd_module_get_module_licenses (ModulemdModule *self);

void
modulemd_module_set_name (ModulemdModule *self, const gchar *name);

const gchar *
modulemd_module_get_name (ModulemdModule *self);

void
modulemd_module_set_profiles (ModulemdModule *self, GHashTable *profiles);

GHashTable *
modulemd_module_get_profiles (ModulemdModule *self);

void
modulemd_module_set_requires (ModulemdModule *self, GHashTable *requires);

GHashTable *
modulemd_module_get_requires (ModulemdModule *self);

void
modulemd_module_set_rpm_api (ModulemdModule *self, ModulemdSimpleSet *apis);

ModulemdSimpleSet *
modulemd_module_get_rpm_api (ModulemdModule *self);

void
modulemd_module_set_rpm_artifacts (ModulemdModule *self,
                                   ModulemdSimpleSet *artifacts);

ModulemdSimpleSet *
modulemd_module_get_rpm_artifacts (ModulemdModule *self);

void
modulemd_module_set_rpm_buildopts (ModulemdModule *self,
                                   GHashTable *buildopts);

GHashTable *
modulemd_module_get_rpm_buildopts (ModulemdModule *self);

void
modulemd_module_set_rpm_components (ModulemdModule *self,
                                    GHashTable *components);

GHashTable *
modulemd_module_get_rpm_components (ModulemdModule *self);

void
modulemd_module_set_rpm_filter (ModulemdModule *self,
                                ModulemdSimpleSet *filter);

ModulemdSimpleSet *
modulemd_module_get_rpm_filter (ModulemdModule *self);

void
modulemd_module_set_stream (ModulemdModule *self, const gchar *stream);

const gchar *
modulemd_module_get_stream (ModulemdModule *self);

void
modulemd_module_set_summary (ModulemdModule *self, const gchar *summary);

const gchar *
modulemd_module_get_summary (ModulemdModule *self);

void
modulemd_module_set_tracker (ModulemdModule *self, const gchar *tracker);

const gchar *
modulemd_module_get_tracker (ModulemdModule *self);

void
modulemd_module_set_version (ModulemdModule *self, const guint64 version);

const guint64
modulemd_module_get_version (ModulemdModule *self);

void
modulemd_module_set_xmd (ModulemdModule *self, GHashTable *xmd);

GHashTable *
modulemd_module_get_xmd (ModulemdModule *self);

G_END_DECLS

#endif /* MODULEMD_MODULE_H */
