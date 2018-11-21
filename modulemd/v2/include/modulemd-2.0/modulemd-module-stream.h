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

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-module-stream
 * @title: Modulemd.ModuleStream
 * @stability: stable
 * @short_description: A parent class for all versions of #ModulemdModuleStream
 * objects.
 */

/**
 * ModulemdModuleStreamVersionEnum:
 * @MD_MODULESTREAM_VERSION_ONE: Represents v1 of the #Modulemd.ModuleStream
 * metadata format.
 * @MD_MODULESTREAM_VERSION_TWO: Represents v2 of the #Modulemd.ModuleStream
 * metadata format.
 * @MD_MODULESTREAM_VERSION_LATEST: Represents the highest-supported version of
 * the #Modulemd.ModuleStream metadata format.
 *
 * Since: 2.0
 */
typedef enum
{
  MD_MODULESTREAM_VERSION_UNSET = 0,

  MD_MODULESTREAM_VERSION_ONE = 1,
  MD_MODULESTREAM_VERSION_TWO = 2,

  MD_MODULESTREAM_VERSION_LATEST = MD_MODULESTREAM_VERSION_TWO
} ModulemdModuleStreamVersionEnum;


#define MODULEMD_TYPE_MODULE_STREAM (modulemd_module_stream_get_type ())

G_DECLARE_DERIVABLE_TYPE (ModulemdModuleStream,
                          modulemd_module_stream,
                          MODULEMD,
                          MODULE_STREAM,
                          GObject)

struct _ModulemdModuleStreamClass
{
  GObjectClass parent_class;

  ModulemdModuleStream *(*copy) (ModulemdModuleStream *self,
                                 const gchar *module_name,
                                 const gchar *module_stream);

  gboolean (*validate) (ModulemdModuleStream *self, GError **error);

  guint64 (*get_mdversion) (ModulemdModuleStream *self);

  /* Padding to allow adding up to 10 new virtual functions without
   * breaking ABI. */
  gpointer padding[10];
};


/**
 * modulemd_module_stream_new:
 * @mdversion: (in): The metadata version of ModuleStream to create.
 * @module_name: (in) (nullable): The name of the module.
 * @module_stream: (in) (nullable): The name of this stream. Optional.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStream object of
 * the requested metadata version.
 *
 * Since: 2.0
 */
ModulemdModuleStream *
modulemd_module_stream_new (guint64 mdversion,
                            const gchar *module_name,
                            const gchar *module_stream);


/**
 * modulemd_module_stream_copy:
 * @self: (in): This #ModulemdModuleStream.
 * @module_name: (in) (nullable): An optional new name for the module of the
 * copied stream.
 * @module_stream: (in) (nullable): An optional new name for the copied stream.
 *
 * Copies a #ModulemdModuleStream, optionally assigning it a new stream name in
 * the process.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStream object
 * that is a complete copy of @self, optionally with a new stream name.
 *
 * Since: 2.0
 */
ModulemdModuleStream *
modulemd_module_stream_copy (ModulemdModuleStream *self,
                             const gchar *module_name,
                             const gchar *module_stream);


/**
 * modulemd_module_stream_upgrade:
 * @self: (in): This #ModulemdModuleStream.
 * @mdversion: (in): The metadata version to upgrade to. If zero, upgrades to
 * the highest-supported version.
 * @error: (out): A #GError that will return the reason for an upgrade error.
 *
 * Return an upgraded copy of this object. Does not modify the original.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStream copy of
 * this object upgraded to the requested version. Returns NULL and sets @error
 * appropriately if the upgrade could not be completed automatically.
 *
 * Since: 2.0
 */
ModulemdModuleStream *
modulemd_module_stream_upgrade (ModulemdModuleStream *self,
                                guint64 mdversion,
                                GError **error);


/**
 * modulemd_module_stream_validate:
 * @self: (in): This #ModulemdModuleStream.
 * @error: (out): A #GError that will return the reason for a validation error.
 *
 * Verifies that all stored values are internally consistent and that the
 * module is sufficiently-complete for emitting. This function is called
 * implicitly before attempting to emit the contents.
 *
 * Returns: TRUE if the #ModulemdModuleStream passed validation. FALSE and sets
 * @error appropriately if validation fails.
 *
 * Since: 2.0
 */
gboolean
modulemd_module_stream_validate (ModulemdModuleStream *self, GError **error);


/**
 * modulemd_module_stream_get_mdversion:
 * @self: (in): This #ModulemdModuleStream.
 *
 * Returns: The metadata version of this #ModulemdModuleStream.
 *
 * Since: 2.0
 */
guint64
modulemd_module_stream_get_mdversion (ModulemdModuleStream *self);


/**
 * modulemd_module_stream_get_module_name:
 * @self: (in): This #ModulemdModuleStream.
 *
 * Returns: (transfer none): The name of the module.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_get_module_name (ModulemdModuleStream *self);


/**
 * modulemd_module_stream_get_stream_name:
 * @self: (in): This #ModulemdModuleStream.
 *
 * Returns: (transfer none): The name of this stream.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_get_stream_name (ModulemdModuleStream *self);


/**
 * modulemd_module_stream_set_version:
 * @self: (in): This #ModulemdModuleStream.
 * @version: (in): The version of this #ModulemdModuleStream
 *
 * Since: 2.0
 */
void
modulemd_module_stream_set_version (ModulemdModuleStream *self,
                                    guint64 version);


/**
 * modulemd_module_stream_get_version:
 * @self: (in): This #ModulemdModuleStream.
 *
 * Returns: The version of this #ModulemdModuleStream.
 */
guint64
modulemd_module_stream_get_version (ModulemdModuleStream *self);

/**
 * modulemd_module_stream_set_context:
 * @self: (in): This #ModulemdModuleStream.
 * @context: (in) (nullable): Module context flag.
 * The context flag serves to distinguish module builds with the same name,
 * stream and version and plays an important role in automatic module stream
 * name expansion.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_set_context (ModulemdModuleStream *self,
                                    const gchar *context);


/**
 * modulemd_module_stream_get_context:
 * @self: (in): This #ModulemdModuleStream.
 *
 * Returns: (transfer none): Module context flag.
 * The context flag serves to distinguish module builds with the same name,
 * stream and version and plays an important role in automatic module stream
 * name expansion.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_get_context (ModulemdModuleStream *self);


G_END_DECLS
