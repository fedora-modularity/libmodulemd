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
#include "modulemd-improvedmodule.h"
#include "private/modulemd-improvedmodule-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdImprovedModule
{
  GObject parent_instance;

  /* The name of this module */
  gchar *name;

  /* Hash table of streams available in this module, indexed by stream name */
  GHashTable *streams;

  /* The defaults for this module */
  ModulemdDefaults *defaults;
};

G_DEFINE_TYPE (ModulemdImprovedModule, modulemd_improvedmodule, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_NAME,
  PROP_DEFAULTS,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdImprovedModule *
modulemd_improvedmodule_new (const gchar *name)
{
  return g_object_new (MODULEMD_TYPE_IMPROVEDMODULE, "name", name, NULL);
}

static void
modulemd_improvedmodule_finalize (GObject *object)
{
  ModulemdImprovedModule *self = (ModulemdImprovedModule *)object;

  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->streams, g_hash_table_unref);
  g_clear_pointer (&self->defaults, g_object_unref);

  G_OBJECT_CLASS (modulemd_improvedmodule_parent_class)->finalize (object);
}


/**
 * modulemd_improvedmodule_set_name:
 * @module_name: (transfer none) (not nullable): The name of this module.
 *
 * Sets the module name.
 *
 * Since: 1.6
 */
void
modulemd_improvedmodule_set_name (ModulemdImprovedModule *self,
                                  const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_IMPROVEDMODULE (self));

  g_clear_pointer (&self->name, g_free);
  self->name = g_strdup (module_name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}


/**
 * modulemd_improvedmodule_get_name:
 *
 * Gets the name of this module.
 *
 * Returns: (transfer full): The name of this module. This value must be freed
 * with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_improvedmodule_get_name (ModulemdImprovedModule *self)
{
  return g_strdup (self->name);
}


/**
 * modulemd_improvedmodule_peek_name: (skip)
 *
 * Gets the name of this module.
 *
 * Returns: (transfer none): The name of this module. This value must be not be
 * modified or freed.
 *
 * Since: 1.6
 */
const gchar *
modulemd_improvedmodule_peek_name (ModulemdImprovedModule *self)
{
  return self->name;
}


/**
 * modulemd_improvedmodule_set_defaults:
 * @defaults: (transfer none) (nullable): A #ModulemdDefaults object describing
 * the defaults for this module.
 *
 * Set the default stream and profiles for this module. Makes no changes if the
 * defaults do not apply to this module.
 *
 * Since: 1.6
 */
void
modulemd_improvedmodule_set_defaults (ModulemdImprovedModule *self,
                                      ModulemdDefaults *defaults)
{
  g_return_if_fail (MODULEMD_IS_IMPROVEDMODULE (self));
  g_return_if_fail (!defaults || MODULEMD_IS_DEFAULTS (defaults));


  /* Return without making any changes if the module names don't match. */
  if (defaults && g_strcmp0 (modulemd_defaults_peek_module_name (defaults),
                             modulemd_improvedmodule_peek_name (self)))
    {
      g_warning ("Attempting to assign defaults for module %s to module %s",
                 modulemd_defaults_peek_module_name (defaults),
                 modulemd_improvedmodule_peek_name (self));
      return;
    }

  g_clear_pointer (&self->defaults, g_object_unref);

  if (defaults)
    {
      self->defaults = modulemd_defaults_copy (defaults);
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DEFAULTS]);
}

/**
 * modulemd_improvedmodule_get_defaults:
 *
 * Returns the #ModulemdDefaults object for this module.
 *
 * Returns: (transfer full): a #ModulemdDefaults object if set, NULL otherwise.
 * This object must be freed with g_object_unref().
 *
 * Since: 1.6
 */
ModulemdDefaults *
modulemd_improvedmodule_get_defaults (ModulemdImprovedModule *self)
{
  return modulemd_defaults_copy (self->defaults);
}


/**
 * modulemd_improvedmodule_peek_defaults: (skip)
 *
 * Returns the #ModulemdDefaults object for this module.
 *
 * Returns: (transfer none): a #ModulemdDefaults object if set, NULL otherwise.
 * This object must be not be modified or freed.
 *
 * Since: 1.6
 */
