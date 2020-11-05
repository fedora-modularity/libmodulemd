/*
 * This file is part of libmodulemd
 * Copyright (C) 2018-2020 Red Hat, Inc.
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
#include <glib.h>
#include <glib/gstdio.h>

#include "modulemd-deprecated.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-module-stream
 * @title: Modulemd.ModuleStream
 * @stability: stable
 * @short_description: A parent class for all versions of #ModulemdModuleStream
 * objects.
 */

/**
 * MMD_MAXCONTEXTLEN:
 *
 * The ModuleStream v3 specification defines the maximum lenth of the context
 * field.
 *
 * Since: 2.10
 */
#define MMD_MAXCONTEXTLEN 10

/* Forward declarations */
typedef struct _ModulemdModuleIndex ModulemdModuleIndex;

/**
 * ModulemdModuleStreamVersionEnum:
 * @MD_MODULESTREAM_VERSION_ERROR: Represents an error handling module stream
 * version.
 * @MD_MODULESTREAM_VERSION_UNSET: Represents an unset module stream version.
 * @MD_MODULESTREAM_VERSION_ONE: Represents v1 of the #ModulemdModuleStream
 * metadata format.
 * @MD_MODULESTREAM_VERSION_TWO: Represents v2 of the #ModulemdModuleStream
 * metadata format.
 * @MD_MODULESTREAM_VERSION_THREE: Represents v3 of the #ModulemdModuleStream
 * metadata format. Since: 2.10
 * @MD_MODULESTREAM_VERSION_LATEST: Represents the highest-supported version of
 * the #ModulemdModuleStream metadata format.
 *
 * Since: 2.0
 */
typedef enum
{
  MD_MODULESTREAM_VERSION_ERROR = -1,
  MD_MODULESTREAM_VERSION_UNSET = 0,

  MD_MODULESTREAM_VERSION_ONE = 1,
  MD_MODULESTREAM_VERSION_TWO = 2,
  MD_MODULESTREAM_VERSION_THREE = 3,

  MD_MODULESTREAM_VERSION_LATEST = MD_MODULESTREAM_VERSION_THREE
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

  gboolean (*depends_on_stream) (ModulemdModuleStream *self,
                                 const gchar *module_name,
                                 const gchar *stream_name);

  gboolean (*build_depends_on_stream) (ModulemdModuleStream *self,
                                       const gchar *module_name,
                                       const gchar *stream_name);

  gboolean (*equals) (ModulemdModuleStream *self_1,
                      ModulemdModuleStream *self_2);

  /* Padding to allow adding up to 7 new virtual functions without
   * breaking ABI. */
  gpointer padding[7];
};


/**
 * modulemd_module_stream_new:
 * @mdversion: (in): The metadata version of #ModulemdModuleStream to create.
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
 * modulemd_module_stream_read_file:
 * @path: (in): The path to a YAML document containing a module stream
 * definition.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @module_name: (in) (nullable): An optional module name to override the
 * document on disk. Mostly useful in cases where the name is being
 * auto-detected from git.
 * @module_stream: (in) (nullable): An optional module stream name to override
 * the document on disk. Mostly useful in cases where the name is being
 * auto-detected from git.
 * @error: (out): A #GError that will return the reason for a failed read.
 *
 * Create a #ModulemdModuleStream object from a YAML file.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStream object if
 * the YAML file was valid and contained exactly one `document: modulemd`
 * subdocument. NULL if the document fails validation or multiple documents are
 * encountered and sets @error appropriately. See #ModulemdModuleIndex for
 * functions to read in multiple-subdocument YAML.
 *
 * Since: 2.0
 */
ModulemdModuleStream *
modulemd_module_stream_read_file (const gchar *path,
                                  gboolean strict,
                                  const gchar *module_name,
                                  const gchar *module_stream,
                                  GError **error);


