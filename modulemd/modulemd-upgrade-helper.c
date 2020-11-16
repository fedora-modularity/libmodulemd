/*
 * This file is part of libmodulemd
 * Copyright (C) 2020 Red Hat, Inc.
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */


#include "modulemd-upgrade-helper.h"
#include "private/modulemd-util.h"

struct _ModulemdUpgradeHelper
{
  GObject parent_instance;

  GHashTable *known_streams; /* <string, set<string>> */
};

G_DEFINE_TYPE (ModulemdUpgradeHelper, modulemd_upgrade_helper, G_TYPE_OBJECT)


ModulemdUpgradeHelper *
modulemd_upgrade_helper_new (void)
{
  return g_object_new (MODULEMD_TYPE_UPGRADE_HELPER, NULL);
}


static void
modulemd_upgrade_helper_finalize (GObject *object)
{
  ModulemdUpgradeHelper *self = (ModulemdUpgradeHelper *)object;

  g_clear_pointer (&self->known_streams, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_upgrade_helper_parent_class)->finalize (object);
}


static void
modulemd_upgrade_helper_class_init (ModulemdUpgradeHelperClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_upgrade_helper_finalize;
}


static void
modulemd_upgrade_helper_init (ModulemdUpgradeHelper *self)
{
  self->known_streams = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, modulemd_hash_table_unref);
}


void
modulemd_upgrade_helper_add_known_stream (ModulemdUpgradeHelper *self,
                                          const gchar *module_name,
                                          const gchar *stream_name)
{
  g_return_if_fail (MODULEMD_IS_UPGRADE_HELPER (self));

  GHashTable *known_module =
    g_hash_table_lookup (self->known_streams, module_name);
  if (NULL == known_module)
    {
      known_module = modulemd_str_set_new ();
      g_hash_table_insert (
        self->known_streams, g_strdup (module_name), known_module);
    }

  g_hash_table_add (known_module, g_strdup (stream_name));
}


GStrv
modulemd_upgrade_helper_get_known_modules_as_strv (ModulemdUpgradeHelper *self)
{
  g_return_val_if_fail (MODULEMD_IS_UPGRADE_HELPER (self), NULL);

  return modulemd_ordered_str_keys_as_strv (self->known_streams);
}


GStrv
modulemd_upgrade_helper_get_known_streams_as_strv (ModulemdUpgradeHelper *self,
                                                   const gchar *module_name)
{
  g_return_val_if_fail (MODULEMD_IS_UPGRADE_HELPER (self), NULL);

  GHashTable *known_module =
    g_hash_table_lookup (self->known_streams, module_name);

  if (NULL == known_module)
    return (GStrv)g_malloc0 (sizeof (gchar *));

  return modulemd_ordered_str_keys_as_strv (known_module);
}
