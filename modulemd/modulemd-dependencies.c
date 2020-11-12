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

#include "modulemd-dependencies.h"
#include "modulemd-errors.h"
#include "private/glib-extensions.h"
#include "private/modulemd-dependencies-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdDependencies
{
  GObject parent_instance;

  /* @key: dependent modules.
   * @value: #GHashTable set of compatible streams
   */
  GHashTable *buildtime_deps;

  /* @key: dependent modules.
   * @value: #GHashTable set of compatible streams
   */
  GHashTable *runtime_deps;
};

G_DEFINE_TYPE (ModulemdDependencies, modulemd_dependencies, G_TYPE_OBJECT)

ModulemdDependencies *
modulemd_dependencies_new (void)
{
  return g_object_new (MODULEMD_TYPE_DEPENDENCIES, NULL);
}


gboolean
modulemd_dependencies_equals (ModulemdDependencies *self_1,
                              ModulemdDependencies *self_2)
{
  if (!self_1 && !self_2)
    {
      return TRUE;
    }

  if (!self_1 || !self_2)
    {
      return FALSE;
    }

  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self_1), FALSE);
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self_2), FALSE);

  if (!modulemd_hash_table_equals (self_1->buildtime_deps,
                                   self_2->buildtime_deps,
                                   modulemd_hash_table_sets_are_equal_wrapper))
    {
      return FALSE;
    }

  if (!modulemd_hash_table_equals (self_1->runtime_deps,
                                   self_2->runtime_deps,
                                   modulemd_hash_table_sets_are_equal_wrapper))
    {
      return FALSE;
    }

  return TRUE;
}


ModulemdDependencies *
modulemd_dependencies_copy (ModulemdDependencies *self)
{
  g_autoptr (ModulemdDependencies) d = NULL;
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self), NULL);

  d = modulemd_dependencies_new ();

  g_hash_table_unref (d->buildtime_deps);
  d->buildtime_deps = g_hash_table_ref (self->buildtime_deps);
  g_hash_table_unref (d->runtime_deps);
  d->runtime_deps = g_hash_table_ref (self->runtime_deps);

  return g_steal_pointer (&d);
}


static void
modulemd_dependencies_finalize (GObject *object)
{
  ModulemdDependencies *self = (ModulemdDependencies *)object;

  g_clear_pointer (&self->buildtime_deps, g_hash_table_unref);
  g_clear_pointer (&self->runtime_deps, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_dependencies_parent_class)->finalize (object);
}


static GHashTable *
modulemd_dependencies_nested_table_get_or_create (GHashTable *table,
                                                  const gchar *key)
{
  g_autofree gchar *keyi = NULL;

  GHashTable *inner = NULL;
  inner = g_hash_table_lookup (table, key);
  if (inner != NULL)
    {
      return inner;
    }

  // We know that the hash table will end up holding on to it for us.
  keyi = g_strdup (key);

  inner = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  g_hash_table_insert (table, keyi, inner);
  keyi = NULL;
  return inner;
}


static void
modulemd_dependencies_nested_table_add (GHashTable *table,
                                        const gchar *key,
                                        const gchar *value)
{
  GHashTable *inner =
    modulemd_dependencies_nested_table_get_or_create (table, key);
  g_return_if_fail (inner);
  if (value != NULL)
    {
      g_hash_table_add (inner, g_strdup (value));
    }
}


static GStrv
modulemd_dependencies_nested_table_values_as_strv (GHashTable *table,
                                                   const gchar *key)
{
  GHashTable *inner = g_hash_table_lookup (table, key);
  if (inner == NULL)
    {
      g_warning ("Streams requested for unknown module: %s", key);
      return NULL;
    }
  return modulemd_ordered_str_keys_as_strv (inner);
}


void
modulemd_dependencies_add_buildtime_stream (ModulemdDependencies *self,
                                            const gchar *module_name,
                                            const gchar *module_stream)
{
  g_return_if_fail (MODULEMD_IS_DEPENDENCIES (self));
  g_return_if_fail (module_name);
  g_return_if_fail (module_stream);
  modulemd_dependencies_nested_table_add (
    self->buildtime_deps, module_name, module_stream);
}


void
modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
  ModulemdDependencies *self, const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_DEPENDENCIES (self));
  g_return_if_fail (module_name);
  modulemd_dependencies_nested_table_add (
    self->buildtime_deps, module_name, NULL);
}


void
modulemd_dependencies_clear_buildtime_dependencies (ModulemdDependencies *self)
{
  g_return_if_fail (MODULEMD_IS_DEPENDENCIES (self));
  g_hash_table_remove_all (self->buildtime_deps);
}


