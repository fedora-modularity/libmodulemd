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


/*
 * This header includes functions for this object that should be considered
 * internal to libmodulemd
 */

#pragma once

#include "modulemd.h"
#include <modulemd-subdocument.h>

G_BEGIN_DECLS

void
modulemd_subdocument_set_doctype (ModulemdSubdocument *self, const GType type);


/**
 * modulemd_subdocument_get_doctype:
 *
 * Returns: A #GType of the GObject that represents this subdocument
 *
 * Since: 1.4
 */
const GType
modulemd_subdocument_get_doctype (ModulemdSubdocument *self);


void
modulemd_subdocument_set_version (ModulemdSubdocument *self,
                                  const guint64 version);


/**
 * modulemd_subdocument_get_version:
 *
 * Returns: A 64-bit integer describing the document version
 *
 * Since: 1.4
 */
const GType
modulemd_subdocument_get_version (ModulemdSubdocument *self);

void
modulemd_subdocument_set_yaml (ModulemdSubdocument *self, const gchar *yaml);

void
modulemd_subdocument_set_gerror (ModulemdSubdocument *self,
                                 const GError *gerror);

G_END_DECLS
