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
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef MODULEMD_DEFAULTS_H
#define MODULEMD_DEFAULTS_H

#include <glib-object.h>
#include <stdio.h>

#include "modulemd-intent.h"
#include "modulemd-simpleset.h"

enum
{
  MD_DEFAULTS_VERSION_UNSET = 0,

  MD_DEFAULTS_VERSION_1 = 1,

  MD_DEFAULTS_VERSION_MAX = G_MAXUINT64
};

#define MD_DEFAULTS_VERSION_LATEST MD_DEFAULTS_VERSION_1

G_BEGIN_DECLS

#define MODULEMD_DEFAULTS_ERROR modulemd_defaults_error_quark ()
GQuark
modulemd_defaults_error_quark (void);

enum ModulemdDefaultsError
{
  MODULEMD_DEFAULTS_ERROR_MISSING_CONTENT,
  MODULEMD_DEFAULTS_ERROR_CONFLICTING_STREAMS,
  MODULEMD_DEFAULTS_ERROR_CONFLICTING_PROFILES,
  MODULEMD_DEFAULTS_ERROR_CONFLICTING_INTENT_STREAM,
  MODULEMD_DEFAULTS_ERROR_CONFLICTING_INTENT_PROFILE
};

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
modulemd_defaults_assign_profiles_for_stream (ModulemdDefaults *self,
                                              const gchar *stream,
                                              ModulemdSimpleSet *profiles);
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

void
modulemd_defaults_add_intent (ModulemdDefaults *self, ModulemdIntent *intent);
void
modulemd_defaults_set_intents (ModulemdDefaults *self, GHashTable *intents);
GHashTable *
modulemd_defaults_peek_intents (ModulemdDefaults *self);
GHashTable *
modulemd_defaults_dup_intents (ModulemdDefaults *self);


ModulemdDefaults *
modulemd_defaults_new_from_file (const gchar *yaml_file, GError **error);

ModulemdDefaults *
modulemd_defaults_new_from_file_ext (const gchar *yaml_file,
                                     GPtrArray **failures,
                                     GError **error);

ModulemdDefaults *
modulemd_defaults_new_from_string (const gchar *yaml_string, GError **error);

ModulemdDefaults *
modulemd_defaults_new_from_string_ext (const gchar *yaml_string,
                                       GPtrArray **failures,
                                       GError **error);

ModulemdDefaults *
modulemd_defaults_new_from_stream (FILE *stream, GError **error);

ModulemdDefaults *
modulemd_defaults_new_from_stream_ext (FILE *stream,
                                       GPtrArray **failures,
                                       GError **error);


void
modulemd_defaults_dump (ModulemdDefaults *self, const gchar *file_path);

void
modulemd_defaults_dumps (ModulemdDefaults *self, gchar **yaml_string);

ModulemdDefaults *
modulemd_defaults_copy (ModulemdDefaults *self);

ModulemdDefaults *
modulemd_defaults_merge (ModulemdDefaults *first,
                         ModulemdDefaults *second,
                         gboolean override,
                         GError **error);

G_END_DECLS

#endif /* MODULEMD_DEFAULTS_H */
