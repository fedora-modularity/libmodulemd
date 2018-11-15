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

#include "modulemd-defaults-v1.h"

struct _ModulemdDefaultsV1
{
  GObject parent_instance;
};

G_DEFINE_TYPE (ModulemdDefaultsV1,
               modulemd_defaults_v1,
               MODULEMD_TYPE_DEFAULTS)

enum
{
  PROP_0,

  PROP_DEFAULT_STREAM,
  PROP_INTENT,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdDefaultsV1 *
modulemd_defaults_v1_new (const gchar *module_name)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_DEFAULTS_V1,
                       "module-name", module_name,
                       NULL);
  // clang-format on
}

static void
modulemd_defaults_v1_finalize (GObject *object)
{
  ModulemdDefaultsV1 *self = (ModulemdDefaultsV1 *)object;

  G_OBJECT_CLASS (modulemd_defaults_v1_parent_class)->finalize (object);
}

static guint64
modulemd_defaults_v1_get_mdversion (ModulemdDefaults *self)
{
  g_return_val_if_fail (MODULEMD_IS_DEFAULTS (self), 0);

  return MD_DEFAULTS_VERSION_ONE;
}


static void
modulemd_defaults_v1_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  ModulemdDefaultsV1 *self = MODULEMD_DEFAULTS_V1 (object);

  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_defaults_v1_set_property (GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  ModulemdDefaultsV1 *self = MODULEMD_DEFAULTS_V1 (object);

  switch (prop_id)
    {
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_defaults_v1_class_init (ModulemdDefaultsV1Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdDefaultsClass *defaults_class =
    MODULEMD_DEFAULTS_CLASS (object_class);

  object_class->finalize = modulemd_defaults_v1_finalize;
  object_class->get_property = modulemd_defaults_v1_get_property;
  object_class->set_property = modulemd_defaults_v1_set_property;

  defaults_class->get_mdversion = modulemd_defaults_v1_get_mdversion;
}

static void
modulemd_defaults_v1_init (ModulemdDefaultsV1 *self)
{
}
