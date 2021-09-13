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
#include <inttypes.h>
#include <yaml.h>

#include "modulemd-errors.h"
#include "modulemd-obsoletes.h"
#include "private/modulemd-obsoletes-private.h"
#include "private/modulemd-util.h"

#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define O_DEFAULT_STRING "__obsoletes_VALUE_UNSET__"
#define O_PLACEHOLDER_STRING "__obsoletes_VALUE_NOT_YET_SET__"

struct _ModulemdObsoletes
{
  GObject parent_instance;

  guint64 mdversion;
  guint64 modified;
  gboolean reset;
  gchar *module_name;
  gchar *module_stream;
  gchar *module_context;
  guint64 eol_date;
  gchar *message;

  /* Stream is obsoleted by exactly one other stream */
  gchar *obsoleted_by_module_name;
  gchar *obsoleted_by_module_stream;
};

G_DEFINE_TYPE (ModulemdObsoletes, modulemd_obsoletes, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_MDVERSION,
  PROP_MODIFIED,
  PROP_RESET,
  PROP_MODULE_NAME,
  PROP_MODULE_STREAM,
  PROP_MODULE_CONTEXT,
  PROP_EOL_DATE,
  PROP_MESSAGE,
  PROP_OBSOLETED_BY_MODULE_NAME,
  PROP_OBSOLETED_BY_MODULE_STREAM,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdObsoletes *
modulemd_obsoletes_new (guint64 mdversion,
                        guint64 modified,
                        const gchar *module_name,
                        const gchar *module_stream,
                        const gchar *message)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_OBSOLETES,
                       "mdversion", mdversion,
                       "modified", modified,
                       "module-name", module_name,
                       "module-stream", module_stream,
                       "message", message,
                       NULL);
  // clang-format on
}


ModulemdObsoletes *
modulemd_obsoletes_copy (ModulemdObsoletes *self)
{
  g_autoptr (ModulemdObsoletes) o = NULL;

  g_return_val_if_fail (MODULEMD_IS_OBSOLETES (self), NULL);


  o = modulemd_obsoletes_new (modulemd_obsoletes_get_mdversion (self),
                              modulemd_obsoletes_get_modified (self),
                              modulemd_obsoletes_get_module_name (self),
                              modulemd_obsoletes_get_module_stream (self),
                              modulemd_obsoletes_get_message (self));
  modulemd_obsoletes_set_module_context (
    o, modulemd_obsoletes_get_module_context (self));
  modulemd_obsoletes_set_reset (o, modulemd_obsoletes_get_reset (self));
  modulemd_obsoletes_set_eol_date (o, modulemd_obsoletes_get_eol_date (self));
  modulemd_obsoletes_set_obsoleted_by_module_name (
    o, modulemd_obsoletes_get_obsoleted_by_module_name (self));
  modulemd_obsoletes_set_obsoleted_by_module_stream (
    o, modulemd_obsoletes_get_obsoleted_by_module_stream (self));

  return g_steal_pointer (&o);
}

