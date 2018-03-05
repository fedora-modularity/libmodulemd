/* modulemd-profile.c
 *
 * Copyright (C) 2017 Stephen Gallagher
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <glib.h>
#include "modulemd.h"
#include "modulemd-util.h"

struct _ModulemdDependencies
{
  GObject parent_instance;

  GHashTable *buildrequires;
  GHashTable *requires;
};

G_DEFINE_TYPE (ModulemdDependencies, modulemd_dependencies, G_TYPE_OBJECT)

enum
{
  DEPS_PROP_0,

  DEPS_PROP_BUILDREQUIRES,
  DEPS_PROP_REQUIRES,

  DEPS_N_PROPS
};

static GParamSpec *deps_properties[DEPS_N_PROPS];


static void
_modulemd_dependencies_add_streams (GHashTable *reqs,
                                    const gchar *module,
                                    const gchar **streams)
{
  ModulemdSimpleSet *streamset =
    MODULEMD_SIMPLESET (g_hash_table_lookup (reqs, module));
  gsize i = 0;

  if (streamset == NULL)
    {
      streamset = modulemd_simpleset_new ();
    }
  else
    {
      g_object_ref (streamset);
    }

  for (i = 0; streams[i]; i++)
    {
      modulemd_simpleset_add (streamset, streams[i]);
    }

  g_hash_table_replace (reqs, g_strdup (module), streamset);
}

static void
_modulemd_dependencies_add_stream (GHashTable *reqs,
                                   const gchar *module,
                                   const gchar *stream)
{
  const gchar **streams = g_new0 (const gchar *, 2);
  streams[0] = g_strdup (stream);

  _modulemd_dependencies_add_streams (reqs, module, streams);
  g_free (streams);
}


/**
 * modulemd_dependencies_add_buildrequires:
 * @module: The module name
 * @streams: (array zero-terminated): The list of streams for this module
 *
 * Add a set of modules and their streams that are required to build another
 * dependent module. The matrix of streams and module names will be calculated
 * by the build-system. If the listed provided module name is already present,
 * the streams will be added (with deduplication).
 */
void
modulemd_dependencies_add_buildrequires (ModulemdDependencies *self,
                                         const gchar *module,
                                         const gchar **streams)
{
  GHashTable *br = g_hash_table_ref (self->buildrequires);

  _modulemd_dependencies_add_streams (br, module, streams);

  g_hash_table_unref (br);

  g_object_notify_by_pspec (G_OBJECT (self),
                            deps_properties[DEPS_PROP_BUILDREQUIRES]);
}


/**
 * modulemd_dependencies_add_buildrequires_single:
 * @module: The module name
 * @stream: The stream for this module
 *
 * Add a single stream of a module that is required to build another dependent
 * module. The matrix of streams and module names will be calculated by the
 * build-system. If the listed provided module name is already present, the
 * streams will be added (with deduplication).
 */
void
modulemd_dependencies_add_buildrequires_single (ModulemdDependencies *self,
                                                const gchar *module,
                                                const gchar *stream)
{
  GHashTable *br = g_hash_table_ref (self->buildrequires);

  _modulemd_dependencies_add_stream (br, module, stream);

  g_hash_table_unref (br);

  g_object_notify_by_pspec (G_OBJECT (self),
                            deps_properties[DEPS_PROP_BUILDREQUIRES]);
}


/**
 * modulemd_dependencies_set_buildrequires:
 * @buildrequires: (nullable) (element-type utf8 ModulemdSimpleSet) (transfer none): The
 * requirements to build this module.
 *
 * Sets the 'buildrequires' property.
 */
void
modulemd_dependencies_set_buildrequires (ModulemdDependencies *self,
                                         GHashTable *buildrequires)
{
  GHashTableIter iter;
  gpointer key, value;
  ModulemdSimpleSet *copy = NULL;
  g_return_if_fail (MODULEMD_IS_DEPENDENCIES (self));

  g_hash_table_remove_all (self->buildrequires);

  if (buildrequires)
    {
      g_hash_table_iter_init (&iter, buildrequires);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          modulemd_simpleset_copy ((ModulemdSimpleSet *)value, &copy);

          g_hash_table_replace (
            self->buildrequires, g_strdup ((gchar *)key), g_object_ref (copy));

          g_clear_pointer (&copy, g_object_unref);
        }
    }

  g_object_notify_by_pspec (G_OBJECT (self),
                            deps_properties[DEPS_PROP_BUILDREQUIRES]);
}


/**
 * modulemd_dependencies_get_buildrequires:
 *
 * Retrieves the "buildrequires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A hash
 * table containing the "buildrequires" property. Returns NULL if there are no
 * buildrequires set.
 *
 * Deprecated: 1.1
 * Use peek_buildrequires() instead.
 */
