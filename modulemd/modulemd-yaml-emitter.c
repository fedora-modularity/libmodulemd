/* modulemd-yaml-emitter.c
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
#include <glib/gstdio.h>
#include <yaml.h>
#include <errno.h>
#include <inttypes.h>
#include "modulemd.h"
#include "modulemd-yaml.h"

#define YAML_EMITTER_EMIT_WITH_ERROR_RETURN(emitter, event, error, msg)       \
  do                                                                          \
    {                                                                         \
      if (!yaml_emitter_emit (emitter, event))                                \
        {                                                                     \
          g_debug ("Error: %s", msg);                                         \
          g_set_error_literal (                                               \
            error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_EMIT, msg);       \
          goto error;                                                         \
        }                                                                     \
      g_debug ("Emitter event: %u", (event)->type);                           \
    }                                                                         \
  while (0)

#define MMD_YAML_EMITTER_ERROR_RETURN(error, msg)                             \
  do                                                                          \
    {                                                                         \
      g_message (msg);                                                        \
      g_set_error_literal (                                                   \
        error, MODULEMD_YAML_ERROR, MODULEMD_YAML_ERROR_EMIT, msg);           \
      goto error;                                                             \
    }                                                                         \
  while (0)

#define MMD_YAML_EMIT_SCALAR(event, scalar, style)                            \
  do                                                                          \
    {                                                                         \
      yaml_scalar_event_initialize (event,                                    \
                                    NULL,                                     \
                                    NULL,                                     \
                                    (yaml_char_t *)scalar,                    \
                                    (int)strlen (scalar),                     \
                                    1,                                        \
                                    1,                                        \
                                    style);                                   \
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (                                   \
        emitter, event, error, "Error writing scalar");                       \
      g_clear_pointer (&scalar, g_free);                                      \
    }                                                                         \
  while (0)

#define MMD_YAML_EMIT_STR_STR_DICT(event, name, value, style)                 \
  do                                                                          \
    {                                                                         \
      yaml_scalar_event_initialize (event,                                    \
                                    NULL,                                     \
                                    NULL,                                     \
                                    (yaml_char_t *)name,                      \
                                    (int)strlen (name),                       \
                                    1,                                        \
                                    1,                                        \
                                    YAML_PLAIN_SCALAR_STYLE);                 \
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (                                   \
        emitter, event, error, "Error writing name");                         \
      g_clear_pointer (&name, g_free);                                        \
                                                                              \
      yaml_scalar_event_initialize (event,                                    \
                                    NULL,                                     \
                                    NULL,                                     \
                                    (yaml_char_t *)value,                     \
                                    (int)strlen (value),                      \
                                    1,                                        \
                                    1,                                        \
                                    style);                                   \
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (                                   \
        emitter, event, error, "Error writing value");                        \
      g_clear_pointer (&value, g_free);                                       \
    }                                                                         \
  while (0)

static gboolean
_emit_modulemd_document (yaml_emitter_t *emitter,
                         ModulemdModule *module,
                         GError **error);
static gboolean
_emit_modulemd_root (yaml_emitter_t *emitter,
                     ModulemdModule *module,
                     GError **error);
static gboolean
_emit_modulemd_data (yaml_emitter_t *emitter,
                     ModulemdModule *module,
                     GError **error);
static gboolean
_emit_modulemd_licenses (yaml_emitter_t *emitter,
                         ModulemdModule *module,
                         GError **error);
static gboolean
_emit_modulemd_xmd (yaml_emitter_t *emitter,
                    ModulemdModule *module,
                    GError **error);
static gboolean
_emit_modulemd_deps (yaml_emitter_t *emitter,
                     ModulemdModule *module,
                     GError **error);
static gboolean
_emit_modulemd_refs (yaml_emitter_t *emitter,
                     ModulemdModule *module,
                     GError **error);
static gboolean
_emit_modulemd_profiles (yaml_emitter_t *emitter,
                         ModulemdModule *module,
                         GError **error);
static gboolean
_emit_modulemd_api (yaml_emitter_t *emitter,
                    ModulemdModule *module,
                    GError **error);
static gboolean
_emit_modulemd_filters (yaml_emitter_t *emitter,
                        ModulemdModule *module,
                        GError **error);
static gboolean
_emit_modulemd_buildopts (yaml_emitter_t *emitter,
                          ModulemdModule *module,
                          GError **error);
static gboolean
_emit_modulemd_components (yaml_emitter_t *emitter,
                           ModulemdModule *module,
                           GError **error);
static gboolean
_emit_modulemd_artifacts (yaml_emitter_t *emitter,
                          ModulemdModule *module,
                          GError **error);

static gboolean
_emit_modulemd_simpleset (yaml_emitter_t *emitter,
                          ModulemdSimpleSet *set,
                          yaml_sequence_style_t style,
                          GError **error);

typedef struct _hash_entry_s
{
  yaml_emitter_t *emitter;
  GError **error;
  yaml_scalar_style_t style;
} hash_entry_ctx;

static gboolean
_emit_modulemd_hashtable (yaml_emitter_t *emitter,
                          GHashTable *htable,
                          yaml_scalar_style_t style,
                          GError **error);

gboolean
emit_yaml_file (ModulemdModule **modules, const gchar *path, GError **error)
{
  gboolean ret = FALSE;
  FILE *yaml_file = NULL;
  yaml_emitter_t emitter;
  yaml_event_t event;

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (modules, FALSE);

  g_debug ("TRACE: entering emit_yaml_file");

  yaml_emitter_initialize (&emitter);

  errno = 0;
  yaml_file = g_fopen (path, "wb");
  if (!yaml_file)
    {
      g_set_error (error,
                   MODULEMD_YAML_ERROR,
                   MODULEMD_YAML_ERROR_OPEN,
                   "Failed to open file: %s",
                   g_strerror (errno));
      goto error;
    }

  yaml_emitter_set_output_file (&emitter, yaml_file);

  yaml_stream_start_event_initialize (&event, YAML_UTF8_ENCODING);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error starting stream");

  for (gsize i = 0; modules[i]; i++)
    {
      /* Write out the YAML */
      if (!_emit_modulemd_document (&emitter, modules[i], error))
        {
          MMD_YAML_EMITTER_ERROR_RETURN (error, "Could not emit YAML");
        }
    }

  yaml_stream_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error ending stream");

  ret = TRUE;

