/*
 * This file is part of libmodulemd
 * Copyright (C) 2017-2018 Stephen Gallagher
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#ifndef MODULEMD_H
#define MODULEMD_H

#ifdef MMD_DISABLE_DEPRECATION_WARNINGS
#define MMD_DEPRECATED extern
#define MMD_DEPRECATED_FOR(f) extern
#define MMD_UNAVAILABLE(maj, min) extern
#define MMD_DEPRECATED_TYPE_FOR(f)
#else
#define MMD_DEPRECATED G_DEPRECATED extern
#define MMD_DEPRECATED_FOR(f) G_DEPRECATED_FOR (f) extern
#define MMD_DEPRECATED_TYPE_FOR(f) G_DEPRECATED_FOR (f)
#define MMD_UNAVAILABLE(maj, min) G_UNAVAILABLE (maj, min) extern
#endif


#include <glib.h>
#include <glib-object.h>
#include <stdio.h>

#include "modulemd-buildopts.h"
#include "modulemd-component.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-defaults.h"
#include "modulemd-dependencies.h"
#include "modulemd-improvedmodule.h"
#include "modulemd-intent.h"
#include "modulemd-module.h"
#include "modulemd-modulestream.h"
#include "modulemd-prioritizer.h"
#include "modulemd-profile.h"
#include "modulemd-simpleset.h"
#include "modulemd-servicelevel.h"
#include "modulemd-subdocument.h"
#include "modulemd-translation.h"
#include "modulemd-translation-entry.h"

G_BEGIN_DECLS

const gchar *
modulemd_get_version (void);

GPtrArray *
modulemd_objects_from_file (const gchar *yaml_file, GError **error);

GPtrArray *
modulemd_objects_from_file_ext (const gchar *yaml_file,
                                GPtrArray **failures,
                                GError **error);

GHashTable *
modulemd_index_from_file (const gchar *yaml_file,
                          GPtrArray **failures,
                          GError **error);

GPtrArray *
modulemd_objects_from_string (const gchar *yaml_string, GError **error);

GPtrArray *
modulemd_objects_from_string_ext (const gchar *yaml_string,
                                  GPtrArray **failures,
                                  GError **error);

GHashTable *
modulemd_index_from_string (const gchar *yaml_string,
                            GPtrArray **failures,
                            GError **error);

GPtrArray *
modulemd_objects_from_stream (FILE *stream, GError **error);

GPtrArray *
modulemd_objects_from_stream_ext (FILE *stream,
                                  GPtrArray **failures,
                                  GError **error);

GHashTable *
modulemd_index_from_stream (FILE *yaml_stream,
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
