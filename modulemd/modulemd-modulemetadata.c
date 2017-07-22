/* modulemd-metadata.c
 *
 * Copyright (C) 2017 Stephen Gallagher
 * Copyright (C) 2017 Igor Gnatenko <ignatenkobrain@fedoraproject.org>
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

#include "modulemd-modulemetadata.h"

struct _ModulemdModuleMetadata
{
  GObject parent_instance;

  guint64 mdversion;
  gchar *name;
  gchar *stream;
  guint64 version;
  gchar *summary;
  gchar *description;
  gchar *community;
  gchar *documentation;
  gchar *tracker;
  GHashTable *buildrequires;
};

G_DEFINE_TYPE (ModulemdModuleMetadata, modulemd_modulemetadata, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_MDVERSION,
  PROP_NAME,
  PROP_STREAM,
  PROP_VERSION,
  PROP_SUMMARY,
  PROP_DESCRIPTION,
  PROP_COMMUNITY,
  PROP_DOCUMENTATION,
  PROP_TRACKER,
  PROP_BUILDREQUIRES,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * modulemd_modulemetadata_get_mdversion:
 *
 * Retrieves the "mdversion" for modulemd.
 *
 * Returns: A 64-bit unsigned integer containing the "mdversion" property.
 */
guint64
modulemd_modulemetadata_get_mdversion (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), 0);

  return self->mdversion;
}

/**
 * modulemd_modulemetadata_set_mdversion
 * @mdversion: the metadata version
 *
 * Sets the "mdversion" property.
 */
void
modulemd_modulemetadata_set_mdversion (ModulemdModuleMetadata *self,
                                       guint64                 mdversion)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));

  if (self->mdversion != mdversion)
    {
      self->mdversion = mdversion;
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_MDVERSION]);
    }
}

/**
 * modulemd_modulemetadata_get_name:
 *
 * Retrieves the "name" for modulemd.
 *
 * Returns: A string containing the "name" property.
 */
const gchar *
modulemd_modulemetadata_get_name (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), NULL);

  return self->name;
}

/**
 * modulemd_modulemetadata_set_name:
 * @name: the module name.
 *
 * Sets the "name" property.
 */
void
modulemd_modulemetadata_set_name (ModulemdModuleMetadata *self,
                                  const gchar            *name)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));

  if (g_strcmp0 (self->name, name))
    {
      g_free (self->name);
      self->name = g_strdup (name);
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_NAME]);
    }
}

/**
 * modulemd_modulemetadata_get_stream:
 *
 * Retrieves the "stream" for modulemd.
 *
 * Returns: A string containing the "stream" property.
 */
const gchar *
modulemd_modulemetadata_get_stream (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), NULL);

  return self->stream;
}

/**
 * modulemd_modulemetadata_set_stream:
 * @stream: the module stream.
 *
 * Sets the "stream" property.
 */
void
modulemd_modulemetadata_set_stream (ModulemdModuleMetadata *self,
                                    const gchar            *stream)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));

  if (g_strcmp0 (self->stream, stream))
    {
      g_free (self->stream);
      self->stream = g_strdup (stream);
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_STREAM]);
    }
}

/**
 * modulemd_modulemetadata_get_version:
 *
 * Retrieves the "version" for modulemd.
 *
 * Returns: A 64-bit unsigned integer containing the "version" property.
 */
guint64
modulemd_modulemetadata_get_version (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), 0);

  return self->version;
}

/**
 * modulemd_modulemetadata_set_version
 * @version: the module version
 *
 * Sets the "version" property.
 */
void
modulemd_modulemetadata_set_version (ModulemdModuleMetadata *self,
                                     guint64                 version)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));

  if (self->version != version)
    {
      self->version = version;
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_VERSION]);
    }
}

/**
 * modulemd_modulemetadata_get_summary:
 *
 * Retrieves the "summary" for modulemd.
 *
 * Returns: A string containing the "summary" property.
 */
const gchar *
modulemd_modulemetadata_get_summary (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), NULL);

  return self->summary;
}

/**
 * modulemd_modulemetadata_set_summary:
 * @summary: the module summary.
 *
 * Sets the "summary" property.
 */
void
modulemd_modulemetadata_set_summary (ModulemdModuleMetadata *self,
                                     const gchar            *summary)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));

  if (g_strcmp0 (self->summary, summary))
    {
      g_free (self->summary);
      self->summary = g_strdup (summary);
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_SUMMARY]);
    }
}

/**
 * modulemd_modulemetadata_get_description:
 *
 * Retrieves the "description" for modulemd.
 *
 * Returns: A string containing the "description" property.
 */
const gchar *
modulemd_modulemetadata_get_description (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), NULL);

  return self->description;
}

/**
 * modulemd_modulemetadata_set_description:
 * @description: the module description.
 *
 * Sets the "description" property.
 */
