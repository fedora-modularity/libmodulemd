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

#ifndef MODULEMD_PROFILE_H
#define MODULEMD_PROFILE_H

#include "modulemd.h"
#include "modulemd-prioritizer.h"

G_BEGIN_DECLS

#define MODULEMD_TYPE_PROFILE modulemd_profile_get_type ()
G_DECLARE_FINAL_TYPE (
  ModulemdProfile, modulemd_profile, MODULEMD, PROFILE, GObject)

ModulemdProfile *
modulemd_profile_new (void);

void
modulemd_profile_set_description (ModulemdProfile *self,
                                  const gchar *description);

MMD_DEPRECATED_FOR (modulemd_profile_peek_description)
const gchar *
modulemd_profile_get_description (ModulemdProfile *self);

/**
 * modulemd_profile_get_localized_description:
 * @locale: (transfer none) (nullable): Specify the locale for the description.
 * If NULL is passed, it will attempt to use the LC_MESSAGES locale. If "C" is
 * passed or if the locale has no translation available, it will treat it as
 * untranslated.
 *
 * Returns: (transfer full): A string containing the "description" property,
 * translated into the language specified by @locale if possible. This string
 * must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_profile_get_localized_description (ModulemdProfile *self,
                                            const gchar *locale);

const gchar *
modulemd_profile_peek_description (ModulemdProfile *self);
gchar *
modulemd_profile_dup_description (ModulemdProfile *self);

void
modulemd_profile_set_name (ModulemdProfile *self, const gchar *name);

MMD_DEPRECATED_FOR (modulemd_profile_peek_name)
const gchar *
modulemd_profile_get_name (ModulemdProfile *self);

const gchar *
modulemd_profile_peek_name (ModulemdProfile *self);
gchar *
modulemd_profile_dup_name (ModulemdProfile *self);

void
modulemd_profile_set_rpms (ModulemdProfile *self, ModulemdSimpleSet *rpms);

MMD_DEPRECATED_FOR (modulemd_profile_peek_rpms)
ModulemdSimpleSet *
modulemd_profile_get_rpms (ModulemdProfile *self);

ModulemdSimpleSet *
modulemd_profile_peek_rpms (ModulemdProfile *self);
ModulemdSimpleSet *
modulemd_profile_dup_rpms (ModulemdProfile *self);

void
modulemd_profile_add_rpm (ModulemdProfile *self, const gchar *rpm);

void
modulemd_profile_remove_rpm (ModulemdProfile *self, const gchar *rpm);

ModulemdProfile *
modulemd_profile_copy (ModulemdProfile *self);


/**
 * modulemd_prioritizer_add_index:
 * @index: (transfer none) (not nullable): A #GHashTable of
 * #ModulemdImprovedModule objects whose contents need to be merged depending on
 * priority.
 * @priority: The priority of the YAML stream these were loaded from. Items at
 * the same priority level will attempt to merge on conflict. Items at higher
 * priority levels will replace on conflict. Valid values are 0 - 1000.
 *
 * Returns: TRUE if the objects could be added without generating a conflict at
 * this priority level. If a conflict was detected, this function returns FALSE
 * and @error is set. The internal state is undefined in the case of an error.
 *
 * Since: 1.6
 */
gboolean
modulemd_prioritizer_add_index (ModulemdPrioritizer *self,
                                GHashTable *index,
                                gint64 priority,
                                GError **error);

/**
 * modulemd_prioritizer_resolve_index:
 *
 * Returns: (element-type utf8 ModulemdImprovedModule) (transfer container):
 * A #GHashTable of #ModulemdImprovedModule objects with all priorities
 * resolved. This hash table must be freed with g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_prioritizer_resolve_index (ModulemdPrioritizer *self, GError **error);

G_END_DECLS

#endif /* MODULEMD_PROFILE_H */
