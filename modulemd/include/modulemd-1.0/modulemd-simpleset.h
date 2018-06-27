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

#ifndef MODULEMD_SIMPLESET_H
#define MODULEMD_SIMPLESET_H

#include "modulemd.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_SIMPLESET modulemd_simpleset_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdSimpleSet, modulemd_simpleset, MODULEMD, SIMPLESET, GObject)

ModulemdSimpleSet *
modulemd_simpleset_new (void);

gboolean
modulemd_simpleset_contains (ModulemdSimpleSet *self, const gchar *value);
guint
modulemd_simpleset_size (ModulemdSimpleSet *self);

void
modulemd_simpleset_set (ModulemdSimpleSet *self, gchar **set);

MMD_DEPRECATED_FOR (modulemd_simpleset_dup)
gchar **
modulemd_simpleset_get (ModulemdSimpleSet *self);

gchar **
modulemd_simpleset_dup (ModulemdSimpleSet *self);

void
modulemd_simpleset_add (ModulemdSimpleSet *self, const gchar *value);

void
modulemd_simpleset_remove (ModulemdSimpleSet *self, const gchar *value);

void
modulemd_simpleset_copy (ModulemdSimpleSet *self, ModulemdSimpleSet **dest);

gboolean
modulemd_simpleset_is_equal (ModulemdSimpleSet *self,
                             ModulemdSimpleSet *other);

/**
 * SimpleSetValidationFn:
 * @str: The current string being validated from the set
 *
 * This function is called once for each string in a #ModulemdSimpleSet
 * when the validate_contents() routine is invoked. It must return TRUE
 * if the string passes validation or FALSE if it does not.
 *
 * Since: 1.4
 */
typedef gboolean (*ModulemdSimpleSetValidationFn) (const gchar *str);

gboolean
modulemd_simpleset_validate_contents (ModulemdSimpleSet *self,
                                      ModulemdSimpleSetValidationFn func,
                                      GPtrArray **failures);

G_END_DECLS

#endif /* MODULEMD_SIMPLESET_H */
