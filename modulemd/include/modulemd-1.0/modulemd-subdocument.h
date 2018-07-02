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

G_BEGIN_DECLS

/**
 * SECTION: modulemd-subdocument
 * @title: Modulemd.Subdocument
 * @short_description: Contains information about individual YAML subdocuments
 * being parsed for modulemd information.
 */

#define MODULEMD_TYPE_SUBDOCUMENT (modulemd_subdocument_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdSubdocument, modulemd_subdocument, MODULEMD, SUBDOCUMENT, GObject)


/**
 * modulemd_subdocument_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdSubdocument. This object
 * must be freed with g_object_unref().
 *
 * Since: 1.4
 */
ModulemdSubdocument *
modulemd_subdocument_new (void);


/**
 * modulemd_subdocument_get_yaml:
 *
 * Returns: A string containing the YAML document
 *
 * Since: 1.4
 */
const gchar *
modulemd_subdocument_get_yaml (ModulemdSubdocument *self);


/**
 * modulemd_subdocument_get_gerror:
 *
 * Returns: The #GError associated with this subdocument
 *
 * Since: 1.4
 */
const GError *
modulemd_subdocument_get_gerror (ModulemdSubdocument *self);

G_END_DECLS
