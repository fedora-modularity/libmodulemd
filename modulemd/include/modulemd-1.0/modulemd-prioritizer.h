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

#ifndef MODULEMD_PRIORITIZER_H
#define MODULEMD_PRIORITIZER_H


#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-prioritizer
 * @title: Modulemd.Prioritizer
 * @short_description: Class to aid in merging module metadata from multiple
 * repositories.
 */

#define MODULEMD_PRIORITIZER_ERROR modulemd_prioritizer_error_quark ()
GQuark
modulemd_prioritizer_error_quark (void);

enum ModulemdPrioritizerError
{
  MODULEMD_PRIORITIZER_NOTHING_TO_PRIORITIZE,
  MODULEMD_PRIORITIZER_PRIORITY_OUT_OF_RANGE
};

#define MODULEMD_PRIORITIZER_PRIORITY_MAX 1000
#define MODULEMD_PRIORITIZER_PRIORITY_MIN 0

#define MODULEMD_TYPE_PRIORITIZER (modulemd_prioritizer_get_type ())

G_DECLARE_FINAL_TYPE (
  ModulemdPrioritizer, modulemd_prioritizer, MODULEMD, PRIORITIZER, GObject)


/**
 * modulemd_prioritizer_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdPrioritizer. This object
 * must be freed with g_object_unref().
 *
 * Since: 1.3
 */
ModulemdPrioritizer *
modulemd_prioritizer_new (void);


/**
 * modulemd_prioritizer_add:
 * @objects: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * module-related objects loaded from a modulemd YAML stream.
 * @priority: The priority of the YAML stream these were loaded from. Items at
 * the same priority level will attempt to merge on conflict. Items at higher
 * priority levels will replace on conflict. Valid values are 0 - 1000.
 *
 * Returns: TRUE if the objects could be added without generating a conflict at
 * this priority level. If a conflict was detected, this function returns FALSE
 * and error is set. The internal state is undefined in the case of an error.
 *
 * Since: 1.3
 */
gboolean
modulemd_prioritizer_add (ModulemdPrioritizer *self,
                          GPtrArray *objects,
                          gint64 priority,
                          GError **error);


/**
 * modulemd_prioritizer_resolve:
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of module-related objects with all priorities resolved. This
 * object must be freed with g_ptr_array_unref().
 *
 * Since: 1.3
 */
GPtrArray *
modulemd_prioritizer_resolve (ModulemdPrioritizer *self, GError **error);


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

#endif /* MODULEMD_PRIORITIZER_H */
