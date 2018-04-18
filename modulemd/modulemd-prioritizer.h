/* modulemd-prioritizer.h
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

#ifndef MODULEMD_PRIORITIZER_H
#define MODULEMD_PRIORITIZER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MODULEMD_PRIORITIZER_ERROR modulemd_prioritizer_error_quark ()
GQuark
modulemd_prioritizer_error_quark (void);

enum ModulemdPrioritizerError
{
  MODULEMD_PRIORITIZER_NOTHING_TO_PRIORITIZE
};

#define MODULEMD_TYPE_PRIORITIZER (modulemd_prioritizer_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdPrioritizer, modulemd_prioritizer, MODULEMD, PRIORITIZER, GObject)

ModulemdPrioritizer *
modulemd_prioritizer_new (void);

gboolean
modulemd_prioritizer_add (ModulemdPrioritizer *self,
                          GPtrArray *objects,
                          gint64 priority,
                          GError **error);

GPtrArray *
modulemd_prioritizer_resolve (ModulemdPrioritizer *self, GError **error);

G_END_DECLS

#endif /* MODULEMD_PRIORITIZER_H */
