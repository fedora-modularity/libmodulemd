/* modulemd-servicelevel.h
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

#ifndef _MODULEMD_SERVICELEVEL_H
#define _MODULEMD_SERVICELEVEL_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MODULEMD_TYPE_SERVICELEVEL modulemd_servicelevel_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdServiceLevel, modulemd_servicelevel, MODULEMD, SERVICELEVEL, GObject)

ModulemdServiceLevel *
modulemd_servicelevel_new (void);

void
modulemd_servicelevel_set_eol (ModulemdServiceLevel *self, const GDate *date);

const GDate *
modulemd_servicelevel_get_eol (ModulemdServiceLevel *self);

const GDate *
modulemd_servicelevel_peek_eol (ModulemdServiceLevel *self);

void
modulemd_servicelevel_set_name (ModulemdServiceLevel *self, const gchar *name);

const gchar *
modulemd_servicelevel_get_name (ModulemdServiceLevel *self);

const gchar *
modulemd_servicelevel_peek_name (ModulemdServiceLevel *self);

G_END_DECLS

#endif /* _MODULEMD_SERVICELEVEL_H */
