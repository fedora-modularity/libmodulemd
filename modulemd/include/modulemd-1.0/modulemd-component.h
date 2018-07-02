/*
 * This file is part of libmodulemd
 * Copyright (C) 2017-2018 Stephen Gallagher
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */

#ifndef MODULEMD_COMPONENT_H
#define MODULEMD_COMPONENT_H

#include "modulemd.h"

G_BEGIN_DECLS

/**
 * SECTION: modulemd-component
 * @title: Modulemd.Component
 * @short_description: Pure virtual parent class for components that go into a
 * module stream. See #ModulemdComponentRPM and #ModulemdComponentModule for
 * specific types.
 */

#define MODULEMD_TYPE_COMPONENT modulemd_component_get_type ()
G_DECLARE_DERIVABLE_TYPE (
  ModulemdComponent, modulemd_component, MODULEMD, COMPONENT, GObject)

struct _ModulemdComponentClass
{
  GObjectClass parent_instance;

  /* Virtual Public Members */
  void (*set_buildorder) (ModulemdComponent *self, guint64 buildorder);
  guint64 (*peek_buildorder) (ModulemdComponent *self);

  void (*set_name) (ModulemdComponent *self, const gchar *name);
  const gchar *(*peek_name) (ModulemdComponent *self);

  void (*set_rationale) (ModulemdComponent *self, const gchar *rationale);
  const gchar *(*peek_rationale) (ModulemdComponent *self);

  gchar *(*dup_name) (ModulemdComponent *self);
  gchar *(*dup_rationale) (ModulemdComponent *self);

  /* Pure Virtual Public Members */
  ModulemdComponent *(*copy) (ModulemdComponent *self);

  /* Padding to allow adding up to 11 new virtual functions without
   * breaking ABI. */
  gpointer padding[9];
};


/**
 * modulemd_component_new:
 *
 * Allocates a new #ModulemdComponent
 *
 * Return value: a new #ModulemdComponent
 *
 * Since: 1.0
 */
ModulemdComponent *
modulemd_component_new (void);


/**
 * modulemd_component_set_buildorder:
 * @buildorder: The order to build this component
 *
 * Sets the 'buildorder' property.
 *
 * Since: 1.0
 */
void
modulemd_component_set_buildorder (ModulemdComponent *self,
                                   guint64 buildorder);


/**
 * modulemd_component_get_buildorder:
 *
 * Returns the 'buildorder' property
 *
 * Deprecated: 1.1
 * Use peek_buildorder() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_component_peek_buildorder)
guint64
modulemd_component_get_buildorder (ModulemdComponent *self);


/**
 * modulemd_component_peek_buildorder:
 *
 * Returns the 'buildorder' property
 *
 * Since: 1.1
 */
guint64
modulemd_component_peek_buildorder (ModulemdComponent *self);


/**
 * modulemd_component_set_name:
 * @name: The name of the component
 *
 * Sets the 'name' property.
 *
 * Since: 1.0
 */
void
modulemd_component_set_name (ModulemdComponent *self, const gchar *name);


/**
 * modulemd_component_get_name:
 *
 * Returns the 'name' property;
 *
 * Deprecated: 1.1
 * Use peek_name() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_component_peek_name)
const gchar *
modulemd_component_get_name (ModulemdComponent *self);


/**
 * modulemd_component_peek_name:
 *
 * Returns the 'name' property;
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_peek_name (ModulemdComponent *self);


/**
 * modulemd_component_dup_name:
 *
 * Returns a copy of the 'name' property;
 *
 * Since: 1.1
 */
gchar *
modulemd_component_dup_name (ModulemdComponent *self);


/**
 * modulemd_component_set_rationale:
 * @rationale: The rationale for including this component
 *
 * Sets the 'rationale' property.
 *
 * Since: 1.0
 */
void
modulemd_component_set_rationale (ModulemdComponent *self,
                                  const gchar *rationale);


/**
 * modulemd_component_get_rationale:
 *
 * Returns the 'rationale' property;
 *
 * Deprecated: 1.1
 * Use peek_rationale() instead.
 *
 * Since: 1.0
 */
MMD_DEPRECATED_FOR (modulemd_component_peek_rationale)
const gchar *
modulemd_component_get_rationale (ModulemdComponent *self);


/**
 * modulemd_component_peek_rationale:
 *
 * Returns the 'rationale' property;
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_peek_rationale (ModulemdComponent *self);


/**
 * modulemd_component_dup_rationale:
 *
 * Returns a copy of the 'rationale' property;
 *
 * Since: 1.1
 */
gchar *
modulemd_component_dup_rationale (ModulemdComponent *self);


/**
 * modulemd_component_copy:
 *
 * Returns a complete copy of this Component.
 *
 * Returns: (transfer full): A copy of this Component.
 *
 * Since: 1.1
 */
ModulemdComponent *
modulemd_component_copy (ModulemdComponent *self);

G_END_DECLS

#endif /* MODULEMD_COMPONENT_H */
