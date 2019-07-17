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

#include "modulemd.h"
#include "modulemd-translation.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

GQuark
modulemd_translation_error_quark (void)
{
  return g_quark_from_static_string ("modulemd-translation-error-quark");
}

struct _ModulemdTranslation
{
  GObject parent_instance;

  guint64 mdversion;

  gchar *module_name;

  gchar *module_stream;

  guint64 modified;

  GHashTable *translations;
};

G_DEFINE_TYPE (ModulemdTranslation, modulemd_translation, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_MDVERSION,
  PROP_MODNAME,
  PROP_MODSTREAM,
  PROP_MODIFIED,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdTranslation *
modulemd_translation_new (void)
{
  return g_object_new (MODULEMD_TYPE_TRANSLATION, NULL);
}


ModulemdTranslation *
modulemd_translation_new_full (const gchar *module_name,
                               const gchar *module_stream,
                               guint64 mdversion,
                               guint64 modified)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_TRANSLATION,
                       "module-name", module_name,
                       "module-stream", module_stream,
                       "mdversion", mdversion,
                       "modified", modified,
                       NULL);
  // clang-format on
}


static void
_modulemd_translation_copy_internal (ModulemdTranslation *dest,
                                     ModulemdTranslation *src)
{
  GHashTableIter iter;
  gpointer key, value;

  modulemd_translation_set_mdversion (dest, src->mdversion);
  modulemd_translation_set_module_name (dest, src->module_name);
  modulemd_translation_set_module_stream (dest, src->module_stream);
  modulemd_translation_set_modified (dest, src->modified);

  g_hash_table_iter_init (&iter, src->translations);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      modulemd_translation_add_entry (dest,
                                      MODULEMD_TRANSLATION_ENTRY (value));
    }
}


ModulemdTranslation *
modulemd_translation_copy (ModulemdTranslation *self)
{
  ModulemdTranslation *copy = NULL;

  if (!self)
    return NULL;

  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), NULL);

  copy = g_object_new (MODULEMD_TYPE_TRANSLATION, NULL);
  _modulemd_translation_copy_internal (copy, self);

  return copy;
}


gboolean
modulemd_translation_import_from_file (ModulemdTranslation *self,
                                       const gchar *yaml_file,
                                       GError **error)
{
  g_autoptr (GPtrArray) data = NULL;

  if (!parse_yaml_file (yaml_file, &data, NULL, error))
    return FALSE;

  if (data->len < 1)
    {
      g_set_error (error,
                   MODULEMD_TRANSLATION_ERROR,
                   MODULEMD_TRANSLATION_ERROR_MISSING_CONTENT,
                   "Provided YAML contained no valid subdocuments");
      return FALSE;
    }
  else if (!MODULEMD_IS_TRANSLATION (g_ptr_array_index (data, 0)))
    {
      g_set_error (error,
                   MODULEMD_TRANSLATION_ERROR,
                   MODULEMD_TRANSLATION_ERROR_MISSING_CONTENT,
                   "Provided YAML was not a valid translation document");
      return FALSE;
    }

  _modulemd_translation_copy_internal (self, g_ptr_array_index (data, 0));

  return TRUE;
}


gboolean
modulemd_translation_import_from_string (ModulemdTranslation *self,
                                         const gchar *yaml,
                                         GError **error)
{
  g_autoptr (GPtrArray) data = NULL;

  if (!parse_yaml_string (yaml, &data, NULL, error))
    return FALSE;

  if (data->len < 1)
    {
      g_set_error (error,
                   MODULEMD_TRANSLATION_ERROR,
                   MODULEMD_TRANSLATION_ERROR_MISSING_CONTENT,
                   "Provided YAML contained no valid subdocuments");
      return FALSE;
    }
  else if (!MODULEMD_IS_TRANSLATION (g_ptr_array_index (data, 0)))
    {
      g_set_error (error,
                   MODULEMD_TRANSLATION_ERROR,
                   MODULEMD_TRANSLATION_ERROR_MISSING_CONTENT,
                   "Provided YAML was not a valid translation document");
      return FALSE;
    }

  _modulemd_translation_copy_internal (self, g_ptr_array_index (data, 0));

  return TRUE;
}


gboolean
modulemd_translation_import_from_stream (ModulemdTranslation *self,
                                         FILE *yaml_stream,
                                         GError **error)
{
  g_autoptr (GPtrArray) data = NULL;

  if (!parse_yaml_stream (yaml_stream, &data, NULL, error))
    return FALSE;

  if (data->len < 1)
    {
      g_set_error (error,
                   MODULEMD_TRANSLATION_ERROR,
                   MODULEMD_TRANSLATION_ERROR_MISSING_CONTENT,
                   "Provided YAML contained no valid subdocuments");
      return FALSE;
    }
  else if (!MODULEMD_IS_TRANSLATION (g_ptr_array_index (data, 0)))
    {
      g_set_error (error,
                   MODULEMD_TRANSLATION_ERROR,
                   MODULEMD_TRANSLATION_ERROR_MISSING_CONTENT,
                   "Provided YAML was not a valid translation document");
      return FALSE;
    }

  _modulemd_translation_copy_internal (self, g_ptr_array_index (data, 0));

  return TRUE;
}


gboolean
modulemd_translation_dump (ModulemdTranslation *self,
                           const gchar *yaml_file,
                           GError **error)
{
  g_autoptr (GPtrArray) objects = g_ptr_array_new ();

  g_ptr_array_add (objects, self);

  if (!emit_yaml_file (objects, yaml_file, error))
    {
      g_debug ("Error emitting YAML file: %s", (*error)->message);
      return FALSE;
    }

  return TRUE;
}