G_DEPRECATED_FOR (modulemd_dependencies_peek_buildrequires)
GHashTable *
modulemd_dependencies_get_buildrequires (ModulemdDependencies *self)
{
  return modulemd_dependencies_peek_buildrequires (self);
}


/**
 * modulemd_dependencies_peek_buildrequires:
 *
 * Retrieves the "buildrequires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A hash
 * table containing the "buildrequires" property. Returns NULL if there are no
 * buildrequires set.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_dependencies_peek_buildrequires (ModulemdDependencies *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self), NULL);

  return self->buildrequires;
}


static GHashTable *
modulemd_dependencies_dup_requires_common (GHashTable *requires)
{
  GHashTable *new_requires = NULL;
  ModulemdSimpleSet *set = NULL;
  GHashTableIter iter;
  gpointer key, value;

  g_return_val_if_fail (requires, NULL);

  new_requires =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  g_hash_table_iter_init (&iter, requires);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      set = NULL;
      modulemd_simpleset_copy (MODULEMD_SIMPLESET (value), &set);
      g_hash_table_replace (
        new_requires, g_strdup ((gchar *)key), (gpointer)set);
    }

  return new_requires;
}


/**
 * modulemd_dependencies_dup_buildrequires:
 *
 * Retrieves a copy of the "buildrequires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer container): A hash
 * table containing the "buildrequires" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_dependencies_dup_buildrequires (ModulemdDependencies *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self), NULL);

  return modulemd_dependencies_dup_requires_common (self->buildrequires);
}


/**
 * modulemd_dependencies_add_requires:
 * @module: The module name
 * @streams: (array zero-terminated=1): The list of streams for this module
 *
 * Add a single stream of a module that is required to build another dependent
 * module. The matrix of streams and module names will be calculated by the
 * build-system. If the listed provided module name is already present, the
 * streams will be added (with deduplication).
 */
void
modulemd_dependencies_add_requires (ModulemdDependencies *self,
                                    const gchar *module,
                                    const gchar **streams)
{
  GHashTable *r = g_hash_table_ref (self->requires);

  _modulemd_dependencies_add_streams (r, module, streams);

  g_hash_table_unref (r);

  g_object_notify_by_pspec (G_OBJECT (self),
                            deps_properties[DEPS_PROP_REQUIRES]);
}


/**
 * modulemd_dependencies_add_requires_single:
 * @module: The module name
 * @stream: The stream for this module
 *
 * Add a set of modules and their streams that are required at runtime by a
 * dependent module. The matrix of streams and module names will be calculated
 * by the build-system. If the listed provided module name is already present,
 * the streams will be added (with deduplication).
 */
void
modulemd_dependencies_add_requires_single (ModulemdDependencies *self,
                                           const gchar *module,
                                           const gchar *stream)
{
  GHashTable *r = g_hash_table_ref (self->requires);

  _modulemd_dependencies_add_stream (r, module, stream);

  g_hash_table_unref (r);

  g_object_notify_by_pspec (G_OBJECT (self),
                            deps_properties[DEPS_PROP_REQUIRES]);
}


/**
 * modulemd_dependencies_set_requires:
 * @requires: (nullable) (element-type utf8 ModulemdSimpleSet) (transfer none):
 * The runtime requirements for this module.
 *
 * Sets the 'requires' property.
 */
void
modulemd_dependencies_set_requires (ModulemdDependencies *self,
                                    GHashTable *requires)
{
  GHashTableIter iter;
  gpointer key, value;
  ModulemdSimpleSet *copy = NULL;
  g_return_if_fail (MODULEMD_IS_DEPENDENCIES (self));

  g_hash_table_remove_all (self->requires);

  if (requires)
    {
      g_hash_table_iter_init (&iter, requires);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          modulemd_simpleset_copy ((ModulemdSimpleSet *)value, &copy);

          g_hash_table_replace (
            self->requires, g_strdup ((gchar *)key), g_object_ref (copy));

          g_clear_pointer (&copy, g_object_unref);
        }
    }

  g_object_notify_by_pspec (G_OBJECT (self),
                            deps_properties[DEPS_PROP_REQUIRES]);
}


/**
 * modulemd_dependencies_get_requires:
 *
 * Retrieves the "requires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A hash
 * table containing the "requires" property.
 *
 * Deprecated: 1.1
 */
G_DEPRECATED_FOR (modulemd_dependencies_peek_requires)
GHashTable *
modulemd_dependencies_get_requires (ModulemdDependencies *self)
{
  return modulemd_dependencies_peek_requires (self);
}


