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


void
modulemd_component_rpm_set_arches (ModulemdComponentRpm *self,
                                   ModulemdSimpleSet *arches)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));
  g_return_if_fail (!arches || MODULEMD_IS_SIMPLESET (arches));

  modulemd_simpleset_copy (arches, &self->arches);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ARCHES]);
}


ModulemdSimpleSet *
modulemd_component_rpm_get_arches (ModulemdComponentRpm *self)
{
  return modulemd_component_rpm_peek_arches (self);
}


ModulemdSimpleSet *
modulemd_component_rpm_peek_arches (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->arches;
}


ModulemdSimpleSet *
modulemd_component_rpm_dup_arches (ModulemdComponentRpm *self)
{
  ModulemdSimpleSet *arches = NULL;
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  modulemd_simpleset_copy (self->arches, &arches);
  return arches;
}


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


const gchar *
modulemd_component_rpm_get_cache (ModulemdComponentRpm *self)
{
  return modulemd_component_rpm_peek_cache (self);
}


const gchar *
modulemd_component_rpm_peek_cache (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->cache;
}


gchar *
modulemd_component_rpm_dup_cache (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return g_strdup (self->cache);
}


void
modulemd_component_rpm_set_multilib (ModulemdComponentRpm *self,
                                     ModulemdSimpleSet *multilib)
{
  g_return_if_fail (MODULEMD_IS_COMPONENT_RPM (self));
  g_return_if_fail (!multilib || MODULEMD_IS_SIMPLESET (multilib));

  modulemd_simpleset_copy (multilib, &self->multilib);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ARCHES]);
}


ModulemdSimpleSet *
modulemd_component_rpm_get_multilib (ModulemdComponentRpm *self)
{
  return modulemd_component_rpm_peek_multilib (self);
}


ModulemdSimpleSet *
modulemd_component_rpm_peek_multilib (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->multilib;
}


ModulemdSimpleSet *
modulemd_component_rpm_dup_multilib (ModulemdComponentRpm *self)
{
  ModulemdSimpleSet *multilib = NULL;
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  modulemd_simpleset_copy (self->multilib, &multilib);

  return multilib;
}


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


const gchar *
modulemd_component_rpm_get_ref (ModulemdComponentRpm *self)
{
  return modulemd_component_rpm_peek_ref (self);
}


const gchar *
modulemd_component_rpm_peek_ref (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->ref;
}


gchar *
modulemd_component_rpm_dup_ref (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return g_strdup (self->ref);
}


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


const gchar *
modulemd_component_rpm_get_repository (ModulemdComponentRpm *self)
{
  return modulemd_component_rpm_peek_repository (self);
}


const gchar *
modulemd_component_rpm_peek_repository (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return self->repo;
}


gchar *
modulemd_component_rpm_dup_repository (ModulemdComponentRpm *self)
{
  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  return g_strdup (self->repo);
}


static ModulemdComponent *
modulemd_component_rpm_copy (ModulemdComponent *self)
{
  ModulemdComponentRpm *old_component = NULL;
  ModulemdComponentRpm *new_component = NULL;

  if (!self)
    return NULL;

  g_return_val_if_fail (MODULEMD_IS_COMPONENT_RPM (self), NULL);

  old_component = MODULEMD_COMPONENT_RPM (self);

  new_component = modulemd_component_rpm_new ();

  modulemd_component_set_buildorder (
    MODULEMD_COMPONENT (new_component),
    modulemd_component_peek_buildorder (self));

  modulemd_component_set_name (MODULEMD_COMPONENT (new_component),
                               modulemd_component_peek_name (self));

  modulemd_component_set_rationale (MODULEMD_COMPONENT (new_component),
                                    modulemd_component_peek_rationale (self));

  modulemd_component_rpm_set_arches (
    new_component, modulemd_component_rpm_peek_arches (old_component));

  modulemd_component_rpm_set_cache (
    new_component, modulemd_component_rpm_peek_cache (old_component));

  modulemd_component_rpm_set_multilib (
    new_component, modulemd_component_rpm_peek_multilib (old_component));

  modulemd_component_rpm_set_ref (
    new_component, modulemd_component_rpm_peek_ref (old_component));

  modulemd_component_rpm_set_repository (
    new_component, modulemd_component_rpm_peek_repository (old_component));

  return MODULEMD_COMPONENT (new_component);
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
      g_value_set_object (value, modulemd_component_rpm_peek_arches (self));
      break;

    case PROP_CACHE:
      g_value_set_string (value, modulemd_component_rpm_peek_cache (self));
      break;

    case PROP_MULTILIB:
      g_value_set_object (value, modulemd_component_rpm_peek_multilib (self));
      break;

    case PROP_REF:
      g_value_set_string (value, modulemd_component_rpm_peek_ref (self));
      break;

    case PROP_REPO:
      g_value_set_string (value,
                          modulemd_component_rpm_peek_repository (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}

static void
modulemd_component_rpm_class_init (ModulemdComponentRpmClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdComponentClass *parent_class = MODULEMD_COMPONENT_CLASS (klass);

  object_class->finalize = modulemd_component_rpm_finalize;
  object_class->get_property = modulemd_component_rpm_get_property;
  object_class->set_property = modulemd_component_rpm_set_property;

  parent_class->copy = modulemd_component_rpm_copy;

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
