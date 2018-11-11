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

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-simpleset
 * @title: Modulemd.SimpleSet
 * @short_description: Stores a set of strings, guaranteeing uniqueness.
 */

#define MODULEMD_TYPE_SIMPLESET modulemd_simpleset_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdSimpleSet, modulemd_simpleset, MODULEMD, SIMPLESET, GObject)


/**
 * modulemd_simpleset_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdSimpleset. This object
 * must be freed with g_object_unref().
 *
 * Since: 1.0
 */
ModulemdSimpleSet *
modulemd_simpleset_new (void);


/**
 * modulemd_simpleset_contains:
 * @value: A string to compare against the set
 *
 * Returns: TRUE if the value existed in the set.
 *
 * Since: 1.0
 */
gboolean
modulemd_simpleset_contains (ModulemdSimpleSet *self, const gchar *value);


/**
 * modulemd_simpleset_size:
 *
 * Returns: The number of elements in the set
 *
 * Since: 1.0
 */
guint
modulemd_simpleset_size (ModulemdSimpleSet *self);


/**
 * modulemd_simpleset_set:
 * @set: (array zero-terminated=1) (transfer none): Extensible metadata block
 *
 * Make the contents of the set equal to an array of strings. This function
 * will trigger a signal only if the resulting set is different. It does not
 * guarantee any order to the resulting set, only that it will be unique.
 *
 * Since: 1.0
 */
void
modulemd_simpleset_set (ModulemdSimpleSet *self, gchar **set);


/**
 * modulemd_simpleset_get:
 *
 * Retrieves the set as an array of strings
 *
 * Returns: (array zero-terminated=1) (transfer full):
 * A list representing a set of string values.
 *
 * Since: 1.0
 */
gchar **
modulemd_simpleset_get (ModulemdSimpleSet *self);


/**
 * modulemd_simpleset_dup:
 *
 * Retrieves the set as an array of strings
 *
 * Returns: (array zero-terminated=1) (transfer full):
 * A list representing a set of string values.
 *
 * Since: 1.1
 */
gchar **
modulemd_simpleset_dup (ModulemdSimpleSet *self);


/**
 * modulemd_simpleset_add
 * @value: A new string to add to the set
 *
 * This routine will add a new value to the set if it is not already present.
 *
 * Since: 1.0
 */
void
modulemd_simpleset_add (ModulemdSimpleSet *self, const gchar *value);


/**
 * modulemd_simpleset_remove
 * @value: A string to remove from the set
 *
 * This routine will remove a value from the set if it is present.
 */
void
modulemd_simpleset_remove (ModulemdSimpleSet *self, const gchar *value);


/**
 * modulemd_simpleset_copy
 * @dest: (out) (transfer full): A reference to the destination #ModulemdSimpleSet
 *
 * This function will copy the contents of this #ModulemdSimpleSet to @dest.
 * If the dereferenced pointer is NULL, a new #ModulemdSimpleSet will be
 * allocated.
 *
 * If the dereferenced pointer is not NULL, it will replace the contents of
 * @dest. All existing values in the set will be freed.
 *
 * In either case, the caller is responsible for calling g_object_unref()
 * later to free it.
 *
 * Since: 1.0
 */
void
modulemd_simpleset_copy (ModulemdSimpleSet *self, ModulemdSimpleSet **dest);


/**
 * modulemd_simpleset_is_equal:
 * @other: A #ModulemdSimpleSet to compare against
 *
 * Returns: True if the sets contain the same strings.
 *
 * Since: 1.3
 */
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
