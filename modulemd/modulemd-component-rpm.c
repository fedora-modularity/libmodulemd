/* modulemd-component-rpm.c
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

#include "modulemd-component-rpm.h"

struct _ModulemdComponentRpm
{
  GObject parent_instance;

  /* == Members == */
  ModulemdSimpleSet *arches;
  gchar *cache;
  ModulemdSimpleSet *multilib;
  gchar *ref;
  gchar *repo;
};

G_DEFINE_TYPE (ModulemdComponentRpm,
               modulemd_component_rpm,
               MODULEMD_TYPE_COMPONENT)

enum
{
  PROP_0,

  PROP_ARCHES,
  PROP_CACHE,
  PROP_MULTILIB,
  PROP_REF,
  PROP_REPO,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdComponentRpm *
modulemd_component_rpm_new (void)
{
  return g_object_new (MODULEMD_TYPE_COMPONENT_RPM, NULL);
}

static void
modulemd_component_rpm_finalize (GObject *object)
{
  ModulemdComponentRpm *self = (ModulemdComponentRpm *)object;

  g_clear_pointer (&self->arches, g_object_unref);
  g_clear_pointer (&self->cache, g_free);
  g_clear_pointer (&self->multilib, g_object_unref);
  g_clear_pointer (&self->ref, g_free);
  g_clear_pointer (&self->repo, g_free);

  G_OBJECT_CLASS (modulemd_component_rpm_parent_class)->finalize (object);
}

/**
 * modulemd_component_rpm_set_arches:
 * @arches: a #ModuleSimpleSet: A set of architectures on which this RPM
 * package should be available. An empty set means  the package is available
 * on all supported architectures.
 */
void
modulemd_component_rpm_set_arches (ModulemdComponentRpm *self,
                                   ModulemdSimpleSet *arches)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));
  g_return_if_fail (MODULEMD_IS_SIMPLESET (arches));

  /* TODO: Test for differences before replacing */
  g_object_unref (self->arches);
  self->arches = g_object_ref (arches);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ARCHES]);
}

/**
 * modulemd_component_rpm_get_arches:
 *
 * Retrieves the set of arches for this component.
 *
 * Returns: (transfer full): A #ModulemdSimpleSet containing the set of
 * supported architectures for this component.
 */
ModulemdSimpleSet *
modulemd_component_rpm_get_arches (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return g_object_ref (self->arches);
}

/**
 * modulemd_component_rpm_set_cache
 * @cache: A string: The URL of the lookaside cache where this package's
 * sources are stored.
 */
void
modulemd_component_rpm_set_cache (ModulemdComponentRpm *self,
                                  const gchar *cache)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  if (g_strcmp0 (self->cache, cache) != 0)
    {
      g_free (self->cache);
      self->cache = g_strdup (cache);

      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CACHE]);
    }
}

/**
 * modulemd_component_rpm_get_cache:
 *
 * Retrieves the lookaside cache URL.
 *
 * Returns: A string containing the URL to the lookaside cache.
 */
const gchar *
modulemd_component_rpm_get_cache (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->cache;
}

/**
 * modulemd_component_rpm_set_multilib:
 * @multilib: a #ModuleSimpleSet: A set of architectures on which this RPM
 * package should be available as multilib. An empty set means the package is
 * not available as multilib on any architecture.
 */
void
modulemd_component_rpm_set_multilib (ModulemdComponentRpm *self,
                                     ModulemdSimpleSet *multilib)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));
  g_return_if_fail (MODULEMD_IS_SIMPLESET (multilib));

  /* TODO: Test for differences before replacing */
  g_object_unref (self->multilib);
  self->multilib = g_object_ref (multilib);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ARCHES]);
}

/**
 * modulemd_component_rpm_get_multilib:
 *
 * Retrieves the set of multilib for this component.
 *
 * Returns: (transfer full): A #ModulemdSimpleSet containing the set of
 * supported multilib architectures for this component.
 */
ModulemdSimpleSet *
modulemd_component_rpm_get_multilib (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return g_object_ref (self->multilib);
}

