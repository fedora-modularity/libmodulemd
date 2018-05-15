/* modulemd-yaml-parser.c
 *
 * Copyright (C) 2018 Stephen Gallagher
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
#include <glib/gstdio.h>
#include <yaml.h>
#include <errno.h>
#include "modulemd.h"
#include "modulemd-yaml.h"
#include "modulemd-util.h"

#define _yaml_parser_defaults_recurse_down(fn)                                \
  do                                                                          \
    {                                                                         \
      result = fn (defaults, parser, error);                                  \
      if (!result)                                                            \
        {                                                                     \
          goto error;                                                         \
        }                                                                     \
    }                                                                         \
  while (0)

static gboolean
_parse_defaults_data (ModulemdDefaults *defaults,
                      yaml_parser_t *parser,
                      GError **error);
static gboolean
_parse_defaults_profiles (ModulemdDefaults *defaults,
                          yaml_parser_t *parser,
                          GError **error);

static gboolean
_parse_defaults_intents (ModulemdDefaults *defaults,
                         yaml_parser_t *parser,
                         GError **error);
static gboolean
_parse_intent (yaml_parser_t *parser,
               const gchar *name,
               ModulemdIntent **intent,
               GError **error);

static gboolean
_parse_intent_profiles (ModulemdIntent *intent,
                        yaml_parser_t *parser,
                        GError **error);


gboolean
_parse_defaults (yaml_parser_t *parser,
                 GObject **object,
                 guint64 version,
                 GError **error)
{
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EVENT (value_event);
  gboolean done = FALSE;
  gboolean result = FALSE;
  const gchar *module_name = NULL;
  guint64 mdversion;
  ModulemdDefaults *defaults = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_defaults");

  defaults = modulemd_defaults_new ();

  /* Use the pre-processed version */
  if (version && version <= MD_DEFAULTS_VERSION_LATEST)
    {
      modulemd_defaults_set_version (defaults, version);
    }
  else
    {
      /* No mdversion was discovered during pre-processing */
      MMD_YAML_ERROR_RETURN (error, "Unknown modulemd defaults version");
    }

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the main document content */
          break;

        case YAML_MAPPING_END_EVENT:
          /* This is the end of the main document content. */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:

          /* Handle "document: modulemd-defaults" */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "document"))
            {
              g_debug ("TRACE: root entry [document]");
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT ||
                  g_strcmp0 ((const gchar *)value_event.data.scalar.value,
                             "modulemd-defaults"))
                {
                  yaml_event_delete (&value_event);
                  MMD_YAML_ERROR_RETURN (error, "Document type mismatch");
                }
              yaml_event_delete (&value_event);
            }
          /* Record the modulemd version for the parser */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "version"))
            {
              g_debug ("TRACE: root entry [version]");
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Unknown modulemd version");
                }

              mdversion = g_ascii_strtoull (
                (const gchar *)value_event.data.scalar.value, NULL, 10);
              yaml_event_delete (&value_event);
              if (!mdversion)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Unknown modulemd defaults version");
                }

              if (mdversion != version)
                {
                  /* Preprocessing and real parser don't match!
                   * This should be impossible
                   */
                  MMD_YAML_ERROR_RETURN (
                    error,
                    "ModuleMD defaults version doesn't match preprocessing");
                }
              modulemd_defaults_set_version (defaults, mdversion);
            }

          /* Process the data section */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "data"))
            {
              g_debug ("TRACE: root entry [data]");
              _yaml_parser_defaults_recurse_down (_parse_defaults_data);
            }

          else
            {
              g_debug ("Unexpected key in root: %s",
                       (const gchar *)event.data.scalar.value);
              MMD_YAML_ERROR_RETURN (error, "Unexpected key in root");
            }
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in root");
          break;
        }

      yaml_event_delete (&event);
    }


  /* We need to validate some things once we have the complete content
   * imported
   */

  /* Ensure that the module name is set */
  module_name = modulemd_defaults_peek_module_name (defaults);
  if (!module_name || !(module_name[0]))
    {
      MMD_YAML_ERROR_RETURN (error, "Module name not specified");
    }

  *object = g_object_ref (G_OBJECT (defaults));
  result = TRUE;

