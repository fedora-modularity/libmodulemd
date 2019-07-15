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

#ifdef MMD_DISABLE_DEPRECATION_WARNINGS
#define MMD_DEPRECATED extern
#define MMD_DEPRECATED_FOR(f) extern
#define MMD_UNAVAILABLE(maj, min) extern
#define MMD_DEPRECATED_TYPE_FOR(f)
#else
#define MMD_DEPRECATED G_DEPRECATED extern
#define MMD_DEPRECATED_FOR(f) G_DEPRECATED_FOR (f) extern
#define MMD_DEPRECATED_TYPE_FOR(f) G_DEPRECATED_FOR (f)
#define MMD_UNAVAILABLE(maj, min) G_UNAVAILABLE (maj, min) extern
#endif

G_END_DECLS
