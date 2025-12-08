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

#include <glib.h>
#include <glib/gtypes.h>

#include "config.h"

/* GDate autoptr cleanup was finally added in GLib 2.63.3. */
#ifndef HAVE_GDATE_AUTOPTR
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GDate, g_date_free)
#endif

/* G_TEST_SUBPROCESS_DEFAULT was added in Glib 2.74. */
#ifndef HAVE_G_TEST_SUBPROCESS_DEFAULT
#define G_TEST_SUBPROCESS_DEFAULT 0
#endif

#ifndef HAVE_EXTEND_AND_STEAL

void
g_ptr_array_extend (GPtrArray *array_to_extend,
                    GPtrArray *array,
                    GCopyFunc func,
                    gpointer user_data);


/*
 * g_ptr_array_extend_and_steal:
 * @array_to_extend: (transfer none): a #GPtrArray.
 * @array: (transfer container): a #GPtrArray to add to the end of
 *     @array_to_extend.
 *
 * Adds all the pointers in @array to the end of @array_to_extend, transferring
 * ownership of each element from @array to @array_to_extend and modifying
 * @array_to_extend in-place. @array is then freed.
 *
 * As with g_ptr_array_free(), @array will be destroyed if its reference count
 * is 1. If its reference count is higher, it will be decremented and the
 * length of @array set to zero.
 *
 * Since: glib 2.62
 */

void
g_ptr_array_extend_and_steal (GPtrArray *array_to_extend, GPtrArray *array);
#endif
