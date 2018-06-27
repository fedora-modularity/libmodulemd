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

#include <glib-object.h>

G_BEGIN_DECLS

#define MODULEMD_TYPE_SUBDOCUMENT (modulemd_subdocument_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdSubdocument, modulemd_subdocument, MODULEMD, SUBDOCUMENT, GObject)

ModulemdSubdocument *
modulemd_subdocument_new (void);

const gchar *
modulemd_subdocument_get_yaml (ModulemdSubdocument *self);

const GError *
modulemd_subdocument_get_gerror (ModulemdSubdocument *self);

G_END_DECLS
