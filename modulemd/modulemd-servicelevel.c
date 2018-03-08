/* modulemd-servicelevel.c
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

#include <glib.h>
#include "modulemd.h"

enum
{
  SL_PROP_0,

  SL_PROP_EOL,
  SL_PROP_NAME,

  SL_N_PROPERTIES
};

static GParamSpec *servicelevel_properties[SL_N_PROPERTIES] = {
  NULL,
};

struct _ModulemdServiceLevel
{
  GObject parent_instance;

  GDate *eol;
  gchar *name;
};

G_DEFINE_TYPE (ModulemdServiceLevel, modulemd_servicelevel, G_TYPE_OBJECT)

/**
 * modulemd_servicelevel_set_eol:
 * @date: (nullable): The date this service level ends
 *
 * Sets the end date of the service level.
 *
 * Since: 1.0
 */
void
modulemd_servicelevel_set_eol (ModulemdServiceLevel *self, const GDate *date)
{
  g_return_if_fail (MODULEMD_IS_SERVICELEVEL (self));
  g_return_if_fail (!date || g_date_valid (date));

  if (!date)
    {
      g_date_clear (self->eol, 1);
    }

  if (!g_date_valid (self->eol) || g_date_compare (date, self->eol) != 0)
    {
      /* Date is changing. Update it */
      g_date_set_year (self->eol, g_date_get_year (date));
      g_date_set_month (self->eol, g_date_get_month (date));
      g_date_set_day (self->eol, g_date_get_day (date));

      g_object_notify_by_pspec (G_OBJECT (self),
                                servicelevel_properties[SL_PROP_EOL]);
    }
}


/**
 * modulemd_servicelevel_get_eol:
 *
 * Retrieves the end-of-life date of this service level.
 *
 * Returns: a #GDate representing the end-of-life date of the service level.
 *
 * Deprecated: 1.1
 * Use peek_eol() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_servicelevel_peek_eol)
const GDate *
modulemd_servicelevel_get_eol (ModulemdServiceLevel *self)
{
  return modulemd_servicelevel_peek_eol (self);
}


/**
 * modulemd_servicelevel_peek_eol:
 *
 * Retrieves the end-of-life date of this service level.
 *
 * Returns: a #GDate representing the end-of-life date of the service level.
 *
 * Since: 1.1
 */
const GDate *
modulemd_servicelevel_peek_eol (ModulemdServiceLevel *self)
{
  g_return_val_if_fail (MODULEMD_IS_SERVICELEVEL (self), NULL);

  if (!g_date_valid (self->eol))
    {
      return NULL;
    }

  return self->eol;
}


/**
 * modulemd_servicelevel_dup_eol:
 *
 * Retrieves a copy of the end-of-life date of this service level.
 *
 * Returns: a #GDate representing the end-of-life date of the service level.
 *
 * Since: 1.1
 */
GDate *
modulemd_servicelevel_dup_eol (ModulemdServiceLevel *self)
{
  g_return_val_if_fail (MODULEMD_IS_SERVICELEVEL (self), NULL);

  if (!g_date_valid (self->eol))
    {
      return NULL;
    }

  return g_date_new_dmy (g_date_get_day (self->eol),
                         g_date_get_month (self->eol),
                         g_date_get_year (self->eol));
}


/**
 * modulemd_servicelevel_set_name:
 * @name: (nullable): The name of this servicelevel
 *
 * Set the name of this service level.
 *
 * Since: 1.0
 */
void
modulemd_servicelevel_set_name (ModulemdServiceLevel *self, const gchar *name)
{
  g_return_if_fail (MODULEMD_IS_SERVICELEVEL (self));

  g_clear_pointer (&self->name, g_free);
  if (name)
    {
      self->name = g_strdup (name);
    }

  g_object_notify_by_pspec (G_OBJECT (self),
                            servicelevel_properties[SL_PROP_NAME]);
}


/**
 * modulemd_servicelevel_get_name:
 *
 * Retrieves the name of this service level
 *
 * Returns: a string representing the name of the service level or NULL if not
 * set.
 *
 * Deprecated: 1.1
 * Use peek_name() instead.
 *
 * Since: 1.0
 */
G_DEPRECATED_FOR (modulemd_servicelevel_peek_name)
const gchar *
modulemd_servicelevel_get_name (ModulemdServiceLevel *self)
{
  return modulemd_servicelevel_peek_name (self);
}


