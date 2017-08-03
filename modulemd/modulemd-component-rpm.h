/* modulemd-component-rpm.h
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

#ifndef MODULEMD_COMPONENT_RPM_H
#define MODULEMD_COMPONENT_RPM_H

#include <glib-object.h>
#include "modulemd-component.h"
#include "modulemd-simpleset.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_COMPONENT_RPM (modulemd_component_rpm_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdComponentRpm,
                      modulemd_component_rpm,
                      MODULEMD,
                      COMPONENT_RPM,
                      ModulemdComponent)

ModulemdComponentRpm *
modulemd_component_rpm_new (void);

void
modulemd_component_rpm_set_arches (ModulemdComponentRpm *self,
                                   ModulemdSimpleSet *arches);
ModulemdSimpleSet *
modulemd_component_rpm_get_arches (ModulemdComponentRpm *self);

void
modulemd_component_rpm_set_cache (ModulemdComponentRpm *self,
                                  const gchar *cache);
const gchar *
modulemd_component_rpm_get_cache (ModulemdComponentRpm *self);

void
modulemd_component_rpm_set_multilib (ModulemdComponentRpm *self,
                                     ModulemdSimpleSet *multilib);
ModulemdSimpleSet *
modulemd_component_rpm_get_multilib (ModulemdComponentRpm *self);

void
modulemd_component_rpm_set_ref (ModulemdComponentRpm *self, const gchar *ref);
const gchar *
modulemd_component_rpm_get_ref (ModulemdComponentRpm *self);

void
modulemd_component_rpm_set_repository (ModulemdComponentRpm *self,
                                       const gchar *repository);
const gchar *
modulemd_component_rpm_get_repository (ModulemdComponentRpm *self);

#endif /* MODULEMD_COMPONENT_RPM_H */
