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

#include "yaml.h"
#include <inttypes.h>

#include "modulemd-errors.h"
#include "modulemd-rpm-map-entry.h"
#include "private/modulemd-rpm-map-entry-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"


struct _ModulemdRpmMapEntry
{
  GObject parent_instance;

  gchar *name;
  guint64 epoch;
  gchar *version;
  gchar *release;
  gchar *arch;
};

G_DEFINE_TYPE (ModulemdRpmMapEntry, modulemd_rpm_map_entry, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_NAME,
  PROP_EPOCH,
  PROP_VERSION,
  PROP_RELEASE,
  PROP_ARCH,
  PROP_NEVRA,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

ModulemdRpmMapEntry *
modulemd_rpm_map_entry_new (const gchar *name,
                            guint64 epoch,
                            const gchar *version,
                            const gchar *release,
                            const gchar *arch)
{
  // clang-format off
  return g_object_new (MODULEMD_TYPE_RPM_MAP_ENTRY,
                       "name", name,
                       "epoch", epoch,
                       "version", version,
                       "release", release,
                       "arch", arch,
                       NULL);
  // clang-format on
}


gboolean
modulemd_rpm_map_entry_equals_wrapper (const void *a, const void *b)
{
  g_return_val_if_fail (MODULEMD_IS_RPM_MAP_ENTRY ((ModulemdRpmMapEntry *)a),
                        FALSE);
  g_return_val_if_fail (MODULEMD_IS_RPM_MAP_ENTRY ((ModulemdRpmMapEntry *)b),
                        FALSE);

  return modulemd_rpm_map_entry_equals ((ModulemdRpmMapEntry *)a,
                                        (ModulemdRpmMapEntry *)b);
}

gboolean
modulemd_RpmMapEntry_hash_table_equals_wrapper (const void *a, const void *b)
{
  return modulemd_hash_table_equals (
    (GHashTable *)a, (GHashTable *)b, modulemd_rpm_map_entry_equals_wrapper);
}

static void
modulemd_rpm_map_entry_finalize (GObject *object)
{
  ModulemdRpmMapEntry *self = (ModulemdRpmMapEntry *)object;

  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->version, g_free);
  g_clear_pointer (&self->release, g_free);
  g_clear_pointer (&self->arch, g_free);

  G_OBJECT_CLASS (modulemd_rpm_map_entry_parent_class)->finalize (object);
}


ModulemdRpmMapEntry *
modulemd_rpm_map_entry_copy (ModulemdRpmMapEntry *self)
{
  return modulemd_rpm_map_entry_new (
    self->name, self->epoch, self->version, self->release, self->arch);
}

gboolean
modulemd_rpm_map_entry_equals (ModulemdRpmMapEntry *self,
                               ModulemdRpmMapEntry *other)
{
  g_autofree gchar *self_nevra = NULL;
  g_autofree gchar *other_nevra = NULL;

  g_return_val_if_fail (MODULEMD_IS_RPM_MAP_ENTRY (self), FALSE);
  g_return_val_if_fail (MODULEMD_IS_RPM_MAP_ENTRY (other), FALSE);

  if (self == other)
    {
      return TRUE;
    }

  /* Since all of the public attributes of these entries are captured by the
   * NEVRA output, we can short-cut the comparison process and just compare
   * the two NEVRAs instead.
   */
  self_nevra = modulemd_rpm_map_entry_get_nevra_as_string (self);
  other_nevra = modulemd_rpm_map_entry_get_nevra_as_string (other);

  return !g_strcmp0 (self_nevra, other_nevra);
}

gboolean
modulemd_rpm_map_entry_validate (ModulemdRpmMapEntry *self, GError **error)
{
  if (!self->name)
    {
      g_set_error_literal (
        error, MODULEMD_ERROR, MMD_ERROR_VALIDATE, "Missing name attribute");
      return FALSE;
    }
  if (!self->version)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Missing version attribute");
      return FALSE;
    }
  if (!self->release)
    {
      g_set_error_literal (error,
                           MODULEMD_ERROR,
                           MMD_ERROR_VALIDATE,
                           "Missing release attribute");
      return FALSE;
    }
  if (!self->arch)
    {
      g_set_error_literal (
        error, MODULEMD_ERROR, MMD_ERROR_VALIDATE, "Missing arch attribute");
      return FALSE;
    }


  return TRUE;
}


MODULEMD_SETTER_GETTER_STRING (
  ModulemdRpmMapEntry, rpm_map_entry, RPM_MAP_ENTRY, name, NAME)

MODULEMD_SETTER_GETTER_STRING (
  ModulemdRpmMapEntry, rpm_map_entry, RPM_MAP_ENTRY, version, VERSION)

MODULEMD_SETTER_GETTER_STRING (
  ModulemdRpmMapEntry, rpm_map_entry, RPM_MAP_ENTRY, release, RELEASE)

MODULEMD_SETTER_GETTER_STRING (
  ModulemdRpmMapEntry, rpm_map_entry, RPM_MAP_ENTRY, arch, ARCH)


