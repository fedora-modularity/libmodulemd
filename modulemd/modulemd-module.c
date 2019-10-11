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

#include <glib.h>
#include <yaml.h>

#include "modulemd-errors.h"
#include "modulemd-module.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-private.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-translation-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdModule
{
  GObject parent_instance;

  gchar *module_name;

  GPtrArray *streams;
  ModulemdDefaults *defaults;
  GHashTable *translations;
};

G_DEFINE_TYPE (ModulemdModule, modulemd_module, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_MODULE_NAME,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdModule *
modulemd_module_new (const gchar *module_name)
{
  return g_object_new (MODULEMD_TYPE_MODULE, "module-name", module_name, NULL);
}


ModulemdModule *
modulemd_module_copy (ModulemdModule *self)
{
  g_autoptr (ModulemdModule) m = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);
  gsize i = 0;

  m = modulemd_module_new (modulemd_module_get_module_name (self));
  m->defaults = modulemd_defaults_copy (self->defaults);

  for (i = 0; i < self->streams->len; i++)
    {
      g_ptr_array_add (m->streams, g_ptr_array_index (self->streams, i));
    }

  return g_steal_pointer (&m);
}


gboolean
modulemd_module_validate (ModulemdModule *self, GError **error)
{
  /* No validation yet */
  return TRUE;
}


static void
modulemd_module_finalize (GObject *object)
{
  ModulemdModule *self = (ModulemdModule *)object;

  g_clear_pointer (&self->module_name, g_free);
  g_clear_object (&self->defaults);
  g_clear_pointer (&self->streams, g_ptr_array_unref);
  g_clear_pointer (&self->translations, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_module_parent_class)->finalize (object);
}


static void
modulemd_module_set_module_name (ModulemdModule *self,
                                 const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (module_name);

  g_clear_pointer (&self->module_name, g_free);
  self->module_name = g_strdup (module_name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_NAME]);
}


const gchar *
modulemd_module_get_module_name (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->module_name;
}


