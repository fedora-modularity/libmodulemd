/* modulemd-component-module.h
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

#ifndef MODULEMD_COMPONENT_MODULE_H
#define MODULEMD_COMPONENT_MODULE_H

#include <glib-object.h>
#include "modulemd-component.h"
#include "modulemd-simpleset.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_COMPONENT_MODULE (modulemd_component_module_get_type())

G_DECLARE_FINAL_TYPE (ModulemdComponentModule, modulemd_component_module,
                      MODULEMD, COMPONENT_MODULE, ModulemdComponent)

ModulemdComponentModule *modulemd_component_module_new (void);

void
modulemd_component_module_set_ref (ModulemdComponentModule *self,
                                   const gchar          *ref);
const gchar *
modulemd_component_module_get_ref (ModulemdComponentModule *self);

void
modulemd_component_module_set_repository (ModulemdComponentModule *self,
                                          const gchar          *repository);
const gchar *
modulemd_component_module_get_repository (ModulemdComponentModule *self);

#endif /* MODULEMD_COMPONENT_MODULE_H */