gboolean
modulemd_obsoletes_validate (ModulemdObsoletes *self, GError **error)
{
  g_return_val_if_fail (MODULEMD_IS_OBSOLETES (self), FALSE);
  guint64 mdversion = modulemd_obsoletes_get_mdversion (self);

  if (!mdversion)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Metadata version is unset.");
      return FALSE;
    }
  if (mdversion > MD_OBSOLETES_VERSION_LATEST)
    {
      g_set_error (error,
                   MODULEMD_ERROR,
                   MMD_ERROR_VALIDATE,
                   "Metadata version unknown: %" PRIu64 ".",
                   mdversion);
      return FALSE;
    }

  if (modulemd_obsoletes_get_modified (self) == 0)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Obsoletes modified is empty.");
      return FALSE;
    }

  if (!g_strcmp0 (modulemd_obsoletes_get_module_name (self),
                  O_PLACEHOLDER_STRING) ||
      modulemd_obsoletes_get_module_name (self)[0] == '\0')
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Obsoletes module name is unset.");
      return FALSE;
    }

  if (!g_strcmp0 (modulemd_obsoletes_get_module_stream (self),
                  O_PLACEHOLDER_STRING) ||
      modulemd_obsoletes_get_module_stream (self)[0] == '\0')
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Obsoletes stream is unset.");
      return FALSE;
    }

  if (!g_strcmp0 (modulemd_obsoletes_get_message (self),
                  O_PLACEHOLDER_STRING) ||
      !g_strcmp0 (modulemd_obsoletes_get_message (self), O_DEFAULT_STRING) ||
      modulemd_obsoletes_get_message (self)[0] == '\0')
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Obsoletes message is unset.");
      return FALSE;
    }

  /* It should not be possible to set reset and eol_date
   * at the same time */
  if (modulemd_obsoletes_get_reset (self) &&
      modulemd_obsoletes_get_eol_date (self))
    {
      g_set_error_literal (
        error,
        MODULEMD_ERROR,
        MMD_ERROR_VALIDATE,
        "Obsoletes cannot have both eol_date and reset attributes set.");
      return FALSE;
    }
  /* It should not be possible to set reset and obsoleted_by*
   * at the same time */
  if (modulemd_obsoletes_get_reset (self) &&
      (modulemd_obsoletes_get_obsoleted_by_module_name (self) ||
       modulemd_obsoletes_get_obsoleted_by_module_stream (self)))
    {
      g_set_error_literal (
        error,
        MODULEMD_ERROR,
        MMD_ERROR_VALIDATE,
        "Obsoletes cannot have both obsoleted_by and reset attributes set.");
      return FALSE;
    }

  if ((modulemd_obsoletes_get_obsoleted_by_module_name (self) &&
       !modulemd_obsoletes_get_obsoleted_by_module_stream (self)) ||
      (!modulemd_obsoletes_get_obsoleted_by_module_name (self) &&
       modulemd_obsoletes_get_obsoleted_by_module_stream (self)))
    {
      g_set_error_literal (
        error,
        MODULEMD_ERROR,
        MMD_ERROR_VALIDATE,
        "Obsoletes obsoleted by module name and module stream "
        "have to be set together.");
      return FALSE;
    }

  return TRUE;
}

static void
modulemd_obsoletes_finalize (GObject *object)
{
  ModulemdObsoletes *self = (ModulemdObsoletes *)object;

  g_clear_pointer (&self->module_name, g_free);
  g_clear_pointer (&self->module_stream, g_free);
  g_clear_pointer (&self->module_context, g_free);
  g_clear_pointer (&self->message, g_free);
  g_clear_pointer (&self->obsoleted_by_module_name, g_free);
  g_clear_pointer (&self->obsoleted_by_module_stream, g_free);

  G_OBJECT_CLASS (modulemd_obsoletes_parent_class)->finalize (object);
}

guint64
modulemd_obsoletes_get_mdversion (ModulemdObsoletes *self)
{
  return self->mdversion;
}


guint64
modulemd_obsoletes_get_modified (ModulemdObsoletes *self)
{
  return self->modified;
}


gboolean
modulemd_obsoletes_get_reset (ModulemdObsoletes *self)
{
  return self->reset;
}


const gchar *
modulemd_obsoletes_get_module_name (ModulemdObsoletes *self)
{
  return self->module_name;
}


const gchar *
modulemd_obsoletes_get_module_stream (ModulemdObsoletes *self)
{
  return self->module_stream;
}


const gchar *
modulemd_obsoletes_get_module_context (ModulemdObsoletes *self)
{
  return self->module_context;
}


guint64
modulemd_obsoletes_get_eol_date (ModulemdObsoletes *self)
{
  return self->eol_date;
}


const gchar *
modulemd_obsoletes_get_message (ModulemdObsoletes *self)
{
  return self->message;
}


const gchar *
modulemd_obsoletes_get_obsoleted_by_module_name (ModulemdObsoletes *self)
{
  return self->obsoleted_by_module_name;
}


const gchar *
modulemd_obsoletes_get_obsoleted_by_module_stream (ModulemdObsoletes *self)
{
  return self->obsoleted_by_module_stream;
}


static void
modulemd_obsoletes_set_mdversion (ModulemdObsoletes *self, guint64 mdversion)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));
  g_return_if_fail (mdversion != 0);

  self->mdversion = mdversion;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MDVERSION]);
}

void
modulemd_obsoletes_set_modified (ModulemdObsoletes *self, guint64 modified)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));

  self->modified = modified;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODIFIED]);
}


void
modulemd_obsoletes_set_reset (ModulemdObsoletes *self, gboolean reset)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));

  self->reset = reset;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_RESET]);
}


