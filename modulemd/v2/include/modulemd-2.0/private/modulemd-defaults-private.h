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


/**
 * SECTION: modulemd-defaults-private
 * @title: Modulemd.Defaults (Private)
 * @stability: private
 * @short_description: #ModulemdDefaults methods that should only be used by
 * internal consumers.
 */


#define DEFAULT_PLACEHOLDER "__DEFAULT_PLACEHOLDER__"


void
modulemd_defaults_set_module_name (ModulemdDefaults *self,
                                   const gchar *module_name);


G_END_DECLS
