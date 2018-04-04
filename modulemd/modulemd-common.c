/* modulemd-common.c
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

#include "modulemd.h"
#include "modulemd-yaml.h"
#include "modulemd-util.h"
#include <glib.h>


/**
 * modulemd_objects_from_file:
 * @yaml_file: A YAML file containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.2
 */
GPtrArray *
modulemd_objects_from_file (const gchar *yaml_file, GError **error)
{
  GPtrArray *data = NULL;
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!parse_yaml_file (yaml_file, &data, error))
    {
      return NULL;
    }

  return data;
}

/**
 * modulemd_objects_from_string:
 * @yaml_string: A YAML string containing the module metadata and other related
 * information such as default streams.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Allocates a #GPtrArray of various supported subdocuments from a file.
 *
 * Returns: (array zero-terminated=1) (element-type GObject) (transfer container):
 * A #GPtrArray of various supported subdocuments from a YAML file. These
 * subdocuments will all be GObjects and their type can be identified with
 * G_OBJECT_TYPE(object)
 *
 * Since: 1.2
 */
GPtrArray *
modulemd_objects_from_string (const gchar *yaml_string, GError **error)
{
  GPtrArray *data = NULL;
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!parse_yaml_string (yaml_string, &data, error))
    {
      return NULL;
    }

  return data;
}


/**
 * modulemd_dump:
 * @objects: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * modulemd or related objects to dump to YAML.
 * @yaml_file: The path to the file that should contain the resulting YAML
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Creates a file containing a series of YAML subdocuments, one per object
 * passed in.
 *
 * Since: 1.2
 */
void
modulemd_dump (GPtrArray *objects, const gchar *yaml_file, GError **error)
{
  g_return_if_fail (error == NULL || *error == NULL);

  emit_yaml_file (objects, yaml_file, error);
}


/**
 * modulemd_dumps:
 * @objects: (array zero-terminated=1) (element-type GObject): A #GPtrArray of
 * modulemd or related objects to dump to YAML.
 * @error: (out): A #GError containing additional information if this function
 * fails.
 *
 * Creates a string containing a series of YAML subdocuments, one per object
 * passed in. This string must be freed with g_free() when no longer needed.
 *
 * Since: 1.2
 */
gchar *
modulemd_dumps (GPtrArray *objects, GError **error)
{
  gchar *yaml_string = NULL;

  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!emit_yaml_string (objects, &yaml_string, error))
    {
      return NULL;
    }

  return yaml_string;
}