static void
modulemd_obsoletes_set_module_name (ModulemdObsoletes *self,
                                    const gchar *module_name)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));
  g_return_if_fail (module_name);
  g_return_if_fail (g_strcmp0 (module_name, O_DEFAULT_STRING));

  g_clear_pointer (&self->module_name, g_free);
  self->module_name = g_strdup (module_name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_NAME]);
}


static void
modulemd_obsoletes_set_module_stream (ModulemdObsoletes *self,
                                      const gchar *module_stream)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));
  g_return_if_fail (module_stream);
  g_return_if_fail (g_strcmp0 (module_stream, O_DEFAULT_STRING));

  g_clear_pointer (&self->module_stream, g_free);
  self->module_stream = g_strdup (module_stream);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_STREAM]);
}


void
modulemd_obsoletes_set_module_context (ModulemdObsoletes *self,
                                       const gchar *module_context)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));

  g_clear_pointer (&self->module_context, g_free);
  self->module_context = g_strdup (module_context);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MODULE_CONTEXT]);
}

void
modulemd_obsoletes_set_eol_date (ModulemdObsoletes *self, guint64 eol_date)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));

  self->eol_date = eol_date;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EOL_DATE]);
}


void
modulemd_obsoletes_set_message (ModulemdObsoletes *self, const gchar *message)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));
  g_return_if_fail (message);

  g_clear_pointer (&self->message, g_free);
  self->message = g_strdup (message);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MESSAGE]);
}

void
modulemd_obsoletes_set_obsoleted_by_module_name (
  ModulemdObsoletes *self, const gchar *obsoleted_by_module_name)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));

  g_clear_pointer (&self->obsoleted_by_module_name, g_free);
  self->obsoleted_by_module_name = g_strdup (obsoleted_by_module_name);

  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_OBSOLETED_BY_MODULE_NAME]);
}

void
modulemd_obsoletes_set_obsoleted_by_module_stream (
  ModulemdObsoletes *self, const gchar *obsoleted_by_module_stream)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));

  g_clear_pointer (&self->obsoleted_by_module_stream, g_free);
  self->obsoleted_by_module_stream = g_strdup (obsoleted_by_module_stream);

  g_object_notify_by_pspec (G_OBJECT (self),
                            properties[PROP_OBSOLETED_BY_MODULE_STREAM]);
}

void
modulemd_obsoletes_set_obsoleted_by (ModulemdObsoletes *self,
                                     const gchar *obsoleted_by_module_name,
                                     const gchar *obsoleted_by_module_stream)
{
  g_return_if_fail (MODULEMD_IS_OBSOLETES (self));

  modulemd_obsoletes_set_obsoleted_by_module_name (self,
                                                   obsoleted_by_module_name);
  modulemd_obsoletes_set_obsoleted_by_module_stream (
    self, obsoleted_by_module_stream);
}