GStrv
modulemd_dependencies_get_buildtime_modules_as_strv (
  ModulemdDependencies *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self), NULL);
  return modulemd_ordered_str_keys_as_strv (self->buildtime_deps);
}


GStrv
modulemd_dependencies_get_buildtime_streams_as_strv (
  ModulemdDependencies *self, const gchar *module)
{
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self), NULL);
  return modulemd_dependencies_nested_table_values_as_strv (
    self->buildtime_deps, module);
}


void
modulemd_dependencies_add_runtime_stream (ModulemdDependencies *self,
                                          const gchar *module_name,
                                          const gchar *module_stream)
{
  g_return_if_fail (MODULEMD_IS_DEPENDENCIES (self));
  g_return_if_fail (module_name);
  g_return_if_fail (module_stream);
  modulemd_dependencies_nested_table_add (
    self->runtime_deps, module_name, module_stream);
}


void
modulemd_dependencies_set_empty_runtime_dependencies_for_module (
  ModulemdDependencies *self, const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_DEPENDENCIES (self));
  g_return_if_fail (module_name);
  modulemd_dependencies_nested_table_add (
    self->runtime_deps, module_name, NULL);
}


void
modulemd_dependencies_clear_runtime_dependencies (ModulemdDependencies *self)
{
  g_return_if_fail (MODULEMD_IS_DEPENDENCIES (self));
  g_hash_table_remove_all (self->runtime_deps);
}


GStrv
modulemd_dependencies_get_runtime_modules_as_strv (ModulemdDependencies *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self), NULL);
  return modulemd_ordered_str_keys_as_strv (self->runtime_deps);
}


GStrv
modulemd_dependencies_get_runtime_streams_as_strv (ModulemdDependencies *self,
                                                   const gchar *module)
{
  g_return_val_if_fail (MODULEMD_IS_DEPENDENCIES (self), NULL);
  return modulemd_dependencies_nested_table_values_as_strv (self->runtime_deps,
                                                            module);
}


static gboolean
modulemd_dependencies_validate_deps (GHashTable *deps, GError **error)
{
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  gchar *module_name = NULL;
  gchar *stream_name = NULL;
  gssize signedness = 0;
  g_autoptr (GPtrArray) set = NULL;

  g_hash_table_iter_init (&iter, deps);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      module_name = (gchar *)key;
      /* The value is a set of strings. Get it and check them all */
      set = modulemd_ordered_str_keys (value, modulemd_strcmp_sort);

      /* An empty set is always valid */
      if (set->len == 0)
        {
          g_clear_pointer (&set, g_ptr_array_unref);
          continue;
        }

      /* The first element will determine the signedness for the whole
       * set.
       */
      if (((const gchar *)g_ptr_array_index (set, 0))[0] == '-')
        {
          signedness = -1;
        }
      else
        {
          signedness = 1;
        }

      for (guint i = 1; i < set->len; i++)
        {
          stream_name = (gchar *)g_ptr_array_index (set, i);
          if ((stream_name[0] == '-' && signedness > 0) ||
              (stream_name[0] != '-' && signedness < 0))
            {
              g_set_error (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Runtime dependency %s contained a mix of positive "
                           "and negative entries.",
                           module_name);
              return FALSE;
            }
        }

      g_clear_pointer (&set, g_ptr_array_unref);
    }

  return TRUE;
}


gboolean
modulemd_dependencies_validate (ModulemdDependencies *self, GError **error)
{
  /* Look through all the runtime dependencies */
  if (!modulemd_dependencies_validate_deps (self->runtime_deps, error))
    {
      return FALSE;
    }

  if (!modulemd_dependencies_validate_deps (self->buildtime_deps, error))
    {
      return FALSE;
    }

  return TRUE;
}