void
modulemd_rpm_map_entry_set_epoch (ModulemdRpmMapEntry *self, guint64 epoch)
{
  g_return_if_fail (MODULEMD_IS_RPM_MAP_ENTRY (self));

  self->epoch = epoch;

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EPOCH]);
}


guint64
modulemd_rpm_map_entry_get_epoch (ModulemdRpmMapEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_RPM_MAP_ENTRY (self), 0);

  return self->epoch;
}


gchar *
modulemd_rpm_map_entry_get_nevra_as_string (ModulemdRpmMapEntry *self)
{
  g_return_val_if_fail (MODULEMD_IS_RPM_MAP_ENTRY (self), 0);

  if (!modulemd_rpm_map_entry_validate (self, NULL))
    {
      /* None of the strings are optional and if any are missing, we can't
       * generate a valid NEVRA
       */
      return NULL;
    }

  return g_strdup_printf ("%s-%" PRIu64 ":%s-%s.%s",
                          self->name,
                          self->epoch,
                          self->version,
                          self->release,
                          self->arch);
}


static void
modulemd_rpm_map_entry_get_property (GObject *object,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdRpmMapEntry *self = MODULEMD_RPM_MAP_ENTRY (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, modulemd_rpm_map_entry_get_name (self));
      break;

    case PROP_EPOCH:
      g_value_set_uint64 (value, modulemd_rpm_map_entry_get_epoch (self));
      break;

    case PROP_VERSION:
      g_value_set_string (value, modulemd_rpm_map_entry_get_version (self));
      break;

    case PROP_RELEASE:
      g_value_set_string (value, modulemd_rpm_map_entry_get_release (self));
      break;

    case PROP_ARCH:
      g_value_set_string (value, modulemd_rpm_map_entry_get_arch (self));
      break;

    case PROP_NEVRA:
      g_value_take_string (value,
                           modulemd_rpm_map_entry_get_nevra_as_string (self));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_rpm_map_entry_set_property (GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
  ModulemdRpmMapEntry *self = MODULEMD_RPM_MAP_ENTRY (object);

  switch (prop_id)
    {
    case PROP_NAME:
      modulemd_rpm_map_entry_set_name (self, g_value_get_string (value));
      break;

    case PROP_EPOCH:
      modulemd_rpm_map_entry_set_epoch (self, g_value_get_uint64 (value));
      break;

    case PROP_VERSION:
      modulemd_rpm_map_entry_set_version (self, g_value_get_string (value));
      break;

    case PROP_RELEASE:
      modulemd_rpm_map_entry_set_release (self, g_value_get_string (value));
      break;

    case PROP_ARCH:
      modulemd_rpm_map_entry_set_arch (self, g_value_get_string (value));
      break;

    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
modulemd_rpm_map_entry_class_init (ModulemdRpmMapEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = modulemd_rpm_map_entry_finalize;
  object_class->get_property = modulemd_rpm_map_entry_get_property;
  object_class->set_property = modulemd_rpm_map_entry_set_property;

  properties[PROP_NAME] = g_param_spec_string (
    "name",
    "Artifact name",
    "The artifact package name",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_EPOCH] = g_param_spec_uint64 (
    "epoch",
    "Artifact epoch",
    "The artifact package epoch",
    0,
    G_MAXUINT32,
    0,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_VERSION] = g_param_spec_string (
    "version",
    "Artifact version",
    "The artifact package version",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_RELEASE] = g_param_spec_string (
    "release",
    "Artifact release string",
    "The artifact package release string",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_ARCH] = g_param_spec_string (
    "arch",
    "Artifact architecture",
    "The artifact package architecture",
    NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  properties[PROP_NEVRA] =
    g_param_spec_string ("nevra",
                         "Artifact N-E:V-R.A",
                         "The artifact package N-E:V-R.A",
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
modulemd_rpm_map_entry_init (ModulemdRpmMapEntry *UNUSED (self))
{
}

/* === YAML Functions === */
ModulemdRpmMapEntry *
modulemd_rpm_map_entry_parse_yaml (yaml_parser_t *parser,
                                   gboolean strict,
                                   GError **error)
{
  MODULEMD_INIT_TRACE ();
  MMD_INIT_YAML_EVENT (event);
  gboolean done = FALSE;
  gboolean seen_epoch = FALSE;
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  g_autoptr (GError) nested_error = NULL;
  guint64 epoch = 0;
  g_autofree gchar *scalar = NULL;
  g_autofree gchar *nevra = NULL;
  g_autofree gchar *built_nevra = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  entry = g_object_new (MODULEMD_TYPE_RPM_MAP_ENTRY, NULL);


  YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
  if (event.type != YAML_MAPPING_START_EVENT)
    {
      MMD_YAML_ERROR_EVENT_EXIT (
        error, event, "Missing mapping in rpm-map entry");
    }


  while (!done)
    {
      YAML_PARSER_PARSE_WITH_EXIT (parser, &event, error);
      switch (event.type)
        {
        case YAML_MAPPING_END_EVENT: done = TRUE; break;

        case YAML_SCALAR_EVENT:
          if (g_str_equal ((const gchar *)event.data.scalar.value, "name"))
            {
              scalar = modulemd_yaml_parse_string (parser, &nested_error);
              if (!scalar)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse package name: %s",
                    nested_error->message);
                }
              modulemd_rpm_map_entry_set_name (entry, scalar);
              g_clear_pointer (&scalar, g_free);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "epoch"))
            {
              epoch = modulemd_yaml_parse_uint64 (parser, &nested_error);
              if (nested_error)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse package epoch: %s",
                    nested_error->message);
                }
              modulemd_rpm_map_entry_set_epoch (entry, epoch);
              seen_epoch = TRUE;
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "version"))
            {
              scalar = modulemd_yaml_parse_string (parser, &nested_error);
              if (!scalar)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse package version: %s",
                    nested_error->message);
                }
              modulemd_rpm_map_entry_set_version (entry, scalar);
              g_clear_pointer (&scalar, g_free);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "release"))
            {
              scalar = modulemd_yaml_parse_string (parser, &nested_error);
              if (!scalar)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse package release: %s",
                    nested_error->message);
                }
              modulemd_rpm_map_entry_set_release (entry, scalar);
              g_clear_pointer (&scalar, g_free);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "arch"))
            {
              scalar = modulemd_yaml_parse_string (parser, &nested_error);
              if (!scalar)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse package architecture: %s",
                    nested_error->message);
                }
              modulemd_rpm_map_entry_set_arch (entry, scalar);
              g_clear_pointer (&scalar, g_free);
            }
          else if (g_str_equal ((const gchar *)event.data.scalar.value,
                                "nevra"))
            {
              nevra = modulemd_yaml_parse_string (parser, &nested_error);
              if (!nevra)
                {
                  MMD_YAML_ERROR_EVENT_EXIT (
                    error,
                    event,
                    "Failed to parse package nevra: %s",
                    nested_error->message);
                }
            }
          else
            {
              SKIP_UNKNOWN (parser,
                            NULL,
                            "Unexpected key in rpm-map entry: %s",
                            (const gchar *)event.data.scalar.value);
              break;
            }
          break;

        default:
          MMD_YAML_ERROR_EVENT_EXIT (
            error,
            event,
            "Unexpected YAML event %s in defaults data",
            mmd_yaml_get_event_name (event.type));
          break;
        }
      yaml_event_delete (&event);
    }

  if (!modulemd_rpm_map_entry_validate (entry, &nested_error))
    {
      g_propagate_prefixed_error (
        error, g_steal_pointer (&nested_error), "Validation of entry failed");
      return NULL;
    }

  /* Check that we got the epoch */
  if (!seen_epoch)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_MISSING_REQUIRED,
                           "Missing 'epoch' in rpm-map entry");
      return NULL;
    }

  /* check that we got the NEVRA and that it matches the exploded version */
  if (!nevra)
    {
      g_set_error_literal (error,
                           MODULEMD_YAML_ERROR,
                           MMD_YAML_ERROR_MISSING_REQUIRED,
                           "Missing 'nevra' in rpm-map entry");
      return NULL;
    }

  built_nevra = modulemd_rpm_map_entry_get_nevra_as_string (entry);
  if (!g_str_equal (nevra, built_nevra))
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MMD_YAML_ERROR_INCONSISTENT,
                   "'nevra' field (%s) differs from exploded version (%s)",
                   nevra,
                   built_nevra);
      return NULL;
    }

  return g_steal_pointer (&entry);
}


