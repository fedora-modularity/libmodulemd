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

#include "modulemd-module-stream-v1.h"
#include "modulemd-subdocument-info.h"
#include <glib-object.h>
#include <yaml.h>


G_BEGIN_DECLS


/**
 * SECTION: modulemd-module-stream-v1-private
 * @title: Modulemd.ModuleStreamV1 (Private)
 * @stability: private
 * @short_description: #ModulemdModuleStreamV1 methods that should only be
 * used by internal consumers.
 */


struct _ModulemdModuleStreamV1
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
  GHashTable *rpm_components; /* <string, Modulemd.ComponentRpm> */
  GHashTable *module_components; /* <string, Modulemd.ComponentModule */

  GHashTable *content_licenses; /* string set */
  GHashTable *module_licenses; /* string set */

  GHashTable *profiles; /* <string, Modulemd.Profile> */

  GHashTable *rpm_api; /* string set */

  GHashTable *rpm_artifacts; /* string set */

  GHashTable *rpm_filters; /* string set */

  GHashTable *servicelevels; /* <string, Modulemd.ServiceLevel */

  GHashTable *buildtime_deps; /* <string, string> */
  GHashTable *runtime_deps; /* <string, string> */

  GVariant *xmd;
};


/**
 * modulemd_module_stream_v1_parse_yaml:
 * @subdoc: (in): A #ModulemdSubdocumentInfo representing a stream v1
 * document
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @error: (out): A #GError that will return the reason for a parsing or
 * validation error.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStreamV1 object
 * read from the YAML. NULL if a parse or validation error occurred and sets
 * @error appropriately.
 *
 * Since: 2.0
 */
ModulemdModuleStreamV1 *
modulemd_module_stream_v1_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                                      gboolean strict,
                                      GError **error);

/**
 * modulemd_module_stream_v1_emit_yaml:
 * @self: This #ModulemdModuleStreamV1 object.
 * @emitter: (inout): A libyaml emitter object positioned where a Module Stream
 * document belongs.
 * @error: (out): A #GError that will return the reason for an emission or
 * validation error.
 *
 * Returns: TRUE if the stream was emitted successfully. FALSE and sets
 * @error appropriately if the YAML could not be emitted.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_stream_v1_emit_yaml (ModulemdModuleStreamV1 *self,
                                     yaml_emitter_t *emitter,
                                     GError **error);


G_END_DECLS