static void
modulemd_module_get_property (GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
  ModulemdModule *self = MODULEMD_MODULE (object);

  switch (prop_id)
    {
    case PROP_MODULE_NAME:
      g_value_set_string (value, modulemd_module_get_module_name (self));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_module_set_property (GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
  ModulemdModule *self = MODULEMD_MODULE (object);

  switch (prop_id)
    {
    case PROP_MODULE_NAME:
      modulemd_module_set_module_name (self, g_value_get_string (value));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_module_class_init (ModulemdModuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_module_finalize;
  object_class->get_property = modulemd_module_get_property;
  object_class->set_property = modulemd_module_set_property;

  properties[PROP_MODULE_NAME] = g_param_spec_string (
    "module-name",
    "Module name",
    "The name of this module.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_module_init (ModulemdModule *self)
{
  self->streams = g_ptr_array_new_full (0, g_object_unref);
  self->translations =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}


ModulemdDefaultsVersionEnum
modulemd_module_set_defaults (ModulemdModule *self,
                              ModulemdDefaults *defaults,
                              ModulemdDefaultsVersionEnum index_mdversion,
                              GError **error)
{
  g_autoptr (ModulemdDefaults) upgraded_defaults = NULL;
  g_autoptr (GError) nested_error = NULL;
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), MD_DEFAULTS_VERSION_ERROR);

  g_clear_object (&self->defaults);
  if (defaults == NULL)
    {
      /* If we are empty here, return MD_DEFAULTS_VERSION_UNSET so the
       * function reports success and does not influence further upgrades.
       */
      return MD_DEFAULTS_VERSION_UNSET;
    }

  /* We should never get a defaults object added whose module name doesn't
   * match.
   */
  if (g_strcmp0 (modulemd_defaults_get_module_name (defaults),
                 modulemd_module_get_module_name (self)) != 0)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_VALIDATE,
                   "Attempted to add defaults for module '%s' to module '%s'",
                   modulemd_defaults_get_module_name (defaults),
                   modulemd_module_get_module_name (self));
      return MD_DEFAULTS_VERSION_ERROR;
    }

  /* For Modulemd.ModuleIndex, we always want all entries to have the same
   * version, so that merges can be performed. If this Defaults object has a
   * lower mdversion than the Index, upgrade it to that version.
   *
   * We only call this if the mdversion is definitely lower, because the
   * upgrade() routine is not designed to handle downgrades.
   */
  if (modulemd_defaults_get_mdversion (defaults) < index_mdversion)
    {
      upgraded_defaults =
        modulemd_defaults_upgrade (defaults, index_mdversion, &nested_error);
      if (!upgraded_defaults)
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return MD_DEFAULTS_VERSION_ERROR;
        }
    }
  else
    {
      /* The new defaults were of the same or a higher version, so just copy it
       * and return that. The ModulemdModuleIndex will handle upgrading other
       * Defaults in the index to match.
       */
      upgraded_defaults = modulemd_defaults_copy (defaults);
    }

  /* Return the mdversion we saved so that the Index can check to see if we
   * need to upgrade other modules to match.
   */
  self->defaults = g_steal_pointer (&upgraded_defaults);
  return modulemd_defaults_get_mdversion (self->defaults);
}


ModulemdDefaults *
modulemd_module_get_defaults (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->defaults;
}


ModulemdModuleStreamVersionEnum
modulemd_module_add_stream (ModulemdModule *self,
                            ModulemdModuleStream *stream,
                            ModulemdModuleStreamVersionEnum index_mdversion,
                            GError **error)
{
  ModulemdModuleStream *old = NULL;
  ModulemdTranslation *translation = NULL;
  ModulemdModuleStream *newstream = NULL;
  g_autoptr (GError) nested_error = NULL;
  const gchar *module_name = NULL;
  const gchar *stream_name = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self),
                        MD_MODULESTREAM_VERSION_ERROR);
  g_return_val_if_fail (MODULEMD_IS_MODULE_STREAM (stream),
                        MD_MODULESTREAM_VERSION_ERROR);

  module_name = modulemd_module_stream_get_module_name (stream);
  stream_name = modulemd_module_stream_get_stream_name (stream);

  if (!module_name)
    {
      g_set_error (
        error,
        MODULEMD_ERROR,
        MODULEMD_ERROR_VALIDATE,
        "Attempted to add stream with no module name to module '%s'",
        modulemd_module_get_module_name (self));
      return MD_MODULESTREAM_VERSION_ERROR;
    }

  if (!stream_name)
    {
      g_set_error (
        error,
        MODULEMD_ERROR,
        MODULEMD_ERROR_VALIDATE,
        "Attempted to add stream with no stream name to module '%s'",
        modulemd_module_get_module_name (self));
      return MD_MODULESTREAM_VERSION_ERROR;
    }

  /* We should never get a stream object added whose module name doesn't
   * match.
   */
  if (g_strcmp0 (module_name, modulemd_module_get_module_name (self)) != 0)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_VALIDATE,
                   "Attempted to add stream for module '%s' to module '%s'",
                   module_name,
                   modulemd_module_get_module_name (self));
      return MD_MODULESTREAM_VERSION_ERROR;
    }

  old = modulemd_module_get_stream_by_NSVCA (
    self,
    modulemd_module_stream_get_stream_name (stream),
    modulemd_module_stream_get_version (stream),
    modulemd_module_stream_get_context (stream),
    modulemd_module_stream_get_arch (stream),
    &nested_error);

  if (old != NULL)
    {
      /* We're probably deduplicating content here, so remove the old one in
       * favor of the new one.
       */

      if (!modulemd_module_stream_equals (old, stream))
        {
          /* The two streams have matching NSVCA, but differ in content */
          g_set_error (error,
                       MODULEMD_ERROR,
                       MODULEMD_ERROR_VALIDATE,
                       "Encountered two streams with matching NSVCA %s but "
                       "differing content",
                       modulemd_module_stream_get_NSVCA_as_string (stream));
          return MD_MODULESTREAM_VERSION_ERROR;
        }

      /* First, drop the existing stream */
      g_ptr_array_remove (self->streams, old);
      old = NULL;
    }
  else if (old == NULL && g_error_matches (nested_error,
                                           MODULEMD_ERROR,
                                           MODULEMD_ERROR_TOO_MANY_MATCHES))
    {
      /* It should be impossible to get more than one error back here */
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return MD_MODULESTREAM_VERSION_ERROR;
    }
  g_clear_error (&nested_error);

  if (modulemd_module_stream_get_mdversion (stream) < index_mdversion)
    {
      /* If the stream we were passed is of a lower version than the index has
       * seen before, upgrade it to the index version.
       *
       * We only call this if the mdversion is definitely lower, because the
       * upgrade() routine is not designed to handle downgrades.
       */
      newstream = modulemd_module_stream_upgrade (
        stream, index_mdversion, &nested_error);
      if (!newstream)
        {
          g_propagate_error (error, g_steal_pointer (&nested_error));
          return MD_MODULESTREAM_VERSION_ERROR;
        }
    }
  else
    {
      newstream = modulemd_module_stream_copy (stream, NULL, NULL);
    }

  g_ptr_array_add (self->streams, newstream);

  translation = g_hash_table_lookup (
    self->translations, modulemd_module_stream_get_stream_name (stream));
  if (translation != NULL)
    {
      modulemd_module_stream_associate_translation (newstream, translation);
    }

  return modulemd_module_stream_get_mdversion (newstream);
}