static void
modulemd_obsoletes_get_property (GObject *object,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdObsoletes *self = MODULEMD_OBSOLETES (object);

  switch (prop_id)
    {
    case PROP_MDVERSION:
      g_value_set_uint64 (value, modulemd_obsoletes_get_mdversion (self));
      break;
    case PROP_MODIFIED:
      g_value_set_uint64 (value, modulemd_obsoletes_get_modified (self));
      break;
    case PROP_RESET:
      g_value_set_boolean (value, modulemd_obsoletes_get_reset (self));
      break;
    case PROP_MODULE_NAME:
      g_value_set_string (value, modulemd_obsoletes_get_module_name (self));
      break;
    case PROP_MODULE_STREAM:
      g_value_set_string (value, modulemd_obsoletes_get_module_stream (self));
      break;
    case PROP_MODULE_CONTEXT:
      g_value_set_string (value, modulemd_obsoletes_get_module_context (self));
      break;
    case PROP_EOL_DATE:
      g_value_set_uint64 (value, modulemd_obsoletes_get_eol_date (self));
      break;
    case PROP_MESSAGE:
      g_value_set_string (value, modulemd_obsoletes_get_message (self));
      break;
    case PROP_OBSOLETED_BY_MODULE_NAME:
      g_value_set_string (
        value, modulemd_obsoletes_get_obsoleted_by_module_name (self));
      break;
    case PROP_OBSOLETED_BY_MODULE_STREAM:
      g_value_set_string (
        value, modulemd_obsoletes_get_obsoleted_by_module_stream (self));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_obsoletes_set_property (GObject *object,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  ModulemdObsoletes *self = MODULEMD_OBSOLETES (object);

  switch (prop_id)
    {
    case PROP_MDVERSION:
      modulemd_obsoletes_set_mdversion (self, g_value_get_uint64 (value));
      break;
    case PROP_MODIFIED:
      modulemd_obsoletes_set_modified (self, g_value_get_uint64 (value));
      break;
    case PROP_RESET:
      modulemd_obsoletes_set_reset (self, g_value_get_boolean (value));
      break;
    case PROP_MODULE_NAME:
      modulemd_obsoletes_set_module_name (self, g_value_get_string (value));
      break;
    case PROP_MODULE_STREAM:
      modulemd_obsoletes_set_module_stream (self, g_value_get_string (value));
      break;
    case PROP_MODULE_CONTEXT:
      modulemd_obsoletes_set_module_context (self, g_value_get_string (value));
      break;
    case PROP_EOL_DATE:
      modulemd_obsoletes_set_eol_date (self, g_value_get_uint64 (value));
      break;
    case PROP_MESSAGE:
      modulemd_obsoletes_set_message (self, g_value_get_string (value));
      break;
    case PROP_OBSOLETED_BY_MODULE_NAME:
      modulemd_obsoletes_set_obsoleted_by_module_name (
        self, g_value_get_string (value));
      break;
    case PROP_OBSOLETED_BY_MODULE_STREAM:
      modulemd_obsoletes_set_obsoleted_by_module_stream (
        self, g_value_get_string (value));
      break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_obsoletes_class_init (ModulemdObsoletesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_obsoletes_finalize;
  object_class->get_property = modulemd_obsoletes_get_property;
  object_class->set_property = modulemd_obsoletes_set_property;

  properties[PROP_MDVERSION] = g_param_spec_uint64 (
    "mdversion",
    "Metadata Version",
    "The metadata version of this obsoletes object.",
    0,
    G_MAXUINT64,
    0,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_MODIFIED] = g_param_spec_uint64 (
    "modified",
    "Modified",
    "The last modified time represented as a 64-bit integer "
    "(such as 201807011200)",
    0,
    G_MAXUINT64,
    0,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_MODULE_NAME] = g_param_spec_string (
    "module-name",
    "Module name",
    "The name of the module to which this obsoletes applies.",
    O_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_RESET] = g_param_spec_boolean (
    "override-previous",
    "Override previous",
    "A boolean option to cancel/reset all previously specified obsoletes.",
    FALSE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_MODULE_STREAM] = g_param_spec_string (
    "module-stream",
    "Module stream",
    "The name of the module stream to which this obsoletes applies.",
    O_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_MODULE_CONTEXT] = g_param_spec_string (
    "module-context",
    "Module context",
    "The name of the module context to which this obsoletes applies.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_EOL_DATE] = g_param_spec_uint64 (
    "eol-date",
    "EOL date",
    "A string representing UTC date in ISO 8601 format: YYYY-MM-DDTHH:MMZ",
    0,
    G_MAXUINT64,
    0,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_MESSAGE] = g_param_spec_string (
    "message",
    "Message",
    "A string describing the change, reason, etc.",
    O_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_OBSOLETED_BY_MODULE_NAME] = g_param_spec_string (
    "obsoleted-by-module-name",
    "Obsoleted by module name",
    "Name of the module that obsoletes this one.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_OBSOLETED_BY_MODULE_STREAM] = g_param_spec_string (
    "obsoleted-by-module-stream",
    "Obsoleted by module stream",
    "Stream of the module that obsoletes this one.",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_obsoletes_init (ModulemdObsoletes *UNUSED (self))
{
}

/* === YAML Functions === */
static gboolean
modulemd_obsoletes_parse_obsoleted_by (yaml_parser_t *parser,
                                       ModulemdObsoletes *o,
                                       GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (GError) nested_error = NULL;
  gchar *value = NULL;
  gboolean done = FALSE;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error, event, "Missing mapping in obsoletes obsoleted_by.");
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT_BOOL (parser, &event, error);
      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          if (g_str_equal (event.data.scalar.value, "module"))
            {
              if (modulemd_obsoletes_get_obsoleted_by_module_name (o))
                {
                  /* We already have a module name. It should not appear
                   * twice in the same document.
                   */
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error,
                    event,
                    "Obsoleted by module name encountered twice.");
                }

              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error,
                    event,
                    "Failed to parse module name in obsoletes obsoleted_by "
                    "data: %s",
                    nested_error->message);
                }
              modulemd_obsoletes_set_obsoleted_by_module_name (o, value);
              g_clear_pointer (&value, g_free);
            }

          if (g_str_equal (event.data.scalar.value, "stream"))
            {
              if (modulemd_obsoletes_get_obsoleted_by_module_stream (o))
                {
                  /* We already have a module stream. It should not appear
                   * twice in the same document.
                   */
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error,
                    event,
                    "Obsoleted by module stream encountered twice.");
                }

              value = modulemd_yaml_parse_string (parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT_BOOL (
                    error,
                    event,
                    "Failed to parse module stream in obsoletes obsoleted_by "
                    "data: %s",
                    nested_error->message);
                }
              modulemd_obsoletes_set_obsoleted_by_module_stream (o, value);
              g_clear_pointer (&value, g_free);
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT_BOOL (
            error, event, "Unexpected YAML event in obsoletes obsoleted_by.");
          break;
        }
      yaml_event_delete (&event);
    }

  return TRUE;
}