error:
  g_clear_pointer (&defaults, g_object_unref);

  g_debug ("TRACE: exiting _parse_defaults");
  return result;
}

static gboolean
_parse_defaults_data (ModulemdDefaults *defaults,
                      yaml_parser_t *parser,
                      GError **error)
{
  MMD_INIT_YAML_EVENT(event);
  MMD_INIT_YAML_EVENT(value_event);
  gboolean done = FALSE;
  gboolean result = FALSE;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_defaults_data");


  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");
      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the data content. */
          break;

        case YAML_MAPPING_END_EVENT:
          /* This is the end of the data content. */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          /* Module Name */
          if (!g_strcmp0 ((const gchar *)event.data.scalar.value, "module"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error, "Failed to parse module name");
                }

              modulemd_defaults_set_module_name (
                defaults, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }

          /* Module default stream */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "stream"))
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (error,
                                         "Failed to parse module stream");
                }

              modulemd_defaults_set_default_stream (
                defaults, (const gchar *)value_event.data.scalar.value);
              yaml_event_delete (&value_event);
            }

          /* Profile defaults */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "profiles"))
            {
              _yaml_parser_defaults_recurse_down (_parse_defaults_profiles);
            }

          /* Intents (Not currently supported) */
          else if (!g_strcmp0 ((const gchar *)event.data.scalar.value,
                               "intents"))
            {
              _yaml_parser_defaults_recurse_down (_parse_defaults_intents);
            }
          break;


        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Unexpected YAML event in data");
          break;
        }

      yaml_event_delete (&event);
    }

  result = TRUE;

error:
  if (!result)
    {
      g_clear_pointer (&defaults, g_object_unref);
    }

  g_debug ("TRACE: exiting _parse_defaults_data");
  return result;
}

static gboolean
_parse_defaults_profiles (ModulemdDefaults *defaults,
                          yaml_parser_t *parser,
                          GError **error)
{
  MMD_INIT_YAML_EVENT(event);
  gboolean result = FALSE;
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  gchar *stream_name = NULL;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_defaults_profiles");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the profile content. */
          in_map = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the profile content */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              /* We got a scalar where we expected a map */
              MMD_YAML_ERROR_RETURN (error,
                                     "Malformed YAML in intent profiles");
              break;
            }

          /* Each scalar event represents a profile */
          stream_name = g_strdup ((const gchar *)event.data.scalar.value);

          if (!_simpleset_from_sequence (parser, &set, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Invalid sequence");
            }
          modulemd_defaults_assign_profiles_for_stream (
            defaults, stream_name, set);
          g_clear_pointer (&set, g_object_unref);
          g_clear_pointer (&stream_name, g_free);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error,
                                 "Unexpected YAML event in default profiles");
          break;
        }
      yaml_event_delete (&event);
    }

  result = TRUE;

error:
  g_clear_pointer (&set, g_object_unref);
  g_clear_pointer (&stream_name, g_free);
  g_debug ("TRACE: exiting _parse_defaults_profiles");
  return result;
}


static gboolean
_parse_defaults_intents (ModulemdDefaults *defaults,
                         yaml_parser_t *parser,
                         GError **error)
{
  MMD_INIT_YAML_EVENT(event);
  gboolean in_map = FALSE;
  gboolean result = FALSE;
  gboolean done = FALSE;
  g_autoptr (ModulemdIntent) intent = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_defaults_intents");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the intent content. */
          in_map = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the intent content */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              /* We got a scalar where we expected a map */
              MMD_YAML_ERROR_RETURN (error, "Malformed YAML in intents");
              break;
            }

          /* Each scalar event represents an intent */
          if (!_parse_intent (parser,
                              (const gchar *)event.data.scalar.value,
                              &intent,
                              error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not parse intent");
              break;
            }

          modulemd_defaults_add_intent (defaults, intent);
          g_clear_pointer (&intent, g_object_unref);

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Malformed YAML in intents");
          break;
        }
      yaml_event_delete (&event);
    }

  result = TRUE;