ModulemdDefaults *
modulemd_improvedmodule_peek_defaults (ModulemdImprovedModule *self)
{
  return self->defaults;
}


/**
 * modulemd_improvedmodule_add_stream:
 * @stream: (transfer none) (not nullable): A #ModulemdModuleStream of this
 * module.
 *
 * Add a #ModulemdModuleStream to this module. If this stream name is already in
 * use, this function will overwrite the existing value. If the module name does
 * not match, this function will silently ignore this stream.
 *
 * Since : 1.6
 */
void
modulemd_improvedmodule_add_stream (ModulemdImprovedModule *self,
                                    ModulemdModuleStream *stream)
{
  g_autofree gchar *stream_name = NULL;
  g_return_if_fail (MODULEMD_IS_IMPROVEDMODULE (self));
  g_return_if_fail (MODULEMD_IS_MODULESTREAM (stream));


  if (g_strcmp0 (self->name, modulemd_modulestream_peek_name (stream)))
    {
      /* This stream doesn't match this module. Ignore it */
      return;
    }

  stream_name = modulemd_modulestream_get_stream (stream);
  if (!stream_name)
    {
      /* The stream name is usually filled in by the build system, so if we're
       * handling a user-edited libmodulemd file, just fill this field with
       * unique placeholder data.
       */
      stream_name =
        g_strdup_printf ("__unknown_%d__", g_hash_table_size (self->streams));
    }

  g_hash_table_replace (self->streams,
                        g_strdup (stream_name),
                        modulemd_modulestream_copy (stream));
}


/**
 * modulemd_improvedmodule_get_stream_by_name:
 * @stream_name: The name of the stream to retrieve.
 *
 * Returns: (transfer full): A #ModulemModuleStream representing the requested
 * module stream. NULL if the stream name was not found.
 *
 * Since: 1.6
 */
ModulemdModuleStream *
modulemd_improvedmodule_get_stream_by_name (ModulemdImprovedModule *self,
                                            const gchar *stream_name)
{
  g_return_val_if_fail (MODULEMD_IS_IMPROVEDMODULE (self), NULL);

  if (!g_hash_table_contains (self->streams, stream_name))
    {
      return NULL;
    }

  return modulemd_modulestream_copy (
    g_hash_table_lookup (self->streams, stream_name));
}


/**
 * modulemd_improvedmodule_get_streams:
 *
 * Returns: (element-type utf8 ModulemdModuleStream) (transfer container): A
 * #GHashTable containing all #ModulemModuleStream objects for this module.
 * This hash table must be freed with g_hash_table_unref().
 *
 * Since: 1.6
 */
GHashTable *
modulemd_improvedmodule_get_streams (ModulemdImprovedModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_IMPROVEDMODULE (self), NULL);

  return g_hash_table_ref (self->streams);
}


/**
 * modulemd_improvedmodule_copy:
 *
 * Make a copy of this module.
 *
 * Returns: (transfer full): A newly-allocated #ModulemdImprovedModule that is
 * a copy of the one passed in.
 *
 * Since: 1.6
 */

ModulemdImprovedModule *
modulemd_improvedmodule_copy (ModulemdImprovedModule *self)
{
  GHashTableIter iter;
  gpointer key, value;
  ModulemdImprovedModule *new_module = NULL;

  if (!self)
    return NULL;

  new_module =
    modulemd_improvedmodule_new (modulemd_improvedmodule_peek_name (self));

  /* Copy all of the streams */
  g_hash_table_iter_init (&iter, self->streams);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      g_hash_table_replace (
        new_module->streams,
        g_strdup ((const gchar *)key),
        modulemd_modulestream_copy ((MODULEMD_MODULESTREAM (value))));
    }

  /* Copy the defaults data */
  modulemd_improvedmodule_set_defaults (
    new_module, modulemd_improvedmodule_peek_defaults (self));

  return new_module;
}


