/*
 * This file is part of libmodulemd
 * Copyright 2018 Stephen Gallagher
 *
 * Fedora-License-Identifier: MIT
 * SPDX-2.0-License-Identifier: MIT
 * SPDX-3.0-License-Identifier: MIT
 *
 * This program is free software.
 * For more information on the license, see COPYING.
 * For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.
 */


#include "modulemd.h"
#include "modulemd-errors.h"
#include "private/modulemd-defaults-v1-private.h"
#include "private/modulemd-module-index-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-obsoletes-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-translation-private.h"
#include "private/modulemd-yaml.h"

#include <errno.h>
#include <glib.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum mmd_verbosity
{
  MMD_QUIET = -1,
  MMD_DEFAULT,
  MMD_VERBOSE,
  MMD_DEBUG
};

/* Identifiers for modulemd type-version documents.
 * We cannot use GTypes, e.g. MODULEMD_TYPE_PACKAGER_V3, because
 * modulemd-packager-v2 GType does not exist (the format is parsed into
 * a modulemd-v2 data structure). Also GTypes are not constants. They are
 * a run-time computed value and hence cannot be used in switch-cases. */
enum mmd_type
{
    MMD_TYPE_INDEX, /* Untyped validation by loading into an index */
    MMD_TYPE_MODULEMD_V1,
    MMD_TYPE_MODULEMD_V2,
    MMD_TYPE_MODULEMD_DEFAULTS_V1,
    MMD_TYPE_MODULEMD_OBSOLETES_V1,
    MMD_TYPE_MODULEMD_PACKAGER_V2,
    MMD_TYPE_MODULEMD_PACKAGER_V3,
    MMD_TYPE_MODULEMD_TRANSLATIONS_V1
};

struct validator_options
{
  enum mmd_verbosity verbosity;
  GType type;
  gchar **filenames;
};

struct validator_options options = { 0 };

static gboolean
print_version (const gchar *option_name,
               const gchar *value,
               gpointer data,
               GError **error)
{
  g_fprintf (stdout, "modulemd-validator %s\n", modulemd_get_version ());
  exit (EXIT_SUCCESS);
}

static gboolean
set_verbosity (const gchar *option_name,
               const gchar *value,
               gpointer data,
               GError **error)
{
  g_autofree gchar *debugging_env = NULL;
  if (g_strcmp0 ("-v", option_name) == 0 ||
      g_strcmp0 ("--verbose", option_name) == 0)
    {
      if (options.verbosity < MMD_VERBOSE)
        {
          options.verbosity = MMD_VERBOSE;
        }
    }
  else if (g_strcmp0 ("--debug", option_name) == 0)
    {
      if (options.verbosity < MMD_DEBUG)
        {
          options.verbosity = MMD_DEBUG;
          const gchar *old_debug = g_getenv ("G_MESSAGES_DEBUG");
          if (old_debug != NULL)
            {
              debugging_env =
                g_strdup_printf ("%s,%s", old_debug, G_LOG_DOMAIN);
            }
          else
            {
              debugging_env = g_strdup (G_LOG_DOMAIN);
            }
          if (!g_setenv ("G_MESSAGES_DEBUG", debugging_env, TRUE))
            {
              g_set_error (
                error,
                G_OPTION_ERROR,
                G_OPTION_ERROR_FAILED,
                "Could not set G_MESSAGES_DEBUG environment variable");
              return FALSE;
            }
        }
    }
  else if (g_strcmp0 ("-q", option_name) == 0 ||
           g_strcmp0 ("--quiet", option_name) == 0)
    {
      options.verbosity = MMD_QUIET;
    }
  else
    {
      /* We shouldn't be called under any other circumstance */
      g_set_error (error,
                   G_OPTION_ERROR,
                   G_OPTION_ERROR_FAILED,
                   "Called for unknown option \"%s\"",
                   option_name);
      return FALSE;
    }
  return TRUE;
}

