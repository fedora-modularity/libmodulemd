/* modulemd-private.h
 *
 * Copyright (C) 2018 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MODULEMD_PRIVATE_H
#define MODULEMD_PRIVATE_H

#include "modulemd.h"

gboolean
modulemd_module_set_mdversion_range (ModulemdModule *self,
                                     const guint64 min_mdversion,
                                     const guint64 max_mdversion);

gboolean
modulemd_module_check_mdversion_range_is_set (ModulemdModule *self);

gboolean
modulemd_module_check_mdversion_range (ModulemdModule *self,
                                       const guint64 version);

gboolean
modulemd_module_check_mdversion_range_full (ModulemdModule *self,
                                            const guint64 min_version,
                                            const guint64 max_version);

#endif /* MODULEMD_PRIVATE_H */