/**
 * modulemd_servicelevel_peek_name:
 *
 * Retrieves the name of this service level
 *
 * Returns: a string representing the name of the service level or NULL if not
 * set.
 *
 * Since: 1.1
 */
const gchar *
modulemd_servicelevel_peek_name (ModulemdServiceLevel *self)
{
  g_return_val_if_fail (MODULEMD_IS_SERVICELEVEL (self), NULL);

  if (!self->name)
    {
      g_warning ("Servicelevel name requested, but has not been set");
    }

  return self->name;
}


/**
 * modulemd_servicelevel_dup_name:
 *
 * Retrieves a copy of the name of this service level
 *
 * Returns: a copy of the string representing the name of the service level or
 * NULL if not set.
 *
 * Since: 1.1
 */
gchar *
modulemd_servicelevel_dup_name (ModulemdServiceLevel *self)
{
  g_return_val_if_fail (MODULEMD_IS_SERVICELEVEL (self), NULL);

  if (!self->name)
    {
      g_warning ("Servicelevel name requested, but has not been set");
      return NULL;
    }

  return g_strdup (self->name);
}


/**
 * modulemd_servicelevel_copy:
 *
 * Create a copy of this #ModulemdServiceLevel object.
 *
 * Returns: (transfer full): a copied #ModulemdServiceLevel object
 *
 * Since: 1.1
 */
ModulemdServiceLevel *
modulemd_servicelevel_copy (ModulemdServiceLevel *self)
{
  ModulemdServiceLevel *new_sl = NULL;
  g_return_val_if_fail (MODULEMD_IS_SERVICELEVEL (self), NULL);

  new_sl = modulemd_servicelevel_new ();

  modulemd_servicelevel_set_eol (new_sl,
                                 modulemd_servicelevel_peek_eol (self));
  modulemd_servicelevel_set_name (new_sl,
                                  modulemd_servicelevel_peek_name (self));

  return new_sl;
}


static void
modulemd_servicelevel_set_property (GObject *gobject,
                                    guint property_id,
                                    const GValue *value,
                                    GParamSpec *pspec)
{
  ModulemdServiceLevel *self = MODULEMD_SERVICELEVEL (gobject);

  switch (property_id)
    {
    case SL_PROP_EOL:
      modulemd_servicelevel_set_eol (self, g_value_get_boxed (value));
      break;

    case SL_PROP_NAME:
      modulemd_servicelevel_set_name (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_servicelevel_get_property (GObject *gobject,
                                    guint property_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  ModulemdServiceLevel *self = MODULEMD_SERVICELEVEL (gobject);

  switch (property_id)
    {
    case SL_PROP_EOL:
      g_value_set_boxed (value, modulemd_servicelevel_peek_eol (self));
      break;

    case SL_PROP_NAME:
      g_value_set_string (value, modulemd_servicelevel_peek_name (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
      break;
    }
}

static void
modulemd_servicelevel_finalize (GObject *gobject)
{
  ModulemdServiceLevel *self = (ModulemdServiceLevel *)gobject;

  g_clear_pointer (&self->eol, g_date_free);

  G_OBJECT_CLASS (modulemd_servicelevel_parent_class)->finalize (gobject);
}

static void
modulemd_servicelevel_class_init (ModulemdServiceLevelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = modulemd_servicelevel_set_property;
  object_class->get_property = modulemd_servicelevel_get_property;
  object_class->finalize = modulemd_servicelevel_finalize;

  servicelevel_properties[SL_PROP_EOL] =
    g_param_spec_boxed ("eol",
                        "End of Life",
                        "An ISO-8601 compatible YYYY-MM-DD value "
                        "representing the end-of-life date of this service "
                        "level.",
                        G_TYPE_DATE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  servicelevel_properties[SL_PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "A human-readable name for this servicelevel",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (
    object_class, SL_N_PROPERTIES, servicelevel_properties);
}

static void
modulemd_servicelevel_init (ModulemdServiceLevel *self)
{
  self->eol = g_date_new ();
}

ModulemdServiceLevel *
modulemd_servicelevel_new (void)
{
  return g_object_new (MODULEMD_TYPE_SERVICELEVEL, NULL);
}
