/* modulemd-metadata.h
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

#ifndef MODULEMD_MODULEMETADATA_H
#define MODULEMD_MODULEMETADATA_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MODULEMD_TYPE_MODULEMETADATA modulemd_modulemetadata_get_type ()
G_DECLARE_FINAL_TYPE (ModulemdModuleMetadata, modulemd_modulemetadata,
                      MODULEMD, MODULEMETADATA, GObject)

ModulemdModuleMetadata *modulemd_modulemetadata_new (void);
void modulemd_modulemetadata_free (ModulemdModuleMetadata *md);

void
modulemd_modulemetadata_set_community (ModulemdModuleMetadata *self,
                                       const gchar *community);

const gchar *
modulemd_modulemetadata_get_community (ModulemdModuleMetadata *self);

void
modulemd_modulemetadata_set_description (ModulemdModuleMetadata *self,
                                         const gchar *description);

const gchar *
modulemd_modulemetadata_get_description (ModulemdModuleMetadata *self);

void
modulemd_modulemetadata_set_documentation (ModulemdModuleMetadata *self,
                                           const gchar *documentation);

const gchar *
modulemd_modulemetadata_get_documentation (ModulemdModuleMetadata *self);

void modulemd_modulemetadata_set_mdversion (ModulemdModuleMetadata *self,
                                            const guint64 mdversion);

const guint64
modulemd_modulemetadata_get_mdversion (ModulemdModuleMetadata *self);

void
modulemd_modulemetadata_set_name (ModulemdModuleMetadata *self,
                                  const gchar *name);

const gchar *
modulemd_modulemetadata_get_name (ModulemdModuleMetadata *self);

void
modulemd_modulemetadata_set_stream (ModulemdModuleMetadata *self,
                                    const gchar *stream);

const gchar *
modulemd_modulemetadata_get_stream (ModulemdModuleMetadata *self);

void
modulemd_modulemetadata_set_summary (ModulemdModuleMetadata *self,
                                     const gchar *summary);

const gchar *
modulemd_modulemetadata_get_summary (ModulemdModuleMetadata *self);

void
modulemd_modulemetadata_set_tracker (ModulemdModuleMetadata *self,
                                     const gchar *tracker);

const gchar *
modulemd_modulemetadata_get_tracker (ModulemdModuleMetadata *self);

void
modulemd_modulemetadata_set_version (ModulemdModuleMetadata *self,
                                     const guint64 version);

const guint64
modulemd_modulemetadata_get_version (ModulemdModuleMetadata *self);

G_END_DECLS

#endif /* MODULEMD_MODULEMETADATA_H */