gboolean
modulemd_rpm_map_entry_emit_yaml (ModulemdRpmMapEntry *self,
                                  yaml_emitter_t *emitter,
                                  GError **error)
{
  MODULEMD_INIT_TRACE ();
  g_autoptr (GError) nested_error = NULL;
  g_autofree gchar *epoch = NULL;
  g_autofree gchar *nevra = NULL;

  if (!modulemd_rpm_map_entry_validate (self, &nested_error))
    {
      g_propagate_prefixed_error (error,
                                  g_steal_pointer (&nested_error),
                                  "rpm-map entry failed to validate: ");
      return FALSE;
    }
  epoch = g_strdup_printf ("%" PRIu64, self->epoch);
  nevra = modulemd_rpm_map_entry_get_nevra_as_string (self);

  EMIT_MAPPING_START_WITH_STYLE (emitter, error, YAML_BLOCK_MAPPING_STYLE);

  EMIT_KEY_VALUE (emitter, error, "name", self->name);
  EMIT_KEY_VALUE (emitter, error, "epoch", epoch);
  EMIT_KEY_VALUE (emitter, error, "version", self->version);
  EMIT_KEY_VALUE (emitter, error, "release", self->release);
  EMIT_KEY_VALUE (emitter, error, "arch", self->arch);
  EMIT_KEY_VALUE (emitter, error, "nevra", nevra);

  EMIT_MAPPING_END (emitter, &nested_error);

  return TRUE;
}