static void
modulemd_dependencies_get_property (GObject *object,
                                    guint prop_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_dependencies_set_property (GObject *object,
                                    guint prop_id,
                                    const GValue *value,
                                    GParamSpec *pspec)
{
  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_dependencies_class_init (ModulemdDependenciesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_dependencies_finalize;
  object_class->get_property = modulemd_dependencies_get_property;
  object_class->set_property = modulemd_dependencies_set_property;
}


static void
modulemd_dependencies_init (ModulemdDependencies *self)
{
  self->buildtime_deps = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_hash_table_destroy);
  self->runtime_deps = g_hash_table_new_full (
    g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_hash_table_destroy);
}

/* === YAML Functions === */

ModulemdDependencies *
modulemd_dependencies_parse_yaml (yaml_parser_t *parser,
                                  gboolean strict,
                                  GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  g_autoptr (ModulemdDependencies) d = NULL;
  g_autoptr (GError) nested_error = NULL;

  d = modulemd_dependencies_new ();

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          if (g_str_equal (event.data.scalar.value, "buildrequires"))
            {
              g_hash_table_unref (d->buildtime_deps);
              d->buildtime_deps =
                modulemd_yaml_parse_nested_set (parser, &nested_error);
              if (d->buildtime_deps == NULL)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse buildtime deps: %s",
                    nested_error->message);
                }
            }
          else if (g_str_equal (event.data.scalar.value, "requires"))
            {
              g_hash_table_unref (d->runtime_deps);
              d->runtime_deps =
                modulemd_yaml_parse_nested_set (parser, &nested_error);
              if (d->runtime_deps == NULL)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse runtime deps: %s",
                    nested_error->message);
                }
            }
          else
            {
              SKIP_UNKNOWN (parser,
                            NULL,
                            "Unexpected key in dependencies body: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event in dependencies: %d",
            event.type);
          break;
        }

      yaml_event_delete (&event);
    }
  return g_steal_pointer (&d);
}


gboolean
modulemd_dependencies_emit_yaml (ModulemdDependencies *self,
                                 yaml_emitter_t *emitter,
                                 GError **error)
{
  MODULEMD_INIT_TRACE ();
  int ret;
  g_autoptr (GError) nested_error = NULL;
  MMD_INIT_YAML_EVENT (event);

  ret = mmd_emitter_start_mapping (
    emitter, YAML_BLOCK_MAPPING_STYLE, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Failed to start dependencies mapping: ");
      return FALSE;
    }

  if (g_hash_table_size (self->buildtime_deps) != 0)
    {
      ret = mmd_emitter_scalar (
        emitter, "buildrequires", YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Failed to emit dependencies buildrequires key: ");
          return FALSE;
        }

      ret = modulemd_yaml_emit_nested_set (
        emitter, self->buildtime_deps, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Failed to emit buildtime dependencies rpms: ");
          return FALSE;
        }
    }

  if (g_hash_table_size (self->runtime_deps) != 0)
    {
      ret = mmd_emitter_scalar (
        emitter, "requires", YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Failed to emit dependencies run-requires key: ");
          return FALSE;
        }

      ret = modulemd_yaml_emit_nested_set (
        emitter, self->runtime_deps, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error,
            g_steal_pointer (&nested_error),
            "Failed to emit runtime dependencies rpms: ");
          return FALSE;
        }
    }

  ret = mmd_emitter_end_mapping (emitter, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Failed to end dependencies mapping");
      return FALSE;
    }
  return TRUE;
}


static gboolean
requires_module_and_stream (GHashTable *modules,
                            const gchar *module_name,
                            const gchar *stream_name)
{
  GHashTable *streams = NULL;
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  g_autofree gchar *negated = NULL;

  streams = g_hash_table_lookup (modules, module_name);
  /* If the module doesn't appear at all, return false */
  if (!streams)
    {
      return FALSE;
    }

  /* Check whether this module is the empty set (which means "all streams") */
  if (g_hash_table_size (streams) == 0)
    {
      return TRUE;
    }

  /* Check whether it includes the stream name explicitly */
  if (g_hash_table_contains (streams, stream_name))
    {
      return TRUE;
    }


  /* Get the first item from the table and check if it's a negation */
  negated = g_strdup_printf ("-%s", stream_name);

  g_hash_table_iter_init (&iter, streams);

  /* We already checked that this hash table is not empty, so if iterating it
   * fails, something has gone horribly wrong. Wrap this in a
   * g_return_val_if_fail() to make static analysis happy.
   */
  g_return_val_if_fail (g_hash_table_iter_next (&iter, &key, &value), FALSE);

  /* If we have a negative value for any entry, they all must be negative.
   * Check whether we're explicitly excluding the requested stream */
  if (((const gchar *)key)[0] == '-' &&
      (!g_hash_table_contains (streams, negated)))
    {
      return TRUE;
    }

  return FALSE;
}


gboolean
modulemd_dependencies_requires_module_and_stream (ModulemdDependencies *self,
                                                  const gchar *module_name,
                                                  const gchar *stream_name)
{
  return requires_module_and_stream (
    self->runtime_deps, module_name, stream_name);
}


gboolean
modulemd_dependencies_buildrequires_module_and_stream (
  ModulemdDependencies *self,
  const gchar *module_name,
  const gchar *stream_name)
{
  return requires_module_and_stream (
    self->buildtime_deps, module_name, stream_name);
}