void
modulemd_modulemetadata_set_description (ModulemdModuleMetadata *self,
                                         const gchar            *description)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));

  if (g_strcmp0 (self->description, description))
    {
      g_free (self->description);
      self->description = g_strdup (description);
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_DESCRIPTION]);
    }
}

/**
 * modulemd_modulemetadata_get_community:
 *
 * Retrieves the "community" for modulemd.
 *
 * Returns: A string containing the "community" property.
 */
const gchar *
modulemd_modulemetadata_get_community (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), NULL);

  return self->community;
}

/**
 * modulemd_modulemetadata_set_community:
 * @community: the module community.
 *
 * Sets the "community" property.
 */
void
modulemd_modulemetadata_set_community (ModulemdModuleMetadata *self,
                                       const gchar            *community)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));

  if (g_strcmp0 (self->community, community))
    {
      g_free (self->community);
      self->community = g_strdup (community);
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_COMMUNITY]);
    }
}

/**
 * modulemd_modulemetadata_get_documentation:
 *
 * Retrieves the "documentation" for modulemd.
 *
 * Returns: A string containing the "documentation" property.
 */
const gchar *
modulemd_modulemetadata_get_documentation (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), NULL);

  return self->documentation;
}

/**
 * modulemd_modulemetadata_set_documentation:
 * @documentation: the module documentation.
 *
 * Sets the "documentation" property.
 */
void
modulemd_modulemetadata_set_documentation (ModulemdModuleMetadata *self,
                                           const gchar            *documentation)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));

  if (g_strcmp0 (self->documentation, documentation))
    {
      g_free (self->documentation);
      self->documentation = g_strdup (documentation);
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_DOCUMENTATION]);
    }
}

/**
 * modulemd_modulemetadata_get_tracker:
 *
 * Retrieves the "tracker" for modulemd.
 *
 * Returns: A string containing the "tracker" property.
 */
const gchar *
modulemd_modulemetadata_get_tracker (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), NULL);

  return self->tracker;
}

/**
 * modulemd_modulemetadata_set_tracker:
 * @tracker: the module tracker.
 *
 * Sets the "tracker" property.
 */
void
modulemd_modulemetadata_set_tracker (ModulemdModuleMetadata *self,
                                     const gchar            *tracker)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));

  if (g_strcmp0 (self->tracker, tracker))
    {
      g_free (self->tracker);
      self->tracker = g_strdup (tracker);
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_TRACKER]);
    }
}

/**
 * modulemd_modulemetadata_get_buildrequires:
 *
 * Retrieves the "buildrequires" for modulemd.
 *
 * Returns: (element-type utf8 utf8) (transfer container): A hash table
 * containing the "buildrequires" property.
 */
GHashTable *
modulemd_modulemetadata_get_buildrequires (ModulemdModuleMetadata *self)
{
  g_return_val_if_fail (MODULEMD_IS_MODULEMETADATA (self), NULL);

  return g_hash_table_ref (self->buildrequires);
}

/**
 * modulemd_modulemetadata_set_buildrequires:
 * @buildrequires: (element-type utf8 utf8): The requirements to build this module
 *
 * Sets the 'buildrequires' property.
 */
void
modulemd_modulemetadata_set_buildrequires (ModulemdModuleMetadata *self,
                                           GHashTable             *buildrequires)
{
  g_return_if_fail (MODULEMD_IS_MODULEMETADATA (self));
  g_return_if_fail (buildrequires);

  if (self->buildrequires != buildrequires)
    {
      g_hash_table_unref (self->buildrequires);
      self->buildrequires = g_hash_table_ref (buildrequires);
      g_object_notify_by_pspec (G_OBJECT (self),
                                properties [PROP_BUILDREQUIRES]);
    }
}

/**
 * modulemd_modulemetadata_new:
 *
 * Allocates a new #ModulemdModuleMetadata.
 *
 * Return value: a new #ModulemdModuleMetadata.
 */
ModulemdModuleMetadata *
modulemd_modulemetadata_new (void)
{
  return g_object_new (MODULEMD_TYPE_MODULEMETADATA, NULL);
}

static void
modulemd_modulemetadata_finalize (GObject *object)
{
  ModulemdModuleMetadata *self = (ModulemdModuleMetadata *)object;

  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->stream, g_free);
  g_clear_pointer (&self->summary, g_free);
  g_clear_pointer (&self->description, g_free);
  g_clear_pointer (&self->community, g_free);
  g_clear_pointer (&self->documentation, g_free);
  g_clear_pointer (&self->tracker, g_free);
  g_clear_pointer (&self->buildrequires, g_hash_table_unref);

  G_OBJECT_CLASS (modulemd_modulemetadata_parent_class)->finalize (object);
}