GStrv
modulemd_module_get_stream_names_as_strv (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  if (!self->streams)
    {
      return NULL;
    }

  g_autoptr (GHashTable) stream_names =
    g_hash_table_new (g_str_hash, g_str_equal);

  for (guint i = 0; i < self->streams->len; i++)
    {
      g_hash_table_add (stream_names,
                        (void *)modulemd_module_stream_get_stream_name (
                          g_ptr_array_index (self->streams, i)));
    }

  return modulemd_ordered_str_keys_as_strv (stream_names);
}


GPtrArray *
modulemd_module_get_all_streams (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->streams;
}


static gint
compare_streams (gconstpointer a, gconstpointer b)
{
  int cmp = 0;
  guint64 a_ver;
  guint64 b_ver;
  ModulemdModuleStream *a_ = *(ModulemdModuleStream **)a;
  ModulemdModuleStream *b_ = *(ModulemdModuleStream **)b;

  /* Sort alphabetically by stream name */
  cmp = g_strcmp0 (modulemd_module_stream_get_stream_name (a_),
                   modulemd_module_stream_get_stream_name (b_));
  if (cmp != 0)
    {
      return cmp;
    }

  /* Sort by the version, highest first */
  a_ver = modulemd_module_stream_get_version (a_);
  b_ver = modulemd_module_stream_get_version (b_);
  if (b_ver > a_ver)
    {
      return 1;
    }
  if (a_ver > b_ver)
    {
      return -1;
    }

  /* Sort alphabetically by context */
  cmp = g_strcmp0 (modulemd_module_stream_get_context (a_),
                   modulemd_module_stream_get_context (b_));
  if (cmp != 0)
    {
      return cmp;
    }

  /* Sort alphabetically by architecture */
  cmp = g_strcmp0 (modulemd_module_stream_get_arch (a_),
                   modulemd_module_stream_get_arch (b_));

  return cmp;
}


GPtrArray *
modulemd_module_get_streams_by_stream_name_as_list (ModulemdModule *self,
                                                    const gchar *stream_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return modulemd_module_search_streams (self, stream_name, 0, NULL, NULL);
}


ModulemdModuleStream *
modulemd_module_get_stream_by_NSVC (ModulemdModule *self,
                                    const gchar *stream_name,
                                    const guint64 version,
                                    const gchar *context)
{
  return modulemd_module_get_stream_by_NSVCA (
    self, stream_name, version, context, NULL, NULL);
}


