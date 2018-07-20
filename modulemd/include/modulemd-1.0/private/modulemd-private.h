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

enum
{
  MD_VERSION_UNSET = 0,

  MD_VERSION_1 = 1,
  MD_VERSION_2 = 2,

  MD_VERSION_MAX = G_MAXUINT64
};

#define MD_VERSION_LATEST MD_VERSION_2

ModulemdModule *
modulemd_module_new_from_modulestream (ModulemdModuleStream *stream);

ModulemdModuleStream *
modulemd_module_peek_modulestream (ModulemdModule *self);