/**
 * modulemd_module_stream_read_string:
 * @yaml_string: (in): A YAML document string containing a module stream
 * definition.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @module_name: (in) (nullable): An optional module name to override the
 * document on disk. Mostly useful in cases where the name is being
 * auto-detected from git.
 * @module_stream: (in) (nullable): An optional module stream name to override
 * the document on disk. Mostly useful in cases where the name is being
 * auto-detected from git.
 * @error: (out): A #GError that will return the reason for a failed read.
 *
 * Create a #ModulemdModuleStream object from a YAML string.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStream object if
 * the YAML string was valid and contained exactly one `document: modulemd`
 * subdocument. NULL if the document fails validation or multiple documents are
 * encountered and sets @error appropriately. See #ModulemdModuleIndex for
 * functions to read in multiple-subdocument YAML.
 *
 * Since: 2.0
 */
ModulemdModuleStream *
modulemd_module_stream_read_string (const gchar *yaml_string,
                                    gboolean strict,
                                    const gchar *module_name,
                                    const gchar *module_stream,
                                    GError **error);


/**
 * modulemd_module_stream_read_stream: (skip)
 * @stream: (in): A YAML document as a FILE * containing a module stream
 * definition.
 * @strict: (in): Whether the parser should return failure if it encounters an
 * unknown mapping key or if it should ignore it.
 * @module_name: (in) (nullable): An optional module name to override the
 * document on disk. Mostly useful in cases where the name is being
 * auto-detected from git.
 * @module_stream: (in) (nullable): An optional module stream name to override
 * the document on disk. Mostly useful in cases where the name is being
 * auto-detected from git.
 * @error: (out): A #GError that will return the reason for a failed read.
 *
 * Create a #ModulemdModuleStream object from a YAML file.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleStream object if
 * the YAML file was valid and contained exactly one `document: modulemd`
 * subdocument. NULL if the document fails validation or multiple documents are
 * encountered and sets NULL appropriately. See #ModulemdModuleIndex for
 * functions to read in multiple-subdocument YAML.
 *
 * Since: 2.0
 */
ModulemdModuleStream *
modulemd_module_stream_read_stream (FILE *stream,
                                    gboolean strict,
                                    const gchar *module_name,
                                    const gchar *module_stream,
                                    GError **error);


/**
 * modulemd_module_stream_equals:
 * @self_1: (in): A #ModulemdModuleStream object.
 * @self_2: (in): A #ModulemdModuleStream object.
 *
 * Checks if @self_1 and @self_2 are identical objects.
 *
 * Returns: TRUE, If both objects are equal. FALSE, otherwise.
 *
 * Since: 2.3
 */
gboolean
modulemd_module_stream_equals (ModulemdModuleStream *self_1,
                               ModulemdModuleStream *self_2);


/**
 * modulemd_module_stream_copy:
 * @self: (in): This #ModulemdModuleStream object.
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
 * @self: (in): This #ModulemdModuleStream object.
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
 * modulemd_module_stream_upgrade_ext:
 * @self: (in): This #ModulemdModuleStream object.
 * @mdversion: (in): The metadata version to upgrade to. If zero, upgrades to
 * the highest-supported version.
 * @error: (out): A #GError that will return the reason for an upgrade error.
 *
 * Return an upgraded copy of this object. Does not modify the original.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdModuleIndex containing a
 * copy of this object upgraded to the requested version. Returns NULL and sets
 * @error appropriately if the upgrade could not be completed automatically.
 *
 * Since: 2.10
 */
ModulemdModuleIndex *
modulemd_module_stream_upgrade_ext (ModulemdModuleStream *self,
                                    guint64 mdversion,
                                    GError **error);

/**
 * modulemd_module_stream_validate:
 * @self: (in): This #ModulemdModuleStream object.
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
 * @self: (in): This #ModulemdModuleStream object.
 *
 * Returns: The metadata version of this #ModulemdModuleStream.
 *
 * Since: 2.0
 */
guint64
modulemd_module_stream_get_mdversion (ModulemdModuleStream *self);


/**
 * modulemd_module_stream_get_module_name:
 * @self: (in): This #ModulemdModuleStream object.
 *
 * Returns: (transfer none): The name of the module.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_get_module_name (ModulemdModuleStream *self);


/**
 * modulemd_module_stream_get_stream_name:
 * @self: (in): This #ModulemdModuleStream object.
 *
 * Returns: (transfer none): The name of this stream.
 *
 * Since: 2.0
 */
