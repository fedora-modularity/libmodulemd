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

ModulemdComponent *
modulemd_component_new (void);

void
modulemd_component_set_buildorder (ModulemdComponent *self,
                                   guint64 buildorder);

MMD_DEPRECATED_FOR (modulemd_component_peek_buildorder)
guint64
modulemd_component_get_buildorder (ModulemdComponent *self);

guint64
modulemd_component_peek_buildorder (ModulemdComponent *self);

void
modulemd_component_set_name (ModulemdComponent *self, const gchar *name);

MMD_DEPRECATED_FOR (modulemd_component_peek_name)
const gchar *
modulemd_component_get_name (ModulemdComponent *self);

const gchar *
modulemd_component_peek_name (ModulemdComponent *self);
gchar *
modulemd_component_dup_name (ModulemdComponent *self);

void
modulemd_component_set_rationale (ModulemdComponent *self,
                                  const gchar *rationale);

MMD_DEPRECATED_FOR (modulemd_component_peek_rationale)
const gchar *
modulemd_component_get_rationale (ModulemdComponent *self);

const gchar *
modulemd_component_peek_rationale (ModulemdComponent *self);
gchar *
modulemd_component_dup_rationale (ModulemdComponent *self);

ModulemdComponent *
modulemd_component_copy (ModulemdComponent *self);

G_END_DECLS

#endif /* MODULEMD_COMPONENT_H */
