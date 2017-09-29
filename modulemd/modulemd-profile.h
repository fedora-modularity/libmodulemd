/* modulemd-profile.h
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

#ifndef MODULEMD_PROFILE_H
#define MODULEMD_PROFILE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MODULEMD_TYPE_PROFILE modulemd_profile_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdProfile, modulemd_profile, MODULEMD, PROFILE, GObject)

ModulemdProfile *
modulemd_profile_new (void);

void
modulemd_profile_set_description (ModulemdProfile *self,
                                  const gchar *description);

const gchar *
modulemd_profile_get_description (ModulemdProfile *self);

void
modulemd_profile_set_rpms (ModulemdProfile *self, ModulemdSimpleSet *rpms);

ModulemdSimpleSet *
modulemd_profile_get_rpms (ModulemdProfile *self);

void
modulemd_profile_add_rpm (ModulemdProfile *self, const gchar *rpm);

void
modulemd_profile_remove_rpm (ModulemdProfile *self, const gchar *rpm);

G_END_DECLS

#endif /* MODULEMD_PROFILE_H */