static gboolean
set_type (const gchar *option_name,
          const gchar *value,
          gpointer data,
          GError **error)
{
    if (!g_strcmp0 (value, "index"))
        options.type = MMD_TYPE_INDEX;
    else if (!g_strcmp0 (value, "modulemd-v1"))
        options.type = MMD_TYPE_MODULEMD_V1;
    else if (!g_strcmp0 (value, "modulemd-v2"))
        options.type = MMD_TYPE_MODULEMD_V2;
    else if (!g_strcmp0 (value, "modulemd-defaults-v1"))
        options.type = MMD_TYPE_MODULEMD_DEFAULTS_V1;
    else if (!g_strcmp0 (value, "modulemd-obsoletes-v1"))
        options.type = MMD_TYPE_MODULEMD_OBSOLETES_V1;
    else if (!g_strcmp0 (value, "modulemd-packager-v2"))
        options.type = MMD_TYPE_MODULEMD_PACKAGER_V2;
    else if (!g_strcmp0 (value, "modulemd-packager-v3"))
        options.type = MMD_TYPE_MODULEMD_PACKAGER_V3;
    else if (!g_strcmp0 (value, "modulemd-translations-v1"))
        options.type = MMD_TYPE_MODULEMD_TRANSLATIONS_V1;
    else
      {
        g_set_error (error,
                    G_OPTION_ERROR,
                    G_OPTION_ERROR_FAILED,
                    "Unknown document type: %s",
                    value);
        return FALSE;
      }
    return TRUE;
}

static const gchar *
mmd_type2astring (enum mmd_type type)
{
    switch (type)
      {
        case MMD_TYPE_INDEX: return "an index";
        case MMD_TYPE_MODULEMD_V1: return "a modulemd-v1";
        case MMD_TYPE_MODULEMD_V2: return "a modulemd-v2";
        case MMD_TYPE_MODULEMD_DEFAULTS_V1: return "a modulemd-defaults-v1";
        case MMD_TYPE_MODULEMD_OBSOLETES_V1: return "a modulemd-obsoletes-v1";
        case MMD_TYPE_MODULEMD_PACKAGER_V2: return "a modulemd-packager-v2";
        case MMD_TYPE_MODULEMD_PACKAGER_V3: return "a modulemd-packager-v3";
        case MMD_TYPE_MODULEMD_TRANSLATIONS_V1: return "a modulemd-translations-v1";
      }
    return "an unknown document type";
}

// clang-format off
static GOptionEntry entries[] = {
  { "debug", 0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, set_verbosity, "Output debugging messages", NULL },
  { "type", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, set_type, "Document type (index, modulemd-v1, modulemd-v2, modulemd-defaults-v1, modulemd-obsoletes-v1, modulemd-packager-v2, modulemd-packager-v3, modulemd-translations-v1; default is index only which one accepts multi-document YAML files)", NULL },
  { "quiet", 'q', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, set_verbosity, "Print no output", NULL },
  { "verbose", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, set_verbosity, "Be verbose", NULL },
  { "version", 'V', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, print_version, "Print version number, then exit", NULL },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &options.filenames, "Files to be validated", NULL },
  { NULL } };
// clang-format on

static const gchar *
ModulemdYamlDocumentTypeEnum2string (ModulemdYamlDocumentTypeEnum type)
{
  switch (type)
   {
     case MODULEMD_YAML_DOC_MODULESTREAM: return "modulemd";
     case MODULEMD_YAML_DOC_DEFAULTS: return "modulemd-defaults";
     case MODULEMD_YAML_DOC_TRANSLATIONS: return "modulemd-translations";
     case MODULEMD_YAML_DOC_PACKAGER: return "modulemd-packager";
     case MODULEMD_YAML_DOC_OBSOLETES: return "modulemd-obsoletes";
     case MODULEMD_YAML_DOC_UNKNOWN: /* fall through */
     default: return "unknown type";
   }
}

/* We cannot load by index as it converts from old versions before
 * a return and as it does not provide enumeration functions for
 * subdocuments. We will use private modulemd_defaults_v1_parse_yaml() etc.
 * parsers. */
