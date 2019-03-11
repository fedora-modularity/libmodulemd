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

  ModulemdComponent *(*copy) (ModulemdComponent *self, const gchar *name);

  gboolean (*equals) (ModulemdComponent *self, ModulemdComponent *other);

  /* Padding to allow adding up to 9 new virtual functions without
   * breaking ABI. */
  gpointer padding[9];
};

/**
 * modulemd_component_copy:
 * @self: This #ModulemdComponent object
 * @name: (in) (nullable): An optional new name for the copied component
 *
 * Returns: (transfer full): A newly-allocated copy of @self
 *
 * Since: 2.0
 */
ModulemdComponent *
modulemd_component_copy (ModulemdComponent *self, const gchar *name);


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
 * modulemd_component_get_name:
 * @self: This #ModulemdComponent object
 *
 * Returns: (transfer none): The name of the component.
 *
 * Since: 2.0
 */
const gchar *
modulemd_component_get_name (ModulemdComponent *self);


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


/**
 * modulemd_component_equals:
 * @self: This #ModulemdComponent.
 * @other: The other #ModulemdComponent being compared.
 *
 * Returns: TRUE if all of the objects and varibales composing the two
 * ModulemdComponents are equal. False, otherwise. 
 *
 * Since: 2.1
 */
gboolean
modulemd_component_equals (ModulemdComponent *self, ModulemdComponent *other);

G_END_DECLS