const gchar *
modulemd_module_stream_get_stream_name (ModulemdModuleStream *self);


/**
 * modulemd_module_stream_set_version:
 * @self: (in): This #ModulemdModuleStream object.
 * @version: (in): The version of this #ModulemdModuleStream.
 *
 * Since: 2.0
 */
void
modulemd_module_stream_set_version (ModulemdModuleStream *self,
                                    guint64 version);


/**
 * modulemd_module_stream_get_version:
 * @self: (in): This #ModulemdModuleStream object.
 *
 * Returns: The version of this #ModulemdModuleStream.
 */
guint64
modulemd_module_stream_get_version (ModulemdModuleStream *self);

/**
 * modulemd_module_stream_set_context:
 * @self: (in): This #ModulemdModuleStream object.
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
 * @self: (in): This #ModulemdModuleStream object.
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


/**
 * modulemd_module_stream_set_arch:
 * @self: (in): This #ModulemdModuleStream object.
 * @arch: (in) (nullable): Module architecture.
 * Indicates to which processor architecture this #ModulemdModuleStream
 * applies.
 *
 * Since: 2.2
 */
void
modulemd_module_stream_set_arch (ModulemdModuleStream *self,
                                 const gchar *arch);


/**
 * modulemd_module_stream_get_arch:
 * @self: (in): This #ModulemdModuleStream object.
 *
 * Returns: (transfer none): Module architecture object.
 * Indicates to which processor architecture this #ModulemdModuleStream
 * applies.
 *
 * Since: 2.2
 */
const gchar *
modulemd_module_stream_get_arch (ModulemdModuleStream *self);


/**
 * modulemd_module_stream_get_nsvc_as_string: (rename-to modulemd_module_stream_get_nsvc)
 * @self: (in): This #ModulemdModuleStream object.
 *
 * Retrieves a representation of the module name, stream name, version and
 * context of this #ModulemdModuleStream in the form
 * `module_name:stream_name:version:context`. Note that this excludes the
 * architecture of the module stream and as such is not guaranteed to be unique
 * within a repository.
 *
 * Returns: (transfer full): The NSVC (name:stream:version[:context]) of this
 * module stream. NULL if module name or stream stream is unknown.
 *
 * Since: 2.0
 */
gchar *
modulemd_module_stream_get_nsvc_as_string (ModulemdModuleStream *self);


/**
 * modulemd_module_stream_get_NSVCA_as_string: (rename-to modulemd_module_stream_get_NSVCA)
 * @self: (in): This #ModulemdModuleStream object.
 *
 * Returns: (transfer full): The NSVCA of this module stream. The returned
 * format is described
 * [here](https://docs.fedoraproject.org/en-US/modularity/architecture/consuming/naming-policy/#_forms).
 * NULL if module name or stream stream is unknown.
 *
 * Since: 2.2
 */
gchar *
modulemd_module_stream_get_NSVCA_as_string (ModulemdModuleStream *self);


/**
 * modulemd_module_stream_depends_on_stream:
 * @self: (not nullable): This #ModulemdModuleStream object.
 * @module_name: (not nullable): A module name.
 * @stream_name: (not nullable): The stream of the module.
 *
 * Returns: TRUE if any of the #ModulemdDependencies objects associated with
 * this module applies to the provided module name and stream in the runtime
 * dependencies.
 *
 * Since: 2.1
 *
 * Stability: unstable
 */
gboolean
modulemd_module_stream_depends_on_stream (ModulemdModuleStream *self,
                                          const gchar *module_name,
                                          const gchar *stream_name);


/**
 * modulemd_module_stream_build_depends_on_stream:
 * @self: (not nullable): This #ModulemdModuleStream object.
 * @module_name: (not nullable): A module name.
 * @stream_name: (not nullable): The stream of the module.
 *
 * Returns: TRUE if any of the #ModulemdDependencies objects associated with
 * this module applies to the provided module name and stream in the build-time
 * dependencies.
 *
 * Since: 2.1
 *
 * Stability: unstable
 */
gboolean
modulemd_module_stream_build_depends_on_stream (ModulemdModuleStream *self,
                                                const gchar *module_name,
                                                const gchar *stream_name);
G_END_DECLS
