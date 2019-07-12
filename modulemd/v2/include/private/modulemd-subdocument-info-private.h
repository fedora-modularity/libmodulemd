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
#include <yaml.h>

#include "modulemd-subdocument-info.h"
#include "private/modulemd-yaml.h"

/**
 * SECTION: modulemd-subdocument-info-private
 * @title: Modulemd.SubdocumentInfo (Private)
 * @stability: Private
 * @short_description: #ModulemdSubdocumentInfo methods that should be used only
 * by internal consumers.
 */


/**
 * modulemd_subdocument_info_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdSubdocumentInfo
 *
 * Since: 2.0
 */
ModulemdSubdocumentInfo *
modulemd_subdocument_info_new (void);


/**
 * modulemd_subdocument_info_copy:
 * @self: This #ModulemdSubdocumentInfo
 *
 * Returns: (transfer full): A deep copy of @self
 *
 * Since: 2.0
 */
ModulemdSubdocumentInfo *
modulemd_subdocument_info_copy (ModulemdSubdocumentInfo *self);


/**
 * modulemd_subdocument_info_set_doctype:
 * @self: This #ModulemdSubdocumentInfo
 * @doctype: The #ModulemdYamlDocumentTypeEnum represented by this subdocument
 *
 * Since: 2.0
 */
void
modulemd_subdocument_info_set_doctype (ModulemdSubdocumentInfo *self,
                                       ModulemdYamlDocumentTypeEnum doctype);


/**
 * modulemd_subdocument_info_get_doctype:
 * @self: This #ModulemdSubdocumentInfo
 *
 * Returns: The type of subdocument represented by this SubdocumentInfo
 *
 * Since: 2.0
 */
ModulemdYamlDocumentTypeEnum
modulemd_subdocument_info_get_doctype (ModulemdSubdocumentInfo *self);


/**
 * modulemd_subdocument_info_set_mdversion:
 * @self: This #ModulemdSubdocumentInfo
 * @mdversion: The metadata version
 *
 * Since: 2.0
 */
void
modulemd_subdocument_info_set_mdversion (ModulemdSubdocumentInfo *self,
                                         guint64 mdversion);


/**
 * modulemd_subdocument_info_get_mdversion:
 * @self: This #ModulemdSubdocumentInfo
 *
 * Returns: The metadata version
 *
 * Since: 2.0
 */
guint64
modulemd_subdocument_info_get_mdversion (ModulemdSubdocumentInfo *self);


/**
 * modulemd_subdocument_info_set_yaml:
 * @self: This #ModulemdSubdocumentInfo
 * @contents: The contents of the document
 *
 * Since: 2.0
 */
void
modulemd_subdocument_info_set_yaml (ModulemdSubdocumentInfo *self,
                                    const gchar *contents);


/**
 * modulemd_subdocument_info_set_gerror:
 * @self: This #ModulemdSubdocumentInfo
 * @error: The error
 *
 * Since: 2.0
 */
void
modulemd_subdocument_info_set_gerror (ModulemdSubdocumentInfo *self,
                                      const GError *error);


/**
 * modulemd_subdocument_info_get_data_parser:
 * @self: This #ModulemdSubdocumentInfo
 * @parser: (inout): An unconfigured libyaml parser
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError containing the parser error if this function fails.
 *
 * Since: 2.0
 */
gboolean
modulemd_subdocument_info_get_data_parser (ModulemdSubdocumentInfo *self,
                                           yaml_parser_t *parser,
                                           gboolean strict,
                                           GError **error);
