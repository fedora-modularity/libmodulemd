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

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * SECTION: modulemd-rpm-map-entry
 * @title: Modulemd.RpmMapEntry
 * @stability: stable
 * @short_description: Provides an exploded and unexploded view of the NEVRA of
 * an RPM artifact.
 */

#define MODULEMD_TYPE_RPM_MAP_ENTRY (modulemd_rpm_map_entry_get_type ())

G_DECLARE_FINAL_TYPE (ModulemdRpmMapEntry,
                      modulemd_rpm_map_entry,
                      MODULEMD,
                      RPM_MAP_ENTRY,
                      GObject)

/**
 * modulemd_rpm_map_entry_new:
 * @name: The name of the package
 * @epoch: The epoch of the package
 * @version: The version of the package
 * @release: The release string of the package
 * @arch: The processor architecture of the package
 *
 * Create a new rpm-map entry. This will contain the decomposed NEVRA of an
 * RPM artifact.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdRpmMapEntry with the
 * provided values.
 */
ModulemdRpmMapEntry *
modulemd_rpm_map_entry_new (const gchar *name,
                            guint64 epoch,
                            const gchar *version,
                            const gchar *release,
                            const gchar *arch);


/**
 * modulemd_rpm_map_entry_copy:
 * @self: This #ModulemdRpmMapEntry
 *
 * Returns: (transfer full): A deep copy of this #ModulemdRpmMapEntry object.
 *
 * Since: 2.2
 */
ModulemdRpmMapEntry *
modulemd_rpm_map_entry_copy (ModulemdRpmMapEntry *self);


/**
 * modulemd_rpm_map_entry_equals_wrapper:
 * @a: const void pointer
 * @b: const void pointer
 *
 * Returns: TRUE if the two entries contain equivalent data. FALSE if they
 * differ.
 *
 * Since: 2.5
 */
gboolean
modulemd_rpm_map_entry_equals_wrapper (const void *a, const void *b);


/**
 * modulemd_RpmMapEntry_hash_table_equals_wrapper:
 * @a: const void pointer
 * @b: const void pointer
 *
 * Returns: TRUE if the two entries contain equivalent data. FALSE if they
 * differ.
 *
 * Since: 2.5
 */
gboolean
modulemd_RpmMapEntry_hash_table_equals_wrapper (const void *a, const void *b);


/**
 * modulemd_rpm_map_entry_equals:
 * @self: A #ModuleRpmMapEntry
 * @other: Another #ModulemdRpmMapEntry
 *
 * Returns: TRUE if the two entries contain equivalent data. FALSE if they
 * differ.
 *
 * Since: 2.2
 */
gboolean
modulemd_rpm_map_entry_equals (ModulemdRpmMapEntry *self,
                               ModulemdRpmMapEntry *other);


/**
 * modulemd_rpm_map_entry_validate:
 * @self: This #ModulemdRpmMapEntry
 * @error: (out): A GError containign the reason the object failed validation.
 * NULL if the validation passed.
 *
 * Since: 2.2
 */
gboolean
modulemd_rpm_map_entry_validate (ModulemdRpmMapEntry *self, GError **error);


/**
 * modulemd_rpm_map_entry_set_name:
 * @self: This #ModulemdRpmMapEntry
 * @name: The package name of this RPM
 *
 * Since: 2.2
 */
void
modulemd_rpm_map_entry_set_name (ModulemdRpmMapEntry *self, const gchar *name);


/**
 * modulemd_rpm_map_entry_get_name:
 * @self: This #ModulemdRpmMapEntry
 *
 * Returns: (transfer none): The package name of this RPM
 *
 * Since: 2.2
 */
const gchar *
modulemd_rpm_map_entry_get_name (ModulemdRpmMapEntry *self);


/**
 * modulemd_rpm_map_entry_set_epoch:
 * @self: This #ModulemdRpmMapEntry
 * @epoch: The package epoch of this RPM
 *
 * Since: 2.2
 */
void
modulemd_rpm_map_entry_set_epoch (ModulemdRpmMapEntry *self, guint64 epoch);


/**
 * modulemd_rpm_map_entry_get_epoch:
 * @self: This #ModulemdRpmMapEntry
 *
 * Returns: The package epoch of this RPM
 *
 * Since: 2.2
 */
guint64
modulemd_rpm_map_entry_get_epoch (ModulemdRpmMapEntry *self);


/**
 * modulemd_rpm_map_entry_set_version:
 * @self: This #ModulemdRpmMapEntry
 * @version: The package version of this RPM
 *
 * Since: 2.2
 */
void
modulemd_rpm_map_entry_set_version (ModulemdRpmMapEntry *self,
                                    const gchar *version);


/**
 * modulemd_rpm_map_entry_get_version:
 * @self: This #ModulemdRpmMapEntry
 *
 * Returns: (transfer none): The package version of this RPM
 *
 * Since: 2.2
 */
const gchar *
modulemd_rpm_map_entry_get_version (ModulemdRpmMapEntry *self);


/**
 * modulemd_rpm_map_entry_set_release:
 * @self: This #ModulemdRpmMapEntry
 * @release: The package release string of this RPM
 *
 * Since: 2.2
 */
void
modulemd_rpm_map_entry_set_release (ModulemdRpmMapEntry *self,
                                    const gchar *release);


/**
 * modulemd_rpm_map_entry_get_release:
 * @self: This #ModulemdRpmMapEntry
 *
 * Returns: (transfer none): The package release of this RPM
 *
 * Since: 2.2
 */
const gchar *
modulemd_rpm_map_entry_get_release (ModulemdRpmMapEntry *self);


/**
 * modulemd_rpm_map_entry_set_arch:
 * @self: This #ModulemdRpmMapEntry
 * @arch: The package architecture of this RPM
 *
 * Since: 2.2
 */
void
modulemd_rpm_map_entry_set_arch (ModulemdRpmMapEntry *self, const gchar *arch);


/**
 * modulemd_rpm_map_entry_get_arch:
 * @self: This #ModulemdRpmMapEntry
 *
 * Returns: (transfer none): The package architecture of this RPM
 *
 * Since: 2.2
 */
const gchar *
modulemd_rpm_map_entry_get_arch (ModulemdRpmMapEntry *self);


/**
 * modulemd_rpm_map_entry_get_nevra_as_string: (rename-to modulemd_rpm_map_entry_get_nevra)
 * @self: This #ModulemdRpmMapEntry
 *
 * Returns: (transfer full): A newly-allocated string containing the complete
 * N-E:V-R.A constructed from the component parts. NULL if any field is
 * missing.
 *
 * Since: 2.2
 */
gchar *
modulemd_rpm_map_entry_get_nevra_as_string (ModulemdRpmMapEntry *self);


G_END_DECLS