GPtrArray *
modulemd_module_search_streams (ModulemdModule *self,
                                const gchar *stream_name,
                                const guint64 version,
                                const gchar *context,
                                const gchar *arch)
{
  gsize i = 0;
  g_autoptr (GPtrArray) matching_streams = NULL;
  ModulemdModuleStream *under_consideration = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  /* Assume the worst-case scenario that all streams match to spare us extra
   * mallocs.
   */
  matching_streams = g_ptr_array_sized_new (self->streams->len);

  for (i = 0; i < self->streams->len; i++)
    {
      under_consideration =
        (ModulemdModuleStream *)g_ptr_array_index (self->streams, i);

      /* Skip this one unless the stream name matches */
      if (g_strcmp0 (
            modulemd_module_stream_get_stream_name (under_consideration),
            stream_name) != 0)
        {
          continue;
        }

      /* Skip this one unless the stream version matches OR the version is zero
       * which indicates that it shouldn't prevent the other cases from
       * matching.
       */
      if (version &&
          modulemd_module_stream_get_version (under_consideration) != version)
        {
          continue;
        }

      if (context &&
          g_strcmp0 (modulemd_module_stream_get_context (under_consideration),
                     context) != 0)
        {
          continue;
        }

      if (arch &&
          g_strcmp0 (modulemd_module_stream_get_arch (under_consideration),
                     arch) != 0)
        {
          continue;
        }

      g_ptr_array_add (matching_streams, under_consideration);
    }

  g_ptr_array_sort (matching_streams, compare_streams);

  return g_steal_pointer (&matching_streams);
}


ModulemdModuleStream *
modulemd_module_get_stream_by_NSVCA (ModulemdModule *self,
                                     const gchar *stream_name,
                                     const guint64 version,
                                     const gchar *context,
                                     const gchar *arch,
                                     GError **error)
{
  g_autoptr (GPtrArray) matching_streams = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  matching_streams =
    modulemd_module_search_streams (self, stream_name, version, context, arch);

  if (matching_streams->len == 0)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_NO_MATCHES,
                   "No streams matched");
      return NULL;
    }
  if (matching_streams->len > 1)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MODULEMD_ERROR_TOO_MANY_MATCHES,
                   "Multiple ModulemdModuleStreams matched");
      return NULL;
    }

  /* Exactly one result, so return it */
  return g_ptr_array_index (matching_streams, 0);
}


typedef struct _modulemd_nsvca
{
  const gchar *stream_name;
  guint64 version;
  const gchar *context;
  const gchar *arch;
} modulemd_nsvca;


