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
#include <modulemd-module-stream.h>

G_BEGIN_DECLS

#define MODULEMD_TYPE_MODULE_STREAM_V2 (modulemd_module_stream_v2_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdModuleStreamV2,
                      modulemd_module_stream_v2,
                      MODULEMD,
                      MODULE_STREAM_V2,
                      ModulemdModuleStream)

ModulemdModuleStreamV2 *
modulemd_module_stream_v2_new (const gchar *module_name,
                               const gchar *module_stream);

G_END_DECLS
