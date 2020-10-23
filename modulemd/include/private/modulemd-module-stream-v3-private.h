/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat, Inc.
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

#include "modulemd-module-stream-v3.h"
#include "modulemd-subdocument-info.h"
#include <glib-object.h>
#include <yaml.h>


G_BEGIN_DECLS


/**
 * SECTION: modulemd-module-stream-v3-private
 * @title: Modulemd.ModuleStreamV3 (Private)
 * @stability: private
 * @short_description: #ModulemdModuleStreamV3 methods that should only be
 * used by internal consumers.
 */


struct _ModulemdModuleStreamV3
{
  GObject parent_instance;

  /* Properties */
  ModulemdBuildopts *buildopts;
  gchar *community;
  gchar *description;
  gchar *documentation;
  gchar *summary;
  gchar *tracker;
  gchar *platform;

  /* Internal Data Structures */
  GHashTable *module_components; /* <string, Modulemd.ComponentModule> */
  GHashTable *rpm_components; /* <string, Modulemd.ComponentRpm> */

  GHashTable *content_licenses; /* string set */
  GHashTable *module_licenses; /* string set */

  GHashTable *profiles; /* <string, Modulemd.Profile> */

  GHashTable *rpm_api; /* string set */

  GHashTable *rpm_artifacts; /* string set */

  /*  < string, GHashTable <string, Modulemd.RpmMapEntry> > */
  GHashTable *rpm_artifact_map;

  GHashTable *rpm_filters; /* string set */

  GHashTable *buildtime_deps; /* <string, string> */
  GHashTable *runtime_deps; /* <string, string> */

  ModulemdObsoletes *obsoletes;

  GVariant *xmd;
};


/**
 * modulemd_module_stream_v3_parse_yaml:
 * @subdoc: (in): A #ModulemdSubdocumentInfo representing a stream v3
 * document.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStreamV3 object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.10
 */
ModulemdModuleStreamV3 *
modulemd_module_stream_v3_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                      gboolean strict,
                                      GError **error);

/**
 * modulemd_module_stream_v3_emit_yaml:
 * @self: This #ModulemdModuleStreamV3 object.
 * @emitter: (inout): A libyaml emitter object positioned where the data
 * section of a #ModulemdModuleStreamV3 belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the stream was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.10
 */
gboolean
modulemd_module_stream_v3_emit_yaml (ModulemdModuleStreamV3 *self,
                                     yaml_emitter_t *emitter,
                                     GError **error);

/**
 * modulemd_module_stream_v3_replace_content_licenses:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @set: (in): A #GHashTable set of licenses under which one or more of the
 * components of this module stream are distributed.
 *
 * Any existing content licenses associated with module stream @self are
 * removed and replaced by @set.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_replace_content_licenses (
  ModulemdModuleStreamV3 *self, GHashTable *set);

/**
 * modulemd_module_stream_v3_replace_module_licenses:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @set: (in): A #GHashTable set of licenses under which this module stream is
 * distributed.
 *
 * Any existing module licenses associated with module stream @self are removed
 * and replaced by @set.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_replace_module_licenses (
  ModulemdModuleStreamV3 *self, GHashTable *set);

/**
 * modulemd_module_stream_v3_replace_rpm_api:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @set: (in): A #GHashTable set of binary RPMs present in this module stream that is
 * considered stable public API.
 *
 * Any existing API RPMs associated with module stream @self are removed and
 * replaced by @set.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_replace_rpm_api (ModulemdModuleStreamV3 *self,
                                           GHashTable *set);

/**
 * modulemd_module_stream_v3_replace_rpm_artifacts:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @set: (in): A #GHashTable set of NEVRs of binary RPMs present in this module
 * stream.
 *
 * Any existing artifact RPMs associated with module stream @self are removed
 * and replaced by @set.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_replace_rpm_artifacts (ModulemdModuleStreamV3 *self,
                                                 GHashTable *set);

/**
 * modulemd_module_stream_v3_replace_rpm_filters:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @set: (in): A #GHashTable set of names of binary RPMs to filter out of this
 * module stream.
 *
 * Any existing filtered binary RPM names associated with module stream @self
 * are removed and replaced by @set.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_replace_rpm_filters (ModulemdModuleStreamV3 *self,
                                               GHashTable *set);

/**
 * modulemd_module_stream_v3_includes_nevra:
 * @self: This #ModulemdModuleStreamV3 object.
 * @nevra_pattern: (not nullable): A [glob](https://www.mankier.com/3/glob)
 * pattern to match against the NEVRA strings of the rpm artifacts in @self.
 *
 * Returns: TRUE if this stream includes at least one RPM artifact that
 * matches @nevra_pattern. FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
modulemd_module_stream_v3_includes_nevra (ModulemdModuleStreamV3 *self,
                                          const gchar *nevra_pattern);

/**
 * modulemd_module_stream_v3_associate_obsoletes:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 * @obsoletes: (in): The #ModulemdObsoletes information to associate with
 * this object.
 *
 * Since: 2.10
 */
void
modulemd_module_stream_v3_associate_obsoletes (ModulemdModuleStreamV3 *self,
                                               ModulemdObsoletes *obsoletes);

/**
 * modulemd_module_stream_v3_get_obsoletes:
 * @self: (in): This #ModulemdModuleStreamV3 object.
 *
 * Returns: (transfer none): The #ModulemdObsoletes information associated with this
 * object. This function doesn't resolve the reset attribute of obsoletes, this means
 * even if obsoletes associated with this stream has attribute reset set the obsoletes
 * object is still returned.
 *
 * Since: 2.10
 */
ModulemdObsoletes *
modulemd_module_stream_v3_get_obsoletes (ModulemdModuleStreamV3 *self);


G_END_DECLS