/**
 * modulemd_component_rpm_set_ref
 * @ref: A string: The particular repository commit hash, branch or tag name
 * used in this module.
 */
void
modulemd_component_rpm_set_ref (ModulemdComponentRpm *self, const gchar *ref)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  if (g_strcmp0 (self->ref, ref) != 0)
    {
      g_free (self->ref);
      self->ref = g_strdup (ref);

      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REF]);
    }
}

/**
 * modulemd_component_rpm_get_ref:
 *
 * Retrieves the repository ref.
 *
 * Returns: A string containing the repository ref.
 */
const gchar *
modulemd_component_rpm_get_ref (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->ref;
}

/**
 * modulemd_component_rpm_set_repository
 * @repository: A string: The VCS repository with the RPM SPEC file, patches and other
 * package data.
 */
void
modulemd_component_rpm_set_repository (ModulemdComponentRpm *self,
                                       const gchar *repository)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));

  if (g_strcmp0 (self->repo, repository) != 0)
    {
      g_free (self->repo);
      self->repo = g_strdup (repository);

      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_REPO]);
    }
}

/**
 * modulemd_component_rpm_get_repository:
 *
 * Retrieves the repository location.
 *
 * Returns: A string containing the repository location.
 */
const gchar *
modulemd_component_rpm_get_repository (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->repo;
}

static void
modulemd_component_rpm_set_property (GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdComponentRpm *self = MODULEMD_COMPONENT_RPM (object);

  switch (prop_id)
    {
    case PROP_ARCHES:
      modulemd_component_rpm_set_arches (self, g_value_get_object (value));
      break;

    case PROP_CACHE:
      modulemd_component_rpm_set_cache (self, g_value_get_string (value));
      break;

    case PROP_MULTILIB:
      modulemd_component_rpm_set_multilib (self, g_value_get_object (value));
      break;

    case PROP_REF:
      modulemd_component_rpm_set_ref (self, g_value_get_string (value));
      break;

    case PROP_REPO:
      modulemd_component_rpm_set_repository (self, g_value_get_string (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}

static void
modulemd_component_rpm_get_property (GObject *object,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdComponentRpm *self = MODULEMD_COMPONENT_RPM (object);

  switch (prop_id)
    {
    case PROP_ARCHES:
      g_value_set_object (value, modulemd_component_rpm_get_arches (self));
      break;

    case PROP_CACHE:
      g_value_set_string (value, modulemd_component_rpm_get_cache (self));
      break;

    case PROP_MULTILIB:
      g_value_set_object (value, modulemd_component_rpm_get_multilib (self));
      break;

    case PROP_REF:
      g_value_set_string (value, modulemd_component_rpm_get_ref (self));
      break;

    case PROP_REPO:
      g_value_set_string (value, modulemd_component_rpm_get_repository (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}

static void
modulemd_component_rpm_class_init (ModulemdComponentRpmClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_component_rpm_finalize;
  object_class->get_property = modulemd_component_rpm_get_property;
  object_class->set_property = modulemd_component_rpm_set_property;

  properties[PROP_ARCHES] =
    g_param_spec_object ("arches",
                         "Supported architectures",
                         "A set of architectures on which this RPM "
                         "package should be available. An empty set means "
                         "the package is available on all supported "
                         "architectures.",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_CACHE] =
    g_param_spec_string ("cache",
                         "Lookaside cache",
                         "The URL of the lookaside cache where this "
                         "package's sources are stored.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_MULTILIB] =
    g_param_spec_object ("multilib",
                         "Supported multilib architectures",
                         "A set of architectures on which this RPM "
                         "package should be available as multilib. What "
                         "this means varies from architecture to "
                         "architecture. An empty set is equal to no "
                         "multilib",
                         MODULEMD_TYPE_SIMPLESET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

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
                         "The VCS repository with the RPM SPEC file, "
                         "patches and other package data.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_component_rpm_init (ModulemdComponentRpm *self)
{
  self->arches = modulemd_simpleset_new ();
  self->multilib = modulemd_simpleset_new ();
}
