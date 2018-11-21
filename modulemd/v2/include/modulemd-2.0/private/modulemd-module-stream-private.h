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
#include "modulemd-module-stream.h"

G_BEGIN_DECLS


/**
 * SECTION: modulemd-module-stream-private
 * @title: Modulemd.ModuleStream (Private)
 * @stability: private
 * @short_description: #ModulemdModuleStream methods that should only be used
 * by internal consumers.
 */


#define MODULESTREAM_PLACEHOLDER "__MODULESTREAM_PLACEHOLDER__"


void
modulemd_module_stream_set_module_name (ModulemdModuleStream *self,
                                        const gchar *module_name);

void
modulemd_module_stream_set_stream_name (ModulemdModuleStream *self,
                                        const gchar *stream_name);


G_END_DECLS