ModulemdObsoletes *
modulemd_obsoletes_parse_yaml (ModulemdSubdocumentInfo *subdoc,
                               gboolean strict,
                               GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (GError) nested_error = NULL;
  gboolean done = FALSE;

  g_autofree gchar *value = NULL;
  g_autofree gchar *obs_by_mod_name = NULL;
  g_autofree gchar *obs_by_mod_stream = NULL;
  guint64 modified;
  guint64 eol_date;
  gboolean reset;

  g_autoptr (ModulemdObsoletes) o = NULL;

  guint64 mdversion = modulemd_subdocument_info_get_mdversion (subdoc);

  if (!modulemd_subdocument_info_get_data_parser (
        subdoc, &parser, strict, error))
    {
      g_debug ("get_data_parser() failed: %s", (*error)->message);
      return NULL;
    }

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  /* Create an obsoletes with placeholder values. We'll verify that this has been
   * changed before we return it. This is because we can't guarantee that we
   * will get the actual values from the YAML before reading any of the other
   * data, but it's easier to process the rest of the contents with the
   * constructed object.
   */
  o = modulemd_obsoletes_new (mdversion,
                              0,
                              O_PLACEHOLDER_STRING,
                              O_PLACEHOLDER_STRING,
                              O_PLACEHOLDER_STRING);

  YAML_PARSER_PARSE_WITH_EXIT (&parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (
        error, event, "Missing START EVENT mapping in obsoletes data entry");
    }

  //parsing loop
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (&parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          if (g_str_equal (event.data.scalar.value, "module"))
            {
              if (!g_str_equal (modulemd_obsoletes_get_module_name (o),
                                O_PLACEHOLDER_STRING))
                {
                  /* The module name was set earlier, which means it is
                   * not expected here
                   */
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error, event, "Module name encountered twice");
                }
              value = modulemd_yaml_parse_string (&parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse module name in obsoletes data: %s",
                    nested_error->message);
                }

              /* Use a private internal function to set the module_name.
               * External consumers should never be allowed to change this
               * value, but we need to be able to modify the placeholder.
               */
              modulemd_obsoletes_set_module_name (o, value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "stream"))
            {
              if (!g_str_equal (modulemd_obsoletes_get_module_stream (o),
                                O_PLACEHOLDER_STRING))
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error, event, "Module stream encountered twice");
                }
              value = modulemd_yaml_parse_string (&parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse module stream in obsoletes data: %s",
                    nested_error->message);
                }

              modulemd_obsoletes_set_module_stream (o, value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "context"))
            {
              if (modulemd_obsoletes_get_module_context (o))
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error, event, "Module context encountered twice");
                }
              value = modulemd_yaml_parse_string (&parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse module context in obsoletes data: %s",
                    nested_error->message);
                }

              modulemd_obsoletes_set_module_context (o, value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "modified"))
            {
              value = modulemd_yaml_parse_string (&parser, &nested_error);
              if (nested_error)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse modified in obsoletes data: %s",
                    nested_error->message);
                }
              modified = modulemd_iso8601date_to_guint64 (value);
              if (modified == 0)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse UTC date in ISO 8601 format: "
                    "YYYY-MM-DDTHH:MMZ modified in eol data: %s",
                    value);
                }

              modulemd_obsoletes_set_modified (o, modified);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "eol_date"))
            {
              value = modulemd_yaml_parse_string (&parser, &nested_error);
              if (nested_error)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse eol_date in obsoletes data: %s",
                    nested_error->message);
                }

              eol_date = modulemd_iso8601date_to_guint64 (value);
              if (eol_date == 0)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse UTC date in ISO 8601 format: "
                    "YYYY-MM-DD[T ]HH:MMZ eol_date in obsoletes data: %s",
                    value);
                }

              modulemd_obsoletes_set_eol_date (o, eol_date);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "reset"))
            {
              reset = modulemd_yaml_parse_bool (&parser, &nested_error);
              if (nested_error)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse reset in obsoletes data: %s",
                    nested_error->message);
                }

              modulemd_obsoletes_set_reset (o, reset);
            }
          else if (g_str_equal (event.data.scalar.value, "message"))
            {
              if (!g_str_equal (modulemd_obsoletes_get_message (o),
                                O_PLACEHOLDER_STRING))
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error, event, "Module message encountered twice");
                }
              value = modulemd_yaml_parse_string (&parser, &nested_error);
              if (!value)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse message in obsoletes data: %s",
                    nested_error->message);
                }

              modulemd_obsoletes_set_message (o, value);
              g_clear_pointer (&value, g_free);
            }
          else if (g_str_equal (event.data.scalar.value, "obsoleted_by"))
            {
              if (!modulemd_obsoletes_parse_obsoleted_by (
                    &parser, o, &nested_error))
                {
                  g_propagate_error (error, g_steal_pointer (&nested_error));
                  return NULL;
                }
            }
          else
            {
              SKIP_UNKNOWN (&parser,
                            NULL,
                            "Unexpected key in obsoletes data: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event %s in obsoletes data",
            mmd_yaml_get_event_name (event.type));
          break;
        }

      yaml_event_delete (&event);
    }

  if (!modulemd_obsoletes_validate (o, &nested_error))
    {
      g_propagate_error (error, g_steal_pointer (&nested_error));
      return NULL;
    }

  return g_steal_pointer (&o);
}


