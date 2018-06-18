/* modulemd-buildopts.h
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

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define MODULEMD_TYPE_BUILDOPTS (modulemd_buildopts_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdBuildopts, modulemd_buildopts, MODULEMD, BUILDOPTS, GObject)

ModulemdBuildopts *
modulemd_buildopts_new (void);

void
modulemd_buildopts_set_rpm_macros (ModulemdBuildopts *self,
                                   const gchar *macros);

gchar *
modulemd_buildopts_get_rpm_macros (ModulemdBuildopts *self);

void
modulemd_buildopts_set_rpm_whitelist (ModulemdBuildopts *self,
                                      GStrv whitelist);

GStrv
modulemd_buildopts_get_rpm_whitelist (ModulemdBuildopts *self);

ModulemdBuildopts *
modulemd_buildopts_copy (ModulemdBuildopts *self);

G_END_DECLS
