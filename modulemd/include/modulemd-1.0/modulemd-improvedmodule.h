/* modulemd-improvedmodule.h
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

#include "modulemd-modulestream.h"
#include "modulemd-defaults.h"

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define MODULEMD_TYPE_IMPROVEDMODULE (modulemd_improvedmodule_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdImprovedModule,
                      modulemd_improvedmodule,
                      MODULEMD,
                      IMPROVEDMODULE,
                      GObject)

ModulemdImprovedModule *
modulemd_improvedmodule_new (const gchar *name);

void
modulemd_improvedmodule_add_stream (ModulemdImprovedModule *self,
                                    ModulemdModuleStream *stream);
ModulemdModuleStream *
modulemd_improvedmodule_get_stream_by_name (ModulemdImprovedModule *self,
                                            const gchar *stream_name);

GHashTable *
modulemd_improvedmodule_get_streams (ModulemdImprovedModule *self);

void
modulemd_improvedmodule_set_name (ModulemdImprovedModule *self,
                                  const gchar *module_name);

gchar *
modulemd_improvedmodule_get_name (ModulemdImprovedModule *self);

const gchar *
modulemd_improvedmodule_peek_name (ModulemdImprovedModule *self);

void
modulemd_improvedmodule_set_defaults (ModulemdImprovedModule *self,
                                      ModulemdDefaults *defaults);

ModulemdDefaults *
modulemd_improvedmodule_get_defaults (ModulemdImprovedModule *self);

ModulemdDefaults *
modulemd_improvedmodule_peek_defaults (ModulemdImprovedModule *self);

ModulemdImprovedModule *
modulemd_improvedmodule_copy (ModulemdImprovedModule *self);

G_END_DECLS
