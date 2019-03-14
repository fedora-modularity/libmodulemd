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

  /* Padding to allow adding up to 8 new virtual functions without
   * breaking ABI. */
  gpointer padding[8];
};

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
 * modulemd_component_set_builderder:
 * @self: This #ModulemdComponent object
 * @buildorder: The order this component should be built relative to others.
 *
 * Since: 2.0
 */
void
modulemd_component_set_buildorder (ModulemdComponent *self, gint64 builderder);


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
 * @rationale: The reason that this component is part of the stream.
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
