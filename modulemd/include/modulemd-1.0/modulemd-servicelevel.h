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

#ifndef _MODULEMD_SERVICELEVEL_H
#define _MODULEMD_SERVICELEVEL_H

#include "modulemd.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_SERVICELEVEL modulemd_servicelevel_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdServiceLevel, modulemd_servicelevel, MODULEMD, SERVICELEVEL, GObject)

ModulemdServiceLevel *
modulemd_servicelevel_new (void);

void
modulemd_servicelevel_set_eol (ModulemdServiceLevel *self, const GDate *date);

MMD_DEPRECATED_FOR (modulemd_servicelevel_peek_eol)
const GDate *
modulemd_servicelevel_get_eol (ModulemdServiceLevel *self);

const GDate *
modulemd_servicelevel_peek_eol (ModulemdServiceLevel *self);

GDate *
modulemd_servicelevel_dup_eol (ModulemdServiceLevel *self);

void
modulemd_servicelevel_set_name (ModulemdServiceLevel *self, const gchar *name);

MMD_DEPRECATED_FOR (modulemd_servicelevel_peek_name)
const gchar *
modulemd_servicelevel_get_name (ModulemdServiceLevel *self);

const gchar *
modulemd_servicelevel_peek_name (ModulemdServiceLevel *self);

gchar *
modulemd_servicelevel_dup_name (ModulemdServiceLevel *self);

ModulemdServiceLevel *
modulemd_servicelevel_copy (ModulemdServiceLevel *self);

G_END_DECLS

#endif /* _MODULEMD_SERVICELEVEL_H */
