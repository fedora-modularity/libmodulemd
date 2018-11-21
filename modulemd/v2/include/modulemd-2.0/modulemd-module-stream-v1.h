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

#define MODULEMD_TYPE_MODULE_STREAM_V1 (modulemd_module_stream_v1_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdModuleStreamV1,
                      modulemd_module_stream_v1,
                      MODULEMD,
                      MODULE_STREAM_V1,
                      ModulemdModuleStream)

ModulemdModuleStreamV1 *
modulemd_module_stream_v1_new (const gchar *module_name,
                               const gchar *module_stream);

G_END_DECLS
