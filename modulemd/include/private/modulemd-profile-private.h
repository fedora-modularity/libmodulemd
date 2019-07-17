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
#include "modulemd-profile.h"
#include <glib.h>

G_BEGIN_DECLS

void
modulemd_profile_associate_translation (ModulemdProfile *self,
                                        ModulemdTranslation *translation);

G_END_DECLS
