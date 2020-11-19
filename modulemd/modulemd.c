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

#include "modulemd.h"
#include "config.h"

const gchar *
modulemd_get_version (void)
{
  return LIBMODULEMD_VERSION;
}


static ModulemdModuleStreamVersionEnum default_mdversion =
  (ModulemdModuleStreamVersionEnum)DEFAULT_STREAM_MDVERSION;

void
modulemd_set_default_stream_mdversion (
  ModulemdModuleStreamVersionEnum mdversion)
{
  g_return_if_fail (mdversion >= MD_MODULESTREAM_VERSION_ONE &&
                    mdversion <= MD_MODULESTREAM_VERSION_LATEST);

  default_mdversion = mdversion;
}

ModulemdModuleStreamVersionEnum
modulemd_get_default_stream_mdversion (void)
{
  return default_mdversion;
}


static ModulemdModuleIndex *
verify_load (int ret,
             ModulemdModuleIndex *idx,
             GPtrArray *failures,
             GError **error,
             GError **nested_error);

ModulemdModuleIndex *
modulemd_load_file (const gchar *yaml_file, GError **error)
{
  gboolean ret;
  g_autoptr (ModulemdModuleIndex) idx = NULL;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (GPtrArray) failures = NULL;

  g_return_val_if_fail (yaml_file, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  failures = g_ptr_array_new_with_free_func (g_object_unref);
  idx = modulemd_module_index_new ();

  ret = modulemd_module_index_update_from_file (
    idx, yaml_file, FALSE, &failures, &nested_error);
  return verify_load (ret, idx, failures, error, &nested_error);
}


ModulemdModuleIndex *
modulemd_load_string (const gchar *yaml_string, GError **error)
{
  gboolean ret;
  g_autoptr (ModulemdModuleIndex) idx = NULL;
  g_autoptr (GError) nested_error = NULL;
  g_autoptr (GPtrArray) failures = NULL;

  g_return_val_if_fail (yaml_string, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  failures = g_ptr_array_new_with_free_func (g_object_unref);
  idx = modulemd_module_index_new ();

  ret = modulemd_module_index_update_from_string (
    idx, yaml_string, FALSE, &failures, &nested_error);
  return verify_load (ret, idx, failures, error, &nested_error);
}


static ModulemdModuleIndex *
verify_load (gboolean ret,
             ModulemdModuleIndex *idx,
             GPtrArray *failures,
             GError **error,
             GError **nested_error)
{
  ModulemdSubdocumentInfo *doc = NULL;

  if (!ret)
    {
      if (*nested_error)
        {
          g_propagate_error (error, g_steal_pointer (nested_error));
          return NULL;
        }
      else if (failures && failures->len)
        {
          g_debug ("%u YAML subdocuments were invalid", failures->len);
          for (gsize i = 0; i < failures->len; i++)
            {
              doc =
                MODULEMD_SUBDOCUMENT_INFO (g_ptr_array_index (failures, i));
              g_debug ("\nFailed subdocument (%s): \n%s\n",
                       modulemd_subdocument_info_get_gerror (doc)->message,
                       modulemd_subdocument_info_get_yaml (doc));
            }

          g_set_error (error,
                       MODULEMD_ERROR,
                       MMD_ERROR_VALIDATE,
                       "One or more YAML subdocuments were invalid.");
          return NULL;
        }

      /* Should never get here, must be a programming error */
      g_set_error (
        error, MODULEMD_ERROR, MMD_ERROR_VALIDATE, "Unknown internal error.");
      g_return_val_if_reached (NULL);
    }

  return g_object_ref (idx);
}
