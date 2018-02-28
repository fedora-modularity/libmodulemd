/* modulemd-component.h
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

#ifndef MODULEMD_COMPONENT_H
#define MODULEMD_COMPONENT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MODULEMD_TYPE_COMPONENT modulemd_component_get_type ()
G_DECLARE_DERIVABLE_TYPE (
  ModulemdComponent, modulemd_component, MODULEMD, COMPONENT, GObject)

struct _ModulemdComponentClass
{
  GObjectClass parent_instance;

  /* Virtual Public Members */
  void (*set_buildorder) (ModulemdComponent *self, guint64 buildorder);
  guint64 (*peek_buildorder) (ModulemdComponent *self);

  void (*set_name) (ModulemdComponent *self, const gchar *name);
  const gchar *(*peek_name) (ModulemdComponent *self);

  void (*set_rationale) (ModulemdComponent *self, const gchar *rationale);
  const gchar *(*peek_rationale) (ModulemdComponent *self);

  /* Padding to allow adding up to 12 new virtual functions without
     * breaking ABI. */
  gpointer padding[12];
};

ModulemdComponent *
modulemd_component_new (void);

void
modulemd_component_set_buildorder (ModulemdComponent *self,
                                   guint64 buildorder);
guint64
modulemd_component_get_buildorder (ModulemdComponent *self);

guint64
modulemd_component_peek_buildorder (ModulemdComponent *self);

void
modulemd_component_set_name (ModulemdComponent *self, const gchar *name);
const gchar *
modulemd_component_get_name (ModulemdComponent *self);
const gchar *
modulemd_component_peek_name (ModulemdComponent *self);

void
modulemd_component_set_rationale (ModulemdComponent *self,
                                  const gchar *rationale);
const gchar *
modulemd_component_get_rationale (ModulemdComponent *self);
const gchar *
modulemd_component_peek_rationale (ModulemdComponent *self);

G_END_DECLS

#endif /* MODULEMD_COMPONENT_H */
