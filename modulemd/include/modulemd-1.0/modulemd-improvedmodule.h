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

#include "modulemd.h"
#include "modulemd-modulestream.h"
#include "modulemd-defaults.h"

#pragma once

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

void
modulemd_improvedmodule_dump (ModulemdImprovedModule *self,
                              const gchar *yaml_file,
                              GError **error);

gchar *
modulemd_improvedmodule_dumps (ModulemdImprovedModule *self, GError **error);

G_END_DECLS
