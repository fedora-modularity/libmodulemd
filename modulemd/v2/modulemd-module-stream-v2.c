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
#include "modulemd-module-stream-v2.h"

struct _ModulemdModuleStreamV2
{
  GObject parent_instance;
};

G_DEFINE_TYPE (ModulemdModuleStreamV2,
               modulemd_module_stream_v2,
               MODULEMD_TYPE_MODULE_STREAM)

enum
{
  PROP_0,
  N_PROPS
};

ModulemdModuleStreamV2 *
modulemd_module_stream_v2_new (const gchar *module_name,
                               const gchar *module_stream)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_MODULE_STREAM_V2,
                       "module-name", module_name,
                       "stream-name", module_stream,
                       NULL);
  // clang-format on
}


static guint64
modulemd_module_stream_v2_get_mdversion (ModulemdModuleStream *self)
{
  return MD_MODULESTREAM_VERSION_TWO;
}


static void
modulemd_module_stream_v2_finalize (GObject *object)
{
  G_OBJECT_CLASS (modulemd_module_stream_v2_parent_class)->finalize (object);
}

static void
modulemd_module_stream_v2_get_property (GObject *object,
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
modulemd_module_stream_v2_set_property (GObject *object,
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
modulemd_module_stream_v2_class_init (ModulemdModuleStreamV2Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ModulemdModuleStreamClass *stream_class =
    MODULEMD_MODULE_STREAM_CLASS (object_class);

  object_class->finalize = modulemd_module_stream_v2_finalize;
  object_class->get_property = modulemd_module_stream_v2_get_property;
  object_class->set_property = modulemd_module_stream_v2_set_property;

  stream_class->get_mdversion = modulemd_module_stream_v2_get_mdversion;
}

static void
modulemd_module_stream_v2_init (ModulemdModuleStreamV2 *self)
{
}
