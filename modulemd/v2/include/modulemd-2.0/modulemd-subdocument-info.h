/*
 * This file is part of libmodulemd
 * Copyright (C) 2018 Red Hat, Inc.
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

/**
 * SECTION: modulemd-subdocument-info
 * @title: Modulemd.SubdocumentInfo
 * @stability: stable
 * @short_description: Stores information regarding a YAML subdocument in a stream.
 */

#define MODULEMD_TYPE_SUBDOCUMENT_INFO (modulemd_subdocument_info_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdSubdocumentInfo,
                      modulemd_subdocument_info,
                      MODULEMD,
                      SUBDOCUMENT_INFO,
                      GObject)


/**
 * modulemd_subdocument_info_get_yaml:
 * @self: This #ModulemdSubdocumentInfo
 *
 * Returns: (transfer none): The associated YAML subdocument
 *
 * Since: 2.0
 */
const gchar *
modulemd_subdocument_info_get_yaml (ModulemdSubdocumentInfo *self);


/**
 * modulemd_subdocument_info_get_gerror:
 * @self: This #ModulemdSubdocumentInfo
 *
 * Returns: (transfer none): A GError containing an error code and message about why this subdocument failed parsing.
 *
 * Since: 2.0
 */
const GError *
modulemd_subdocument_info_get_gerror (ModulemdSubdocumentInfo *self);
