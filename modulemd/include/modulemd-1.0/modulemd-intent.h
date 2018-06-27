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

#pragma once

#include "modulemd.h"

#include "modulemd-simpleset.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_INTENT (modulemd_intent_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdIntent, modulemd_intent, MODULEMD, INTENT, GObject)

ModulemdIntent *
modulemd_intent_new (const gchar *name);

void
modulemd_intent_set_intent_name (ModulemdIntent *self, const gchar *name);
const gchar *
modulemd_intent_peek_intent_name (ModulemdIntent *self);
gchar *
modulemd_intent_dup_intent_name (ModulemdIntent *self);


void
modulemd_intent_set_default_stream (ModulemdIntent *self, const gchar *stream);
const gchar *
modulemd_intent_peek_default_stream (ModulemdIntent *self);
gchar *
modulemd_intent_dup_default_stream (ModulemdIntent *self);


void
modulemd_intent_set_profiles_for_stream (ModulemdIntent *self,
                                         const gchar *stream,
                                         gchar **profiles);
void
modulemd_intent_assign_profiles_for_stream (ModulemdIntent *self,
                                            const gchar *stream,
                                            ModulemdSimpleSet *profiles);
void
modulemd_intent_set_profile_defaults (ModulemdIntent *self,
                                      GHashTable *profile_defaults);
gchar **
modulemd_intent_dup_profiles_for_stream (ModulemdIntent *self,
                                         const gchar *stream);
GHashTable *
modulemd_intent_peek_profile_defaults (ModulemdIntent *self);
GHashTable *
modulemd_intent_dup_profile_defaults (ModulemdIntent *self);

ModulemdIntent *
modulemd_intent_copy (ModulemdIntent *self);

G_END_DECLS
