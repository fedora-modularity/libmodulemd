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

#include "modulemd-module-stream.h"
#include "modulemd-module-stream-v1.h"

struct _ModulemdModuleStreamV1
{
  GObject parent_instance;
};

G_DEFINE_TYPE (ModulemdModuleStreamV1,
               modulemd_module_stream_v1,
               MODULEMD_TYPE_MODULE_STREAM)

enum
{
  PROP_0,
  N_PROPS
};

ModulemdModuleStreamV1 *
modulemd_module_stream_v1_new (const gchar *module_name,
                               const gchar *module_stream)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_MODULE_STREAM_V1,
                       "module-name", module_name,
                       "stream-name", module_stream,
                       NULL);
  // clang-format on
}


static guint64
modulemd_module_stream_v1_get_mdversion (ModulemdModuleStream *self)
{
  return MD_MODULESTREAM_VERSION_ONE;
}


static void
modulemd_module_stream_v1_finalize (GObject *object)
{
  G_OBJECT_CLASS (modulemd_module_stream_v1_parent_class)->finalize (object);
}

static void
modulemd_module_stream_v1_get_property (GObject *object,
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
modulemd_module_stream_v1_set_property (GObject *object,
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
modulemd_module_stream_v1_class_init (ModulemdModuleStreamV1Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdModuleStreamClass *stream_class =
    MODULEMD_MODULE_STREAM_CLASS (object_class);

  object_class->finalize = modulemd_module_stream_v1_finalize;
  object_class->get_property = modulemd_module_stream_v1_get_property;
  object_class->set_property = modulemd_module_stream_v1_set_property;

  stream_class->get_mdversion = modulemd_module_stream_v1_get_mdversion;
}

static void
modulemd_module_stream_v1_init (ModulemdModuleStreamV1 *self)
{
}
