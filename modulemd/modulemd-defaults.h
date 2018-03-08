/* modulemd-defaults.h
 *
 * Copyright (C) 2018 Stephen Gallagher
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


#ifndef MODULEMD_DEFAULTS_H
#define MODULEMD_DEFAULTS_H

#include <glib-object.h>

enum
{
  MD_DEFAULTS_VERSION_UNSET = 0,

  MD_DEFAULTS_VERSION_1 = 1,

  MD_DEFAULTS_VERSION_MAX = G_MAXUINT64
};

#define MD_DEFAULTS_VERSION_LATEST MD_DEFAULTS_VERSION_1

G_BEGIN_DECLS

#define MODULEMD_TYPE_DEFAULTS (modulemd_defaults_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdDefaults, modulemd_defaults, MODULEMD, DEFAULTS, GObject)

ModulemdDefaults *
modulemd_defaults_new (void);

void
modulemd_defaults_set_version (ModulemdDefaults *self, guint64 version);
guint64
modulemd_defaults_peek_version (ModulemdDefaults *self);


void
modulemd_defaults_set_module_name (ModulemdDefaults *self, const gchar *name);
const gchar *
modulemd_defaults_peek_module_name (ModulemdDefaults *self);
gchar *
modulemd_defaults_dup_module_name (ModulemdDefaults *self);


void
modulemd_defaults_set_default_stream (ModulemdDefaults *self,
                                      const gchar *stream);
const gchar *
modulemd_defaults_peek_default_stream (ModulemdDefaults *self);
gchar *
modulemd_defaults_dup_default_stream (ModulemdDefaults *self);


void
modulemd_defaults_set_profiles_for_stream (ModulemdDefaults *self,
                                           const gchar *stream,
                                           gchar **profiles);
void
modulemd_defaults_set_profile_defaults (ModulemdDefaults *self,
                                        GHashTable *profile_defaults);
gchar **
modulemd_defaults_dup_profiles_for_stream (ModulemdDefaults *self,
                                           const gchar *stream);
GHashTable *
modulemd_defaults_peek_profile_defaults (ModulemdDefaults *self);
GHashTable *
modulemd_defaults_dup_profile_defaults (ModulemdDefaults *self);


G_END_DECLS

#endif /* MODULEMD_DEFAULTS_H */
