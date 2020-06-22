/*
 * This file is part of libmodulemd
 * Copyright (C) 2018 Stephen Gallagher
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

#include "modulemd-module-index.h"
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-module-index-merger
 * @title: Modulemd.ModuleIndexMerger
 * @stability: stable
 * @short_description: Class to aid in merging metadata from multiple
 * repositories.
 *
 * ModuleIndexMerger is used to resolve merges between multiple repository
 * metadata sources, possibly with varying priorities.
 *
 * It is expected to be used as follows (python example) by tools such as `yum`:
 *
 * |[<!-- language="Python" -->
 * merger = Modulemd.ModuleIndexMerger.new()
 *
 * merger.associate_index(fedora_repo_index, 0)
 * merger.associate_index(updates_repo_index, 0)
 * merger.associate_index(updates_testing_repo_index, 0)
 *
 * merged_index = merger.resolve()
 *
 * ]|
 *
 * In the above code, merged_index will be a complete, merged view of the
 * metadata from all of the added #ModulemdModuleIndex instances.
 *
 * When merging module streams, entries will be deduplicated based on whether
 * they share the same module name, stream name, version number, and context.
 * If the repository configuration is broken and there exists two
 * #ModulemdModuleStream entries that have different content for the same
 * NSVCA, the behavior is undefined.
 *
 * Merging #ModulemdDefaults entries behaves as follows (note that this
 * behavior has changed slightly as of 2.8.1):
 *
 * - Any module defaults object that is provided by a single
 *   #ModulemdModuleIndex will be the defaults object in the resulting merged
 *   #ModulemdModuleIndex.
 * - If the #ModulemdModuleIndex inputs have different priorities (not common),
 *   then the defaults from the highest priority #ModulemdModuleIndex will be
 *   used and the others entirely discarded. The `modified` value will not be
 *   considered at all. (Priority is intended for providing a total override,
 *   including an on-disk configuration).
 * - If the repos have the same priority (such as "fedora" and "updates" in the
 *   Fedora Project) and `modified` value, the entries will be merged as
 *   follows for default streams:
 *   - If both #ModulemdModuleIndex objects specify the same default stream for
 *     the module, that one will be used.
 *   - If either #ModulemdModuleIndex specifies a default stream for the module
 *     and the other does not, the provided one will be used.
 *   - If both #ModulemdModuleIndex objects specify different default streams
 *     and have different `modified` values, the default stream from the
 *     #ModulemdDefaults object with the higher `modified` value will be used.
 *   - If both #ModulemdModuleIndex objects specify different default streams
 *     and have the same `modified` value, the merge will unset the default
 *     stream and leave no default stream in the resulting merged
 *     #ModulemdModuleIndex. This behavior can be controlled by using
 *     modulemd_module_index_merger_resolve_ext() and setting
 *     `strict_default_streams` to #TRUE. In that case, an error will be
 *     returned if conflicting default streams have been provided.
 * - and for profile defaults:
 *   - If both #ModulemdModuleIndex objects specify a set of default profiles
 *     for a particular module and stream and the sets are equivalent, use that
 *     set.
 *   - If one #ModulemdModuleIndex object specifies a set of default profiles
 *     for a module and stream and the other does not, use the provided set.
 *   - If both #ModulemdModuleIndex objects specify a set of default profiles
 *     for a stream, each are providing a different set and the `modified`
 *     value differs, the set from the object with the higher `modified` value
 *     will be used.
 *   - If both #ModulemdModuleIndex objects specify a set of default profiles
 *     for a stream, each are providing a different set and the `modified`
 *     value is the same, this is an unresolvable merge conflict and the merge
 *     resolution will fail and return an error.
 *   - Intents behave in exactly the same manner as described for the top-level
 *     defaults, except that they merge beneath each intent name.
 *
 * Merging #ModulemdTranslations entries behaves as follows:
 *
 * - For each translated summary, description and profile description, compare
 *   the `modified` value of the Translation document matching this module name
 *   and stream. Whichever has the higher value will be used. Any translation
 *   containing the empty string will be interpreted as removing the
 *   translation. Subsequent processing of a higher modified value may restore
 *   it.
 *
 * Merging #ModulemdObsoletes entries with identical module, stream, context
 * and modified values behaves as follows (when accessed through module):
 *
 * - Any obsoletes object that is provided by a single #ModulemdModuleIndex
 *   will be the obsoletes object in the resulting merged #ModulemdModuleIndex.
 * - If the #ModulemdModuleIndex inputs have different priorities (not common),
 *   then the obsoletes from the highest priority #ModulemdModuleIndex will be
 *   used and the others entirely discarded. (Priority is intended for providing
 *   a total override, including an on-disk configuration).
 * - If the repos have the same priority (such as "fedora" and "updates" in the
 *   Fedora Project) the behaviour is undefined.
 */


