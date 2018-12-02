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

#include "modulemd-module.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

struct _ModulemdModule
{
  GObject parent_instance;

  gchar *module_name;

  GPtrArray *streams;
  ModulemdDefaults *defaults;
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
    g_ptr_array_add (m->streams, g_ptr_array_index (self->streams, i));

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
}


void
modulemd_module_set_defaults (ModulemdModule *self, ModulemdDefaults *defaults)
{
  g_return_if_fail (MODULEMD_IS_MODULE (self));

  g_clear_object (&self->defaults);

  if (defaults == NULL)
    return;

  if (!g_str_equal (modulemd_defaults_get_module_name (defaults),
                    modulemd_module_get_module_name (self)))
    return; /* Silently ignore it */

  self->defaults = modulemd_defaults_copy (defaults);
}


ModulemdDefaults *
modulemd_module_get_defaults (ModulemdModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  return self->defaults;
}


void
modulemd_module_add_stream (ModulemdModule *self, ModulemdModuleStream *stream)
{
  ModulemdModuleStream *old = NULL;
  g_return_if_fail (MODULEMD_IS_MODULE (self));
  g_return_if_fail (stream);

  old = modulemd_module_get_stream_by_NSVC (
    self,
    modulemd_module_stream_get_stream_name (stream),
    modulemd_module_stream_get_version (stream),
    modulemd_module_stream_get_context (stream));
  if (old != NULL)
    {
      /* First, drop the existing stream */
      g_ptr_array_remove (self->streams, old);
      old = NULL;
    }

  g_ptr_array_add (self->streams,
                   modulemd_module_stream_copy (stream, NULL, NULL));
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
  ModulemdModuleStream *a_ = *(ModulemdModuleStream **)a;
  ModulemdModuleStream *b_ = *(ModulemdModuleStream **)b;

  return modulemd_module_stream_get_version (b_) -
         modulemd_module_stream_get_version (a_);
}


GPtrArray *
modulemd_module_get_streams_by_stream_name_as_list (ModulemdModule *self,
                                                    const gchar *stream_name)
{
  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  gsize i = 0;
  ModulemdModuleStream *stream = NULL;
  g_autoptr (GPtrArray) streams =
    g_ptr_array_new_with_free_func (g_object_unref);

  for (i = 0; i < self->streams->len; i++)
    {
      stream = (ModulemdModuleStream *)g_ptr_array_index (self->streams, i);

      if (!g_str_equal (modulemd_module_stream_get_stream_name (stream),
                        stream_name))
        continue;

      g_ptr_array_add (streams, g_object_ref (stream));
    }

  g_ptr_array_sort (streams, compare_streams);

  return g_steal_pointer (&streams);
}


ModulemdModuleStream *
modulemd_module_get_stream_by_NSVC (ModulemdModule *self,
                                    const gchar *stream_name,
                                    const guint64 version,
                                    const gchar *context)
{
  gsize i = 0;
  ModulemdModuleStream *under_consideration = NULL;

  g_return_val_if_fail (MODULEMD_IS_MODULE (self), NULL);

  for (i = 0; i < self->streams->len; i++)
    {
      under_consideration =
        (ModulemdModuleStream *)g_ptr_array_index (self->streams, i);

      if (!g_str_equal (
            modulemd_module_stream_get_stream_name (under_consideration),
            stream_name))
        continue;

      if (modulemd_module_stream_get_version (under_consideration) != version)
        continue;

      if (!g_str_equal (
            modulemd_module_stream_get_context (under_consideration), context))
        continue;

      return under_consideration;
    }

  return NULL;
}