error:
  yaml_emitter_delete (&emitter);
  fclose (yaml_file);

  g_debug ("TRACE: exiting emit_yaml_file");
  return ret;
}

struct modulemd_yaml_string
{
  char *str;
  size_t len;
};

static int
_write_string (void *data, unsigned char *buffer, size_t size)
{
  struct modulemd_yaml_string *yaml_string =
    (struct modulemd_yaml_string *)data;

  yaml_string->str =
    g_realloc_n (yaml_string->str, yaml_string->len + size + 1, sizeof (char));

  memcpy (yaml_string->str + yaml_string->len, buffer, size);
  yaml_string->len += size;
  yaml_string->str[yaml_string->len] = '\0';

  return 1;
}

gboolean
emit_yaml_string (ModulemdModule **modules, gchar **_yaml, GError **error)
{
  gboolean ret = FALSE;
  yaml_emitter_t emitter;
  yaml_event_t event;
  struct modulemd_yaml_string *yaml_string =
    g_malloc0_n (1, sizeof (struct modulemd_yaml_string));

  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (modules, FALSE);

  g_debug ("TRACE: entering emit_yaml_string");

  yaml_emitter_initialize (&emitter);

  yaml_emitter_set_output (&emitter, _write_string, (void *)yaml_string);

  yaml_stream_start_event_initialize (&event, YAML_UTF8_ENCODING);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error starting stream");

  for (gsize i = 0; modules[i]; i++)
    {
      /* Write out the YAML */
      if (!_emit_modulemd_document (&emitter, modules[i], error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error, "Could not emit YAML");
        }
    }

  yaml_stream_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    &emitter, &event, error, "Error ending stream");

  *_yaml = yaml_string->str;
  ret = TRUE;

