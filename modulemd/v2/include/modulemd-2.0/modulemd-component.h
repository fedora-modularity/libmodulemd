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
 * SECTION: modulemd-component
 * @title: Modulemd.Component
 * @stability: stable
 * @short_description: Pure virtual parent class for components that go into a module stream.
 */

#define MODULEMD_TYPE_COMPONENT (modulemd_component_get_type ())
G_DECLARE_DERIVABLE_TYPE (
  ModulemdComponent, modulemd_component, MODULEMD, COMPONENT, GObject)

struct _ModulemdComponentClass
{
  GObjectClass parent_class;

  ModulemdComponent *(*copy) (ModulemdComponent *self, const gchar *key);
  void (*set_name) (ModulemdComponent *self, const gchar *name);
  const gchar *(*get_name) (ModulemdComponent *self);
  gboolean (*validate) (ModulemdComponent *self, GError **error);
  gboolean (*equals) (ModulemdComponent *self_1, ModulemdComponent *self_2);

  /* Padding to allow adding up to 7 new virtual functions without
   * breaking ABI. */
  gpointer padding[7];
};

/**
 * modulemd_component_equals:
 * @self_1: A #ModulemdComponent object
 * @self_2: A #ModulemdComponent object
 *
 * Returns: TRUE, if both the objects are equal. FALSE, otherwise
 *
 * Since: 2.3
 */
gboolean
modulemd_component_equals (ModulemdComponent *self_1,
                           ModulemdComponent *self_2);


/**
 * modulemd_component_copy:
 * @self: This #ModulemdComponent object
 * @key: (in) (nullable): An optional new key for the copied component which is
 * used as the lookup key when this component is attached to a
 * #ModulemdModuleStream.
 *
 * Returns: (transfer full): A newly-allocated copy of @self
 *
 * Since: 2.0
 */
ModulemdComponent *
modulemd_component_copy (ModulemdComponent *self, const gchar *key);


/**
 * modulemd_component_validate:
 * @self: (in): This #ModulemdComponent.
 * @error: (out): A #GError that will return the reason for a validation error.
 *
 * Verifies that all stored values are internally consistent and that the
 * component is sufficiently-complete for emitting. This function is called
 * implicitly before attempting to emit the contents.
 *
 * Returns: TRUE if the #ModulemdComponent passed validation. FALSE and sets
 * @error appropriately if validation fails.
 *
 * Since: 2.2
 */
gboolean
modulemd_component_validate (ModulemdComponent *self, GError **error);


/**
 * modulemd_component_add_buildafter:
 * @self: This #ModulemdComponent object
 * @key: (in): A key representing another component in the
 * #ModulemdModuleStream components map.
 *
 * Add a build dependency of this component.
 *
 * Since: 2.2
 */
void
modulemd_component_add_buildafter (ModulemdComponent *self, const gchar *key);


/**
 * modulemd_component_clear_buildafter:
 * @self: This #ModulemdComponent object
 *
 * Remove all buildafter entries for this component.
 *
 * Since: 2.5
 */
void
modulemd_component_clear_buildafter (ModulemdComponent *self);


/**
 * modulemd_component_get_buildafter_as_strv: (rename-to modulemd_component_get_buildafter)
 * @self: This #ModulemdComponent object
 *
 * Returns: (transfer full): The set of component keys that this component
 * depends upon.
 *
 * Since: 2.2
 */
GStrv
modulemd_component_get_buildafter_as_strv (ModulemdComponent *self);


/**
 * modulemd_component_set_buildonly:
 * @self: This #ModulemdComponent object
 * @buildonly: Whether this component is used only for building this module. If
 * set to TRUE, the build system should add any artifacts produced by this
 * component to the data.filters section of the output modulemd.
 *
 * Since: 2.2
 */
void
modulemd_component_set_buildonly (ModulemdComponent *self, gboolean buildonly);


/**
 * modulemd_component_get_buildonly:
 * @self: This #ModulemdComponent object
 *
 * Returns: TRUE if this component is used only for building this module.
 *
 * Since: 2.2
 */
gboolean
modulemd_component_get_buildonly (ModulemdComponent *self);


/**
 * modulemd_component_set_buildorder:
 * @self: This #ModulemdComponent object
 * @buildorder: The order this component should be built relative to others.
 *
 * Since: 2.0
 */
void
modulemd_component_set_buildorder (ModulemdComponent *self, gint64 buildorder);


/**
 * modulemd_component_get_buildorder:
 * @self: This #ModulemdComponent object
 *
 * Returns: The value of the buildorder
 *
 * Since: 2.0
 */
gint64
modulemd_component_get_buildorder (ModulemdComponent *self);


/**
 * modulemd_component_set_name:
 * @self: This #ModulemdComponent object.
 * @name: (nullable): The name of this component. Note that this is different
 * from the key used to save this component to a #ModulemdModuleStream. If this
 * value is set, it adds a "name:" attribute to this component. This is used in
 * bootstrapping cases where the key is a different name used to differentiate
 * multiple ordered builds of the same component name. This function is
 * currently only implemented for #ModulemdComponentRpm and has no effect on
 * other #ModulemdComponent types.
 *
 * Since: 2.2
 */
void
modulemd_component_set_name (ModulemdComponent *self, const gchar *name);


/**
 * modulemd_component_get_name:
 * @self: This #ModulemdComponent object
 *
 * Returns: (transfer none): The name of the component. Note that this may be
 * different from the key used to save this component to a
 * #ModulemdModuleStream. If you specifically need the key, use
 * modulemd_component_get_key() instead.
 *
 * Since: 2.0
 */
const gchar *
modulemd_component_get_name (ModulemdComponent *self);


/**
 * modulemd_component_get_key:
 * @self: This #ModulemdComponent object
 *
 * Returns: (transfer none): The name of the key used to attach this component
 * to a #ModulemdModuleStream.
 *
 * Since: 2.2
 */
const gchar *
modulemd_component_get_key (ModulemdComponent *self);


/**
 * modulemd_component_set_rationale:
 * @self: This #ModulemdComponent object
 * @rationale: (in) (nullable): The reason that this component is part of the
 * stream.
 *
 * Since: 2.0
 */
void
modulemd_component_set_rationale (ModulemdComponent *self,
                                  const gchar *rationale);


/**
 * modulemd_component_get_rationale:
 * @self: This #ModulemdComponent object
 *
 * Returns: (transfer none): The rationale.
 *
 * Since: 2.0
 */
const gchar *
modulemd_component_get_rationale (ModulemdComponent *self);

G_END_DECLS
