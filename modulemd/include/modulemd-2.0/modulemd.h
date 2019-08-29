/*
 * This file is part of libmodulemd
 * Copyright (C) 2018 Red Hat, Inc.
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

#include "modulemd-buildopts.h"
#include "modulemd-component.h"
#include "modulemd-component-module.h"
#include "modulemd-component-rpm.h"
#include "modulemd-compression.h"
#include "modulemd-defaults.h"
#include "modulemd-defaults-v1.h"
#include "modulemd-dependencies.h"
#include "modulemd-deprecated.h"
#include "modulemd-errors.h"
#include "modulemd-module.h"
#include "modulemd-module-index.h"
#include "modulemd-module-index-merger.h"
#include "modulemd-module-stream.h"
#include "modulemd-module-stream-v1.h"
#include "modulemd-module-stream-v2.h"
#include "modulemd-profile.h"
#include "modulemd-rpm-map-entry.h"
#include "modulemd-service-level.h"
#include "modulemd-subdocument-info.h"
#include "modulemd-translation.h"
#include "modulemd-translation-entry.h"

G_BEGIN_DECLS


/**
 * SECTION: modulemd
 * @title: Modulemd
 * @stability: stable
 * @short_description: User's Guide for libmodulemd
 *
 * # Working with repodata (DNF use-case) #
 * The libmodulemd API provides a number of convenience tools for interacting
 * with repodata (that is, streams of YAML that contains information on multiple
 * streams, default data and translations). The documentation will use two
 * repositories, called "fedora" and "updates" for demonstrative purposes. It
 * will assume that the content of the YAML module metadata from those two
 * repositories have been loaded into string variables `fedora_yaml` and
 * `updates_yaml`, respectively.
 *
 * First step is to load the metadata from these two repositories into
 * #ModulemdModuleIndex objects. This is done as follows:
 *
 * In C:
 * |[<!-- language="C" -->
 * ModulemdModuleIndex * fedora_index = modulemd_module_index_new ();
 * gboolean ret = modulemd_module_index_update_from_string (fedora_index,
 *                                                          fedora_yaml,
 *                                                          TRUE,
 *                                                          &failures,
 *                                                          &error);
 *
 * ModulemdModuleIndex * updates_index = modulemd_module_index_new ();
 * gboolean ret2 = modulemd_module_index_update_from_string (updates_index,
 *                                                           updates_yaml,
 *                                                           TRUE,
 *                                                           &failures,
 *                                                           &error);
 * ]|
 *
 * In Python:
 *
 * |[<!-- language="Python" -->
 * fedora_index = Modulemd.ModuleIndex.new()
 * ret, failures = fedora_index.update_from_string(fedora_yaml, True)
 *
 * fedora_index = Modulemd.ModuleIndex.new()
 * ret, failures = updates_index.update_from_string(updates_yaml, True)
 * ]|
 *
 * The @failures are a list of subdocuments in the YAML that failed parsing,
 * along with the reason they failed. Hence, by checking the return value of
 * @failures we will know if the YAML parsing was successful or not.
 *
 * Since it doesn't really make sense to view the contents from separate
 * repositories in isolation (in most cases), the next step is to merge the two
 * indexes into a combined one:
 *
 * In C:
 * |[<!-- language="C" -->
 * ModulemdModuleIndexMerger * merger = modulemd_module_index_merger_new ();
 *
 * modulemd_module_index_merger_associate_index (merger, fedora_index, 0);
 * modulemd_module_index_merger_associate_index (merger, updates_index, 0);
 *
 * ModulemdModuleIndex * merged_index = modulemd_module_index_merger_resolve (merger, &error);
 * ]|
 *
 * In Python:
 * |[<!-- language="Python" -->
 * merger = Modulemd.ModuleIndexMerger.new()
 *
 * merger.associate_index(fedora_index, 0)
 * merger.associate_index(updates_index, 0)
 *
 * merged_index = merger.resolve()
 * ]|
 *
 * At this point, you now have either a complete view of the merged repodata,
 * or else have received an error describing why the merge was unable to
 * complete successfully. Additionally, it should be noted that the combined
 * metadata in any #ModulemdModuleIndex will have all of its component parts
 * upgraded to match the highest version of those objects seen. So for example
 * if the repodata has a mix of v1 and v2 #ModulemdModuleStream objects, the
 * index will contain only v2 objects (with the v1 objects automatically
 * upgraded internally).
 *
 * Now, we can start operating on the retrieved data. This guide will
 * give only a brief overview of the most common operations. See the API
 * specification for a full list of information that can be retrieved.
 *
 * ## Discover the default stream for a particular module.
 * In C:
 * |[<!-- language="C" -->
 * ModulemdModule * module =  modulemd_module_index_get_module (merged_index, "modulename");
 * ModulemdDefaults * defaults = modulemd_module_get_defaults (module);
 * printf ("Default stream for modulename is %s\n",
 *         modulemd_defaults_v1_get_default_stream (MODULEMD_DEFAULTS_V1 (defaults), NULL));
 * ]|
 *
 * In Python:
 * |[<!-- language="Python" -->
 * module = merged_index.get_module ('modulename')
 * defaults = module.get_defaults()
 * print ('Default stream for modulename is %s' % (
 *        defaults.get_default_stream())
 * ]|
 *
 *
 * ## Get the list of RPMs defining the public API for a particular module NSVCA
 * First, query the #ModulemdModuleIndex for the module with a given name.
 *
 * In C:
 * |[<!-- language="C" -->
 * ModulemdModule * module = modulemd_module_index_get_module (merged_index, "modulename");
 * ]|
 *
 * In Python:
 * |[<!-- language="Python" -->
 * module = merged_index.get_module ('modulename')
 * ]|
 *
 * Then, query the #ModulemdModule for the #ModulemdModuleStream associated with the
 * provided NSVCA (name-stream-version-context-architecture identifier).
 *
 * In C:
 * |[<!-- language="C" -->
 * ModulemdModuleStream * stream = modulemd_module_get_stream_by_NSVCA (module,
 *                                                                      "modulestream",
 *                                                                      0,
 *                                                                      "deadbeef",
 *                                                                      "coolarch",
 *                                                                      &error);
 * ]|
 *
 * In Python:
 * |[<!-- language="Python" -->
 * stream = module.get_stream_by_NSVCA('modulestream', 0, 'deadbeef', 'coolarch')
 * ]|
 *
 * Lastly, read the RPM API from the #ModulemdModuleStream. Here, `api_list` is
 * a list of strings containing package names.
 *
 * In C:
 * |[<!-- language="C" -->
 * GStrv api_list = modulemd_module_stream_v2_get_rpm_api_as_strv (MODULEMD_MODULE_STREAM_V2 (stream));
 * ]|
 *
 * In Python:
 * |[<!-- language="Python" -->
 * api_list = stream.get_rpm_api()
 * ]|
 *
 *
 * ## Retrieve the modular runtime dependencies for a particular module NSVCA
 * In C:
 * |[<!-- language="C" -->
 * ModulemdModule * module = modulemd_module_index_get_module (merged_index, "modulename");
 * ModulemdModuleStream * stream = modulemd_module_get_stream_by_NSVCA (module, "modulestream", 0, "deadbeef", "coolarch", &error);
 * GPtrArray * deps_list = modulemd_module_stream_v2_get_dependencies (MODULEMD_MODULE_STREAM_V2 (stream));
 *
 * for (gint i = 0; i < deps_list->len; i++)
 *   {
 *     stuff with g_ptr_array_index(deps_list, i);
 *   }
 * ]|
 *
 * In Python:
 * |[<!-- language="Python" -->
 * module = merged_index.get_module ('modulename')
 * stream = module.get_stream_by_NSVCA('modulestream', 0, 'deadbeef', 'coolarch')
 * deps_list = stream.get_dependencies()
 *
 * for dep in deps_list:
 *   depstream_list = dep.get_runtime_streams('depstreamname')
 *   <do_stuff>
 * ]|
 *
 *
 * # Working with a single module stream (Packager/MBS use-case)
 * One limitation of the #ModulemdModuleIndex format is that it requires that
 * all module streams loaded into it have both a name and a stream name.
 * This however is not possible when dealing with streams such as a packager
 * would be using (since the build-system auto-generates the module name and
 * stream name from the git repository information. In this case, we need to
 * work with a single module stream document at a time. For this, we will
 * use the #ModulemdModuleStream interface.
 *
 * This example will assume that the module name and stream name have
 * already been determined from the repodata and that they are stored in
 * string variables named `module_name` and `stream_name`, respectively.
 *
 * |[<!-- language="Python" -->
 * stream = Modulemd.ModuleStream.read_file ('/path/to/module_name.yaml',
 *                                           True,
 *                                           module_name,
 *                                           stream_name)
 * v2_stream = stream.upgrade(Modulemd.ModuleStreamVersionEnum.TWO)
 * v2_stream.validate()
 * ]|
 *
 * In the example above, we upgraded the stream to v2, in case we were reading
 * from v1 metadata. This will allow us to avoid having to manage multiple
 * code-paths and support only the latest we understand. After that, it calls
 * validate() to ensure that the content that was read in was valid both
 * syntactically and referentially.
 *
 * Also available is `Modulemd.ModuleStreamVersionEnum.LATEST` which will
 * always represent the highest-supported version of the
 * #ModulemdModuleStream metadata format. This may change at any time.
 */


/**
 * modulemd_get_version:
 *
 * Returns: (transfer none): A string describing the version of libmodulemd.
 *
 * Since: 2.0
 */
const gchar *
modulemd_get_version (void);


G_END_DECLS
