/* modulemd-component-module.c
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

#include "modulemd-component-module.h"

struct _ModulemdComponentModule
{
  GObject parent_instance;

  /* == Members == */
  gchar *ref;
  gchar *repo;
};

G_DEFINE_TYPE (ModulemdComponentModule,
               modulemd_component_module,
               MODULEMD_TYPE_COMPONENT)

enum
{
  PROP_0,

  PROP_REF,
  PROP_REPO,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdComponentModule *
modulemd_component_module_new (void)
{
  return g_object_new (MODULEMD_TYPE_COMPONENT_MODULE, NULL);
}

static void
modulemd_component_module_finalize (GObject *object)
{
  ModulemdComponentModule *self = (ModulemdComponentModule *)object;

  g_clear_pointer (&self->ref, g_free);
  g_clear_pointer (&self->repo, g_free);

  G_OBJECT_CLASS (modulemd_component_module_parent_class)->finalize (object);
}

/**
 * modulemd_component_module_set_ref
 * @ref: (nullable): A string: The particular repository commit hash, branch or tag name
 * used in this module.
 *
 * Since: 1.0
 */
void
modulemd_component_module_set_ref (ModulemdComponentModule *self,
                                   const gchar *ref)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_MODULE (self));

  if (g_strcmp0 (self->ref, ref) != 0)
    {
      g_free (self->ref);
      self->ref = g_strdup (ref);

      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REF]);
    }
}


/**
 * modulemd_component_module_get_ref:
 *
 * Retrieves the repository ref.
 *
 * Returns: A string containing the repository ref.
 *
 * Deprecated: 1.1
 * Use peek_ref() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_component_module_peek_ref)
const gchar *
modulemd_component_module_get_ref (ModulemdComponentModule *self)
{
  return modulemd_component_module_peek_ref (self);
}


/**
 * modulemd_component_module_peek_ref:
 *
 * Retrieves the repository ref.
 *
 * Returns: A string containing the repository ref.
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_module_peek_ref (ModulemdComponentModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_MODULE (self), NULL);

  return self->ref;
}


/**
 * modulemd_component_module_dup_ref:
 *
 * Retrieves a copy of the repository ref.
 *
 * Returns: A copy of the string containing the repository ref.
 *
 * Since: 1.1
 */
gchar *
modulemd_component_module_dup_ref (ModulemdComponentModule *self)
{
  return g_strdup (modulemd_component_module_peek_ref (self));
}


/**
 * modulemd_component_module_set_repository
 * @repository: (nullable): A string: The VCS repository with the modulemd file, and other
 * module data.
 *
 * Since: 1.0
 */
void
modulemd_component_module_set_repository (ModulemdComponentModule *self,
                                          const gchar *repository)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_MODULE (self));

  if (g_strcmp0 (self->repo, repository) != 0)
    {
      g_free (self->repo);
      self->repo = g_strdup (repository);

      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REPO]);
    }
}


/**
 * modulemd_component_module_get_repository:
 *
 * Retrieves the repository location.
 *
 * Returns: A string containing the repository location.
 *
 * Deprecated: 1.1
 * Use peek_repository() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_component_module_peek_repository)
const gchar *
modulemd_component_module_get_repository (ModulemdComponentModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_MODULE (self), NULL);

  return self->repo;
}


/**
 * modulemd_component_module_peek_repository:
 *
 * Retrieves the repository location.
 *
 * Returns: A string containing the repository location.
 *
 * Since: 1.1
 */
const gchar *
modulemd_component_module_peek_repository (ModulemdComponentModule *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_MODULE (self), NULL);

  return self->repo;
}


/**
 * modulemd_component_module_dup_repository:
 *
 * Retrieves a copy of the repository location.
 *
 * Returns: A copy of the string containing the repository location.
 *
 * Since: 1.1
 */
gchar *
modulemd_component_module_dup_repository (ModulemdComponentModule *self)
{
  return g_strdup (modulemd_component_module_peek_repository (self));
}


static ModulemdComponent *
modulemd_component_module_copy (ModulemdComponent *self)
{
  ModulemdComponentModule *old_component = NULL;
  ModulemdComponentModule *new_component = NULL;

  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_MODULE (self), NULL);

  old_component = MODULEMD_COMPONENT_MODULE (self);

  new_component = modulemd_component_module_new ();

  modulemd_component_set_buildorder (
    MODULEMD_COMPONENT (new_component),
    modulemd_component_peek_buildorder (self));

  modulemd_component_set_name (MODULEMD_COMPONENT (new_component),
                               modulemd_component_peek_name (self));

  modulemd_component_set_rationale (MODULEMD_COMPONENT (new_component),
                                    modulemd_component_peek_rationale (self));

  modulemd_component_module_set_ref (
    new_component, modulemd_component_module_peek_ref (old_component));

  modulemd_component_module_set_repository (
    new_component, modulemd_component_module_peek_repository (old_component));

  return MODULEMD_COMPONENT (new_component);
}


static void
modulemd_component_module_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
  ModulemdComponentModule *self = MODULEMD_COMPONENT_MODULE (object);

  switch (prop_id)
    {
    case PROP_REF:
      modulemd_component_module_set_ref (self, g_value_get_string (value));
      break;

    case PROP_REPO:
      modulemd_component_module_set_repository (self,
                                                g_value_get_string (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}

static void
modulemd_component_module_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
  ModulemdComponentModule *self = MODULEMD_COMPONENT_MODULE (object);

  switch (prop_id)
    {
    case PROP_REF:
      g_value_set_string (value, modulemd_component_module_peek_ref (self));
      break;

    case PROP_REPO:
      g_value_set_string (value,
                          modulemd_component_module_peek_repository (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}

static void
modulemd_component_module_class_init (ModulemdComponentModuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdComponentClass *parent_class = MODULEMD_COMPONENT_CLASS (klass);

  object_class->finalize = modulemd_component_module_finalize;
  object_class->get_property = modulemd_component_module_get_property;
  object_class->set_property = modulemd_component_module_set_property;

  parent_class->copy = modulemd_component_module_copy;

  properties[PROP_REF] =
    g_param_spec_string ("ref",
                         "git <commit-ish>",
                         "The particular repository commit hash, branch "
                         "or tag name used in this module.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_REPO] =
    g_param_spec_string ("repository",
                         "VCS repository",
                         "The VCS repository with the modulemd file, "
                         "and other module data.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_component_module_init (ModulemdComponentModule *self)
{
}