static void
modulemd_nsvca_free (gpointer nsvca)
{
  /* All of the fields are just pointers to static data,
   * so nothing to free here.
   */

  g_free ((modulemd_nsvca *)nsvca);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (modulemd_nsvca, modulemd_nsvca_free);


static gboolean
match_nsvca (gconstpointer haystraw, gconstpointer needle)
{
  ModulemdModuleStream *stream = (ModulemdModuleStream *)haystraw;
  modulemd_nsvca *nsvca = (modulemd_nsvca *)needle;

  if (!g_str_equal (nsvca->stream_name,
                    modulemd_module_stream_get_stream_name (stream)))
    {
      return FALSE;
    }

  if (nsvca->version)
    {
      if (nsvca->version != modulemd_module_stream_get_version (stream))
        {
          return FALSE;
        }
    }

  if (nsvca->context)
    {
      if (!g_str_equal (nsvca->context,
                        modulemd_module_stream_get_context (stream)))
        {
          return FALSE;
        }
    }

  if (nsvca->arch)
    {
      if (!g_str_equal (nsvca->arch, modulemd_module_stream_get_arch (stream)))
        {
          return FALSE;
        }
    }

  return TRUE;
}


void
modulemd_module_remove_streams_by_NSVCA (ModulemdModule *self,
                                         const gchar *stream_name,
                                         const guint64 version,
                                         const gchar *context,
                                         const gchar *arch)
{
  gboolean found = FALSE;
  guint index;
  g_autoptr (modulemd_nsvca) nsvca = g_malloc0_n (1, sizeof (modulemd_nsvca));

  nsvca->stream_name = stream_name;
  nsvca->version = version;
  nsvca->context = context;
  nsvca->arch = arch;

  /* Iterate through the streams and remove any that match the requested
   * parameters
   */
  do
    {
      found = g_ptr_array_find_with_equal_func (
        self->streams, nsvca, match_nsvca, &index);
      if (found)
        {
          g_ptr_array_remove_index (self->streams, index);
        }
    }
  while (found);
}


void
modulemd_module_add_translation (ModulemdModule *self,
                                 ModulemdTranslation *translation)
{
  gsize i;
  ModulemdModuleStream *stream = NULL;
  ModulemdTranslation *newtrans = NULL;

  g_return_if_fail (
    g_str_equal (modulemd_translation_get_module_name (translation),
                 modulemd_module_get_module_name (self)));

  newtrans = modulemd_translation_copy (translation);

  g_hash_table_replace (
    self->translations,
    g_strdup (modulemd_translation_get_module_stream (translation)),
    newtrans);

  for (i = 0; i < self->streams->len; i++)
    {
      stream = (ModulemdModuleStream *)g_ptr_array_index (self->streams, i);

      if (!g_str_equal (modulemd_translation_get_module_stream (newtrans),
                        modulemd_module_stream_get_stream_name (stream)))
        {
          continue;
        }

      modulemd_module_stream_associate_translation (stream, newtrans);
    }
}


GPtrArray *
modulemd_module_get_translated_streams (ModulemdModule *self)
{
  return modulemd_ordered_str_keys (self->translations, modulemd_strcmp_sort);
}


ModulemdTranslation *
modulemd_module_get_translation (ModulemdModule *self, const gchar *stream)
{
  return g_hash_table_lookup (self->translations, stream);
}


gboolean
modulemd_module_upgrade_streams (ModulemdModule *self,
                                 ModulemdModuleStreamVersionEnum mdversion,
                                 GError **error)
{
  g_autoptr (GPtrArray) new_streams = NULL;
  ModulemdModuleStreamVersionEnum current_mdversion;
  g_autoptr (ModulemdModuleStream) modulestream = NULL;
  g_autoptr (ModulemdModuleStream) upgraded_stream = NULL;
  g_autofree gchar *nsvca = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self), FALSE);

  new_streams = g_ptr_array_new_full (self->streams->len, g_object_unref);

  for (guint i = 0; i < self->streams->len; i++)
    {
      modulestream = g_object_ref (
        MODULEMD_MODULE_STREAM (g_ptr_array_index (self->streams, i)));
      current_mdversion = modulemd_module_stream_get_mdversion (modulestream);
      nsvca = modulemd_module_stream_get_NSVCA_as_string (modulestream);

      if (current_mdversion <= MD_MODULESTREAM_VERSION_UNSET)
        {
          g_set_error (error,
                       MODULEMD_ERROR,
                       MODULEMD_ERROR_VALIDATE,
                       "ModuleStream %s had invalid mdversion %i",
                       nsvca,
                       current_mdversion);
          return FALSE;
        }
      if (current_mdversion == mdversion)
        {
          /* Already at the right version, so just add it to the new list */
          g_ptr_array_add (new_streams, g_steal_pointer (&modulestream));
        }
      else
        {
          upgraded_stream = modulemd_module_stream_upgrade (
            modulestream, mdversion, &nested_error);
          if (!upgraded_stream)
            {
              g_propagate_prefixed_error (error,
                                          g_steal_pointer (&nested_error),
                                          "Error upgrading module stream %s",
                                          nsvca);
              return FALSE;
            }
          g_ptr_array_add (new_streams, g_steal_pointer (&upgraded_stream));
        }

      g_clear_pointer (&nsvca, g_free);
      g_clear_object (&modulestream);
    }

  /* Replace the old stream list with the new one */
  g_ptr_array_unref (self->streams);
  self->streams = g_steal_pointer (&new_streams);

  return TRUE;
}
