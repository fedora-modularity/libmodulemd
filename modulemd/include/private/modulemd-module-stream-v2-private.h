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

#include "modulemd-module-stream-v2.h"
#include "modulemd-subdocument-info.h"
#include <glib-object.h>
#include <yaml.h>


G_BEGIN_DECLS


/**
 * SECTION: modulemd-module-stream-v2-private
 * @title: Modulemd.ModuleStreamV2 (Private)
 * @stability: private
 * @short_description: #ModulemdModuleStreamV2 methods that should only be
 * used by internal consumers.
 */


struct _ModulemdModuleStreamV2
{
  GObject parent_instance;

  /* Properties */
  ModulemdBuildopts *buildopts;
  gchar *community;
  gchar *description;
  gchar *documentation;
  gchar *summary;
  gchar *tracker;

  /* Internal Data Structures */
  GHashTable *module_components; /* <string, Modulemd.ComponentModule */
  GHashTable *rpm_components; /* <string, Modulemd.ComponentRpm> */

  GHashTable *content_licenses; /* string set */
  GHashTable *module_licenses; /* string set */

  GHashTable *profiles; /* <string, Modulemd.Profile> */

  GHashTable *rpm_api; /* string set */

  GHashTable *rpm_artifacts; /* string set */

  /*  < string, GHashTable <string, Modulemd.RpmMapEntry> > */
  GHashTable *rpm_artifact_map;

  GHashTable *rpm_filters; /* string set */

  GHashTable *servicelevels; /* <string, Modulemd.ServiceLevel */

  GPtrArray *dependencies; /* <Modulemd.Dependencies> */

  GVariant *xmd;
};


/**
 * modulemd_module_stream_v2_parse_yaml:
 * @subdoc: (in): A #ModulemdSubdocumentInfo representing a stream v2
 * document.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStreamV2 object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.0
 */
ModulemdModuleStreamV2 *
modulemd_module_stream_v2_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                      gboolean strict,
                                      GError **error);

/**
 * modulemd_module_stream_v2_emit_yaml:
 * @self: This #ModulemdModuleStreamV2 object.
 * @emitter: (inout): A libyaml emitter object positioned where the data
 * section of a #ModulemdModuleStreamV2 belongs in the YAML document.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the stream was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_stream_v2_emit_yaml (ModulemdModuleStreamV2 *self,
                                     yaml_emitter_t *emitter,
                                     GError **error);

/**
 * modulemd_module_stream_v2_replace_content_licenses:
 * @self: (in): This #ModulemdModuleStreamV2 object.
 * @set: (in): A #GHashTable set of licenses under which one or more of the
 * components of this module stream are distributed.
 *
 * Any existing content licenses associated with module stream @self are
 * removed and replaced by @set.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v2_replace_content_licenses (
  ModulemdModuleStreamV2 *self, GHashTable *set);

/**
 * modulemd_module_stream_v2_replace_module_licenses:
 * @self: (in): This #ModulemdModuleStreamV2 object.
 * @set: (in): A #GHashTable set of licenses under which this module stream is
 * distributed.
 *
 * Any existing module licenses associated with module stream @self are removed
 * and replaced by @set.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v2_replace_module_licenses (
  ModulemdModuleStreamV2 *self, GHashTable *set);

/**
 * modulemd_module_stream_v2_replace_rpm_api:
 * @self: (in): This #ModulemdModuleStreamV2 object.
 * @set: (in): A #GHashTable set of binary RPMs present in this module stream that is
 * considered stable public API.
 *
 * Any existing API RPMs associated with module stream @self are removed and
 * replaced by @set.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v2_replace_rpm_api (ModulemdModuleStreamV2 *self,
                                           GHashTable *set);

/**
 * modulemd_module_stream_v2_replace_rpm_artifacts:
 * @self: (in): This #ModulemdModuleStreamV2 object.
 * @set: (in): A #GHashTable set of NEVRs of binary RPMs present in this module
 * stream.
 *
 * Any existing artifact RPMs associated with module stream @self are removed
 * and replaced by @set.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v2_replace_rpm_artifacts (ModulemdModuleStreamV2 *self,
                                                 GHashTable *set);

/**
 * modulemd_module_stream_v2_replace_rpm_filters:
 * @self: (in): This #ModulemdModuleStreamV2 object.
 * @set: (in): A #GHashTable set of names of binary RPMs to filter out of this
 * module stream.
 *
 * Any existing filtered binary RPM names associated with module stream @self
 * are removed and replaced by @set.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v2_replace_rpm_filters (ModulemdModuleStreamV2 *self,
                                               GHashTable *set);

/**
 * modulemd_module_stream_v2_replace_dependencies:
 * @self: (in): This #ModulemdModuleStreamV2 object.
 * @array: (in): A #GPtrArray of #ModulemdDependencies objects for this module
 * stream.
 *
 * Any existing dependencies associated with module stream @self are removed
 * and replaced by @array.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_v2_replace_dependencies (ModulemdModuleStreamV2 *self,
                                                GPtrArray *array);


/**
 * modulemd_module_stream_v2_includes_nevra:
 * @self: This #ModulemdModuleStreamV2 object.
 * @nevra_pattern: (not nullable): A [glob](https://www.mankier.com/3/glob)
 * pattern to match against the NEVRA strings of the rpm artifacts in @self.
 *
 * Returns: TRUE if this stream includes at least one RPM artifact that
 * matches @nevra_pattern. FALSE otherwise.
 *
 * Since: 2.9
 */
gboolean
modulemd_module_stream_v2_includes_nevra (ModulemdModuleStreamV2 *self,
                                          const gchar *nevra_pattern);

G_END_DECLS
