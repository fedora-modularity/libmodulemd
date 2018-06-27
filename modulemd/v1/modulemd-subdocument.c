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

#include "modulemd-subdocument.h"
#include "private/modulemd-subdocument-private.h"


/**
 * SECTION: modulemd-subdocument
 * @title: Modulemd.Subdocument
 * @short_description: Contains information about individual YAML subdocuments
 * being parsed for modulemd information.
 */


struct _ModulemdSubdocument
{
  GObject parent_instance;

  GType doctype;
  guint64 version;
  gchar *yaml;
  GError *gerror;
};

G_DEFINE_TYPE (ModulemdSubdocument, modulemd_subdocument, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_YAML,
  PROP_GERROR,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdSubdocument *
modulemd_subdocument_new (void)
{
  return g_object_new (MODULEMD_TYPE_SUBDOCUMENT, NULL);
}

static void
modulemd_subdocument_finalize (GObject *object)
{
  ModulemdSubdocument *self = (ModulemdSubdocument *)object;

  g_clear_pointer (&self->yaml, g_free);
  g_clear_error (&self->gerror);

  G_OBJECT_CLASS (modulemd_subdocument_parent_class)->finalize (object);
}

void
modulemd_subdocument_set_doctype (ModulemdSubdocument *self, const GType type)
{
  g_return_if_fail (MODULEMD_IS_SUBDOCUMENT (self));

  self->doctype = type;
}


/**
 * modulemd_subdocument_get_doctype:
 *
 * Returns: A #GType of the GObject that represents this subdocument
 *
 * Since: 1.4
 */
const GType
modulemd_subdocument_get_doctype (ModulemdSubdocument *self)
{
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT (self), G_TYPE_INVALID);

  return self->doctype;
}

void
modulemd_subdocument_set_version (ModulemdSubdocument *self,
                                  const guint64 version)
{
  g_return_if_fail (MODULEMD_IS_SUBDOCUMENT (self));

  self->version = version;
}


/**
 * modulemd_subdocument_get_version:
 *
 * Returns: A 64-bit integer describing the document version
 *
 * Since: 1.4
 */
const GType
modulemd_subdocument_get_version (ModulemdSubdocument *self)
{
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT (self), 0);

  return self->version;
}


void
modulemd_subdocument_set_yaml (ModulemdSubdocument *self, const gchar *yaml)
{
  g_return_if_fail (MODULEMD_IS_SUBDOCUMENT (self));

  g_clear_pointer (&self->yaml, g_free);

  if (yaml)
    {
      self->yaml = g_strdup (yaml);
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_YAML]);
}


/**
 * modulemd_subdocument_get_yaml:
 *
 * Returns: A string containing the YAML document
 *
 * Since: 1.4
 */
const gchar *
modulemd_subdocument_get_yaml (ModulemdSubdocument *self)
{
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT (self), NULL);

  return self->yaml;
}


void
modulemd_subdocument_set_gerror (ModulemdSubdocument *self,
                                 const GError *gerror)
{
  g_return_if_fail (MODULEMD_IS_SUBDOCUMENT (self));

  g_clear_error (&self->gerror);
  if (gerror)
    self->gerror = g_error_copy (gerror);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_GERROR]);
}


/**
 * modulemd_subdocument_get_gerror:
 *
 * Returns: The #GError associated with this subdocument
 *
 * Since: 1.4
 */
const GError *
modulemd_subdocument_get_gerror (ModulemdSubdocument *self)
{
  g_return_val_if_fail (MODULEMD_IS_SUBDOCUMENT (self), NULL);

  return self->gerror;
}


static void
modulemd_subdocument_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  ModulemdSubdocument *self = MODULEMD_SUBDOCUMENT (object);

  switch (prop_id)
    {
    case PROP_YAML:
      g_value_set_string (value, modulemd_subdocument_get_yaml (self));
      break;

    case PROP_GERROR:
      g_value_set_boxed (value, modulemd_subdocument_get_gerror (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}


static void
modulemd_subdocument_set_property (GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}


static void
modulemd_subdocument_class_init (ModulemdSubdocumentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_subdocument_finalize;
  object_class->get_property = modulemd_subdocument_get_property;
  object_class->set_property = modulemd_subdocument_set_property;

  properties[PROP_YAML] =
    g_param_spec_string ("yaml",
                         "YAML document string",
                         "A string containing the YAML subdocument",
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  properties[PROP_GERROR] =
    g_param_spec_boxed ("gerror",
                        "Error information",
                        "A GError containing an error code and message about "
                        "why this subdocument failed parsing",
                        G_TYPE_ERROR,
                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_subdocument_init (ModulemdSubdocument *self)
{
  self->doctype = G_TYPE_INVALID;
  self->version = 0;
}