static gboolean
parse_file_as_subdoc (const gchar *filename,
                      enum mmd_type validation_type,
                      ModulemdYamlDocumentTypeEnum expected_type,
                      guint64 expected_version,
                      GError **error)
{
  g_autoptr (FILE) file = NULL;
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  const GError *subdoc_error;
  ModulemdYamlDocumentTypeEnum type;
  guint64 version;
  /*g_autoptr (ModulemdDefaultsV1) defaults_object = NULL;
  g_autoptr (ModulemdDefaultsV1) defaults_object = NULL;*/
  g_autoptr (GObject) object = NULL;

  file = fopen(filename, "r");
  if (!file)
    {
      g_set_error (
        error,
        MODULEMD_YAML_ERROR,
        MMD_YAML_ERROR_OPEN,
        "Could not open %s file: %s",
        filename,
        strerror(errno)
      );
      return FALSE;
    }
  yaml_parser_set_input_file (&parser, file);
  if (!yaml_parser_parse (&parser, &event))
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Invalid YAML"
      );
    }
  if (event.type != YAML_STREAM_START_EVENT) {
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "YAML parser could not find a start of a YAML stream"
      );
    }
  }
  yaml_event_delete (&event);
  if (!yaml_parser_parse (&parser, &event))
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Invalid YAML"
      );
    }
  if (event.type != YAML_DOCUMENT_START_EVENT) {
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "YAML parser could not find a start of a YAML document"
      );
    }
  }
  yaml_event_delete (&event);
  subdoc = modulemd_yaml_parse_document_type (&parser);
  subdoc_error = modulemd_subdocument_info_get_gerror (subdoc);
  if (subdoc_error)
    {
      *error = g_error_copy (subdoc_error);
      return FALSE;
    }
  type = modulemd_subdocument_info_get_doctype (subdoc);
  if (type != expected_type)
    {
      g_set_error(
        error,
        MODULEMD_ERROR,
        0,
        "Not %s document; it is %s",
        mmd_type2astring (validation_type),
        ModulemdYamlDocumentTypeEnum2string (type)
      );
      return FALSE;
    }
  version = modulemd_subdocument_info_get_mdversion (subdoc);
  if (version != expected_version)
    {
      g_set_error(
        error,
        MODULEMD_ERROR,
        0,
        "Not %s document; it is %" G_GUINT64_FORMAT " version",
        mmd_type2astring (validation_type),
        version
      );
      return FALSE;
    }
  if (type == MODULEMD_YAML_DOC_DEFAULTS)
      object = G_OBJECT (modulemd_defaults_v1_parse_yaml (subdoc, TRUE, error));
  else if (type == MODULEMD_YAML_DOC_MODULESTREAM)
      object = G_OBJECT (modulemd_module_stream_v1_parse_yaml (subdoc, TRUE, error));
  else if (type == MODULEMD_YAML_DOC_OBSOLETES)
      object = G_OBJECT (modulemd_obsoletes_parse_yaml (subdoc, TRUE, error));
  else if (type == MODULEMD_YAML_DOC_PACKAGER)
      object = G_OBJECT (modulemd_module_stream_v2_parse_yaml (subdoc,
        TRUE, TRUE, error));
  else if (type == MODULEMD_YAML_DOC_TRANSLATIONS)
      object = G_OBJECT (modulemd_translation_parse_yaml (subdoc, TRUE, error));
  else {
      g_set_error(
        error,
        MODULEMD_ERROR,
        0,
        "Internal error: %s type is not supported",
        mmd_type2astring (validation_type)
      );
      return FALSE;
  }
  /* Check for a garbage past the first document */
  if (!yaml_parser_parse (&parser, &event))
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Invalid YAML after first document"
      );
    }
  if (event.type != YAML_STREAM_END_EVENT) {
    {
      MMD_YAML_ERROR_EVENT_EXIT_BOOL (
        error,
        event,
        "Another YAML document after the first one"
      );
      return FALSE;
    }
  }
  yaml_event_delete (&event);
  /* Already validated by modulemd_defaults_v1_parse_yaml() etc. call. */
  return (NULL != object);
}

