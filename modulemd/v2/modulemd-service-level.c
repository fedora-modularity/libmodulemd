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

#include <glib.h>
#include <yaml.h>

#include "modulemd-service-level.h"
#include "private/glib-extensions.h"
#include "private/modulemd-service-level-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"

#define SL_DEFAULT_STRING "__NAME_UNSET__"

struct _ModulemdServiceLevel
{
  GObject parent_instance;

  gchar *name;
  GDate *eol;
};

G_DEFINE_TYPE (ModulemdServiceLevel, modulemd_service_level, G_TYPE_OBJECT)

enum
{
  PROP_0,

  PROP_NAME,
  PROP_EOL,

  N_PROPS
};

static GParamSpec *properties[N_PROPS];


ModulemdServiceLevel *
modulemd_service_level_new (const gchar *name)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_SERVICE_LEVEL,
                       "name", name,
                       NULL);
  // clang-format on
}


ModulemdServiceLevel *
modulemd_service_level_copy (ModulemdServiceLevel *self)
{
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  g_return_val_if_fail (MODULEMD_IS_SERVICE_LEVEL (self), NULL);

  sl = modulemd_service_level_new (modulemd_service_level_get_name (self));

  modulemd_service_level_set_eol (sl, modulemd_service_level_get_eol (self));

  return g_object_ref (sl);
}


static void
modulemd_service_level_finalize (GObject *object)
{
  ModulemdServiceLevel *self = (ModulemdServiceLevel *)object;

  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->eol, g_date_free);

  G_OBJECT_CLASS (modulemd_service_level_parent_class)->finalize (object);
}


static void
modulemd_service_level_set_name (ModulemdServiceLevel *self, const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_SERVICE_LEVEL (self));

  /* It is a coding error if we ever get a NULL name here */
  g_return_if_fail (name);

  /* It is a coding error if we ever get the default name here */
  g_return_if_fail (g_strcmp0 (name, SL_DEFAULT_STRING));

  g_clear_pointer (&self->name, g_free);
  self->name = g_strdup (name);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}


const gchar *
modulemd_service_level_get_name (ModulemdServiceLevel *self)
{
  g_return_val_if_fail (MODULEMD_IS_SERVICE_LEVEL (self), NULL);

  return self->name;
}


void
modulemd_service_level_set_eol (ModulemdServiceLevel *self, GDate *date)
{
  g_return_if_fail (MODULEMD_IS_SERVICE_LEVEL (self));

  if (!date || !g_date_valid (date))
    {
      g_date_clear (self->eol, 1);
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EOL]);
      return;
    }

  if (!g_date_valid (self->eol) || g_date_compare (date, self->eol) != 0)
    {
      /* Date is changing. Update it */
      g_date_set_year (self->eol, g_date_get_year (date));
      g_date_set_month (self->eol, g_date_get_month (date));
      g_date_set_day (self->eol, g_date_get_day (date));
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EOL]);
    }
}


void
modulemd_service_level_set_eol_ymd (ModulemdServiceLevel *self,
                                    GDateYear year,
                                    GDateMonth month,
                                    GDateDay day)
{
  g_autoptr (GDate) date = NULL;
  g_return_if_fail (MODULEMD_IS_SERVICE_LEVEL (self));

  if (!g_date_valid_dmy (day, month, year))
    {
      /* Treat invalid dates as NULL */
      return modulemd_service_level_set_eol (self, NULL);
    }

  date = g_date_new_dmy (day, month, year);
  return modulemd_service_level_set_eol (self, date);
}


void
modulemd_service_level_remove_eol (ModulemdServiceLevel *self)
{
  return modulemd_service_level_set_eol (self, NULL);
}


GDate *
modulemd_service_level_get_eol (ModulemdServiceLevel *self)
{
  g_return_val_if_fail (MODULEMD_IS_SERVICE_LEVEL (self), NULL);

  if (self->eol && g_date_valid (self->eol))
    {
      return self->eol;
    }

  return NULL;
}


gchar *
modulemd_service_level_get_eol_string (ModulemdServiceLevel *self)
{
  if (self->eol && g_date_valid (self->eol))
    {
      return g_strdup_printf ("%.4d-%.2d-%.2d",
                              g_date_get_year (self->eol),
                              g_date_get_month (self->eol),
                              g_date_get_day (self->eol));
    }

  return NULL;
}


