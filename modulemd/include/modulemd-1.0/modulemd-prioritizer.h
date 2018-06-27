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

#ifndef MODULEMD_PRIORITIZER_H
#define MODULEMD_PRIORITIZER_H

#include "modulemd.h"

G_BEGIN_DECLS

#define MODULEMD_PRIORITIZER_ERROR modulemd_prioritizer_error_quark ()
GQuark
modulemd_prioritizer_error_quark (void);

enum ModulemdPrioritizerError
{
  MODULEMD_PRIORITIZER_NOTHING_TO_PRIORITIZE,
  MODULEMD_PRIORITIZER_PRIORITY_OUT_OF_RANGE
};

#define MODULEMD_PRIORITIZER_PRIORITY_MAX 1000
#define MODULEMD_PRIORITIZER_PRIORITY_MIN 0

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