static gboolean
parse_file (const gchar *filename, GPtrArray **failures, GError **error)
{
  if (options.verbosity >= MMD_VERBOSE)
    {
      g_fprintf (stdout, "Validating %s\n", filename);
    }

  switch (options.type)
    {
    case MMD_TYPE_INDEX:
      {
        g_autoptr (ModulemdModuleIndex) index = NULL;
        index = modulemd_module_index_new ();
        return modulemd_module_index_update_from_file_ext (
          index, filename, TRUE, TRUE, failures, error);
      }
    case MMD_TYPE_MODULEMD_DEFAULTS_V1:
        return parse_file_as_subdoc (filename,
                                     options.type,
                                     MODULEMD_YAML_DOC_DEFAULTS,
                                     1u,
                                     error);
    case MMD_TYPE_MODULEMD_OBSOLETES_V1:
        return parse_file_as_subdoc (filename,
                                     options.type,
                                     MODULEMD_YAML_DOC_OBSOLETES,
                                     1u,
                                     error);
    case MMD_TYPE_MODULEMD_V1:
        return parse_file_as_subdoc (filename,
                                     options.type,
                                     MODULEMD_YAML_DOC_MODULESTREAM,
                                     1u,
                                     error);
    case MMD_TYPE_MODULEMD_V2:
      {
        GType type;
        g_autoptr (GObject) object = NULL;
        type = modulemd_read_packager_file (filename, &object, error);
        if (type == G_TYPE_INVALID)
          return FALSE;
        if (type != MODULEMD_TYPE_MODULE_STREAM_V2)
          {
            g_set_error(
              error,
              MODULEMD_ERROR,
              0,
              "Not a modulemd-v2 document; it is %s",
              g_type_name(type)
            );
            return FALSE;
          }
        return modulemd_module_stream_validate (MODULEMD_MODULE_STREAM (object),
          error);
      }
    case MMD_TYPE_MODULEMD_PACKAGER_V2:
        return parse_file_as_subdoc (filename,
                                     options.type,
                                     MODULEMD_YAML_DOC_PACKAGER,
                                     2u,
                                     error);
    case MMD_TYPE_MODULEMD_PACKAGER_V3:
      {
        GType type;
        g_autoptr (GObject) object = NULL;
        type = modulemd_read_packager_file (filename, &object, error);
        if (type == G_TYPE_INVALID)
          return FALSE;
        if (type != MODULEMD_TYPE_PACKAGER_V3)
          {
            g_set_error(
              error,
              MODULEMD_ERROR,
              0,
              "Not a modulemd-packager-v3 document; it is %s",
              g_type_name(type)
            );
            return FALSE;
          }
        /* modulemd_packager_v3 is validated implicitly by
         * modulemd_read_packager_file (). */
        return TRUE;
      }
    case MMD_TYPE_MODULEMD_TRANSLATIONS_V1:
        return parse_file_as_subdoc (filename,
                                     options.type,
                                     MODULEMD_YAML_DOC_TRANSLATIONS,
                                     1u,
                                     error);
    }
  g_fprintf (
    stderr,
    "Internal error: unsupported document type: %s\n",
    mmd_type2astring(options.type)
  );
  exit (EXIT_FAILURE);
}


int
main (int argc, char *argv[])
{
  const char *filename;
  g_autoptr (GOptionContext) context = NULL;
  g_autoptr (GError) error = NULL;
  gsize num_invalid = 0;
  gboolean ret;
  g_autoptr (GPtrArray) failures = NULL;
  ModulemdSubdocumentInfo *doc = NULL;

  setlocale (LC_ALL, "");

  options.type = MMD_TYPE_INDEX;
  context = g_option_context_new ("FILES - Simple modulemd YAML validator");
  g_option_context_add_main_entries (context, entries, "modulemd-validator");
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print ("option parsing failed: %s\n", error->message);
      exit (EXIT_FAILURE);
    }

  if (!(options.filenames && options.filenames[0]))
    {
      g_fprintf (stderr,
                 "At least one file must be specified on the command-line\n");
      exit (EXIT_FAILURE);
    }

  for (gsize i = 0; options.filenames[i]; i++)
    {
      filename = options.filenames[i];

      ret = parse_file (filename, &failures, &error);
      if (!ret)
        {
          num_invalid++;
          if (options.verbosity >= MMD_DEFAULT)
            {
              g_fprintf (stderr, "%s failed to validate\n", filename);

              if (error != NULL)
                {
                  /* Unparseable content */
                  g_fprintf (stderr,
                             "%s could not be read in its entirety: %s\n",
                             filename,
                             error->message);
                }
              if (failures)
                {
                  for (gsize j = 0; j < failures->len; j++)
                    {
                      doc = MODULEMD_SUBDOCUMENT_INFO (
                        g_ptr_array_index (failures, j));
                      g_printf (
                        "\nFailed subdocument (%s): \n%s\n",
                        modulemd_subdocument_info_get_gerror (doc)->message,
                        modulemd_subdocument_info_get_yaml (doc));
                    }
                }
            }
        }
      else
        {
          if (options.verbosity >= MMD_DEFAULT)
            {
              g_printf ("%s validated successfully\n", filename);
            }
        }
      g_clear_error (&error);
      g_clear_pointer (&failures, g_ptr_array_unref);
    }

  return num_invalid;
}