static gboolean
modulemd_obsoletes_emit_obsoleted_by (ModulemdObsoletes *self,
                                      yaml_emitter_t *emitter,
                                      GError **error)
{
  /* Start the "obsoleted_by:" section */
  if (!mmd_emitter_scalar (
        emitter, "obsoleted_by", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  /* Start the mapping for "obsoleted_by:" */
  if (!mmd_emitter_start_mapping (emitter, YAML_BLOCK_MAPPING_STYLE, error))
    {
      return FALSE;
    }

  /* The module name is mandatory if already in obsoleted_by */
  if (!mmd_emitter_scalar (emitter, "module", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (emitter,
                           modulemd_obsoletes_get_obsoleted_by_module_name (
                             MODULEMD_OBSOLETES (self)),
                           YAML_PLAIN_SCALAR_STYLE,
                           error))
    {
      return FALSE;
    }

  /* The module stream is mandatory if already in obsoleted_by */
  if (!mmd_emitter_scalar (emitter, "stream", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (emitter,
                           modulemd_obsoletes_get_obsoleted_by_module_stream (
                             MODULEMD_OBSOLETES (self)),
                           YAML_DOUBLE_QUOTED_SCALAR_STYLE,
                           error))
    {
      return FALSE;
    }

  /* End the mapping for "obsoleted_by:" */
  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }
  return TRUE;
}


gboolean
modulemd_obsoletes_emit_yaml (ModulemdObsoletes *self,
                              yaml_emitter_t *emitter,
                              GError **error)
{
  MODULEMD_INIT_TRACE ();
  g_autoptr (GError) nested_error = NULL;
  guint64 eol_date;
  g_autofree gchar *modified_string = NULL;
  g_autofree gchar *eol_date_string = NULL;
  const gchar *module_context = NULL;

  if (!modulemd_obsoletes_validate (MODULEMD_OBSOLETES (self), &nested_error))
    {
      /* Validation failed */
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "Obsoletes object failed validation: ");
      return FALSE;
    }

  /* First emit the standard document headers */
  if (!modulemd_yaml_emit_document_headers (
        emitter,
        MODULEMD_YAML_DOC_OBSOLETES,
        modulemd_obsoletes_get_mdversion (MODULEMD_OBSOLETES (self)),
        error))
    {
      return FALSE;
    }

  /* Start the data: section mapping */
  if (!mmd_emitter_start_mapping (emitter, YAML_BLOCK_MAPPING_STYLE, error))
    {
      return FALSE;
    }

  /* Fill in the default data */

  /* The modified field is mandatory */
  if (!mmd_emitter_scalar (
        emitter, "modified", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }
  modified_string = modulemd_guint64_to_iso8601date (
    modulemd_obsoletes_get_modified (MODULEMD_OBSOLETES (self)));
  if (!modified_string)
    {
      g_set_error (
        error,
        MODULEMD_ERROR,
        MMD_ERROR_VALIDATE,
        "Cannot convert modified date: %" PRIu64 " to iso8601 date.",
        modulemd_obsoletes_get_modified (MODULEMD_OBSOLETES (self)));
      return FALSE;
    }
  if (!mmd_emitter_scalar (
        emitter, modified_string, YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  /* Only output reset if it's TRUE */
  if (modulemd_obsoletes_get_reset (self))
    {
      EMIT_KEY_VALUE (emitter, error, "reset", "true");
    }

  /* The module name is mandatory */
  if (!mmd_emitter_scalar (emitter, "module", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (
        emitter,
        modulemd_obsoletes_get_module_name (MODULEMD_OBSOLETES (self)),
        YAML_PLAIN_SCALAR_STYLE,
        error))
    {
      return FALSE;
    }

  /* The module stream is mandatory */
  if (!mmd_emitter_scalar (emitter, "stream", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (
        emitter,
        modulemd_obsoletes_get_module_stream (MODULEMD_OBSOLETES (self)),
        YAML_DOUBLE_QUOTED_SCALAR_STYLE,
        error))
    {
      return FALSE;
    }

  /* The module context is optional */
  module_context =
    modulemd_obsoletes_get_module_context (MODULEMD_OBSOLETES (self));
  if (module_context)
    {
      if (!mmd_emitter_scalar (
            emitter, "context", YAML_PLAIN_SCALAR_STYLE, error))
        {
          return FALSE;
        }

      if (!mmd_emitter_scalar (
            emitter, module_context, YAML_PLAIN_SCALAR_STYLE, error))
        {
          return FALSE;
        }
    }

  /* The eol_date field is optional */
  eol_date = modulemd_obsoletes_get_eol_date (MODULEMD_OBSOLETES (self));
  if (eol_date)
    {
      eol_date_string = modulemd_guint64_to_iso8601date (eol_date);
      if (eol_date_string == NULL)
        {
          g_set_error (error,
                       MODULEMD_ERROR,
                       MMD_ERROR_VALIDATE,
                       "Cannot convert eol_date: %" PRIu64 " to iso8601 date.",
                       eol_date);
          return FALSE;
        }
      EMIT_KEY_VALUE (emitter, error, "eol_date", eol_date_string);
    }

  /* The message is mandatory */
  if (!mmd_emitter_scalar (emitter, "message", YAML_PLAIN_SCALAR_STYLE, error))
    {
      return FALSE;
    }

  if (!mmd_emitter_scalar (
        emitter,
        modulemd_obsoletes_get_message (MODULEMD_OBSOLETES (self)),
        YAML_PLAIN_SCALAR_STYLE,
        error))
    {
      return FALSE;
    }

  /* Obsoleted_by are optional */
  if (modulemd_obsoletes_get_obsoleted_by_module_name (
        MODULEMD_OBSOLETES (self)) &&
      modulemd_obsoletes_get_obsoleted_by_module_stream (
        MODULEMD_OBSOLETES (self)))
    {
      if (!modulemd_obsoletes_emit_obsoleted_by (self, emitter, error))
        {
          return FALSE;
        }
    }

  /* Close the data: section mapping */
  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }

  /* Close the top-level section mapping */
  if (!mmd_emitter_end_mapping (emitter, error))
    {
      return FALSE;
    }

  /* End the document */
  if (!mmd_emitter_end_document (emitter, error))
    {
      return FALSE;
    }

  return TRUE;
}

gboolean
modulemd_obsoletes_is_active (ModulemdObsoletes *self)
{
  time_t rawtime;
  struct tm *tm;
  time (&rawtime);
  tm = gmtime (&rawtime);

  char buf[255];
  strftime (buf, sizeof (buf), "%Y%m%d%H%M", tm);
  guint64 now = g_ascii_strtoull (buf, NULL, 0);

  if (now >= modulemd_obsoletes_get_eol_date (self))
    {
      return TRUE;
    }

  return FALSE;
}