error:
  g_debug ("TRACE: exiting _parse_defaults_intents");
  return result;
}

static gboolean
_parse_intent (yaml_parser_t *parser,
               const gchar *name,
               ModulemdIntent **intent,
               GError **error)
{
  gboolean result = FALSE;
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  MMD_INIT_YAML_EVENT(event);
  MMD_INIT_YAML_EVENT(value_event);
  g_autoptr (ModulemdIntent) _intent = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_debug ("TRACE: entering _parse_intent");

  _intent = modulemd_intent_new (name);

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the intent content. */
          in_map = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the intent content */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              /* We got a scalar where we expected a map */
              MMD_YAML_ERROR_RETURN (error, "Malformed YAML in intents");
              break;
            }

          /* Default Stream */

          if (g_strcmp0 ("stream", (const gchar *)event.data.scalar.value) ==
              0)
            {
              YAML_PARSER_PARSE_WITH_ERROR_RETURN (
                parser, &value_event, error, "Parser error");
              if (value_event.type != YAML_SCALAR_EVENT)
                {
                  MMD_YAML_ERROR_RETURN (
                    error, "Failed to parse default module stream");
                }

              modulemd_intent_set_default_stream (
                _intent, (const gchar *)value_event.data.scalar.value);

              yaml_event_delete (&value_event);
            }
          else if (g_strcmp0 ("profiles",
                              (const gchar *)event.data.scalar.value) == 0)
            {
              if (!_parse_intent_profiles (_intent, parser, error))
                {
                  MMD_YAML_ERROR_RETURN_RETHROW (
                    error, "Could not parse intent profiles");
                }
            }
          else
            {
              /* Unexpected key in the map */
              MMD_YAML_ERROR_RETURN (error, "Unexpected key in intent");
              break;
            }

          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error, "Malformed YAML in intents");
          break;
        }
      yaml_event_delete (&event);
    }

  result = TRUE;
  if (intent)
    {
      *intent = g_object_ref (_intent);
    }

error:
  g_debug ("TRACE: exiting _parse_intent");
  return result;
}


static gboolean
_parse_intent_profiles (ModulemdIntent *intent,
                        yaml_parser_t *parser,
                        GError **error)
{
  MMD_INIT_YAML_EVENT(event);
  gboolean result = FALSE;
  gboolean done = FALSE;
  gboolean in_map = FALSE;
  gchar *stream_name = NULL;
  ModulemdSimpleSet *set = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_debug ("TRACE: entering _parse_intent_profiles");

  while (!done)
    {
      YAML_PARSER_PARSE_WITH_ERROR_RETURN (
        parser, &event, error, "Parser error");

      switch (event.type)
        {
        case YAML_MAPPING_START_EVENT:
          /* This is the start of the profile content. */
          in_map = TRUE;
          break;

        case YAML_MAPPING_END_EVENT:
          /* We're done processing the profile content */
          done = TRUE;
          break;

        case YAML_SCALAR_EVENT:
          if (!in_map)
            {
              /* We got a scalar where we expected a map */
              MMD_YAML_ERROR_RETURN (error,
                                     "Malformed YAML in intent profiles");
              break;
            }

          /* Each scalar event represents a profile */
          stream_name = g_strdup ((const gchar *)event.data.scalar.value);

          if (!_simpleset_from_sequence (parser, &set, error))
            {
              MMD_YAML_ERROR_RETURN_RETHROW (error, "Invalid sequence");
            }
          modulemd_intent_assign_profiles_for_stream (
            intent, stream_name, set);
          g_clear_pointer (&set, g_object_unref);
          g_clear_pointer (&stream_name, g_free);
          break;

        default:
          /* We received a YAML event we shouldn't expect at this level */
          MMD_YAML_ERROR_RETURN (error,
                                 "Unexpected YAML event in intent profiles");
          break;
        }
      yaml_event_delete (&event);
    }

  result = TRUE;

error:
  g_clear_pointer (&set, g_object_unref);
  g_clear_pointer (&stream_name, g_free);
  g_debug ("TRACE: exiting _parse_intent_profiles");
  return result;
}