#define MODULEMD_TYPE_MODULE_INDEX_MERGER                                     \
  (modulemd_module_index_merger_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdModuleIndexMerger,
                      modulemd_module_index_merger,
                      MODULEMD,
                      MODULE_INDEX_MERGER,
                      GObject)


/**
 * modulemd_module_index_merger_new:
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleIndexMerger
 * object.
 *
 * Since: 2.0
 */
ModulemdModuleIndexMerger *
modulemd_module_index_merger_new (void);


/**
 * modulemd_module_index_merger_associate_index:
 * @self: (in): This #ModulemdModuleIndexMerger object.
 * @index: (in) (transfer none): A #ModulemdModuleIndex, usually constructed by
 * reading the module metadata from a repository with
 * modulemd_module_index_update_from_file(),
 * modulemd_module_index_update_from_string(), or
 * `modulemd_module_index_update_from_stream()`. This function take a reference
 * on @index, so the caller must not modify it while the
 * #ModulemdModuleIndexMerger is in use.
 * @priority: (in): The priority of the repository that the entries in @index
 * came from. This is used to determine when @index should override rather then
 * merge. In most cases, this will be zero. See the Description section for the
 * #ModulemdModuleIndexMerger class for details on the merge logic. Acceptable
 * values are in the range of 0-1000.
 *
 * Enqueues a #ModulemdModuleIndex representing the parsed metadata from a
 * repository into this #ModulemdModuleIndexMerger for merging and
 * deduplication of other repositories.
 *
 * Once all repositories have been added, call
 * modulemd_module_index_merger_resolve() to perform the merge.
 *
 * Since: 2.0
 */
void
modulemd_module_index_merger_associate_index (ModulemdModuleIndexMerger *self,
                                              ModulemdModuleIndex *index,
                                              gint32 priority);


/**
 * modulemd_module_index_merger_resolve:
 * @self: (in): This #ModulemdModuleIndexMerger object.
 * @error: (out): A #GError containing the reason for a failure to resolve the
 * merges.
 *
 * Merges all added #ModulemdModuleIndex objects according to their priority.
 * The logic of this merge is described in the Description of
 * #ModulemdModuleIndexMerger.
 *
 * Once this function has been called, the internal state of the
 * #ModulemdModuleIndexMerger is undefined. The only valid action on it after
 * that point is g_object_unref().
 *
 * This function is equivalent to calling
 * modulemd_module_index_merger_resolve_ext() with
 * `strict_default_streams=FALSE`.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleIndex object
 * containing the merged results. If this function encounters an unresolvable
 * merge conflict, it will return NULL and set @error appropriately.
 *
 * Since: 2.0
 */
ModulemdModuleIndex *
modulemd_module_index_merger_resolve (ModulemdModuleIndexMerger *self,
                                      GError **error);


/**
 * modulemd_module_index_merger_resolve_ext:
 * @self: (in): This #ModulemdModuleIndexMerger object.
 * @strict_default_streams: (in): If TRUE, merging two #ModulemdDefaults with
 * conflicting default streams will raise an error. If FALSE, the module will
 * have its default stream blocked.
 * @error: (out): A #GError containing the reason for a failure to resolve the
 * merges.
 *
 * Merges all added #ModulemdModuleIndex objects according to their priority.
 * The logic of this merge is described in the Description of
 * #ModulemdModuleIndexMerger.
 *
 * Once this function has been called, the internal state of the
 * #ModulemdModuleIndexMerger is undefined. The only valid action on it after
 * that point is g_object_unref().
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleIndex object
 * containing the merged results. If this function encounters an unresolvable
 * merge conflict, it will return NULL and set @error appropriately.
 *
 * Since: 2.6
 */
ModulemdModuleIndex *
modulemd_module_index_merger_resolve_ext (ModulemdModuleIndexMerger *self,
                                          gboolean strict_default_streams,
                                          GError **error);

G_END_DECLS