static void
modulemd_modulemetadata_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  ModulemdModuleMetadata *self = MODULEMD_MODULEMETADATA (object);

  switch (prop_id)
    {
    case PROP_MDVERSION:
      g_value_set_uint64 (value, modulemd_modulemetadata_get_mdversion (self));
      break;

    case PROP_NAME:
      g_value_set_string (value, modulemd_modulemetadata_get_name (self));
      break;

    case PROP_STREAM:
      g_value_set_string (value, modulemd_modulemetadata_get_stream (self));
      break;

    case PROP_VERSION:
      g_value_set_uint64 (value, modulemd_modulemetadata_get_version (self));
      break;

    case PROP_SUMMARY:
      g_value_set_string (value, modulemd_modulemetadata_get_summary (self));
      break;

    case PROP_DESCRIPTION:
      g_value_set_string (value, modulemd_modulemetadata_get_description (self));
      break;

    case PROP_COMMUNITY:
      g_value_set_string (value, modulemd_modulemetadata_get_community (self));
      break;

    case PROP_DOCUMENTATION:
      g_value_set_string (value, modulemd_modulemetadata_get_documentation (self));
      break;

    case PROP_TRACKER:
      g_value_set_string (value, modulemd_modulemetadata_get_tracker (self));
      break;

    case PROP_BUILDREQUIRES:
      g_value_set_boxed (value, modulemd_modulemetadata_get_buildrequires (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_modulemetadata_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  ModulemdModuleMetadata *self = MODULEMD_MODULEMETADATA (object);

  switch (prop_id)
    {
    case PROP_MDVERSION:
      modulemd_modulemetadata_set_mdversion (self, g_value_get_uint64 (value));
      break;

    case PROP_NAME:
      modulemd_modulemetadata_set_name (self, g_value_get_string (value));
      break;

    case PROP_STREAM:
      modulemd_modulemetadata_set_stream (self, g_value_get_string (value));
      break;

    case PROP_VERSION:
      modulemd_modulemetadata_set_version (self, g_value_get_uint64 (value));
      break;

    case PROP_SUMMARY:
      modulemd_modulemetadata_set_summary (self, g_value_get_string (value));
      break;

    case PROP_DESCRIPTION:
      modulemd_modulemetadata_set_description (self, g_value_get_string (value));
      break;

    case PROP_COMMUNITY:
      modulemd_modulemetadata_set_community (self, g_value_get_string (value));
      break;

    case PROP_DOCUMENTATION:
      modulemd_modulemetadata_set_documentation (self, g_value_get_string (value));
      break;

    case PROP_TRACKER:
      modulemd_modulemetadata_set_tracker (self, g_value_get_string (value));
      break;

    case PROP_BUILDREQUIRES:
      modulemd_modulemetadata_set_buildrequires (self, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_modulemetadata_class_init (ModulemdModuleMetadataClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_modulemetadata_finalize;
  object_class->get_property = modulemd_modulemetadata_get_property;
  object_class->set_property = modulemd_modulemetadata_set_property;

  properties [PROP_MDVERSION] =
    g_param_spec_uint64 ("mdversion",
                         "Module Metadata Version",
                         "An int property representing the metadata "
                         "format version used.",
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties [PROP_NAME] =
    g_param_spec_string ("name",
                         "Module Name",
                         "A string property representing the name of "
                         "the module.",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties [PROP_STREAM] =
    g_param_spec_string ("stream",
                         "Module Stream",
                         "A string property representing the stream name "
                         "of the module.",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties [PROP_VERSION] =
    g_param_spec_uint64 ("version",
                         "Module Version",
                         "An integer property representing the version of "
                         "the module.",
                         0,
                         G_MAXUINT64,
                         0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties [PROP_SUMMARY] =
    g_param_spec_string ("summary",
                         "Module Short Description",
                         "A string property representing a short summary "
                         "of the module.",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties [PROP_DESCRIPTION] =
    g_param_spec_string ("description",
                         "Module Description",
                         "A string property representing a detailed "
                         "description of the module.",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties [PROP_COMMUNITY] =
    g_param_spec_string ("community",
                         "Module Community",
                         "A string property representing a link to the "
                         "upstream community for this module.",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties [PROP_DOCUMENTATION] =
    g_param_spec_string ("documentation",
                         "Module Documentation",
                         "A string property representing a link to the "
                         "upstream documentation for this module.",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_TRACKER] =
    g_param_spec_string ("tracker",
                         "Module Bug Tracker",
                         "A string property representing a link to the "
                         "upstream bug tracker for this module.",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * ModulemdModuleMetadata:buildrequires: (type GLib.HashTable(utf8,utf8)) (transfer container)
   */
  properties [PROP_BUILDREQUIRES] =
    g_param_spec_boxed ("buildrequires",
                        "Module BuildRequires",
                        "A dictionary property representing the required "
                        "build dependencies of the module. Keys are the "
                        "required module names (strings), values are their "
                        "required stream names (also strings).",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_modulemetadata_init (ModulemdModuleMetadata *self)
{
  self->buildrequires = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               g_free, g_free);
}
