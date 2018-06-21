/* modulemd.h
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

#ifndef MODULEMD_H
#define MODULEMD_H

#include <glib.h>
#include <stdio.h>

#include "modulemd-buildopts.h"
#include "modulemd-component.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-defaults.h"
#include "modulemd-dependencies.h"
#include "modulemd-intent.h"
#include "modulemd-module.h"
#include "modulemd-modulestream.h"
#include "modulemd-prioritizer.h"
#include "modulemd-profile.h"
#include "modulemd-simpleset.h"
#include "modulemd-servicelevel.h"
#include "modulemd-subdocument.h"

G_BEGIN_DECLS

const gchar *
modulemd_get_version (void);

GPtrArray *
modulemd_objects_from_file (const gchar *yaml_file, GError **error);

GPtrArray *
modulemd_objects_from_file_ext (const gchar *yaml_file,
                                GPtrArray **failures,
                                GError **error);

GPtrArray *
modulemd_objects_from_string (const gchar *yaml_string, GError **error);

GPtrArray *
modulemd_objects_from_string_ext (const gchar *yaml_string,
                                  GPtrArray **failures,
                                  GError **error);

GPtrArray *
modulemd_objects_from_stream (FILE *stream, GError **error);

GPtrArray *
modulemd_objects_from_stream_ext (FILE *stream,
                                  GPtrArray **failures,
                                  GError **error);

void
modulemd_dump (GPtrArray *objects, const gchar *yaml_file, GError **error);

gchar *
modulemd_dumps (GPtrArray *objects, GError **error);

GPtrArray *
modulemd_merge_defaults (const GPtrArray *first,
                         const GPtrArray *second,
                         gboolean override,
                         GError **error);

G_END_DECLS

#endif /* MODULEMD_H */