static void
modulemd_service_level_get_property (GObject *object,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdServiceLevel *self = MODULEMD_SERVICE_LEVEL (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, modulemd_service_level_get_name (self));
      break;

    case PROP_EOL:
      g_value_set_boxed (value, modulemd_service_level_get_eol (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_service_level_set_property (GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdServiceLevel *self = MODULEMD_SERVICE_LEVEL (object);

  switch (prop_id)
    {
    case PROP_NAME:
      modulemd_service_level_set_name (self, g_value_get_string (value));
      break;

    case PROP_EOL:
      modulemd_service_level_set_eol (self, g_value_get_boxed (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
modulemd_service_level_class_init (ModulemdServiceLevelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_service_level_finalize;
  object_class->get_property = modulemd_service_level_get_property;
  object_class->set_property = modulemd_service_level_set_property;

  properties[PROP_NAME] = g_param_spec_string (
    "name",
    "Name",
    "A human-readable name for this servicelevel",
    SL_DEFAULT_STRING,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_EOL] =
    g_param_spec_boxed ("eol",
                        "End of Life",
                        "An ISO-8601 compatible YYYY-MM-DD value "
                        "representing the end-of-life date of this service "
                        "level.",
                        G_TYPE_DATE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}


static void
modulemd_service_level_init (ModulemdServiceLevel *self)
{
  self->eol = g_date_new ();
}


/* === YAML Functions === */

ModulemdServiceLevel *
modulemd_service_level_parse_yaml (yaml_parser_t *parser, GError **error)
{
  MODULEMD_INIT_TRACE
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  GDate *eol = NULL;
  g_autoptr (GError) nested_error = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Read in the name of the service level */
  YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
  if (event.type != YAML_SCALAR_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (error, event, "Missing service level name");
    }
  sl = modulemd_service_level_new ((const gchar *)event.data.scalar.value);
  yaml_event_delete (&event);

  /* Read in any supplementary attributes of the service level,
   * such as 'eol'
   */
  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the service level content. */
          in_map = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the service level content */
          in_map = FALSE;
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              /* We must be in the map before we handle scalars */
              MMD_YAML_ERROR_EVENT_EXIT (
                error, event, "Missing mapping in service level");
              break;
            }
          /* Only "eol" is supported right now */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "eol"))
            {
              /* Get the EOL date */
              eol = modulemd_yaml_parse_date (parser, &nested_error);
              if (!eol)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse EOL date in service level: %s",
                    nested_error->message);
                }

              modulemd_service_level_set_eol (sl, eol);
              g_date_free (eol);
            }
          else
            {
              /* Unknown field in service level */
              MMD_YAML_ERROR_EVENT_EXIT (
                error, event, "Unknown key in service level body");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_EVENT_EXIT (
            error, event, "Unexpected YAML event in service level");
          break;
        }
      yaml_event_delete (&event);
    }

  return g_object_ref (sl);
}

gboolean
modulemd_service_level_emit_yaml (ModulemdServiceLevel *self,
                                  yaml_emitter_t *emitter,
                                  GError **error)
{
  int ret;
  g_autoptr (GError) nested_error = NULL;
  g_autofree gchar *eol_string = NULL;
  MMD_INIT_YAML_EVENT (event);

  /* Emit the Service Level Name */
  ret = mmd_emitter_scalar (emitter,
                            modulemd_service_level_get_name (self),
                            YAML_PLAIN_SCALAR_STYLE,
                            &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to emit service level name: ");
      return FALSE;
    }

  /* Start the mapping for additional attributes of this service level */
  ret = mmd_emitter_start_mapping (
    emitter, YAML_BLOCK_MAPPING_STYLE, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to start service level mapping: ");
      return FALSE;
    }

  /* Add service level attributes if available */
  if (modulemd_service_level_get_eol (self) != NULL)
    {
      ret = mmd_emitter_scalar (
        emitter, "eol", YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (
            error, nested_error, "Failed to emit EOL key: ");
          return FALSE;
        }

      eol_string = modulemd_service_level_get_eol_string (self);
      ret = mmd_emitter_scalar (
        emitter, eol_string, YAML_PLAIN_SCALAR_STYLE, &nested_error);
      if (!ret)
        {
          g_propagate_prefixed_error (error,
                                      nested_error,
                                      "Failed to emit EOL string [%s]: ",
                                      eol_string);
          return FALSE;
        }
    }

  /* End the mapping */
  ret = mmd_emitter_end_mapping (emitter, &nested_error);
  if (!ret)
    {
      g_propagate_prefixed_error (
        error, nested_error, "Failed to end service level mapping: ");
      return FALSE;
    }

  return TRUE;
}