/**
 * modulemd_dependencies_peek_requires:
 *
 * Retrieves the "requires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer none): A hash
 * table containing the "requires" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_dependencies_peek_requires (ModulemdDependencies *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self), NULL);

  return self->requires;
}


/**
 * modulemd_dependencies_dup_requires:
 *
 * Retrieves a copy of the "requires" for these dependencies.
 *
 * Returns: (element-type utf8 ModulemdSimpleSet) (transfer container): A hash
 * table containing the "requires" property.
 *
 * Since: 1.1
 */
GHashTable *
modulemd_dependencies_dup_requires (ModulemdDependencies *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self), NULL);

  return modulemd_dependencies_dup_requires_common (self->requires);
}


/**
 * modulemd_dependencies_copy:
 * @dest: (out): A reference to the destination #ModulemdDependencies
 *
 * This function will copy the contents of this #ModulemdDependencies to @dest.
 * If the dereferenced pointer is NULL, a new #ModulemdDependencies will be
 * allocated.
 *
 * If the dereferenced pointer is not NULL, it will replace the contents of
 * @dest. All existing internal variables will be freed.
 *
 * In either case, the caller is responsible for calling g_object_unref() later
 * to free it.
 */
void
modulemd_dependencies_copy (ModulemdDependencies *self,
                            ModulemdDependencies **dest)
{
  g_return_if_fail (self && MODULEMD_IS_DEPENDENCIES (self));
  g_return_if_fail (dest);
  g_return_if_fail (*dest == NULL ||
                    (*dest != NULL && MODULEMD_IS_DEPENDENCIES (*dest)));

  /* Allocate a Modulemd.Dependencies if needed */
  if (*dest == NULL)
    {
      *dest = modulemd_dependencies_new ();
    }

  modulemd_dependencies_set_buildrequires (*dest, self->buildrequires);
  modulemd_dependencies_set_requires (*dest, self->requires);
}


ModulemdDependencies *
modulemd_dependencies_new (void)
{
  return g_object_new (MODULEMD_TYPE_DEPENDENCIES, NULL);
}

static void
modulemd_dependencies_finalize (GObject *object)
{
  ModulemdDependencies *self = (ModulemdDependencies *)object;

  g_clear_pointer (&self->buildrequires, g_hash_table_unref);
  g_clear_pointer (&self->requires, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_dependencies_parent_class)->finalize (object);
}

static void
modulemd_dependencies_get_property (GObject *object,
                                    guint prop_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  ModulemdDependencies *self = MODULEMD_DEPENDENCIES (object);

  switch (prop_id)
    {
    case DEPS_PROP_BUILDREQUIRES:
      g_value_set_boxed (value,
                         modulemd_dependencies_peek_buildrequires (self));
      break;

    case DEPS_PROP_REQUIRES:
      g_value_set_boxed (value, modulemd_dependencies_peek_requires (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}

static void
modulemd_dependencies_set_property (GObject *object,
                                    guint prop_id,
                                    const GValue *value,
                                    GParamSpec *pspec)
{
  ModulemdDependencies *self = MODULEMD_DEPENDENCIES (object);

  switch (prop_id)
    {
    case DEPS_PROP_BUILDREQUIRES:
      modulemd_dependencies_set_buildrequires (self,
                                               g_value_get_boxed (value));
      break;

    case DEPS_PROP_REQUIRES:
      modulemd_dependencies_set_requires (self, g_value_get_boxed (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}

static void
modulemd_dependencies_class_init (ModulemdDependenciesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_dependencies_finalize;
  object_class->get_property = modulemd_dependencies_get_property;
  object_class->set_property = modulemd_dependencies_set_property;

  /**
   * ModulemdDependencies:buildrequires: (type GLib.HashTable(utf8,ModulemdSimpleSet)) (transfer none)
   */
  deps_properties[DEPS_PROP_BUILDREQUIRES] = g_param_spec_boxed (
    "buildrequires",
    "Build dependencies",
    "A dictionary of module streams that this module must build against. The "
    "build system will create a matrix of builds from the modules and streams "
    "specified.",
    G_TYPE_HASH_TABLE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdDependencies:requires: (type GLib.HashTable(utf8,ModulemdSimpleSet)) (transfer none)
   */
  deps_properties[DEPS_PROP_REQUIRES] = g_param_spec_boxed (
    "requires",
    "Runtime dependencies",
    "A dictionary of module streams that this module requires at runtime. The "
    "buildsystem will match this to the buildrequires matrix as appropriate.",
    G_TYPE_HASH_TABLE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (
    object_class, DEPS_N_PROPS, deps_properties);
}

static void
modulemd_dependencies_init (ModulemdDependencies *self)
{
  /* Allocate the members */
  self->buildrequires =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  self->requires =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}