static void
modulemd_improvedmodule_get_property (GObject *object,
                                      guint prop_id,
                                      GValue *value,
                                      GParamSpec *pspec)
{
  ModulemdImprovedModule *self = MODULEMD_IMPROVEDMODULE (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_take_string (value, modulemd_improvedmodule_get_name (self));
      break;

    case PROP_DEFAULTS:
      g_value_take_object (value, modulemd_improvedmodule_get_defaults (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_improvedmodule_set_property (GObject *object,
                                      guint prop_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
  ModulemdImprovedModule *self = MODULEMD_IMPROVEDMODULE (object);

  switch (prop_id)
    {
    case PROP_NAME:
      modulemd_improvedmodule_set_name (self, g_value_get_string (value));
      break;

    case PROP_DEFAULTS:
      modulemd_improvedmodule_set_defaults (self, g_value_get_object (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_improvedmodule_class_init (ModulemdImprovedModuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_improvedmodule_finalize;
  object_class->get_property = modulemd_improvedmodule_get_property;
  object_class->set_property = modulemd_improvedmodule_set_property;


  properties[PROP_NAME] = g_param_spec_string (
    "name",
    "Module Name",
    "The name of this module",
    NULL,
    G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  properties[PROP_DEFAULTS] =
    g_param_spec_object ("defaults",
                         "Module Defaults",
                         "Object describing the default stream and profiles "
                         "for this module.",
                         MODULEMD_TYPE_DEFAULTS,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_improvedmodule_init (ModulemdImprovedModule *self)
{
  self->streams =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}


GPtrArray *
modulemd_improvedmodule_serialize (ModulemdImprovedModule *self)
{
  g_autoptr (GPtrArray) objects = NULL;
  g_autoptr (GPtrArray) keys = NULL;

  g_return_val_if_fail (MODULEMD_IS_IMPROVEDMODULE (self), NULL);

  /* First export all of the ModuleStream objects */
  keys = _modulemd_ordered_str_keys (self->streams, _modulemd_strcmp_sort);

  /* Preallocate the array to hold the full set of streams, plus the defaults */
  objects = g_ptr_array_new_full (keys->len + 1, g_object_unref);

  for (gsize i = 0; i < keys->len; i++)
    {
      g_ptr_array_add (objects,
                       modulemd_improvedmodule_get_stream_by_name (
                         self, g_ptr_array_index (keys, i)));
    }

  /* Then write out the default object if it exists */
  if (modulemd_improvedmodule_peek_defaults (self))
    {
      g_ptr_array_add (objects, modulemd_improvedmodule_get_defaults (self));
    }

  return g_ptr_array_ref (objects);
}


/**
 * modulemd_improvedmodule_dump:
 * @yaml_file: A string containing the path to the output file
 *
 * Writes this module out to a YAML document on disk.
 *
 * Since: 1.6
 */
void
modulemd_improvedmodule_dump (ModulemdImprovedModule *self,
                              const gchar *yaml_file,
                              GError **error)
{
  g_autoptr (GPtrArray) objects = NULL;

  g_return_if_fail (MODULEMD_IS_IMPROVEDMODULE (self));

  objects = modulemd_improvedmodule_serialize (self);

  if (!emit_yaml_file (objects, yaml_file, error))
    {
      g_debug ("Error emitting YAML file: %s", (*error)->message);
    }
}

/**
 * modulemd_improvedmodule_dumps:
 *
 * Writes this module out to a YAML document string.
 *
 * Return value: (transfer full): A string containing a YAML representation of
 * this module and all of its streams. This string must be freed with g_free().
 *
 * Since: 1.6
 */
gchar *
modulemd_improvedmodule_dumps (ModulemdImprovedModule *self, GError **error)
{
  gchar *yaml = NULL;
  g_autoptr (GPtrArray) objects = NULL;

  objects = modulemd_improvedmodule_serialize (self);

  if (!emit_yaml_string (objects, &yaml, error))
    {
      g_debug ("Error emitting YAML string: %s", (*error)->message);
      g_clear_pointer (&yaml, g_free);
    }

  return yaml;
}