gchar *
modulemd_translation_dumps (ModulemdTranslation *self, GError **error)
{
  gchar *yaml = NULL;
  g_autoptr (GPtrArray) objects = g_ptr_array_new ();

  g_ptr_array_add (objects, self);

  if (!emit_yaml_string (objects, &yaml, error))
    {
      g_debug ("Error emitting YAML file: %s", (*error)->message);
      g_clear_pointer (&yaml, g_free);
    }

  return yaml;
}


static void
modulemd_translation_finalize (GObject *object)
{
  ModulemdTranslation *self = (ModulemdTranslation *)object;

  g_clear_pointer (&self->module_name, g_free);
  g_clear_pointer (&self->module_stream, g_free);
  g_clear_pointer (&self->translations, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_translation_parent_class)->finalize (object);
}


void
modulemd_translation_set_mdversion (ModulemdTranslation *self,
                                    guint64 mdversion)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));

  self->mdversion = mdversion;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MDVERSION]);
}


guint64
modulemd_translation_get_mdversion (ModulemdTranslation *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), 0);

  return self->mdversion;
}


void
modulemd_translation_set_module_name (ModulemdTranslation *self,
                                      const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));

  g_clear_pointer (&self->module_name, g_free);
  self->module_name = g_strdup (module_name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODNAME]);
}


gchar *
modulemd_translation_get_module_name (ModulemdTranslation *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), NULL);

  return g_strdup (self->module_name);
}


const gchar *
modulemd_translation_peek_module_name (ModulemdTranslation *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), NULL);

  return self->module_name;
}


void
modulemd_translation_set_module_stream (ModulemdTranslation *self,
                                        const gchar *module_stream)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));

  g_clear_pointer (&self->module_stream, g_free);
  self->module_stream = g_strdup (module_stream);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODSTREAM]);
}


gchar *
modulemd_translation_get_module_stream (ModulemdTranslation *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), NULL);

  return g_strdup (self->module_stream);
}


const gchar *
modulemd_translation_peek_module_stream (ModulemdTranslation *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), NULL);

  return self->module_stream;
}


void
modulemd_translation_set_modified (ModulemdTranslation *self, guint64 modified)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));

  self->modified = modified;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODIFIED]);
}


guint64
modulemd_translation_get_modified (ModulemdTranslation *self)
{
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), 0);

  return self->modified;
}


void
modulemd_translation_add_entry (ModulemdTranslation *self,
                                ModulemdTranslationEntry *entry)
{
  g_return_if_fail (MODULEMD_IS_TRANSLATION (self));
  g_return_if_fail (MODULEMD_IS_TRANSLATION_ENTRY (entry));

  g_hash_table_replace (self->translations,
                        modulemd_translation_entry_get_locale (entry),
                        modulemd_translation_entry_copy (entry));
}


ModulemdTranslationEntry *
modulemd_translation_get_entry_by_locale (ModulemdTranslation *self,
                                          const gchar *locale)
{
  ModulemdTranslationEntry *entry = NULL;
  g_return_val_if_fail (MODULEMD_IS_TRANSLATION (self), NULL);

  entry = g_hash_table_lookup (self->translations, locale);
  if (!entry)
    return NULL;

  return modulemd_translation_entry_copy (entry);
}


GPtrArray *
modulemd_translation_get_locales (ModulemdTranslation *self)
{
  return _modulemd_ordered_str_keys (self->translations,
                                     _modulemd_strcmp_sort);
}


static void
modulemd_translation_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  ModulemdTranslation *self = MODULEMD_TRANSLATION (object);

  switch (prop_id)
    {
    case PROP_MDVERSION:
      g_value_set_uint64 (value, modulemd_translation_get_mdversion (self));
      break;

    case PROP_MODNAME:
      g_value_take_string (value, modulemd_translation_get_module_name (self));
      break;

    case PROP_MODSTREAM:
      g_value_take_string (value,
                           modulemd_translation_get_module_stream (self));
      break;

    case PROP_MODIFIED:
      g_value_set_uint64 (value, modulemd_translation_get_modified (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_translation_set_property (GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  ModulemdTranslation *self = MODULEMD_TRANSLATION (object);

  switch (prop_id)
    {
    case PROP_MDVERSION:
      modulemd_translation_set_mdversion (self, g_value_get_uint64 (value));
      break;

    case PROP_MODNAME:
      modulemd_translation_set_module_name (self, g_value_get_string (value));
      break;

    case PROP_MODSTREAM:
      modulemd_translation_set_module_stream (self,
                                              g_value_get_string (value));
      break;

    case PROP_MODIFIED:
      modulemd_translation_set_modified (self, g_value_get_uint64 (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_translation_class_init (ModulemdTranslationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_translation_finalize;
  object_class->get_property = modulemd_translation_get_property;
  object_class->set_property = modulemd_translation_set_property;

  properties[PROP_MDVERSION] =
    g_param_spec_uint64 ("mdversion",
                         "Metadata Version",
                         "The metadata version of this modulemd-translation",
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_MODNAME] = g_param_spec_string (
    "module-name",
    "Module Name",
    "The name of the module to which these translations apply.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_MODSTREAM] = g_param_spec_string (
    "module-stream",
    "Module Stream",
    "The name of the module stream to which these translations apply",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_MODIFIED] =
    g_param_spec_uint64 ("modified",
                         "Last Modified",
                         "The last modification time",
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_translation_init (ModulemdTranslation *self)
{
  self->translations =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}