error:
  yaml_emitter_delete (&emitter);
  if (ret != TRUE)
    {
      g_clear_pointer (&yaml_string->str, g_free);
    }
  g_free (yaml_string);

  g_debug ("TRACE: exiting emit_yaml_string");
  return ret;
}

static gboolean
_emit_modulemd_document (yaml_emitter_t *emitter,
                         ModulemdModule *module,
                         GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;

  g_debug ("TRACE: entering _emit_modulemd_document");
  yaml_document_start_event_initialize (&event, NULL, NULL, NULL, 0);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting document");

  if (!_emit_modulemd_root (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to process root");
    }

  yaml_document_end_event_initialize (&event, 0);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending document");

  ret = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_document");
  return ret;
}

static gboolean
_emit_modulemd_root (yaml_emitter_t *emitter,
                     ModulemdModule *module,
                     GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  gchar *value = NULL;

  g_debug ("TRACE: entering _emit_modulemd_root");

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting root mapping");


  /* document: modulemd */
  name = g_strdup ("document");
  value = g_strdup ("modulemd");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* The modulemd version */
  name = g_strdup ("version");
  value = g_strdup_printf ("%" PRIu64, modulemd_module_get_mdversion (module));
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* The data */
  name = g_strdup ("data");

  yaml_scalar_event_initialize (&event,
                                NULL,
                                NULL,
                                (yaml_char_t *)name,
                                (int)strlen (name),
                                1,
                                1,
                                YAML_PLAIN_SCALAR_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error writing data");
  g_clear_pointer (&name, g_free);

  if (!_emit_modulemd_data (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit data");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending root mapping");

  ret = TRUE;
error:
  g_free (name);
  g_free (value);

  g_debug ("TRACE: exiting _emit_modulemd_root");
  return ret;
}

static gboolean
_emit_modulemd_data (yaml_emitter_t *emitter,
                     ModulemdModule *module,
                     GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  gchar *value = NULL;
  guint64 version = 0;

  g_debug ("TRACE: entering _emit_modulemd_data");

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting data mapping");


  /* Module name */
  value = g_strdup (modulemd_module_get_name (module));
  if (value)
    {
      name = g_strdup ("name");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }


  /* Module stream */
  value = g_strdup (modulemd_module_get_stream (module));
  if (value)
    {
      name = g_strdup ("stream");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }


  /* Module version */
  version = modulemd_module_get_version (module);
  if (version)
    {
      name = g_strdup ("version");
      value = g_strdup_printf ("%" PRIu64, version);
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }


  /* Module summary */
  value = g_strdup (modulemd_module_get_summary (module));
  if (!value)
    {
      /* Summary is mandatory */
      MMD_YAML_EMITTER_ERROR_RETURN (error,
                                     "Missing required option data.summary");
    }
  name = g_strdup ("summary");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);


  /* Module description */
  value = g_strdup (modulemd_module_get_description (module));
  if (!value)
    {
      /* Description is mandatory */
      MMD_YAML_EMITTER_ERROR_RETURN (
        error, "Missing required option data.description");
    }
  name = g_strdup ("description");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_FOLDED_SCALAR_STYLE);

  /* Module Licenses */
  if (!_emit_modulemd_licenses (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit licenses");
    }


  /* Extensible Metadata Block */
  if (!_emit_modulemd_xmd (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit xmd");
    }


  /* Dependencies */
  if (!_emit_modulemd_deps (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit dependencies");
    }


  /* References */
  if (!_emit_modulemd_refs (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit references");
    }


  /* Profiles */
  if (!_emit_modulemd_profiles (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit references");
    }


  /* API */
  if (!_emit_modulemd_api (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit API");
    }


  /* API */
  if (!_emit_modulemd_filters (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit filters");
    }


  /* Build options */
  if (!_emit_modulemd_buildopts (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit buildopts");
    }


  /* Components */
  if (!_emit_modulemd_components (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit buildopts");
    }


  /* Artifacts */
  if (!_emit_modulemd_artifacts (emitter, module, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Failed to emit buildopts");
    }


  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending data mapping");

  ret = TRUE;
error:
  g_free (name);
  g_free (value);

  g_debug ("TRACE: exiting _emit_modulemd_data");
  return ret;
}

static gboolean
_emit_modulemd_licenses (yaml_emitter_t *emitter,
                         ModulemdModule *module,
                         GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  ModulemdSimpleSet *set = NULL;

  g_debug ("TRACE: entering _emit_modulemd_licenses");

  name = g_strdup ("license");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting license mapping");


  /* Module Licenses */
  set = modulemd_module_get_module_licenses (module);
  if (!set)
    {
      /* Module license is mandatory */
      MMD_YAML_EMITTER_ERROR_RETURN (
        error, "Missing required option data.license.module");
    }
  name = g_strdup ("module");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_modulemd_simpleset (
        emitter, set, YAML_BLOCK_SEQUENCE_STYLE, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing module licenses");
    }
  g_clear_pointer (&set, g_object_unref);


  /* Content licenses */
  set = modulemd_module_get_content_licenses (module);
  if (set)
    {
      /* Content licenses are optional */
      name = g_strdup ("content");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_simpleset (
            emitter, set, YAML_BLOCK_SEQUENCE_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error writing module licenses");
        }
      g_clear_pointer (&set, g_object_unref);
    }


  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending license mapping");

  ret = TRUE;
error:
  g_free (name);
  if (set)
    {
      g_object_unref (set);
    }

  g_debug ("TRACE: exiting _emit_modulemd_licenses");
  return ret;
}

static gboolean
_emit_modulemd_xmd (yaml_emitter_t *emitter,
                    ModulemdModule *module,
                    GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  GHashTable *htable = NULL;

  g_debug ("TRACE: entering _emit_modulemd_xmd");

  htable = modulemd_module_get_xmd (module);
  if (htable)
    {
      name = g_strdup ("xmd");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_hashtable (
            emitter, htable, YAML_PLAIN_SCALAR_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing module xmd");
        }
      g_clear_pointer (&htable, g_hash_table_unref);
    }
  ret = TRUE;
error:
  g_free (name);
  if (htable)
    {
      g_hash_table_unref (htable);
    }

  g_debug ("TRACE: exiting _emit_modulemd_xmd");
  return ret;
}

static gboolean
_emit_modulemd_deps (yaml_emitter_t *emitter,
                     ModulemdModule *module,
                     GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  GHashTable *requires = NULL;
  GHashTable *buildrequires = NULL;

  g_debug ("TRACE: entering _emit_modulemd_deps");

  buildrequires = modulemd_module_get_buildrequires (module);
  requires = modulemd_module_get_requires (module);
  if (!(buildrequires || requires))
    {
      /* No dependencies for this module.
       * Unlikely, but not impossible
       */
      ret = TRUE;
      goto error;
    }

  name = g_strdup ("dependencies");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting dependency mapping");

  if (buildrequires)
    {
      name = g_strdup ("buildrequires");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_hashtable (
            emitter, buildrequires, YAML_PLAIN_SCALAR_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error writing module build deps");
        }
      g_clear_pointer (&buildrequires, g_hash_table_unref);
    }

  if (requires)
    {
      name = g_strdup ("requires");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_hashtable (
            emitter, requires, YAML_PLAIN_SCALAR_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error writing module runtime deps");
        }
      g_clear_pointer (&requires, g_hash_table_unref);
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending dependency mapping");

  ret = TRUE;
error:
  g_free (name);
  if (buildrequires)
    {
      g_hash_table_unref (buildrequires);
    }
  if (requires)
    {
      g_hash_table_unref (requires);
    }

  g_debug ("TRACE: exiting _emit_modulemd_deps");
  return ret;
}

static gboolean
_emit_modulemd_refs (yaml_emitter_t *emitter,
                     ModulemdModule *module,
                     GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  gchar *community = NULL;
  gchar *documentation = NULL;
  gchar *tracker = NULL;

  g_debug ("TRACE: entering _emit_modulemd_refs");

  community = g_strdup (modulemd_module_get_community (module));
  documentation = g_strdup (modulemd_module_get_documentation (module));
  tracker = g_strdup (modulemd_module_get_tracker (module));

  if (!(community || documentation || tracker))
    {
      /* No references for this module. */
      ret = TRUE;
      goto error;
    }

  name = g_strdup ("references");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting reference mapping");

  if (community)
    {
      name = g_strdup ("community");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, community, YAML_PLAIN_SCALAR_STYLE);
    }

  if (documentation)
    {
      name = g_strdup ("documentation");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, documentation, YAML_PLAIN_SCALAR_STYLE);
    }

  if (tracker)
    {
      name = g_strdup ("tracker");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, tracker, YAML_PLAIN_SCALAR_STYLE);
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending reference mapping");

  ret = TRUE;
error:
  g_free (name);
  g_free (community);
  g_free (documentation);
  g_free (tracker);

  g_debug ("TRACE: exiting _emit_modulemd_refs");
  return ret;
}

static void
_emit_profile_entries (gpointer _key, gpointer _value, gpointer user_data)
{
  yaml_event_t event;
  yaml_emitter_t *emitter = ((hash_entry_ctx *)user_data)->emitter;
  GError **error = ((hash_entry_ctx *)user_data)->error;

  if (error && *error)
    {
      /* Don't continue */
      return;
    }

  gchar *name = NULL;
  gchar *description = NULL;
  ModulemdProfile *profile = g_object_ref (MODULEMD_PROFILE (_value));
  ModulemdSimpleSet *rpms = NULL;

  name = g_strdup (_key);
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting profile inner mapping");

  /* Description */
  description = g_strdup (modulemd_profile_get_description (profile));
  if (description)
    {
      name = g_strdup ("description");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, description, YAML_PLAIN_SCALAR_STYLE);
    }

  /* RPMs */
  rpms = modulemd_profile_get_rpms (profile);
  if (rpms && modulemd_simpleset_size (rpms) > 0)
    {
      name = g_strdup ("rpms");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_simpleset (
            emitter, rpms, YAML_BLOCK_SEQUENCE_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing profile rpms");
        }
      g_clear_pointer (&rpms, g_object_unref);
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending profile inner mapping");

error:
  g_free (name);
  g_free (description);
  g_object_unref (profile);

  if (rpms)
    {
      g_object_unref (rpms);
    }
}

static gboolean
_emit_modulemd_profiles (yaml_emitter_t *emitter,
                         ModulemdModule *module,
                         GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  GHashTable *profiles = NULL;
  hash_entry_ctx hctx = { emitter, error };

  g_debug ("TRACE: entering _emit_modulemd_profiles");

  profiles = modulemd_module_get_profiles (module);

  if (!(profiles || g_hash_table_size (profiles) > 0))
    {
      /* No profiles for this module. */
      ret = TRUE;
      goto error;
    }

  name = g_strdup ("profiles");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting profile mapping");

  g_hash_table_foreach (profiles, _emit_profile_entries, &hctx);
  if (error && *error)
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error,
                                     "Error processing profile entries");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending reference mapping");

  ret = TRUE;
error:
  g_free (name);
  if (profiles)
    {
      g_hash_table_unref (profiles);
    }

  g_debug ("TRACE: exiting _emit_modulemd_profiles");
  return ret;
}

static gboolean
_emit_modulemd_api (yaml_emitter_t *emitter,
                    ModulemdModule *module,
                    GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  ModulemdSimpleSet *api = NULL;

  g_debug ("TRACE: entering _emit_modulemd_api");
  api = modulemd_module_get_rpm_api (module);

  name = g_strdup ("api");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting API mapping");

  name = g_strdup ("rpms");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_modulemd_simpleset (
        emitter, api, YAML_BLOCK_SEQUENCE_STYLE, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing API rpms");
    }
  g_clear_pointer (&api, g_object_unref);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending API mapping");

  ret = TRUE;
error:
  g_free (name);
  if (api)
    {
      g_object_unref (api);
    }

  g_debug ("TRACE: exiting _emit_modulemd_api");
  return ret;
}

static gboolean
_emit_modulemd_filters (yaml_emitter_t *emitter,
                        ModulemdModule *module,
                        GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  ModulemdSimpleSet *filters = NULL;

  g_debug ("TRACE: entering _emit_modulemd_filters");
  filters = modulemd_module_get_rpm_filter (module);

  name = g_strdup ("filter");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting filter mapping");

  name = g_strdup ("rpms");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_modulemd_simpleset (
        emitter, filters, YAML_BLOCK_SEQUENCE_STYLE, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing filter rpms");
    }
  g_clear_pointer (&filters, g_object_unref);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending filter mapping");

  ret = TRUE;
error:
  g_free (name);
  if (filters)
    {
      g_object_unref (filters);
    }

  g_debug ("TRACE: exiting _emit_modulemd_filters");
  return ret;
}

static gboolean
_emit_modulemd_buildopts (yaml_emitter_t *emitter,
                          ModulemdModule *module,
                          GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  GHashTable *buildopts = NULL;

  g_debug ("TRACE: entering _emit_modulemd_buildopts");
  buildopts = modulemd_module_get_rpm_buildopts (module);
  if (!(buildopts || g_hash_table_size (buildopts) > 0))
    {
      ret = TRUE;
      goto error;
    }

  name = g_strdup ("buildopts");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting buildopt mapping");

  name = g_strdup ("rpms");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_modulemd_hashtable (
        emitter, buildopts, YAML_LITERAL_SCALAR_STYLE, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing buildopts");
    }
  g_clear_pointer (&buildopts, g_hash_table_unref);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending buildopt mapping");

  ret = TRUE;
error:
  g_free (name);
  if (buildopts)
    {
      g_hash_table_unref (buildopts);
    }

  g_debug ("TRACE: exiting _emit_modulemd_buildopts");
  return ret;
}

static void
_emit_rpm_components (gpointer _key, gpointer _value, gpointer user_data)
{
  yaml_event_t event;
  yaml_emitter_t *emitter = ((hash_entry_ctx *)user_data)->emitter;
  GError **error = ((hash_entry_ctx *)user_data)->error;
  gchar *value = NULL;
  guint64 buildorder;
  ModulemdSimpleSet *set = NULL;

  if (error && *error)
    {
      /* Don't proceed if we failed earlier */
      return;
    }

  gchar *name = g_strdup (_key);
  ModulemdComponentRpm *rpm_component =
    g_object_ref (MODULEMD_COMPONENT_RPM (_value));

  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting RPM component inner mapping");

  /* Rationale */
  value = g_strdup (
    modulemd_component_get_rationale ((MODULEMD_COMPONENT (rpm_component))));
  if (!value)
    {
      MMD_YAML_EMITTER_ERROR_RETURN (error,
                                     "Missing required option: rationale");
    }
  name = g_strdup ("rationale");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  /* Repository */
  value = g_strdup (modulemd_component_rpm_get_repository (rpm_component));
  if (value)
    {
      name = g_strdup ("repository");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Cache */
  value = g_strdup (modulemd_component_rpm_get_cache (rpm_component));
  if (value)
    {
      name = g_strdup ("cache");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Ref */
  value = g_strdup (modulemd_component_rpm_get_ref (rpm_component));
  if (value)
    {
      name = g_strdup ("ref");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Buildorder */
  buildorder =
    modulemd_component_get_buildorder (MODULEMD_COMPONENT (rpm_component));
  if (buildorder)
    {
      name = g_strdup ("buildorder");
      value = g_strdup_printf ("%" PRIu64, buildorder);
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Arches */
  set = modulemd_component_rpm_get_arches (rpm_component);
  if (set && modulemd_simpleset_size (set) > 0)
    {
      name = g_strdup ("arches");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_simpleset (
            emitter, set, YAML_FLOW_SEQUENCE_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Error writing RPM component arches");
        }
    }


  /* Multilib */
  set = modulemd_component_rpm_get_multilib (rpm_component);
  if (set && modulemd_simpleset_size (set) > 0)
    {
      name = g_strdup ("multilib");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      if (!_emit_modulemd_simpleset (
            emitter, set, YAML_FLOW_SEQUENCE_STYLE, error))
        {
          MMD_YAML_ERROR_RETURN_RETHROW (
            error, "Error writing RPM component multilib");
        }
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending RPM component inner mapping");

error:
  g_free (name);
  g_free (value);
  g_object_unref (rpm_component);
  if (set)
    {
      g_object_unref (set);
    }
}

static void
_emit_module_components (gpointer _key, gpointer _value, gpointer user_data)
{
  yaml_event_t event;
  yaml_emitter_t *emitter = ((hash_entry_ctx *)user_data)->emitter;
  GError **error = ((hash_entry_ctx *)user_data)->error;
  gchar *value = NULL;
  guint64 buildorder;

  gchar *name = g_strdup (_key);
  ModulemdComponentModule *module_component =
    g_object_ref (MODULEMD_COMPONENT_MODULE (_value));

  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting module component inner mapping");

  /* Rationale */
  value = g_strdup (modulemd_component_get_rationale (
    (MODULEMD_COMPONENT (module_component))));
  if (!value)
    {
      MMD_YAML_EMITTER_ERROR_RETURN (error,
                                     "Missing required option: rationale");
    }
  name = g_strdup ("rationale");
  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, YAML_PLAIN_SCALAR_STYLE);

  /* Repository */
  value =
    g_strdup (modulemd_component_module_get_repository (module_component));
  if (value)
    {
      name = g_strdup ("repository");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Ref */
  value =
    g_strdup (modulemd_component_module_get_repository (module_component));
  if (value)
    {
      name = g_strdup ("ref");
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  /* Buildorder */
  buildorder =
    modulemd_component_get_buildorder (MODULEMD_COMPONENT (module_component));
  if (buildorder)
    {
      name = g_strdup ("buildorder");
      value = g_strdup_printf ("%" PRIu64, buildorder);
      MMD_YAML_EMIT_STR_STR_DICT (
        &event, name, value, YAML_PLAIN_SCALAR_STYLE);
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending module component inner mapping");

error:
  g_free (value);
  g_object_unref (module_component);
}

static gboolean
_emit_modulemd_components (yaml_emitter_t *emitter,
                           ModulemdModule *module,
                           GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  GHashTable *rpm_components;
  GHashTable *module_components;
  hash_entry_ctx hctx = { emitter, error, YAML_PLAIN_SCALAR_STYLE };

  g_debug ("TRACE: entering _emit_modulemd_components");

  rpm_components = modulemd_module_get_rpm_components (module);
  module_components = modulemd_module_get_module_components (module);

  if (!(rpm_components || module_components))
    {
      /* Omit this section */
      ret = TRUE;
      goto error;
    }

  name = g_strdup ("components");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting component mapping");

  if (rpm_components)
    {
      name = g_strdup ("rpms");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      yaml_mapping_start_event_initialize (
        &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error starting RPM component mapping");
      g_hash_table_foreach (rpm_components, _emit_rpm_components, &hctx);
      if (error && *error)
        {
          MMD_YAML_ERROR_RETURN_RETHROW (error,
                                         "Could not process RPM components");
        }

      yaml_mapping_end_event_initialize (&event);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error ending RPM component mapping");
    }

  if (module_components)
    {
      name = g_strdup ("modules");
      MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

      yaml_mapping_start_event_initialize (
        &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error starting module component mapping");
      g_hash_table_foreach (module_components, _emit_module_components, &hctx);

      yaml_mapping_end_event_initialize (&event);
      YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
        emitter, &event, error, "Error ending module component mapping");
    }

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending component mapping");

  ret = TRUE;
error:
  g_free (name);
  if (rpm_components)
    {
      g_hash_table_unref (rpm_components);
    }
  if (module_components)
    {
      g_hash_table_unref (module_components);
    }

  g_debug ("TRACE: exiting _emit_modulemd_components");
  return ret;
}

static gboolean
_emit_modulemd_artifacts (yaml_emitter_t *emitter,
                          ModulemdModule *module,
                          GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  gchar *name = NULL;
  ModulemdSimpleSet *artifacts = NULL;

  g_debug ("TRACE: entering _emit_modulemd_artifacts");

  artifacts = modulemd_module_get_rpm_artifacts (module);
  name = g_strdup ("artifacts");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);

  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting artifact mapping");

  name = g_strdup ("rpms");
  MMD_YAML_EMIT_SCALAR (&event, name, YAML_PLAIN_SCALAR_STYLE);

  if (!_emit_modulemd_simpleset (
        emitter, artifacts, YAML_BLOCK_SEQUENCE_STYLE, error))
    {
      MMD_YAML_ERROR_RETURN_RETHROW (error, "Error writing artifact rpms");
    }
  g_clear_pointer (&artifacts, g_object_unref);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending artifact mapping");

  ret = TRUE;
error:
  g_free (name);
  if (artifacts)
    {
      g_object_unref (artifacts);
    }

  g_debug ("TRACE: exiting _emit_modulemd_artifacts");
  return ret;
}

static gboolean
_emit_modulemd_simpleset (yaml_emitter_t *emitter,
                          ModulemdSimpleSet *set,
                          yaml_sequence_style_t style,
                          GError **error)
{
  gboolean ret = FALSE;
  gsize i;
  yaml_event_t event;
  gchar **strv = modulemd_simpleset_get_as_strv (set);

  g_debug ("TRACE: entering _emit_modulemd_simpleset");

  yaml_sequence_start_event_initialize (&event, NULL, NULL, 1, style);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting simpleset sequence");

  for (i = 0; strv[i]; i++)
    {
      MMD_YAML_EMIT_SCALAR (&event, strv[i], YAML_PLAIN_SCALAR_STYLE);
    }

  yaml_sequence_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending simpleset sequence");

  ret = TRUE;
error:
  for (gsize i = 0; strv[i]; i++)
    {
      g_free (strv[i]);
    }
  g_free (strv);

  g_debug ("TRACE: exiting _emit_modulemd_simpleset");
  return ret;
}

static void
_emit_hash_entries (gpointer _key, gpointer _value, gpointer user_data)
{
  yaml_event_t event;
  yaml_emitter_t *emitter = ((hash_entry_ctx *)user_data)->emitter;
  GError **error = ((hash_entry_ctx *)user_data)->error;
  yaml_scalar_style_t style = ((hash_entry_ctx *)user_data)->style;

  gchar *name = g_strdup (_key);
  gchar *value = g_strdup (_value);

  MMD_YAML_EMIT_STR_STR_DICT (&event, name, value, style);

error:
  g_free (name);
  g_free (value);
}

static gboolean
_emit_modulemd_hashtable (yaml_emitter_t *emitter,
                          GHashTable *htable,
                          yaml_scalar_style_t style,
                          GError **error)
{
  gboolean ret = FALSE;
  yaml_event_t event;
  hash_entry_ctx hctx = { emitter, error, style };

  g_debug ("TRACE: entering _emit_modulemd_hashtable");

  yaml_mapping_start_event_initialize (
    &event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error starting hashtable mapping");

  g_hash_table_foreach (htable, _emit_hash_entries, &hctx);

  yaml_mapping_end_event_initialize (&event);
  YAML_EMITTER_EMIT_WITH_ERROR_RETURN (
    emitter, &event, error, "Error ending hashtable mapping");

  ret = TRUE;
error:

  g_debug ("TRACE: exiting _emit_modulemd_hashtable");
  return ret;
}
