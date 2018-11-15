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
#include "modulemd-defaults.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-defaults-v1
 * @title: Modulemd.DefaultsV1
 * @short_description: Object representing a defaults document (version 1)
 */

#define MODULEMD_TYPE_DEFAULTS_V1 (modulemd_defaults_v1_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdDefaultsV1,
                      modulemd_defaults_v1,
                      MODULEMD,
                      DEFAULTS_V1,
                      ModulemdDefaults)


/**
 * modulemd_defaults_v1_new:
 * @module_name: (in): The name of the module to which these defaults apply.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdDefaultsV1 object.
 *
 * Since: 2.0
 */
ModulemdDefaultsV1 *
modulemd_defaults_v1_new (const gchar *module_name);

G_END_DECLS
