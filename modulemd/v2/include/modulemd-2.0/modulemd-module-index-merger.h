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

#include <glib-object.h>
#include "modulemd-module-index.h"

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
 * At present, libmodulemd does not interrogate more closely to determine if
 * they have the same content, so if the repository configuration is broken and
 * there exists two #ModulemdModuleStream entries that have different content
 * for the same NSVC, the behavior is undefined.
 *
 * Merging #ModulemdDefaults entries behaves as follows:
 *
 * - Any module default that is provided by a single repository is
 *   authoritative.
 * - If the repos have different priorities (not common), then the default for
 *   this module and stream name coming from the repo of higher priority will
 *   be used and the default from the lower-priority repo will not be included.
 * - If the repos have the same priority (such as "fedora" and "updates" in the
 *   Fedora Project), the entries will be merged as follows:
 *   - If both repositories specify a default stream for the module, use it.
 *   - If either repository specifies a default stream for the module and the
 *     other does not, use the one specified.
 *   - If both repositories specify different streams, this is an unresolvable
 *     merge conflict and the merge resolution will fail and report an error.
 *   - If both repositories specify a set of default profiles for a stream and
 *     the sets are equivalent, use that set.
 *   - If one repository specifies a set of default profiles for a stream and
 *     the other does not, use the one specified.
 *   - If both repositories specify a set of default profiles for a stream and
 *     each are providing a different set, this is an unresolvable merge
 *     conflict and the merge resolution will fail and report an error.
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
 * Returns: (transfer full): A newly-allocated Modulemd.ModuleIndexMerger
 * object.
 *
 * Since:2.0
 */
ModulemdModuleIndexMerger *
modulemd_module_index_merger_new (void);


/**
 * modulemd_module_index_merger_associate_index:
 * @self: (in): This #ModulemdModuleIndexMerger object.
 * @index: (in) (transfer none): A #ModuleIndex, usually constructed by reading
 * the module metadata from a repository with
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
 * Returns: (transfer full): A newly-allocated #ModulemdModuleIndex containing
 * the merged results. If this function encounters an unresolvable merge
 * conflict, it will return NULL and set @error appropriately.
 *
 * Since: 2.0
 */
ModulemdModuleIndex *
modulemd_module_index_merger_resolve (ModulemdModuleIndexMerger *self,
                                      GError **error);

G_END_DECLS
