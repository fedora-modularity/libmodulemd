/* modulemd-subdocument-private.h
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

/*
 * This header includes functions for this object that should be considered
 * internal to libmodulemd
 */

#pragma once

#include <glib.h>
#include <modulemd-subdocument.h>

G_BEGIN_DECLS

void
modulemd_subdocument_set_doctype (ModulemdSubdocument *self, const GType type);

const GType
modulemd_subdocument_get_doctype (ModulemdSubdocument *self);

void
modulemd_subdocument_set_version (ModulemdSubdocument *self,
                                  const guint64 version);

const GType
modulemd_subdocument_get_version (ModulemdSubdocument *self);

void
modulemd_subdocument_set_yaml (ModulemdSubdocument *self, const gchar *yaml);

void
modulemd_subdocument_set_gerror (ModulemdSubdocument *self,
                                 const GError *gerror);

G_END_DECLS
