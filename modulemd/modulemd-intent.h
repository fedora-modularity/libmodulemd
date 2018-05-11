/* modulemd-intent.h
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

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define MODULEMD_TYPE_INTENT (modulemd_intent_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdIntent, modulemd_intent, MODULEMD, INTENT, GObject)

ModulemdIntent *
modulemd_intent_new (const gchar *name);

void
modulemd_intent_set_intent_name (ModulemdIntent *self, const gchar *name);
const gchar *
modulemd_intent_peek_intent_name (ModulemdIntent *self);
gchar *
modulemd_intent_dup_intent_name (ModulemdIntent *self);


void
modulemd_intent_set_default_stream (ModulemdIntent *self, const gchar *stream);
const gchar *
modulemd_intent_peek_default_stream (ModulemdIntent *self);
gchar *
modulemd_intent_dup_default_stream (ModulemdIntent *self);


void
modulemd_intent_set_profiles_for_stream (ModulemdIntent *self,
                                         const gchar *stream,
                                         gchar **profiles);
void
modulemd_intent_assign_profiles_for_stream (ModulemdIntent *self,
                                            const gchar *stream,
                                            ModulemdSimpleSet *profiles);
void
modulemd_intent_set_profile_defaults (ModulemdIntent *self,
                                      GHashTable *profile_defaults);
gchar **
modulemd_intent_dup_profiles_for_stream (ModulemdIntent *self,
                                         const gchar *stream);
GHashTable *
modulemd_intent_peek_profile_defaults (ModulemdIntent *self);
GHashTable *
modulemd_intent_dup_profile_defaults (ModulemdIntent *self);

ModulemdIntent *
modulemd_intent_copy (ModulemdIntent *self);

G_END_DECLS
