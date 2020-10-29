/*
 * This file is part of libmodulemd
 * Copyright (C) 2018-2020 Red Hat, Inc.
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
#include <glib/gstdio.h>
#include <inttypes.h>
#include <locale.h>
#include <signal.h>

#include "modulemd-module-index.h"
#include "modulemd-module-stream.h"
#include "private/glib-extensions.h"
#include "private/modulemd-build-config.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-obsoletes-private.h"
#include "private/modulemd-util.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

#define MMD_TEST_DOC_TEXT "http://example.com"
#define MMD_TEST_DOC_TEXT2 "http://redhat.com"
#define MMD_TEST_DOC_PROP "documentation"
#define MMD_TEST_COM_PROP "community"
#define MMD_TEST_DOC_UNICODE_TEXT                                             \
  "À϶￥🌭∮⇒⇔¬β∀₂⌀ıəˈ⍳⍴V)"                           \
  "═€ίζησθლბშიнстемองจึองታሽ።ደለᚢᛞᚦᚹ⠳⠞⠊⠎▉▒▒▓😃"
#define MMD_TEST_TRACKER_PROP "tracker"
#define MMD_TEST_DESC_TEXT "A different description"
#define MMD_TEST_SUM_TEXT "A different summary"


static void
module_stream_test_construct (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      /* Test that the new() function works */
      stream = modulemd_module_stream_new (version, "foo", "latest");
      g_assert_nonnull (stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (stream));

      g_assert_cmpint (
        modulemd_module_stream_get_mdversion (stream), ==, version);
      g_assert_cmpstr (
        modulemd_module_stream_get_module_name (stream), ==, "foo");
      g_assert_cmpstr (
        modulemd_module_stream_get_stream_name (stream), ==, "latest");

      g_clear_object (&stream);

      /* Test that the new() function works without a stream name */
      stream = modulemd_module_stream_new (version, "foo", NULL);
      g_assert_nonnull (stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (stream));

      g_assert_cmpint (
        modulemd_module_stream_get_mdversion (stream), ==, version);
      g_assert_cmpstr (
        modulemd_module_stream_get_module_name (stream), ==, "foo");
      g_assert_null (modulemd_module_stream_get_stream_name (stream));

      g_clear_object (&stream);

      /* Test with no module name */
      stream = modulemd_module_stream_new (version, NULL, NULL);
      g_assert_nonnull (stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (stream));

      g_assert_cmpint (
        modulemd_module_stream_get_mdversion (stream), ==, version);
      g_assert_null (modulemd_module_stream_get_module_name (stream));
      g_assert_null (modulemd_module_stream_get_stream_name (stream));

      g_clear_object (&stream);
    }

  /* Test with a zero mdversion */
  stream = modulemd_module_stream_new (0, "foo", "latest");
  g_assert_null (stream);
  g_clear_object (&stream);


  /* Test with an unknown mdversion */
  stream = modulemd_module_stream_new (
    MD_MODULESTREAM_VERSION_LATEST + 1, "foo", "latest");
  g_assert_null (stream);
  g_clear_object (&stream);
}


static void
module_stream_test_arch (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;
  g_autofree gchar *arch = NULL;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      /* Test the parent class set_arch() and get_arch() */
      stream = modulemd_module_stream_new (version, "foo", "latest");
      g_assert_nonnull (stream);

      g_assert_null (modulemd_module_stream_get_arch (stream));

      // clang-format off
      g_object_get (stream,
                    "arch", &arch,
                    NULL);
      // clang-format on
      g_assert_null (arch);

      modulemd_module_stream_set_arch (stream, "x86_64");
      g_assert_cmpstr (modulemd_module_stream_get_arch (stream), ==, "x86_64");

      // clang-format off
      g_object_set (stream,
                    "arch", "aarch64",
                    NULL);
      g_object_get (stream,
                    "arch", &arch,
                    NULL);
      // clang-format on
      g_assert_cmpstr (arch, ==, "aarch64");
      g_clear_pointer (&arch, g_free);

      g_clear_object (&stream);
    }
}


static void
module_stream_v1_test_licenses (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_auto (GStrv) licenses = NULL;

  stream = modulemd_module_stream_v1_new (NULL, NULL);

  modulemd_module_stream_v1_add_content_license (stream, "GPLv2+");
  licenses = modulemd_module_stream_v1_get_content_licenses_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)licenses, "GPLv2+"));
  g_assert_cmpint (g_strv_length (licenses), ==, 1);

  g_clear_pointer (&licenses, g_strfreev);

  modulemd_module_stream_v1_add_module_license (stream, "MIT");
  licenses = modulemd_module_stream_v1_get_module_licenses_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)licenses, "MIT"));
  g_assert_cmpint (g_strv_length (licenses), ==, 1);

  g_clear_pointer (&licenses, g_strfreev);

  modulemd_module_stream_v1_remove_content_license (stream, "GPLv2+");
  licenses = modulemd_module_stream_v1_get_content_licenses_as_strv (stream);
  g_assert_cmpint (g_strv_length (licenses), ==, 0);

  g_clear_pointer (&licenses, g_strfreev);

  modulemd_module_stream_v1_remove_module_license (stream, "MIT");
  licenses = modulemd_module_stream_v1_get_module_licenses_as_strv (stream);
  g_assert_cmpint (g_strv_length (licenses), ==, 0);

  g_clear_pointer (&licenses, g_strfreev);
  g_clear_object (&stream);
}

static void
module_stream_v2_test_licenses (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_auto (GStrv) licenses = NULL;

  stream = modulemd_module_stream_v2_new (NULL, NULL);

  modulemd_module_stream_v2_add_content_license (stream, "GPLv2+");
  licenses = modulemd_module_stream_v2_get_content_licenses_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)licenses, "GPLv2+"));
  g_assert_cmpint (g_strv_length (licenses), ==, 1);

  g_clear_pointer (&licenses, g_strfreev);

  modulemd_module_stream_v2_add_module_license (stream, "MIT");
  licenses = modulemd_module_stream_v2_get_module_licenses_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)licenses, "MIT"));
  g_assert_cmpint (g_strv_length (licenses), ==, 1);

  g_clear_pointer (&licenses, g_strfreev);

  modulemd_module_stream_v2_remove_content_license (stream, "GPLv2+");
  licenses = modulemd_module_stream_v2_get_content_licenses_as_strv (stream);
  g_assert_cmpint (g_strv_length (licenses), ==, 0);

  g_clear_pointer (&licenses, g_strfreev);

  modulemd_module_stream_v2_remove_module_license (stream, "MIT");
  licenses = modulemd_module_stream_v2_get_module_licenses_as_strv (stream);
  g_assert_cmpint (g_strv_length (licenses), ==, 0);

  g_clear_pointer (&licenses, g_strfreev);
  g_clear_object (&stream);
}

static void
module_stream_v3_test_licenses (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_auto (GStrv) licenses = NULL;

  stream = modulemd_module_stream_v3_new (NULL, NULL);

  modulemd_module_stream_v3_add_content_license (stream, "GPLv2+");
  licenses = modulemd_module_stream_v3_get_content_licenses_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)licenses, "GPLv2+"));
  g_assert_cmpint (g_strv_length (licenses), ==, 1);

  g_clear_pointer (&licenses, g_strfreev);

  modulemd_module_stream_v3_add_module_license (stream, "MIT");
  licenses = modulemd_module_stream_v3_get_module_licenses_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)licenses, "MIT"));
  g_assert_cmpint (g_strv_length (licenses), ==, 1);

  g_clear_pointer (&licenses, g_strfreev);

  modulemd_module_stream_v3_remove_content_license (stream, "GPLv2+");
  licenses = modulemd_module_stream_v3_get_content_licenses_as_strv (stream);
  g_assert_cmpint (g_strv_length (licenses), ==, 0);

  g_clear_pointer (&licenses, g_strfreev);

  modulemd_module_stream_v3_remove_module_license (stream, "MIT");
  licenses = modulemd_module_stream_v3_get_module_licenses_as_strv (stream);
  g_assert_cmpint (g_strv_length (licenses), ==, 0);

  g_clear_pointer (&licenses, g_strfreev);
  g_clear_object (&stream);
}


static void
module_stream_v1_test_profiles (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_autoptr (ModulemdProfile) profile = NULL;
  g_auto (GStrv) profiles = NULL;
  g_auto (GStrv) rpms = NULL;

  stream = modulemd_module_stream_v1_new ("sssd", NULL);

  profile = modulemd_profile_new ("client");
  modulemd_profile_add_rpm (profile, "sssd-client");

  modulemd_module_stream_v1_add_profile (stream, profile);
  profiles = modulemd_module_stream_v1_get_profile_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (profiles), ==, 1);
  g_assert_true (g_strv_contains ((const gchar *const *)profiles, "client"));

  g_clear_pointer (&profiles, g_strfreev);

  rpms = modulemd_profile_get_rpms_as_strv (
    modulemd_module_stream_v1_get_profile (stream, "client"));
  g_assert_true (g_strv_contains ((const gchar *const *)rpms, "sssd-client"));

  modulemd_module_stream_v1_clear_profiles (stream);
  profiles = modulemd_module_stream_v1_get_profile_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (profiles), ==, 0);

  g_clear_object (&stream);
  g_clear_object (&profile);
  g_clear_pointer (&profiles, g_strfreev);
  g_clear_pointer (&rpms, g_strfreev);
}

static void
module_stream_v2_test_profiles (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdProfile) profile = NULL;
  g_auto (GStrv) profiles = NULL;
  g_auto (GStrv) rpms = NULL;

  stream = modulemd_module_stream_v2_new ("sssd", NULL);

  profile = modulemd_profile_new ("client");
  modulemd_profile_add_rpm (profile, "sssd-client");

  modulemd_module_stream_v2_add_profile (stream, profile);
  profiles = modulemd_module_stream_v2_get_profile_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (profiles), ==, 1);
  g_assert_true (g_strv_contains ((const gchar *const *)profiles, "client"));

  g_clear_pointer (&profiles, g_strfreev);

  rpms = modulemd_profile_get_rpms_as_strv (
    modulemd_module_stream_v2_get_profile (stream, "client"));
  g_assert_true (g_strv_contains ((const gchar *const *)rpms, "sssd-client"));

  modulemd_module_stream_v2_clear_profiles (stream);
  profiles = modulemd_module_stream_v2_get_profile_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (profiles), ==, 0);

  g_clear_object (&stream);
  g_clear_object (&profile);
  g_clear_pointer (&profiles, g_strfreev);
  g_clear_pointer (&rpms, g_strfreev);
}

static void
module_stream_v3_test_profiles (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_autoptr (ModulemdProfile) profile = NULL;
  g_auto (GStrv) profiles = NULL;
  g_auto (GStrv) rpms = NULL;

  stream = modulemd_module_stream_v3_new ("sssd", NULL);

  profile = modulemd_profile_new ("client");
  modulemd_profile_add_rpm (profile, "sssd-client");

  modulemd_module_stream_v3_add_profile (stream, profile);
  profiles = modulemd_module_stream_v3_get_profile_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (profiles), ==, 1);
  g_assert_true (g_strv_contains ((const gchar *const *)profiles, "client"));

  g_clear_pointer (&profiles, g_strfreev);

  rpms = modulemd_profile_get_rpms_as_strv (
    modulemd_module_stream_v3_get_profile (stream, "client"));
  g_assert_true (g_strv_contains ((const gchar *const *)rpms, "sssd-client"));

  modulemd_module_stream_v3_clear_profiles (stream);
  profiles = modulemd_module_stream_v3_get_profile_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (profiles), ==, 0);

  g_clear_object (&stream);
  g_clear_object (&profile);
  g_clear_pointer (&profiles, g_strfreev);
  g_clear_pointer (&rpms, g_strfreev);
}


static void
module_stream_v1_test_summary (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  const gchar *summary = NULL;

  stream = modulemd_module_stream_v1_new (NULL, NULL);

  // Check the defaults
  summary = modulemd_module_stream_v1_get_summary (stream, "C");
  g_assert_null (summary);

  // Test setting summary
  modulemd_module_stream_v1_set_summary (stream, MMD_TEST_SUM_TEXT);
  summary = modulemd_module_stream_v1_get_summary (stream, "C");
  g_assert_cmpstr (summary, ==, MMD_TEST_SUM_TEXT);

  // Test setting it back to NULL
  modulemd_module_stream_v1_set_summary (stream, NULL);
  summary = modulemd_module_stream_v1_get_summary (stream, "C");
  g_assert_null (summary);

  // Test setting unicode characters
  modulemd_module_stream_v1_set_summary (stream, MMD_TEST_DOC_UNICODE_TEXT);
  summary = modulemd_module_stream_v1_get_summary (stream, "C");
  g_assert_cmpstr (summary, ==, MMD_TEST_DOC_UNICODE_TEXT);
}

static void
module_stream_v2_test_summary (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  const gchar *summary = NULL;

  stream = modulemd_module_stream_v2_new (NULL, NULL);

  // Check the defaults
  summary = modulemd_module_stream_v2_get_summary (stream, "C");
  g_assert_null (summary);

  // Test setting summary
  modulemd_module_stream_v2_set_summary (stream, MMD_TEST_SUM_TEXT);
  summary = modulemd_module_stream_v2_get_summary (stream, "C");
  g_assert_cmpstr (summary, ==, MMD_TEST_SUM_TEXT);

  // Test setting it back to NULL
  modulemd_module_stream_v2_set_summary (stream, NULL);
  summary = modulemd_module_stream_v2_get_summary (stream, "C");
  g_assert_null (summary);

  // Test setting unicode characters
  modulemd_module_stream_v2_set_summary (stream, MMD_TEST_DOC_UNICODE_TEXT);
  summary = modulemd_module_stream_v2_get_summary (stream, "C");
  g_assert_cmpstr (summary, ==, MMD_TEST_DOC_UNICODE_TEXT);
}


static void
module_stream_v3_test_summary (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  const gchar *summary = NULL;

  stream = modulemd_module_stream_v3_new (NULL, NULL);

  // Check the defaults
  summary = modulemd_module_stream_v3_get_summary (stream, "C");
  g_assert_null (summary);

  // Test setting summary
  modulemd_module_stream_v3_set_summary (stream, MMD_TEST_SUM_TEXT);
  summary = modulemd_module_stream_v3_get_summary (stream, "C");
  g_assert_cmpstr (summary, ==, MMD_TEST_SUM_TEXT);

  // Test setting it back to NULL
  modulemd_module_stream_v3_set_summary (stream, NULL);
  summary = modulemd_module_stream_v3_get_summary (stream, "C");
  g_assert_null (summary);

  // Test setting unicode characters
  modulemd_module_stream_v3_set_summary (stream, MMD_TEST_DOC_UNICODE_TEXT);
  summary = modulemd_module_stream_v3_get_summary (stream, "C");
  g_assert_cmpstr (summary, ==, MMD_TEST_DOC_UNICODE_TEXT);
}


static void
module_stream_v1_test_description (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  const gchar *description = NULL;

  stream = modulemd_module_stream_v1_new (NULL, NULL);

  // Check the defaults
  description = modulemd_module_stream_v1_get_description (stream, "C");
  g_assert_null (description);

  // Test setting description
  modulemd_module_stream_v1_set_description (stream, MMD_TEST_DESC_TEXT);
  description = modulemd_module_stream_v1_get_description (stream, "C");
  g_assert_cmpstr (description, ==, MMD_TEST_DESC_TEXT);

  // Test setting it back to NULL
  modulemd_module_stream_v1_set_description (stream, NULL);
  description = modulemd_module_stream_v1_get_description (stream, "C");
  g_assert_null (description);

  // Test unicode characters
  modulemd_module_stream_v1_set_description (stream,
                                             MMD_TEST_DOC_UNICODE_TEXT);
  description = modulemd_module_stream_v1_get_description (stream, "C");
  g_assert_cmpstr (description, ==, MMD_TEST_DOC_UNICODE_TEXT);

  g_clear_object (&stream);
}

static void
module_stream_v2_test_description (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  const gchar *description = NULL;

  stream = modulemd_module_stream_v2_new (NULL, NULL);

  // Check the defaults
  description = modulemd_module_stream_v2_get_description (stream, "C");
  g_assert_null (description);

  // Test setting description
  modulemd_module_stream_v2_set_description (stream, MMD_TEST_DESC_TEXT);
  description = modulemd_module_stream_v2_get_description (stream, "C");
  g_assert_cmpstr (description, ==, MMD_TEST_DESC_TEXT);

  // Test setting it back to NULL
  modulemd_module_stream_v2_set_description (stream, NULL);
  description = modulemd_module_stream_v2_get_description (stream, "C");
  g_assert_null (description);

  // Test unicode characters
  modulemd_module_stream_v2_set_description (stream,
                                             MMD_TEST_DOC_UNICODE_TEXT);
  description = modulemd_module_stream_v2_get_description (stream, "C");
  g_assert_cmpstr (description, ==, MMD_TEST_DOC_UNICODE_TEXT);

  g_clear_object (&stream);
}

static void
module_stream_v3_test_description (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  const gchar *description = NULL;

  stream = modulemd_module_stream_v3_new (NULL, NULL);

  // Check the defaults
  description = modulemd_module_stream_v3_get_description (stream, "C");
  g_assert_null (description);

  // Test setting description
  modulemd_module_stream_v3_set_description (stream, MMD_TEST_DESC_TEXT);
  description = modulemd_module_stream_v3_get_description (stream, "C");
  g_assert_cmpstr (description, ==, MMD_TEST_DESC_TEXT);

  // Test setting it back to NULL
  modulemd_module_stream_v3_set_description (stream, NULL);
  description = modulemd_module_stream_v3_get_description (stream, "C");
  g_assert_null (description);

  // Test unicode characters
  modulemd_module_stream_v3_set_description (stream,
                                             MMD_TEST_DOC_UNICODE_TEXT);
  description = modulemd_module_stream_v3_get_description (stream, "C");
  g_assert_cmpstr (description, ==, MMD_TEST_DOC_UNICODE_TEXT);

  g_clear_object (&stream);
}


static void
module_stream_v1_test_rpm_api (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_auto (GStrv) rpm_apis = NULL;

  stream = modulemd_module_stream_v1_new ("sssd", NULL);

  modulemd_module_stream_v1_add_rpm_api (stream, "sssd-common");
  rpm_apis = modulemd_module_stream_v1_get_rpm_api_as_strv (stream);

  g_assert_true (
    g_strv_contains ((const gchar *const *)rpm_apis, "sssd-common"));
  g_assert_cmpint (g_strv_length (rpm_apis), ==, 1);

  g_clear_pointer (&rpm_apis, g_strfreev);

  modulemd_module_stream_v1_remove_rpm_api (stream, "sssd-common");
  rpm_apis = modulemd_module_stream_v1_get_rpm_api_as_strv (stream);

  g_assert_cmpint (g_strv_length (rpm_apis), ==, 0);

  g_clear_pointer (&rpm_apis, g_strfreev);
  g_clear_object (&stream);
}

static void
module_stream_v2_test_rpm_api (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_auto (GStrv) rpm_apis = NULL;

  stream = modulemd_module_stream_v2_new ("sssd", NULL);

  modulemd_module_stream_v2_add_rpm_api (stream, "sssd-common");
  rpm_apis = modulemd_module_stream_v2_get_rpm_api_as_strv (stream);

  g_assert_true (
    g_strv_contains ((const gchar *const *)rpm_apis, "sssd-common"));
  g_assert_cmpint (g_strv_length (rpm_apis), ==, 1);

  g_clear_pointer (&rpm_apis, g_strfreev);

  modulemd_module_stream_v2_remove_rpm_api (stream, "sssd-common");
  rpm_apis = modulemd_module_stream_v2_get_rpm_api_as_strv (stream);

  g_assert_cmpint (g_strv_length (rpm_apis), ==, 0);

  g_clear_pointer (&rpm_apis, g_strfreev);
  g_clear_object (&stream);
}

static void
module_stream_v3_test_rpm_api (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_auto (GStrv) rpm_apis = NULL;

  stream = modulemd_module_stream_v3_new ("sssd", NULL);

  modulemd_module_stream_v3_add_rpm_api (stream, "sssd-common");
  rpm_apis = modulemd_module_stream_v3_get_rpm_api_as_strv (stream);

  g_assert_true (
    g_strv_contains ((const gchar *const *)rpm_apis, "sssd-common"));
  g_assert_cmpint (g_strv_length (rpm_apis), ==, 1);

  g_clear_pointer (&rpm_apis, g_strfreev);

  modulemd_module_stream_v3_remove_rpm_api (stream, "sssd-common");
  rpm_apis = modulemd_module_stream_v3_get_rpm_api_as_strv (stream);

  g_assert_cmpint (g_strv_length (rpm_apis), ==, 0);

  g_clear_pointer (&rpm_apis, g_strfreev);
  g_clear_object (&stream);
}


static void
module_stream_v1_test_rpm_filters (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_auto (GStrv) filters = NULL;

  stream = modulemd_module_stream_v1_new ("sssd", NULL);

  // Test add_rpm_filter
  modulemd_module_stream_v1_add_rpm_filter (stream, "foo");
  modulemd_module_stream_v1_add_rpm_filter (stream, "bar");
  filters = modulemd_module_stream_v1_get_rpm_filters_as_strv (stream);

  g_assert_true (g_strv_contains ((const gchar *const *)filters, "foo"));
  g_assert_true (g_strv_contains ((const gchar *const *)filters, "bar"));
  g_assert_cmpint (g_strv_length (filters), ==, 2);
  g_clear_pointer (&filters, g_strfreev);

  // Test remove_rpm_filter
  modulemd_module_stream_v1_remove_rpm_filter (stream, "foo");
  filters = modulemd_module_stream_v1_get_rpm_filters_as_strv (stream);

  g_assert_true (g_strv_contains ((const gchar *const *)filters, "bar"));
  g_assert_cmpint (g_strv_length (filters), ==, 1);
  g_clear_pointer (&filters, g_strfreev);

  // Test clear_rpm_filters
  modulemd_module_stream_v1_clear_rpm_filters (stream);
  filters = modulemd_module_stream_v1_get_rpm_filters_as_strv (stream);
  g_assert_cmpint (g_strv_length (filters), ==, 0);

  g_clear_pointer (&filters, g_strfreev);
  g_clear_object (&stream);
}

static void
module_stream_v2_test_rpm_filters (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_auto (GStrv) filters = NULL;

  stream = modulemd_module_stream_v2_new ("sssd", NULL);

  // Test add_rpm_filter
  modulemd_module_stream_v2_add_rpm_filter (stream, "foo");
  modulemd_module_stream_v2_add_rpm_filter (stream, "bar");
  filters = modulemd_module_stream_v2_get_rpm_filters_as_strv (stream);

  g_assert_true (g_strv_contains ((const gchar *const *)filters, "foo"));
  g_assert_true (g_strv_contains ((const gchar *const *)filters, "bar"));
  g_assert_cmpint (g_strv_length (filters), ==, 2);
  g_clear_pointer (&filters, g_strfreev);

  // Test remove_rpm_filter
  modulemd_module_stream_v2_remove_rpm_filter (stream, "foo");
  filters = modulemd_module_stream_v2_get_rpm_filters_as_strv (stream);

  g_assert_true (g_strv_contains ((const gchar *const *)filters, "bar"));
  g_assert_cmpint (g_strv_length (filters), ==, 1);
  g_clear_pointer (&filters, g_strfreev);

  // Test clear_rpm_filters
  modulemd_module_stream_v2_clear_rpm_filters (stream);
  filters = modulemd_module_stream_v2_get_rpm_filters_as_strv (stream);
  g_assert_cmpint (g_strv_length (filters), ==, 0);

  g_clear_pointer (&filters, g_strfreev);
  g_clear_object (&stream);
}

static void
module_stream_v3_test_rpm_filters (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_auto (GStrv) filters = NULL;

  stream = modulemd_module_stream_v3_new ("sssd", NULL);

  // Test add_rpm_filter
  modulemd_module_stream_v3_add_rpm_filter (stream, "foo");
  modulemd_module_stream_v3_add_rpm_filter (stream, "bar");
  filters = modulemd_module_stream_v3_get_rpm_filters_as_strv (stream);

  g_assert_true (g_strv_contains ((const gchar *const *)filters, "foo"));
  g_assert_true (g_strv_contains ((const gchar *const *)filters, "bar"));
  g_assert_cmpint (g_strv_length (filters), ==, 2);
  g_clear_pointer (&filters, g_strfreev);

  // Test remove_rpm_filter
  modulemd_module_stream_v3_remove_rpm_filter (stream, "foo");
  filters = modulemd_module_stream_v3_get_rpm_filters_as_strv (stream);

  g_assert_true (g_strv_contains ((const gchar *const *)filters, "bar"));
  g_assert_cmpint (g_strv_length (filters), ==, 1);
  g_clear_pointer (&filters, g_strfreev);

  // Test clear_rpm_filters
  modulemd_module_stream_v3_clear_rpm_filters (stream);
  filters = modulemd_module_stream_v3_get_rpm_filters_as_strv (stream);
  g_assert_cmpint (g_strv_length (filters), ==, 0);

  g_clear_pointer (&filters, g_strfreev);
  g_clear_object (&stream);
}


static void
module_stream_test_upgrade_v1_to_v2 (void)
{
  gboolean ret;
  g_autoptr (ModulemdModuleStreamV1) streamV1 = NULL;
  g_autoptr (ModulemdModuleStream) updated_stream = NULL;
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml_str = NULL;

  streamV1 = modulemd_module_stream_v1_new ("SuperModule", "latest");

  modulemd_module_stream_v1_set_summary (streamV1, "Summary");
  modulemd_module_stream_v1_set_description (streamV1, "Description");
  modulemd_module_stream_v1_add_module_license (streamV1, "BSD");

  modulemd_module_stream_v1_add_buildtime_requirement (
    streamV1, "ModuleA", "streamZ");
  modulemd_module_stream_v1_add_buildtime_requirement (
    streamV1, "ModuleB", "streamY");
  modulemd_module_stream_v1_add_runtime_requirement (
    streamV1, "ModuleA", "streamZ");
  modulemd_module_stream_v1_add_runtime_requirement (
    streamV1, "ModuleB", "streamY");

  updated_stream = modulemd_module_stream_upgrade (
    MODULEMD_MODULE_STREAM (streamV1), MD_MODULESTREAM_VERSION_TWO, &error);

  g_assert_no_error (error);
  g_assert_nonnull (updated_stream);

  index = modulemd_module_index_new ();
  ret = modulemd_module_index_add_module_stream (
    index, MODULEMD_MODULE_STREAM (updated_stream), &error);

  g_assert_no_error (error);
  g_assert_true (ret);

  yaml_str = modulemd_module_index_dump_to_string (index, &error);

  g_assert_no_error (error);
  g_assert_cmpstr (yaml_str,
                   ==,
                   "---\n"
                   "document: modulemd\n"
                   "version: 2\n"
                   "data:\n"
                   "  name: SuperModule\n"
                   "  stream: \"latest\"\n"
                   "  summary: Summary\n"
                   "  description: >-\n"
                   "    Description\n"
                   "  license:\n"
                   "    module:\n"
                   "    - BSD\n"
                   "  dependencies:\n"
                   "  - buildrequires:\n"
                   "      ModuleA: [streamZ]\n"
                   "      ModuleB: [streamY]\n"
                   "    requires:\n"
                   "      ModuleA: [streamZ]\n"
                   "      ModuleB: [streamY]\n"
                   "...\n");

  g_clear_object (&streamV1);
  g_clear_object (&updated_stream);
  g_clear_object (&index);
  g_clear_object (&error);
  g_clear_pointer (&yaml_str, g_free);
}

static void
module_stream_test_upgrade_v2_to_v3 (void)
{
  /* TODO: implement test */
  g_autoptr (ModulemdModuleStream) stream = NULL;
  ModulemdModuleStreamV2 *streamV2 = NULL;
  g_autoptr (ModulemdModuleIndex) index = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *yaml_str = NULL;

  stream = modulemd_module_stream_read_string (
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  name: modulename\n"
    "  stream: streamname\n"
    "  version: 1\n"
    "  context: c0ffe3\n"
    "  arch: x86_64\n"
    "  summary: Module Summary\n"
    "  description: >-\n"
    "    Module Description\n"
    "  api:\n"
    "    rpms:\n"
    "      - rpm_a\n"
    "      - rpm_b\n"
    "  filter:\n"
    "    rpms: rpm_c\n"

    "  artifacts:\n"
    "    rpms:\n"
    "      - bar-0:1.23-1.module_deadbeef.x86_64\n"

    "  servicelevels:\n"
    "    rawhide: {}\n"
    "    production:\n"
    "      eol: 2099-12-31\n"

    "  license:\n"
    "    content:\n"
    "      - BSD\n"
    "      - GPLv2+\n"
    "    module: MIT\n"

    "  dependencies:\n"
    "    - buildrequires:\n"
    "          platform: [f27, f28, epel7]\n"
    "      requires:\n"
    "          platform: [f27, f28, epel7]\n"
    "    - buildrequires:\n"
    "          platform: [f27]\n"
    "          buildtools: [v1, v2]\n"
    "          compatible: [v3]\n"
    "      requires:\n"
    "          platform: [f27]\n"
    "          compatible: [v3, v4]\n"
    "    - buildrequires:\n"
    "          platform: [f28]\n"
    "      requires:\n"
    "          platform: [f28]\n"
    "          runtime: [a, b]\n"
    "    - buildrequires:\n"
    "          platform: [epel7]\n"
    "          extras: [v1]\n"
    "          moreextras: [foo, bar]\n"
    "      requires:\n"
    "          platform: [epel7]\n"
    "          extras: [v1]\n"
    "          moreextras: [foo, bar]\n"
    "  references:\n"
    "        community: http://www.example.com/\n"
    "        documentation: http://www.example.com/\n"
    "        tracker: http://www.example.com/\n"
    "  profiles:\n"
    "        default:\n"
    "            rpms:\n"
    "                - bar\n"
    "                - bar-extras\n"
    "                - baz\n"
    "        container:\n"
    "            rpms:\n"
    "                - bar\n"
    "                - bar-devel\n"
    "        minimal:\n"
    "            description: Minimal profile installing only the bar "
    "package.\n"
    "            rpms:\n"
    "                - bar\n"
    "        buildroot:\n"
    "            rpms:\n"
    "                - bar-devel\n"
    "        srpm-buildroot:\n"
    "            rpms:\n"
    "                - bar-extras\n"
    "  buildopts:\n"
    "        rpms:\n"
    "            macros: |\n"
    "                %demomacro 1\n"
    "                %demomacro2 %{demomacro}23\n"
    "            whitelist:\n"
    "                - fooscl-1-bar\n"
    "                - fooscl-1-baz\n"
    "                - xxx\n"
    "                - xyz\n"
    "        arches: [i686, x86_64]\n"
    "  components:\n"
    "        rpms:\n"
    "            bar:\n"
    "                rationale: We need this to demonstrate stuff.\n"
    "                repository: https://pagure.io/bar.git\n"
    "                cache: https://example.com/cache\n"
    "                ref: 26ca0c0\n"
    "            baz:\n"
    "                rationale: This one is here to demonstrate other stuff.\n"
    "            xxx:\n"
    "                rationale: xxx demonstrates arches and multilib.\n"
    "                arches: [i686, x86_64]\n"
    "                multilib: [x86_64]\n"
    "            xyz:\n"
    "                rationale: xyz is a bundled dependency of xxx.\n"
    "                buildorder: 10\n"
    "        modules:\n"
    "            includedmodule:\n"
    "                rationale: Included in the stack, just because.\n"
    "                repository: https://pagure.io/includedmodule.git\n"
    "                ref: somecoolbranchname\n"
    "                buildorder: 100\n"
    "  xmd:\n"
    "        some_key: some_data\n"
    "        some_list:\n"
    "            - a\n"
    "            - b\n"
    "        some_dict:\n"
    "            a: alpha\n"
    "            b: beta\n"
    "            some_other_list:\n"
    "                - c\n"
    "                - d\n"
    "            some_other_dict:\n"
    "                another_key: more_data\n"
    "                yet_another_key:\n"
    "                    - this\n"
    "                    - is\n"
    "                    - getting\n"
    "                    - silly\n"
    "        can_bool: TRUE\n"
    "...\n",
    TRUE,
    NULL,
    NULL,
    &error);

  g_assert_no_error (error);
  g_assert_nonnull (stream);

  streamV2 = MODULEMD_MODULE_STREAM_V2 (stream);
  g_assert_nonnull (streamV2);

  index = modulemd_module_stream_upgrade_v2_to_v3_ext (stream, &error);
  g_assert_no_error (error);
  g_assert_nonnull (index);

  yaml_str = modulemd_module_index_dump_to_string (index, &error);

  g_assert_no_error (error);

  g_assert_nonnull (yaml_str);
  /* g_assert_cmpstr (yaml_str, ==, ""); */

  g_debug ("YAML dump of upgraded module index:\n%s", yaml_str);

  /* TODO: fix this test to do something useful */

  g_clear_pointer (&yaml_str, g_free);
}

static void
module_stream_test_upgrade_v1_to_v3 (void)
{
  /* TODO: implement test */
}


static void
module_stream_test_stream_deps_expansion_v2_to_v3 (void)
{
  gboolean ret;
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdDependencies) dep = NULL;
  g_autoptr (GPtrArray) expanded_deps = NULL;
  g_autoptr (GError) error = NULL;
  ModulemdBuildConfig *ex_dep = NULL;
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);

  dep = modulemd_dependencies_new ();

  modulemd_dependencies_add_buildtime_stream (dep, "buildtools", "v1");
  modulemd_dependencies_add_buildtime_stream (dep, "buildtools", "v2");
  modulemd_dependencies_add_buildtime_stream (dep, "compatible", "v3");
  modulemd_dependencies_add_buildtime_stream (dep, "platform", "f27");
  modulemd_dependencies_add_buildtime_stream (dep, "platform", "f28");

  modulemd_dependencies_add_runtime_stream (dep, "compatible", "v3");
  modulemd_dependencies_add_runtime_stream (dep, "compatible", "v4");
  modulemd_dependencies_add_runtime_stream (dep, "platform", "f27");
  modulemd_dependencies_add_runtime_stream (dep, "platform", "f28");

  stream = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream, dep);

  expanded_deps = modulemd_module_stream_expand_v2_to_v3_deps (stream, &error);
  g_assert_no_error (error);
  g_assert_nonnull (expanded_deps);

  g_debug ("Got %d expanded dependencies", expanded_deps->len);

  /* validate each dependency and dump as YAML for debugging */
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  for (guint i = 0; i < expanded_deps->len; i++)
    {
      ex_dep = (ModulemdBuildConfig *)g_ptr_array_index (expanded_deps, i);
      g_assert_true (MODULEMD_IS_BUILD_CONFIG (ex_dep));

      g_assert_true (modulemd_build_config_validate (ex_dep, &error));

      g_assert_true (mmd_emitter_start_document (&emitter, &error));
      ret = modulemd_build_config_emit_yaml (ex_dep, &emitter, &error);
      g_assert_no_error (error);
      g_assert_true (ret);
      g_assert_true (mmd_emitter_end_document (&emitter, &error));
    }
  g_assert_true (mmd_emitter_end_stream (&emitter, &error));
  g_debug ("YAML dump of expanded dependencies:\n%s", yaml_string->str);

  /* TODO: fix this test to do something useful */

  g_clear_pointer (&expanded_deps, g_ptr_array_unref);
  g_clear_object (&error);
  g_clear_object (&dep);
  g_clear_object (&stream);
}

static void
module_stream_test_stream_deps_expansion_v2_to_v3_no_streams (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdDependencies) dep = NULL;
  g_autoptr (GPtrArray) expanded_deps = NULL;
  g_autoptr (GError) error = NULL;

  /* Only the MBS can do "all active existing streams" expansion */

  dep = modulemd_dependencies_new ();

  modulemd_dependencies_set_empty_buildtime_dependencies_for_module (
    dep, "buildtime_no_deps");

  stream = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream, dep);

  expanded_deps = modulemd_module_stream_expand_v2_to_v3_deps (stream, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_UPGRADE);
  g_assert_null (expanded_deps);

  g_clear_error (&error);
  g_clear_object (&dep);
  g_clear_object (&stream);
  g_clear_pointer (&expanded_deps, g_ptr_array_unref);

  dep = modulemd_dependencies_new ();

  modulemd_dependencies_set_empty_runtime_dependencies_for_module (
    dep, "runtime_no_deps");

  stream = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream, dep);

  expanded_deps = modulemd_module_stream_expand_v2_to_v3_deps (stream, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_UPGRADE);
  g_assert_null (expanded_deps);

  g_clear_error (&error);
  g_clear_object (&dep);
  g_clear_object (&stream);
  g_clear_pointer (&expanded_deps, g_ptr_array_unref);
}

static void
module_stream_test_stream_deps_expansion_v2_to_v3_exclusions (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdDependencies) dep = NULL;
  g_autoptr (GPtrArray) expanded_deps = NULL;
  g_autoptr (GError) error = NULL;

  /* Only the MBS can do expansion with stream exclusions */

  dep = modulemd_dependencies_new ();

  modulemd_dependencies_add_buildtime_stream (dep, "platform", "-f27");

  stream = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream, dep);

  expanded_deps = modulemd_module_stream_expand_v2_to_v3_deps (stream, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_UPGRADE);
  g_assert_null (expanded_deps);

  g_clear_error (&error);
  g_clear_object (&dep);
  g_clear_object (&stream);
  g_clear_pointer (&expanded_deps, g_ptr_array_unref);

  dep = modulemd_dependencies_new ();

  modulemd_dependencies_add_runtime_stream (dep, "platform", "-f27");

  stream = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream, dep);

  expanded_deps = modulemd_module_stream_expand_v2_to_v3_deps (stream, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_UPGRADE);
  g_assert_null (expanded_deps);

  g_clear_error (&error);
  g_clear_object (&dep);
  g_clear_object (&stream);
  g_clear_pointer (&expanded_deps, g_ptr_array_unref);
}

static void
module_stream_test_stream_deps_expansion_v2_to_v3_no_platform (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdDependencies) dep = NULL;
  g_autoptr (GPtrArray) expanded_deps = NULL;
  g_autoptr (GError) error = NULL;

  /* Dependencies must have either a buildtime or runtime platform to be */
  /* expanded */

  dep = modulemd_dependencies_new ();

  modulemd_dependencies_add_buildtime_stream (dep, "foo", "A");
  modulemd_dependencies_add_buildtime_stream (dep, "foo", "B");

  stream = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream, dep);

  expanded_deps = modulemd_module_stream_expand_v2_to_v3_deps (stream, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_UPGRADE);
  g_assert_null (expanded_deps);

  g_clear_error (&error);
  g_clear_object (&dep);
  g_clear_object (&stream);
  g_clear_pointer (&expanded_deps, g_ptr_array_unref);

  dep = modulemd_dependencies_new ();

  modulemd_dependencies_add_runtime_stream (dep, "bar", "C");
  modulemd_dependencies_add_runtime_stream (dep, "bar", "D");

  stream = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream, dep);

  expanded_deps = modulemd_module_stream_expand_v2_to_v3_deps (stream, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_UPGRADE);
  g_assert_null (expanded_deps);

  g_clear_error (&error);
  g_clear_object (&dep);
  g_clear_object (&stream);
  g_clear_pointer (&expanded_deps, g_ptr_array_unref);
}

static void
module_stream_test_stream_deps_expansion_v2_to_v3_conflicting_platforms (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdDependencies) dep = NULL;
  g_autoptr (GPtrArray) expanded_deps = NULL;
  g_autoptr (GError) error = NULL;

  /* Dependencies can't be expanded if they have only conflicting buildtime */
  /* and runtime platforms */

  dep = modulemd_dependencies_new ();

  modulemd_dependencies_add_buildtime_stream (dep, "platform", "f32");
  modulemd_dependencies_add_runtime_stream (dep, "platform", "f33");

  stream = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream, dep);

  expanded_deps = modulemd_module_stream_expand_v2_to_v3_deps (stream, &error);
  g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_UPGRADE);
  g_assert_null (expanded_deps);

  g_clear_error (&error);
  g_clear_object (&dep);
  g_clear_object (&stream);
  g_clear_pointer (&expanded_deps, g_ptr_array_unref);
}


static void
module_stream_test_v2_yaml (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  ModulemdModuleStreamV2 *streamV2 = NULL;
  g_autofree gchar *module_name_prop = NULL;
  g_autofree gchar *stream_name_prop = NULL;
  g_autofree gchar *context_prop = NULL;
  g_autofree gchar *arch_prop = NULL;
  guint64 version_prop = 0;
  g_autoptr (GError) error = NULL;

  g_auto (GStrv) rpm_apis = NULL;
  g_auto (GStrv) rpm_filters = NULL;
  g_auto (GStrv) rpm_artifacts = NULL;
  g_auto (GStrv) servicelevel_names = NULL;

  ModulemdServiceLevel *sl = NULL;
  g_autofree gchar *sl_name_prop = NULL;
  g_autofree gchar *sl_eol_string = NULL;

  g_auto (GStrv) content_licenses = NULL;
  g_auto (GStrv) module_licenses = NULL;
  g_auto (GStrv) stream_dependencies = NULL;

  g_autofree gchar *community_prop = NULL;
  g_autofree gchar *documentation_prop = NULL;
  g_autofree gchar *tracker_prop = NULL;
  g_auto (GStrv) profile_names = NULL;

  ModulemdBuildopts *buildopts = NULL;
  g_autofree gchar *buildopts_rpm_macros_prop = NULL;
  g_auto (GStrv) buildopts_rpm_whitelist = NULL;
  g_auto (GStrv) buildopts_arches = NULL;

  GVariant *tmp_variant = NULL;
  GVariantDict *xmd_dict = NULL;
  GVariantDict *tmp_dict = NULL;
  gchar *tmp_str = NULL;
  gboolean *tmp_bool = NULL;

  GPtrArray *dependencies = NULL;

  stream = modulemd_module_stream_read_string (
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  name: modulename\n"
    "  stream: \"streamname\"\n"
    "  version: 1\n"
    "  context: c0ffe3\n"
    "  arch: x86_64\n"
    "  summary: Module Summary\n"
    "  description: >-\n"
    "    Module Description\n"
    "  api:\n"
    "    rpms:\n"
    "      - rpm_a\n"
    "      - rpm_b\n"
    "  filter:\n"
    "    rpms: rpm_c\n"

    "  artifacts:\n"
    "    rpms:\n"
    "      - bar-0:1.23-1.module_deadbeef.x86_64\n"

    "  servicelevels:\n"
    "    rawhide: {}\n"
    "    production:\n"
    "      eol: 2099-12-31\n"

    "  license:\n"
    "    content:\n"
    "      - BSD\n"
    "      - GPLv2+\n"
    "    module: MIT\n"

    "  dependencies:\n"
    "    - buildrequires:\n"
    "          platform: [-f27, -f28, -epel7]\n"
    "      requires:\n"
    "          platform: [-f27, -f28, -epel7]\n"
    "    - buildrequires:\n"
    "          platform: [f27]\n"
    "          buildtools: [v1, v2]\n"
    "          compatible: [v3]\n"
    "      requires:\n"
    "          platform: [f27]\n"
    "          compatible: [v3, v4]\n"
    "    - buildrequires:\n"
    "          platform: [f28]\n"
    "      requires:\n"
    "          platform: [f28]\n"
    "          runtime: [a, b]\n"
    "    - buildrequires:\n"
    "          platform: [epel7]\n"
    "          extras: []\n"
    "          moreextras: [foo, bar]\n"
    "      requires:\n"
    "          platform: [epel7]\n"
    "          extras: []\n"
    "          moreextras: [foo, bar]\n"
    "  references:\n"
    "        community: http://www.example.com/\n"
    "        documentation: http://www.example.com/\n"
    "        tracker: http://www.example.com/\n"
    "  profiles:\n"
    "        default:\n"
    "            rpms:\n"
    "                - bar\n"
    "                - bar-extras\n"
    "                - baz\n"
    "        container:\n"
    "            rpms:\n"
    "                - bar\n"
    "                - bar-devel\n"
    "        minimal:\n"
    "            description: Minimal profile installing only the bar "
    "package.\n"
    "            rpms:\n"
    "                - bar\n"
    "        buildroot:\n"
    "            rpms:\n"
    "                - bar-devel\n"
    "        srpm-buildroot:\n"
    "            rpms:\n"
    "                - bar-extras\n"
    "  buildopts:\n"
    "        rpms:\n"
    "            macros: |\n"
    "                %demomacro 1\n"
    "                %demomacro2 %{demomacro}23\n"
    "            whitelist:\n"
    "                - fooscl-1-bar\n"
    "                - fooscl-1-baz\n"
    "                - xxx\n"
    "                - xyz\n"
    "        arches: [i686, x86_64]\n"
    "  components:\n"
    "        rpms:\n"
    "            bar:\n"
    "                rationale: We need this to demonstrate stuff.\n"
    "                repository: https://pagure.io/bar.git\n"
    "                cache: https://example.com/cache\n"
    "                ref: 26ca0c0\n"
    "            baz:\n"
    "                rationale: This one is here to demonstrate other stuff.\n"
    "            xxx:\n"
    "                rationale: xxx demonstrates arches and multilib.\n"
    "                arches: [i686, x86_64]\n"
    "                multilib: [x86_64]\n"
    "            xyz:\n"
    "                rationale: xyz is a bundled dependency of xxx.\n"
    "                buildorder: 10\n"
    "        modules:\n"
    "            includedmodule:\n"
    "                rationale: Included in the stack, just because.\n"
    "                repository: https://pagure.io/includedmodule.git\n"
    "                ref: somecoolbranchname\n"
    "                buildorder: 100\n"
    "  xmd:\n"
    "        some_key: some_data\n"
    "        some_list:\n"
    "            - a\n"
    "            - b\n"
    "        some_dict:\n"
    "            a: alpha\n"
    "            b: beta\n"
    "            some_other_list:\n"
    "                - c\n"
    "                - d\n"
    "            some_other_dict:\n"
    "                another_key: more_data\n"
    "                yet_another_key:\n"
    "                    - this\n"
    "                    - is\n"
    "                    - getting\n"
    "                    - silly\n"
    "        can_bool: TRUE\n"
    "...\n",
    TRUE,
    NULL,
    NULL,
    &error);

  g_assert_no_error (error);

  g_assert_nonnull (stream);
  streamV2 = MODULEMD_MODULE_STREAM_V2 (stream);

  g_object_get (streamV2, "module-name", &module_name_prop, NULL);
  g_object_get (streamV2, "stream-name", &stream_name_prop, NULL);
  g_object_get (streamV2, "version", &version_prop, NULL);
  g_object_get (streamV2, "context", &context_prop, NULL);
  g_object_get (streamV2, "arch", &arch_prop, NULL);

  g_assert_cmpstr (module_name_prop, ==, "modulename");
  g_assert_cmpstr (stream_name_prop, ==, "streamname");
  g_assert_cmpuint (version_prop, ==, 1);
  g_assert_cmpstr (context_prop, ==, "c0ffe3");
  g_assert_cmpstr (arch_prop, ==, "x86_64");
  g_assert_cmpstr (modulemd_module_stream_v2_get_summary (streamV2, "C"),
                   ==,
                   "Module Summary");
  g_assert_cmpstr (modulemd_module_stream_v2_get_description (streamV2, "C"),
                   ==,
                   "Module Description");

  rpm_apis = modulemd_module_stream_v2_get_rpm_api_as_strv (streamV2);
  rpm_filters = modulemd_module_stream_v2_get_rpm_filters_as_strv (streamV2);
  rpm_artifacts =
    modulemd_module_stream_v2_get_rpm_artifacts_as_strv (streamV2);
  servicelevel_names =
    modulemd_module_stream_v2_get_servicelevel_names_as_strv (streamV2);

  g_assert_true (g_strv_contains ((const gchar *const *)rpm_apis, "rpm_a"));
  g_assert_true (g_strv_contains ((const gchar *const *)rpm_apis, "rpm_b"));

  g_assert_true (g_strv_contains ((const gchar *const *)rpm_filters, "rpm_c"));

  g_assert_true (g_strv_contains ((const gchar *const *)rpm_artifacts,
                                  "bar-0:1.23-1.module_deadbeef.x86_64"));

  g_assert_true (
    g_strv_contains ((const gchar *const *)servicelevel_names, "rawhide"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)servicelevel_names, "production"));

  sl = modulemd_module_stream_v2_get_servicelevel (streamV2, "rawhide");
  g_assert_nonnull (sl);
  g_object_get (sl, "name", &sl_name_prop, NULL);
  g_assert_cmpstr (sl_name_prop, ==, "rawhide");
  g_assert_null (modulemd_service_level_get_eol (sl));

  g_clear_pointer (&sl_name_prop, g_free);

  sl = modulemd_module_stream_v2_get_servicelevel (streamV2, "production");
  g_assert_nonnull (sl);
  g_object_get (sl, "name", &sl_name_prop, NULL);
  g_assert_cmpstr (sl_name_prop, ==, "production");
  g_assert_nonnull (modulemd_service_level_get_eol (sl));
  sl_eol_string = modulemd_service_level_get_eol_as_string (sl);
  g_assert_cmpstr (sl_eol_string, ==, "2099-12-31");

  g_clear_pointer (&sl_name_prop, g_free);
  g_clear_pointer (&sl_eol_string, g_free);

  content_licenses =
    modulemd_module_stream_v2_get_content_licenses_as_strv (streamV2);
  module_licenses =
    modulemd_module_stream_v2_get_module_licenses_as_strv (streamV2);

  g_assert_true (
    g_strv_contains ((const gchar *const *)content_licenses, "BSD"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)content_licenses, "GPLv2+"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)module_licenses, "MIT"));

  dependencies = modulemd_module_stream_v2_get_dependencies (streamV2);
  g_assert_cmpint (dependencies->len, ==, 4);

  g_object_get (streamV2, "community", &community_prop, NULL);
  g_object_get (streamV2, "documentation", &documentation_prop, NULL);
  g_object_get (streamV2, "tracker", &tracker_prop, NULL);

  g_assert_cmpstr (community_prop, ==, "http://www.example.com/");
  g_assert_cmpstr (documentation_prop, ==, "http://www.example.com/");
  g_assert_cmpstr (tracker_prop, ==, "http://www.example.com/");

  profile_names =
    modulemd_module_stream_v2_get_profile_names_as_strv (streamV2);
  g_assert_cmpint (g_strv_length (profile_names), ==, 5);

  buildopts = modulemd_module_stream_v2_get_buildopts (streamV2);
  g_assert_nonnull (buildopts);

  g_object_get (buildopts, "rpm_macros", &buildopts_rpm_macros_prop, NULL);
  g_assert_cmpstr (buildopts_rpm_macros_prop,
                   ==,
                   "%demomacro 1\n%demomacro2 %{demomacro}23\n");

  buildopts_rpm_whitelist =
    modulemd_buildopts_get_rpm_whitelist_as_strv (buildopts);
  buildopts_arches = modulemd_buildopts_get_arches_as_strv (buildopts);

  g_assert_true (g_strv_contains (
    (const gchar *const *)buildopts_rpm_whitelist, "fooscl-1-bar"));
  g_assert_true (g_strv_contains (
    (const gchar *const *)buildopts_rpm_whitelist, "fooscl-1-baz"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)buildopts_rpm_whitelist, "xxx"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)buildopts_rpm_whitelist, "xyz"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)buildopts_arches, "i686"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)buildopts_arches, "x86_64"));


  // Load XMD into dictionary
  tmp_variant = modulemd_module_stream_v2_get_xmd (streamV2);
  g_assert_nonnull (tmp_variant);
  xmd_dict = g_variant_dict_new (tmp_variant);

  // Check xmd["some_key"] == "some_data"
  g_assert_true (g_variant_dict_contains (xmd_dict, "some_key"));
  g_assert_true (g_variant_dict_lookup (xmd_dict, "some_key", "&s", &tmp_str));
  g_assert_cmpstr (tmp_str, ==, "some_data");

  // Check xmd["some_list"][0] == "a" and xmd["some_list"][1] == "b"
  g_assert_true (g_variant_dict_contains (xmd_dict, "some_list"));
  g_assert_true (
    g_variant_dict_lookup (xmd_dict, "some_list", "@as", &tmp_variant));

  g_variant_get_child (tmp_variant, 0, "&s", &tmp_str);
  g_assert_cmpstr (tmp_str, ==, "a");

  g_variant_get_child (tmp_variant, 1, "&s", &tmp_str);
  g_assert_cmpstr (tmp_str, ==, "b");

  g_clear_pointer (&tmp_variant, g_variant_unref);

  // Check xmd["some_dict"]["a"] == "alpha"
  g_assert_true (g_variant_dict_contains (xmd_dict, "some_dict"));
  g_assert_true (
    g_variant_dict_lookup (xmd_dict, "some_dict", "@a{sv}", &tmp_variant));
  tmp_dict = g_variant_dict_new (tmp_variant);

  g_assert_true (g_variant_dict_contains (tmp_dict, "a"));
  g_assert_true (g_variant_dict_lookup (tmp_dict, "a", "&s", &tmp_str));
  g_assert_cmpstr (tmp_str, ==, "alpha");

  g_clear_pointer (&tmp_variant, g_variant_unref);

  // Check xmd["some_dict"]["some_other_dict"]["another_key"] == "more_data"
  g_assert_true (g_variant_dict_contains (tmp_dict, "some_other_dict"));
  g_assert_true (g_variant_dict_lookup (
    tmp_dict, "some_other_dict", "@a{sv}", &tmp_variant));
  g_clear_pointer (&tmp_dict, g_variant_dict_unref);
  tmp_dict = g_variant_dict_new (tmp_variant);

  g_assert_true (g_variant_dict_contains (tmp_dict, "another_key"));
  g_assert_true (
    g_variant_dict_lookup (tmp_dict, "another_key", "&s", &tmp_str));
  g_assert_cmpstr (tmp_str, ==, "more_data");

  g_clear_pointer (&tmp_variant, g_variant_unref);

  // Check xmd["some_dict"]["some_other_dict"]["yet_another_key"][3] == "silly"
  g_assert_true (
    g_variant_dict_lookup (tmp_dict, "yet_another_key", "@as", &tmp_variant));

  g_variant_get_child (tmp_variant, 3, "&s", &tmp_str);
  g_assert_cmpstr (tmp_str, ==, "silly");

  g_clear_pointer (&tmp_variant, g_variant_unref);
  g_clear_pointer (&tmp_dict, g_variant_dict_unref);

  // Check xmd["can_bool"] == TRUE
  g_assert_true (g_variant_dict_lookup (xmd_dict, "can_bool", "b", &tmp_bool));
  g_assert_true (tmp_bool);

  g_clear_pointer (&xmd_dict, g_variant_dict_unref);


  // Cleanup
  g_clear_object (&stream);
  g_clear_pointer (&module_name_prop, g_free);
  g_clear_pointer (&stream_name_prop, g_free);
  g_clear_pointer (&context_prop, g_free);
  g_clear_pointer (&arch_prop, g_free);
  g_clear_pointer (&error, g_error_free);

  g_clear_pointer (&rpm_apis, g_strfreev);
  g_clear_pointer (&rpm_filters, g_strfreev);
  g_clear_pointer (&rpm_artifacts, g_strfreev);
  g_clear_pointer (&servicelevel_names, g_strfreev);

  g_clear_pointer (&sl_name_prop, g_free);
  g_clear_pointer (&sl_eol_string, g_free);

  g_clear_pointer (&content_licenses, g_strfreev);
  g_clear_pointer (&module_licenses, g_strfreev);
  g_clear_pointer (&stream_dependencies, g_strfreev);

  g_clear_pointer (&community_prop, g_free);
  g_clear_pointer (&documentation_prop, g_free);
  g_clear_pointer (&tracker_prop, g_free);
  g_clear_pointer (&profile_names, g_strfreev);

  g_clear_pointer (&buildopts_rpm_macros_prop, g_free);
  g_clear_pointer (&buildopts_rpm_whitelist, g_strfreev);
  g_clear_pointer (&buildopts_arches, g_strfreev);


  // Validate a trivial modulemd
  stream = modulemd_module_stream_read_string (
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license:\n"
    "    module: MIT\n"
    "...\n",
    TRUE,
    NULL,
    NULL,
    &error);

  g_assert_no_error (error);
  g_assert_nonnull (stream);

  g_clear_object (&stream);


  // Sanity check spec.v2.yaml
  gchar *specV2Path = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v2.yaml",
                                       g_getenv ("MESON_SOURCE_ROOT"));
  stream =
    modulemd_module_stream_read_file (specV2Path, TRUE, NULL, NULL, &error);

  g_assert_no_error (error);
  g_assert_nonnull (stream);

  g_clear_object (&stream);
  g_clear_pointer (&specV2Path, g_free);
}

static void
module_stream_test_v3_yaml (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  ModulemdModuleStreamV3 *streamV3 = NULL;
  g_autofree gchar *module_name_prop = NULL;
  g_autofree gchar *stream_name_prop = NULL;
  g_autofree gchar *context_prop = NULL;
  g_autofree gchar *arch_prop = NULL;
  guint64 version_prop = 0;
  g_autoptr (GError) error = NULL;

  g_auto (GStrv) rpm_apis = NULL;
  g_auto (GStrv) rpm_filters = NULL;
  g_auto (GStrv) rpm_artifacts = NULL;

  g_auto (GStrv) content_licenses = NULL;
  g_auto (GStrv) module_licenses = NULL;

  g_autofree gchar *community_prop = NULL;
  g_autofree gchar *documentation_prop = NULL;
  g_autofree gchar *tracker_prop = NULL;
  g_auto (GStrv) profile_names = NULL;

  ModulemdBuildopts *buildopts = NULL;
  g_autofree gchar *buildopts_rpm_macros_prop = NULL;
  g_auto (GStrv) buildopts_rpm_whitelist = NULL;
  g_auto (GStrv) buildopts_arches = NULL;

  g_auto (GStrv) build_deps = NULL;
  g_auto (GStrv) run_deps = NULL;
  g_auto (GStrv) streams = NULL;

  GVariant *tmp_variant = NULL;
  GVariantDict *xmd_dict = NULL;
  GVariantDict *tmp_dict = NULL;
  gchar *tmp_str = NULL;
  gboolean *tmp_bool = NULL;

  stream = modulemd_module_stream_read_string (
    "---\n"
    "document: modulemd\n"
    "version: 3\n"
    "data:\n"
    "  name: modulename\n"
    "  stream: streamname\n"
    "  version: 1\n"
    "  context: c0ffe3\n"
    "  arch: x86_64\n"
    "  summary: Module Summary\n"
    "  description: >-\n"
    "    Module Description\n"
    "  api:\n"
    "    rpms:\n"
    "      - rpm_a\n"
    "      - rpm_b\n"
    "  filter:\n"
    "    rpms: rpm_c\n"

    "  artifacts:\n"
    "    rpms:\n"
    "      - bar-0:1.23-1.module_deadbeef.x86_64\n"

    "  license:\n"
    "    content:\n"
    "      - BSD\n"
    "      - GPLv2+\n"
    "    module: MIT\n"

    "  dependencies:\n"
    "    platform: f28\n"
    "    buildrequires:\n"
    "        buildtools: v1\n"
    "        compatible: v3\n"
    "    requires:\n"
    "        compatible: v3\n"
    "        runtime: a\n"
    "        extras: foo\n"
    "  references:\n"
    "        community: http://www.example.com/\n"
    "        documentation: http://www.example.com/\n"
    "        tracker: http://www.example.com/\n"
    "  profiles:\n"
    "        default:\n"
    "            rpms:\n"
    "                - bar\n"
    "                - bar-extras\n"
    "                - baz\n"
    "        container:\n"
    "            rpms:\n"
    "                - bar\n"
    "                - bar-devel\n"
    "        minimal:\n"
    "            description: Minimal profile installing only the bar "
    "package.\n"
    "            rpms:\n"
    "                - bar\n"
    "        buildroot:\n"
    "            rpms:\n"
    "                - bar-devel\n"
    "        srpm-buildroot:\n"
    "            rpms:\n"
    "                - bar-extras\n"
    "  buildopts:\n"
    "        rpms:\n"
    "            macros: |\n"
    "                %demomacro 1\n"
    "                %demomacro2 %{demomacro}23\n"
    "            whitelist:\n"
    "                - fooscl-1-bar\n"
    "                - fooscl-1-baz\n"
    "                - xxx\n"
    "                - xyz\n"
    "        arches: [i686, x86_64]\n"
    "  components:\n"
    "        rpms:\n"
    "            bar:\n"
    "                rationale: We need this to demonstrate stuff.\n"
    "                repository: https://pagure.io/bar.git\n"
    "                cache: https://example.com/cache\n"
    "                ref: 26ca0c0\n"
    "            baz:\n"
    "                rationale: This one is here to demonstrate other stuff.\n"
    "            xxx:\n"
    "                rationale: xxx demonstrates arches and multilib.\n"
    "                arches: [i686, x86_64]\n"
    "                multilib: [x86_64]\n"
    "            xyz:\n"
    "                rationale: xyz is a bundled dependency of xxx.\n"
    "                buildorder: 10\n"
    "        modules:\n"
    "            includedmodule:\n"
    "                rationale: Included in the stack, just because.\n"
    "                repository: https://pagure.io/includedmodule.git\n"
    "                ref: somecoolbranchname\n"
    "                buildorder: 100\n"
    "  xmd:\n"
    "        some_key: some_data\n"
    "        some_list:\n"
    "            - a\n"
    "            - b\n"
    "        some_dict:\n"
    "            a: alpha\n"
    "            b: beta\n"
    "            some_other_list:\n"
    "                - c\n"
    "                - d\n"
    "            some_other_dict:\n"
    "                another_key: more_data\n"
    "                yet_another_key:\n"
    "                    - this\n"
    "                    - is\n"
    "                    - getting\n"
    "                    - silly\n"
    "        can_bool: TRUE\n"
    "...\n",
    TRUE,
    NULL,
    NULL,
    &error);

  g_assert_no_error (error);

  g_assert_nonnull (stream);
  streamV3 = MODULEMD_MODULE_STREAM_V3 (stream);

  g_object_get (streamV3, "module-name", &module_name_prop, NULL);
  g_object_get (streamV3, "stream-name", &stream_name_prop, NULL);
  g_object_get (streamV3, "version", &version_prop, NULL);
  g_object_get (streamV3, "context", &context_prop, NULL);
  g_object_get (streamV3, "arch", &arch_prop, NULL);

  g_assert_cmpstr (module_name_prop, ==, "modulename");
  g_assert_cmpstr (stream_name_prop, ==, "streamname");
  g_assert_cmpuint (version_prop, ==, 1);
  g_assert_cmpstr (context_prop, ==, "c0ffe3");
  g_assert_cmpstr (arch_prop, ==, "x86_64");
  g_assert_cmpstr (modulemd_module_stream_v3_get_summary (streamV3, "C"),
                   ==,
                   "Module Summary");
  g_assert_cmpstr (modulemd_module_stream_v3_get_description (streamV3, "C"),
                   ==,
                   "Module Description");

  rpm_apis = modulemd_module_stream_v3_get_rpm_api_as_strv (streamV3);
  rpm_filters = modulemd_module_stream_v3_get_rpm_filters_as_strv (streamV3);
  rpm_artifacts =
    modulemd_module_stream_v3_get_rpm_artifacts_as_strv (streamV3);

  g_assert_true (g_strv_contains ((const gchar *const *)rpm_apis, "rpm_a"));
  g_assert_true (g_strv_contains ((const gchar *const *)rpm_apis, "rpm_b"));

  g_assert_true (g_strv_contains ((const gchar *const *)rpm_filters, "rpm_c"));

  g_assert_true (g_strv_contains ((const gchar *const *)rpm_artifacts,
                                  "bar-0:1.23-1.module_deadbeef.x86_64"));

  content_licenses =
    modulemd_module_stream_v3_get_content_licenses_as_strv (streamV3);
  module_licenses =
    modulemd_module_stream_v3_get_module_licenses_as_strv (streamV3);

  g_assert_true (
    g_strv_contains ((const gchar *const *)content_licenses, "BSD"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)content_licenses, "GPLv2+"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)module_licenses, "MIT"));

  g_assert_cmpstr (
    modulemd_module_stream_v3_get_platform (streamV3), ==, "f28");

  build_deps =
    modulemd_module_stream_v3_get_buildtime_modules_as_strv (streamV3);
  run_deps = modulemd_module_stream_v3_get_runtime_modules_as_strv (streamV3);

  g_assert_cmpint (g_strv_length (build_deps), ==, 2);
  g_assert_true (
    g_strv_contains ((const gchar *const *)build_deps, "buildtools"));
  g_assert_cmpstr (modulemd_module_stream_v3_get_buildtime_requirement_stream (
                     streamV3, "buildtools"),
                   ==,
                   "v1");
  g_assert_true (
    g_strv_contains ((const gchar *const *)build_deps, "compatible"));
  g_assert_cmpstr (modulemd_module_stream_v3_get_buildtime_requirement_stream (
                     streamV3, "compatible"),
                   ==,
                   "v3");
  g_assert_cmpint (g_strv_length (run_deps), ==, 3);
  g_assert_true (
    g_strv_contains ((const gchar *const *)run_deps, "compatible"));
  g_assert_cmpstr (modulemd_module_stream_v3_get_runtime_requirement_stream (
                     streamV3, "compatible"),
                   ==,
                   "v3");
  g_assert_true (g_strv_contains ((const gchar *const *)run_deps, "runtime"));
  g_assert_cmpstr (modulemd_module_stream_v3_get_runtime_requirement_stream (
                     streamV3, "runtime"),
                   ==,
                   "a");
  g_assert_true (g_strv_contains ((const gchar *const *)run_deps, "extras"));
  g_assert_cmpstr (modulemd_module_stream_v3_get_runtime_requirement_stream (
                     streamV3, "extras"),
                   ==,
                   "foo");

  /* spot check alternate interfaces that return stream wrapped in a list */
  streams =
    modulemd_module_stream_v3_get_buildtime_requirement_streams_as_strv (
      streamV3, "buildtools");
  g_assert_nonnull (streams);
  g_assert_cmpstr (streams[0], ==, "v1");
  g_assert_null (streams[1]);
  g_clear_pointer (&streams, g_strfreev);

  streams = modulemd_module_stream_v3_get_runtime_requirement_streams_as_strv (
    streamV3, "runtime");
  g_assert_nonnull (streams);
  g_assert_cmpstr (streams[0], ==, "a");
  g_assert_null (streams[1]);
  g_clear_pointer (&streams, g_strfreev);


  g_object_get (streamV3, "community", &community_prop, NULL);
  g_object_get (streamV3, "documentation", &documentation_prop, NULL);
  g_object_get (streamV3, "tracker", &tracker_prop, NULL);

  g_assert_cmpstr (community_prop, ==, "http://www.example.com/");
  g_assert_cmpstr (documentation_prop, ==, "http://www.example.com/");
  g_assert_cmpstr (tracker_prop, ==, "http://www.example.com/");

  profile_names =
    modulemd_module_stream_v3_get_profile_names_as_strv (streamV3);
  g_assert_cmpint (g_strv_length (profile_names), ==, 5);

  buildopts = modulemd_module_stream_v3_get_buildopts (streamV3);
  g_assert_nonnull (buildopts);

  g_object_get (buildopts, "rpm_macros", &buildopts_rpm_macros_prop, NULL);
  g_assert_cmpstr (buildopts_rpm_macros_prop,
                   ==,
                   "%demomacro 1\n%demomacro2 %{demomacro}23\n");

  buildopts_rpm_whitelist =
    modulemd_buildopts_get_rpm_whitelist_as_strv (buildopts);
  buildopts_arches = modulemd_buildopts_get_arches_as_strv (buildopts);

  g_assert_true (g_strv_contains (
    (const gchar *const *)buildopts_rpm_whitelist, "fooscl-1-bar"));
  g_assert_true (g_strv_contains (
    (const gchar *const *)buildopts_rpm_whitelist, "fooscl-1-baz"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)buildopts_rpm_whitelist, "xxx"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)buildopts_rpm_whitelist, "xyz"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)buildopts_arches, "i686"));
  g_assert_true (
    g_strv_contains ((const gchar *const *)buildopts_arches, "x86_64"));


  // Load XMD into dictionary
  tmp_variant = modulemd_module_stream_v3_get_xmd (streamV3);
  g_assert_nonnull (tmp_variant);
  xmd_dict = g_variant_dict_new (tmp_variant);

  // Check xmd["some_key"] == "some_data"
  g_assert_true (g_variant_dict_contains (xmd_dict, "some_key"));
  g_assert_true (g_variant_dict_lookup (xmd_dict, "some_key", "&s", &tmp_str));
  g_assert_cmpstr (tmp_str, ==, "some_data");

  // Check xmd["some_list"][0] == "a" and xmd["some_list"][1] == "b"
  g_assert_true (g_variant_dict_contains (xmd_dict, "some_list"));
  g_assert_true (
    g_variant_dict_lookup (xmd_dict, "some_list", "@as", &tmp_variant));

  g_variant_get_child (tmp_variant, 0, "&s", &tmp_str);
  g_assert_cmpstr (tmp_str, ==, "a");

  g_variant_get_child (tmp_variant, 1, "&s", &tmp_str);
  g_assert_cmpstr (tmp_str, ==, "b");

  g_clear_pointer (&tmp_variant, g_variant_unref);

  // Check xmd["some_dict"]["a"] == "alpha"
  g_assert_true (g_variant_dict_contains (xmd_dict, "some_dict"));
  g_assert_true (
    g_variant_dict_lookup (xmd_dict, "some_dict", "@a{sv}", &tmp_variant));
  tmp_dict = g_variant_dict_new (tmp_variant);

  g_assert_true (g_variant_dict_contains (tmp_dict, "a"));
  g_assert_true (g_variant_dict_lookup (tmp_dict, "a", "&s", &tmp_str));
  g_assert_cmpstr (tmp_str, ==, "alpha");

  g_clear_pointer (&tmp_variant, g_variant_unref);

  // Check xmd["some_dict"]["some_other_dict"]["another_key"] == "more_data"
  g_assert_true (g_variant_dict_contains (tmp_dict, "some_other_dict"));
  g_assert_true (g_variant_dict_lookup (
    tmp_dict, "some_other_dict", "@a{sv}", &tmp_variant));
  g_clear_pointer (&tmp_dict, g_variant_dict_unref);
  tmp_dict = g_variant_dict_new (tmp_variant);

  g_assert_true (g_variant_dict_contains (tmp_dict, "another_key"));
  g_assert_true (
    g_variant_dict_lookup (tmp_dict, "another_key", "&s", &tmp_str));
  g_assert_cmpstr (tmp_str, ==, "more_data");

  g_clear_pointer (&tmp_variant, g_variant_unref);

  // Check xmd["some_dict"]["some_other_dict"]["yet_another_key"][3] == "silly"
  g_assert_true (
    g_variant_dict_lookup (tmp_dict, "yet_another_key", "@as", &tmp_variant));

  g_variant_get_child (tmp_variant, 3, "&s", &tmp_str);
  g_assert_cmpstr (tmp_str, ==, "silly");

  g_clear_pointer (&tmp_variant, g_variant_unref);
  g_clear_pointer (&tmp_dict, g_variant_dict_unref);

  // Check xmd["can_bool"] == TRUE
  g_assert_true (g_variant_dict_lookup (xmd_dict, "can_bool", "b", &tmp_bool));
  g_assert_true (tmp_bool);

  g_clear_pointer (&xmd_dict, g_variant_dict_unref);


  // Cleanup
  g_clear_object (&stream);
  g_clear_pointer (&module_name_prop, g_free);
  g_clear_pointer (&stream_name_prop, g_free);
  g_clear_pointer (&context_prop, g_free);
  g_clear_pointer (&arch_prop, g_free);
  g_clear_pointer (&error, g_error_free);

  g_clear_pointer (&rpm_apis, g_strfreev);
  g_clear_pointer (&rpm_filters, g_strfreev);
  g_clear_pointer (&rpm_artifacts, g_strfreev);

  g_clear_pointer (&content_licenses, g_strfreev);
  g_clear_pointer (&module_licenses, g_strfreev);

  g_clear_pointer (&community_prop, g_free);
  g_clear_pointer (&documentation_prop, g_free);
  g_clear_pointer (&tracker_prop, g_free);
  g_clear_pointer (&profile_names, g_strfreev);

  g_clear_pointer (&buildopts_rpm_macros_prop, g_free);
  g_clear_pointer (&buildopts_rpm_whitelist, g_strfreev);
  g_clear_pointer (&buildopts_arches, g_strfreev);

  g_clear_pointer (&build_deps, g_strfreev);
  g_clear_pointer (&run_deps, g_strfreev);

  // Validate a trivial modulemd
  stream = modulemd_module_stream_read_string (
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  summary: Trivial Summary\n"
    "  description: >-\n"
    "    Trivial Description\n"
    "  license:\n"
    "    module: MIT\n"
    "...\n",
    TRUE,
    NULL,
    NULL,
    &error);

  g_assert_no_error (error);
  g_assert_nonnull (stream);

  g_clear_object (&stream);


  // Sanity check spec.v3.yaml
  gchar *specV3Path = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v3.yaml",
                                       g_getenv ("MESON_SOURCE_ROOT"));
  stream =
    modulemd_module_stream_read_file (specV3Path, TRUE, NULL, NULL, &error);

  g_assert_no_error (error);
  g_assert_nonnull (stream);

  g_clear_object (&stream);
  g_clear_pointer (&specV3Path, g_free);
}


static void
module_packager_v2_sanity (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (GError) error = NULL;

  g_autofree gchar *specV2Path = g_strdup_printf (
    "%s/yaml_specs/modulemd_packager_v2.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  stream =
    modulemd_module_stream_read_file (specV2Path, TRUE, NULL, NULL, &error);
  g_assert_no_error (error);
  g_assert_nonnull (stream);

  g_clear_object (&stream);
  g_clear_pointer (&specV2Path, g_free);
}


static void
module_packager_v3_sanity (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (GError) error = NULL;

  g_autofree gchar *packagerV3Path = g_strdup_printf (
    "%s/yaml_specs/modulemd_packager_v3.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  stream = modulemd_module_stream_read_file (
    packagerV3Path, TRUE, NULL, NULL, &error);
  g_assert_no_error (error);
  g_assert_nonnull (stream);

  /* confirm packager v3 document was returned as stream v2 */
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V2 (stream));

  g_clear_object (&stream);
  g_clear_pointer (&packagerV3Path, g_free);
}


static void
module_stream_v1_test_rpm_artifacts (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_auto (GStrv) artifacts = NULL;

  stream = modulemd_module_stream_v1_new (NULL, NULL);

  modulemd_module_stream_v1_add_rpm_artifact (
    stream, "bar-0:1.23-1.module_deadbeef.x86_64");
  artifacts = modulemd_module_stream_v1_get_rpm_artifacts_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)artifacts,
                                  "bar-0:1.23-1.module_deadbeef.x86_64"));
  g_assert_cmpint (g_strv_length (artifacts), ==, 1);

  g_clear_pointer (&artifacts, g_strfreev);

  modulemd_module_stream_v1_remove_rpm_artifact (
    stream, "bar-0:1.23-1.module_deadbeef.x86_64");
  artifacts = modulemd_module_stream_v1_get_rpm_artifacts_as_strv (stream);
  g_assert_cmpint (g_strv_length (artifacts), ==, 0);

  g_clear_pointer (&artifacts, g_strfreev);
  g_clear_object (&stream);
}

static void
module_stream_v2_test_rpm_artifacts (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_auto (GStrv) artifacts = NULL;

  stream = modulemd_module_stream_v2_new (NULL, NULL);

  modulemd_module_stream_v2_add_rpm_artifact (
    stream, "bar-0:1.23-1.module_deadbeef.x86_64");
  artifacts = modulemd_module_stream_v2_get_rpm_artifacts_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)artifacts,
                                  "bar-0:1.23-1.module_deadbeef.x86_64"));
  g_assert_cmpint (g_strv_length (artifacts), ==, 1);

  g_clear_pointer (&artifacts, g_strfreev);

  modulemd_module_stream_v2_remove_rpm_artifact (
    stream, "bar-0:1.23-1.module_deadbeef.x86_64");
  artifacts = modulemd_module_stream_v2_get_rpm_artifacts_as_strv (stream);
  g_assert_cmpint (g_strv_length (artifacts), ==, 0);

  g_clear_pointer (&artifacts, g_strfreev);
  g_clear_object (&stream);
}

static void
module_stream_v3_test_rpm_artifacts (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_auto (GStrv) artifacts = NULL;

  stream = modulemd_module_stream_v3_new (NULL, NULL);

  modulemd_module_stream_v3_add_rpm_artifact (
    stream, "bar-0:1.23-1.module_deadbeef.x86_64");
  artifacts = modulemd_module_stream_v3_get_rpm_artifacts_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)artifacts,
                                  "bar-0:1.23-1.module_deadbeef.x86_64"));
  g_assert_cmpint (g_strv_length (artifacts), ==, 1);

  g_clear_pointer (&artifacts, g_strfreev);

  modulemd_module_stream_v3_remove_rpm_artifact (
    stream, "bar-0:1.23-1.module_deadbeef.x86_64");
  artifacts = modulemd_module_stream_v3_get_rpm_artifacts_as_strv (stream);
  g_assert_cmpint (g_strv_length (artifacts), ==, 0);

  g_clear_pointer (&artifacts, g_strfreev);
  g_clear_object (&stream);
}


static void
module_stream_v1_test_servicelevels (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_auto (GStrv) servicelevel_names = NULL;
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  ModulemdServiceLevel *sl_retrieved = NULL;
  g_autofree gchar *name_prop = NULL;
  g_autofree gchar *eol_str = NULL;

  stream = modulemd_module_stream_v1_new (NULL, NULL);
  sl = modulemd_service_level_new ("rawhide");
  modulemd_service_level_set_eol_ymd (sl, 1980, 3, 2);

  modulemd_module_stream_v1_add_servicelevel (stream, sl);

  servicelevel_names =
    modulemd_module_stream_v1_get_servicelevel_names_as_strv (stream);
  g_assert_true (
    g_strv_contains ((const gchar *const *)servicelevel_names, "rawhide"));
  g_assert_cmpint (g_strv_length (servicelevel_names), ==, 1);

  sl_retrieved =
    modulemd_module_stream_v1_get_servicelevel (stream, "rawhide");
  g_object_get (sl_retrieved, "name", &name_prop, NULL);
  eol_str = modulemd_service_level_get_eol_as_string (sl_retrieved);

  g_assert_cmpstr (name_prop, ==, "rawhide");
  g_assert_cmpstr (eol_str, ==, "1980-03-02");

  g_clear_object (&sl);
  g_clear_object (&stream);
  g_clear_pointer (&name_prop, g_free);
  g_clear_pointer (&eol_str, g_free);
  g_clear_pointer (&servicelevel_names, g_strfreev);
}


static void
module_stream_v2_test_servicelevels (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_auto (GStrv) servicelevel_names = NULL;
  g_autoptr (ModulemdServiceLevel) sl = NULL;
  ModulemdServiceLevel *sl_retrieved = NULL;
  g_autofree gchar *name_prop = NULL;
  g_autofree gchar *eol_str = NULL;

  stream = modulemd_module_stream_v2_new (NULL, NULL);
  sl = modulemd_service_level_new ("rawhide");
  modulemd_service_level_set_eol_ymd (sl, 1980, 3, 2);

  modulemd_module_stream_v2_add_servicelevel (stream, sl);

  servicelevel_names =
    modulemd_module_stream_v2_get_servicelevel_names_as_strv (stream);
  g_assert_true (
    g_strv_contains ((const gchar *const *)servicelevel_names, "rawhide"));
  g_assert_cmpint (g_strv_length (servicelevel_names), ==, 1);

  sl_retrieved =
    modulemd_module_stream_v2_get_servicelevel (stream, "rawhide");
  g_object_get (sl_retrieved, "name", &name_prop, NULL);
  eol_str = modulemd_service_level_get_eol_as_string (sl_retrieved);

  g_assert_cmpstr (name_prop, ==, "rawhide");
  g_assert_cmpstr (eol_str, ==, "1980-03-02");

  g_clear_object (&sl);
  g_clear_object (&stream);
  g_clear_pointer (&name_prop, g_free);
  g_clear_pointer (&eol_str, g_free);
  g_clear_pointer (&servicelevel_names, g_strfreev);
}


static void
module_stream_v1_test_documentation (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  const gchar *documentation = NULL;
  g_autofree gchar *documentation_prop = NULL;

  stream = modulemd_module_stream_v1_new (NULL, NULL);

  // Check the defaults
  documentation = modulemd_module_stream_v1_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_null (documentation);
  g_assert_null (documentation_prop);

  g_clear_pointer (&documentation_prop, g_free);

  // Test property setting
  g_object_set (stream, MMD_TEST_DOC_PROP, MMD_TEST_DOC_TEXT, NULL);

  documentation = modulemd_module_stream_v1_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_cmpstr (documentation_prop, ==, MMD_TEST_DOC_TEXT);
  g_assert_cmpstr (documentation, ==, MMD_TEST_DOC_TEXT);

  g_clear_pointer (&documentation_prop, g_free);

  // Test set_documentation()
  modulemd_module_stream_v1_set_documentation (stream, MMD_TEST_DOC_TEXT2);

  documentation = modulemd_module_stream_v1_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_cmpstr (documentation_prop, ==, MMD_TEST_DOC_TEXT2);
  g_assert_cmpstr (documentation, ==, MMD_TEST_DOC_TEXT2);

  g_clear_pointer (&documentation_prop, g_free);

  // Test setting to NULL
  g_object_set (stream, MMD_TEST_DOC_PROP, NULL, NULL);

  documentation = modulemd_module_stream_v1_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_null (documentation);
  g_assert_null (documentation_prop);

  g_clear_pointer (&documentation_prop, g_free);

  // Test unicode characters
  modulemd_module_stream_v1_set_documentation (stream,
                                               MMD_TEST_DOC_UNICODE_TEXT);

  documentation = modulemd_module_stream_v1_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_cmpstr (documentation_prop, ==, MMD_TEST_DOC_UNICODE_TEXT);
  g_assert_cmpstr (documentation, ==, MMD_TEST_DOC_UNICODE_TEXT);

  g_clear_pointer (&documentation_prop, g_free);

  g_clear_object (&stream);
}

static void
module_stream_v2_test_documentation (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  const gchar *documentation = NULL;
  g_autofree gchar *documentation_prop = NULL;

  stream = modulemd_module_stream_v2_new (NULL, NULL);

  // Check the defaults
  documentation = modulemd_module_stream_v2_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_null (documentation);
  g_assert_null (documentation_prop);

  g_clear_pointer (&documentation_prop, g_free);

  // Test property setting
  g_object_set (stream, MMD_TEST_DOC_PROP, MMD_TEST_DOC_TEXT, NULL);

  documentation = modulemd_module_stream_v2_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_cmpstr (documentation_prop, ==, MMD_TEST_DOC_TEXT);
  g_assert_cmpstr (documentation, ==, MMD_TEST_DOC_TEXT);

  g_clear_pointer (&documentation_prop, g_free);

  // Test set_documentation()
  modulemd_module_stream_v2_set_documentation (stream, MMD_TEST_DOC_TEXT2);

  documentation = modulemd_module_stream_v2_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_cmpstr (documentation_prop, ==, MMD_TEST_DOC_TEXT2);
  g_assert_cmpstr (documentation, ==, MMD_TEST_DOC_TEXT2);

  g_clear_pointer (&documentation_prop, g_free);

  // Test setting to NULL
  g_object_set (stream, MMD_TEST_DOC_PROP, NULL, NULL);

  documentation = modulemd_module_stream_v2_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_null (documentation);
  g_assert_null (documentation_prop);

  g_clear_pointer (&documentation_prop, g_free);

  // Test unicode characters
  modulemd_module_stream_v2_set_documentation (stream,
                                               MMD_TEST_DOC_UNICODE_TEXT);

  documentation = modulemd_module_stream_v2_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_cmpstr (documentation_prop, ==, MMD_TEST_DOC_UNICODE_TEXT);
  g_assert_cmpstr (documentation, ==, MMD_TEST_DOC_UNICODE_TEXT);

  g_clear_pointer (&documentation_prop, g_free);

  g_clear_object (&stream);
}

static void
module_stream_v3_test_documentation (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  const gchar *documentation = NULL;
  g_autofree gchar *documentation_prop = NULL;

  stream = modulemd_module_stream_v3_new (NULL, NULL);

  // Check the defaults
  documentation = modulemd_module_stream_v3_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_null (documentation);
  g_assert_null (documentation_prop);

  g_clear_pointer (&documentation_prop, g_free);

  // Test property setting
  g_object_set (stream, MMD_TEST_DOC_PROP, MMD_TEST_DOC_TEXT, NULL);

  documentation = modulemd_module_stream_v3_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_cmpstr (documentation_prop, ==, MMD_TEST_DOC_TEXT);
  g_assert_cmpstr (documentation, ==, MMD_TEST_DOC_TEXT);

  g_clear_pointer (&documentation_prop, g_free);

  // Test set_documentation()
  modulemd_module_stream_v3_set_documentation (stream, MMD_TEST_DOC_TEXT2);

  documentation = modulemd_module_stream_v3_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_cmpstr (documentation_prop, ==, MMD_TEST_DOC_TEXT2);
  g_assert_cmpstr (documentation, ==, MMD_TEST_DOC_TEXT2);

  g_clear_pointer (&documentation_prop, g_free);

  // Test setting to NULL
  g_object_set (stream, MMD_TEST_DOC_PROP, NULL, NULL);

  documentation = modulemd_module_stream_v3_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_null (documentation);
  g_assert_null (documentation_prop);

  g_clear_pointer (&documentation_prop, g_free);

  // Test unicode characters
  modulemd_module_stream_v3_set_documentation (stream,
                                               MMD_TEST_DOC_UNICODE_TEXT);

  documentation = modulemd_module_stream_v3_get_documentation (stream);
  g_object_get (stream, MMD_TEST_DOC_PROP, &documentation_prop, NULL);
  g_assert_cmpstr (documentation_prop, ==, MMD_TEST_DOC_UNICODE_TEXT);
  g_assert_cmpstr (documentation, ==, MMD_TEST_DOC_UNICODE_TEXT);

  g_clear_pointer (&documentation_prop, g_free);

  g_clear_object (&stream);
}


static void
module_stream_v1_test_tracker (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_autofree gchar *tracker_prop = NULL;
  const gchar *tracker = NULL;

  stream = modulemd_module_stream_v1_new (NULL, NULL);

  // Check the defaults
  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v1_get_tracker (stream);

  g_assert_null (tracker);
  g_assert_null (tracker_prop);

  g_clear_pointer (&tracker_prop, g_free);

  // Test property setting
  g_object_set (stream, MMD_TEST_TRACKER_PROP, MMD_TEST_DOC_TEXT, NULL);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v1_get_tracker (stream);

  g_assert_cmpstr (tracker, ==, MMD_TEST_DOC_TEXT);
  g_assert_cmpstr (tracker_prop, ==, MMD_TEST_DOC_TEXT);

  g_clear_pointer (&tracker_prop, g_free);

  // Test set_tracker
  modulemd_module_stream_v1_set_tracker (stream, MMD_TEST_DOC_TEXT2);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v1_get_tracker (stream);

  g_assert_cmpstr (tracker, ==, MMD_TEST_DOC_TEXT2);
  g_assert_cmpstr (tracker_prop, ==, MMD_TEST_DOC_TEXT2);

  g_clear_pointer (&tracker_prop, g_free);

  // Test setting it to NULL
  g_object_set (stream, MMD_TEST_TRACKER_PROP, NULL, NULL);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v1_get_tracker (stream);

  g_assert_null (tracker);
  g_assert_null (tracker_prop);

  g_clear_pointer (&tracker_prop, g_free);

  // Test Unicode values
  modulemd_module_stream_v1_set_tracker (stream, MMD_TEST_DOC_UNICODE_TEXT);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v1_get_tracker (stream);

  g_assert_cmpstr (tracker, ==, MMD_TEST_DOC_UNICODE_TEXT);
  g_assert_cmpstr (tracker_prop, ==, MMD_TEST_DOC_UNICODE_TEXT);

  g_clear_pointer (&tracker_prop, g_free);
  g_clear_object (&stream);
}

static void
module_stream_v2_test_tracker (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autofree gchar *tracker_prop = NULL;
  const gchar *tracker = NULL;

  stream = modulemd_module_stream_v2_new (NULL, NULL);

  // Check the defaults
  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v2_get_tracker (stream);

  g_assert_null (tracker);
  g_assert_null (tracker_prop);

  g_clear_pointer (&tracker_prop, g_free);

  // Test property setting
  g_object_set (stream, MMD_TEST_TRACKER_PROP, MMD_TEST_DOC_TEXT, NULL);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v2_get_tracker (stream);

  g_assert_cmpstr (tracker, ==, MMD_TEST_DOC_TEXT);
  g_assert_cmpstr (tracker_prop, ==, MMD_TEST_DOC_TEXT);

  g_clear_pointer (&tracker_prop, g_free);

  // Test set_tracker
  modulemd_module_stream_v2_set_tracker (stream, MMD_TEST_DOC_TEXT2);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v2_get_tracker (stream);

  g_assert_cmpstr (tracker, ==, MMD_TEST_DOC_TEXT2);
  g_assert_cmpstr (tracker_prop, ==, MMD_TEST_DOC_TEXT2);

  g_clear_pointer (&tracker_prop, g_free);

  // Test setting it to NULL
  g_object_set (stream, MMD_TEST_TRACKER_PROP, NULL, NULL);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v2_get_tracker (stream);

  g_assert_null (tracker);
  g_assert_null (tracker_prop);

  g_clear_pointer (&tracker_prop, g_free);

  // Test Unicode values
  modulemd_module_stream_v2_set_tracker (stream, MMD_TEST_DOC_UNICODE_TEXT);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v2_get_tracker (stream);

  g_assert_cmpstr (tracker, ==, MMD_TEST_DOC_UNICODE_TEXT);
  g_assert_cmpstr (tracker_prop, ==, MMD_TEST_DOC_UNICODE_TEXT);

  g_clear_pointer (&tracker_prop, g_free);
  g_clear_object (&stream);
}

static void
module_stream_v3_test_tracker (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_autofree gchar *tracker_prop = NULL;
  const gchar *tracker = NULL;

  stream = modulemd_module_stream_v3_new (NULL, NULL);

  // Check the defaults
  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v3_get_tracker (stream);

  g_assert_null (tracker);
  g_assert_null (tracker_prop);

  g_clear_pointer (&tracker_prop, g_free);

  // Test property setting
  g_object_set (stream, MMD_TEST_TRACKER_PROP, MMD_TEST_DOC_TEXT, NULL);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v3_get_tracker (stream);

  g_assert_cmpstr (tracker, ==, MMD_TEST_DOC_TEXT);
  g_assert_cmpstr (tracker_prop, ==, MMD_TEST_DOC_TEXT);

  g_clear_pointer (&tracker_prop, g_free);

  // Test set_tracker
  modulemd_module_stream_v3_set_tracker (stream, MMD_TEST_DOC_TEXT2);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v3_get_tracker (stream);

  g_assert_cmpstr (tracker, ==, MMD_TEST_DOC_TEXT2);
  g_assert_cmpstr (tracker_prop, ==, MMD_TEST_DOC_TEXT2);

  g_clear_pointer (&tracker_prop, g_free);

  // Test setting it to NULL
  g_object_set (stream, MMD_TEST_TRACKER_PROP, NULL, NULL);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v3_get_tracker (stream);

  g_assert_null (tracker);
  g_assert_null (tracker_prop);

  g_clear_pointer (&tracker_prop, g_free);

  // Test Unicode values
  modulemd_module_stream_v3_set_tracker (stream, MMD_TEST_DOC_UNICODE_TEXT);

  g_object_get (stream, MMD_TEST_TRACKER_PROP, &tracker_prop, NULL);
  tracker = modulemd_module_stream_v3_get_tracker (stream);

  g_assert_cmpstr (tracker, ==, MMD_TEST_DOC_UNICODE_TEXT);
  g_assert_cmpstr (tracker_prop, ==, MMD_TEST_DOC_UNICODE_TEXT);

  g_clear_pointer (&tracker_prop, g_free);
  g_clear_object (&stream);
}


static void
module_stream_v1_test_components (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_autoptr (ModulemdComponentRpm) rpm_component = NULL;
  g_autoptr (ModulemdComponentModule) module_component = NULL;
  ModulemdComponent *retrieved_component = NULL;
  g_auto (GStrv) component_names = NULL;

  stream = modulemd_module_stream_v1_new (NULL, NULL);

  // Add a RPM component to a stream
  rpm_component = modulemd_component_rpm_new ("rpmcomponent");
  modulemd_module_stream_v1_add_component (stream,
                                           (ModulemdComponent *)rpm_component);
  component_names =
    modulemd_module_stream_v1_get_rpm_component_names_as_strv (stream);
  g_assert_true (
    g_strv_contains ((const gchar *const *)component_names, "rpmcomponent"));
  g_assert_cmpint (g_strv_length (component_names), ==, 1);

  retrieved_component =
    (ModulemdComponent *)modulemd_module_stream_v1_get_rpm_component (
      stream, "rpmcomponent");
  g_assert_nonnull (retrieved_component);
  g_assert_true (modulemd_component_equals (
    retrieved_component, (ModulemdComponent *)rpm_component));

  g_clear_pointer (&component_names, g_strfreev);

  // Add a Module component to a stream
  module_component = modulemd_component_module_new ("modulecomponent");
  modulemd_module_stream_v1_add_component (
    stream, (ModulemdComponent *)module_component);
  component_names =
    modulemd_module_stream_v1_get_module_component_names_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)component_names,
                                  "modulecomponent"));
  g_assert_cmpint (g_strv_length (component_names), ==, 1);

  retrieved_component =
    (ModulemdComponent *)modulemd_module_stream_v1_get_module_component (
      stream, "modulecomponent");
  g_assert_nonnull (retrieved_component);
  g_assert_true (modulemd_component_equals (
    retrieved_component, (ModulemdComponent *)module_component));

  g_clear_pointer (&component_names, g_strfreev);

  // Remove an RPM component from a stream
  modulemd_module_stream_v1_remove_rpm_component (stream, "rpmcomponent");
  component_names =
    modulemd_module_stream_v1_get_rpm_component_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (component_names), ==, 0);

  g_clear_pointer (&component_names, g_strfreev);

  // Remove a Module component from a stream
  modulemd_module_stream_v1_remove_module_component (stream,
                                                     "modulecomponent");
  component_names =
    modulemd_module_stream_v1_get_module_component_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (component_names), ==, 0);

  g_clear_pointer (&component_names, g_strfreev);

  g_clear_object (&module_component);
  g_clear_object (&rpm_component);
  g_clear_object (&stream);
}

static void
module_stream_v2_test_components (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdComponentRpm) rpm_component = NULL;
  g_autoptr (ModulemdComponentModule) module_component = NULL;
  ModulemdComponent *retrieved_component = NULL;
  g_auto (GStrv) component_names = NULL;

  stream = modulemd_module_stream_v2_new (NULL, NULL);

  // Add a RPM component to a stream
  rpm_component = modulemd_component_rpm_new ("rpmcomponent");
  modulemd_module_stream_v2_add_component (stream,
                                           (ModulemdComponent *)rpm_component);
  component_names =
    modulemd_module_stream_v2_get_rpm_component_names_as_strv (stream);
  g_assert_true (
    g_strv_contains ((const gchar *const *)component_names, "rpmcomponent"));
  g_assert_cmpint (g_strv_length (component_names), ==, 1);

  retrieved_component =
    (ModulemdComponent *)modulemd_module_stream_v2_get_rpm_component (
      stream, "rpmcomponent");
  g_assert_nonnull (retrieved_component);
  g_assert_true (modulemd_component_equals (
    retrieved_component, (ModulemdComponent *)rpm_component));

  g_clear_pointer (&component_names, g_strfreev);

  // Add a Module component to a stream
  module_component = modulemd_component_module_new ("modulecomponent");
  modulemd_module_stream_v2_add_component (
    stream, (ModulemdComponent *)module_component);
  component_names =
    modulemd_module_stream_v2_get_module_component_names_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)component_names,
                                  "modulecomponent"));
  g_assert_cmpint (g_strv_length (component_names), ==, 1);

  retrieved_component =
    (ModulemdComponent *)modulemd_module_stream_v2_get_module_component (
      stream, "modulecomponent");
  g_assert_nonnull (retrieved_component);
  g_assert_true (modulemd_component_equals (
    retrieved_component, (ModulemdComponent *)module_component));

  g_clear_pointer (&component_names, g_strfreev);

  // Remove an RPM component from a stream
  modulemd_module_stream_v2_remove_rpm_component (stream, "rpmcomponent");
  component_names =
    modulemd_module_stream_v2_get_rpm_component_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (component_names), ==, 0);

  g_clear_pointer (&component_names, g_strfreev);

  // Remove a Module component from a stream
  modulemd_module_stream_v2_remove_module_component (stream,
                                                     "modulecomponent");
  component_names =
    modulemd_module_stream_v2_get_module_component_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (component_names), ==, 0);

  g_clear_pointer (&component_names, g_strfreev);

  g_clear_object (&module_component);
  g_clear_object (&rpm_component);
  g_clear_object (&stream);
}

static void
module_stream_v3_test_components (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_autoptr (ModulemdComponentRpm) rpm_component = NULL;
  g_autoptr (ModulemdComponentModule) module_component = NULL;
  ModulemdComponent *retrieved_component = NULL;
  g_auto (GStrv) component_names = NULL;

  stream = modulemd_module_stream_v3_new (NULL, NULL);

  // Add a RPM component to a stream
  rpm_component = modulemd_component_rpm_new ("rpmcomponent");
  modulemd_module_stream_v3_add_component (stream,
                                           (ModulemdComponent *)rpm_component);
  component_names =
    modulemd_module_stream_v3_get_rpm_component_names_as_strv (stream);
  g_assert_true (
    g_strv_contains ((const gchar *const *)component_names, "rpmcomponent"));
  g_assert_cmpint (g_strv_length (component_names), ==, 1);

  retrieved_component =
    (ModulemdComponent *)modulemd_module_stream_v3_get_rpm_component (
      stream, "rpmcomponent");
  g_assert_nonnull (retrieved_component);
  g_assert_true (modulemd_component_equals (
    retrieved_component, (ModulemdComponent *)rpm_component));

  g_clear_pointer (&component_names, g_strfreev);

  // Add a Module component to a stream
  module_component = modulemd_component_module_new ("modulecomponent");
  modulemd_module_stream_v3_add_component (
    stream, (ModulemdComponent *)module_component);
  component_names =
    modulemd_module_stream_v3_get_module_component_names_as_strv (stream);
  g_assert_true (g_strv_contains ((const gchar *const *)component_names,
                                  "modulecomponent"));
  g_assert_cmpint (g_strv_length (component_names), ==, 1);

  retrieved_component =
    (ModulemdComponent *)modulemd_module_stream_v3_get_module_component (
      stream, "modulecomponent");
  g_assert_nonnull (retrieved_component);
  g_assert_true (modulemd_component_equals (
    retrieved_component, (ModulemdComponent *)module_component));

  g_clear_pointer (&component_names, g_strfreev);

  // Remove an RPM component from a stream
  modulemd_module_stream_v3_remove_rpm_component (stream, "rpmcomponent");
  component_names =
    modulemd_module_stream_v3_get_rpm_component_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (component_names), ==, 0);

  g_clear_pointer (&component_names, g_strfreev);

  // Remove a Module component from a stream
  modulemd_module_stream_v3_remove_module_component (stream,
                                                     "modulecomponent");
  component_names =
    modulemd_module_stream_v3_get_module_component_names_as_strv (stream);
  g_assert_cmpint (g_strv_length (component_names), ==, 0);

  g_clear_pointer (&component_names, g_strfreev);

  g_clear_object (&module_component);
  g_clear_object (&rpm_component);
  g_clear_object (&stream);
}


static void
module_stream_test_copy (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdModuleStream) copied_stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      /* Test copying with a stream name */
      stream = modulemd_module_stream_new (version, "foo", "latest");
      copied_stream = modulemd_module_stream_copy (stream, NULL, NULL);
      g_assert_nonnull (copied_stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_module_name (stream),
                       ==,
                       modulemd_module_stream_get_module_name (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_stream_name (stream),
                       ==,
                       modulemd_module_stream_get_stream_name (copied_stream));
      g_clear_object (&stream);
      g_clear_object (&copied_stream);


      /* Test copying without a stream name */
      stream = modulemd_module_stream_new (version, "foo", NULL);
      copied_stream = modulemd_module_stream_copy (stream, NULL, NULL);
      g_assert_nonnull (copied_stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_module_name (stream),
                       ==,
                       modulemd_module_stream_get_module_name (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_stream_name (stream),
                       ==,
                       modulemd_module_stream_get_stream_name (copied_stream));
      g_clear_object (&stream);
      g_clear_object (&copied_stream);

      /* Test copying with and renaming the stream name */
      stream = modulemd_module_stream_new (version, "foo", "latest");
      copied_stream = modulemd_module_stream_copy (stream, NULL, "earliest");
      g_assert_nonnull (copied_stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_module_name (stream),
                       ==,
                       modulemd_module_stream_get_module_name (copied_stream));
      g_assert_cmpstr (
        modulemd_module_stream_get_stream_name (stream), ==, "latest");
      g_assert_cmpstr (modulemd_module_stream_get_stream_name (copied_stream),
                       ==,
                       "earliest");
      g_clear_object (&stream);
      g_clear_object (&copied_stream);
    }
}


static void
module_stream_test_equals (void)
{
  g_autoptr (ModulemdModuleStream) stream_1 = NULL;
  g_autoptr (ModulemdModuleStream) stream_2 = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      /* Test equality with same stream and module names */
      stream_1 = modulemd_module_stream_new (version, "foo", "latest");
      stream_2 = modulemd_module_stream_new (version, "foo", "latest");

      g_assert_true (modulemd_module_stream_equals (stream_1, stream_2));
      g_clear_object (&stream_1);
      g_clear_object (&stream_2);


      /* Test equality with different stream names*/
      stream_1 = modulemd_module_stream_new (version, "foo", NULL);
      stream_2 = modulemd_module_stream_new (version, "bar", NULL);

      g_assert_false (modulemd_module_stream_equals (stream_1, stream_2));
      g_clear_object (&stream_1);
      g_clear_object (&stream_2);

      /* Test equality with different module name */
      stream_1 = modulemd_module_stream_new (version, "bar", "thor");
      stream_2 = modulemd_module_stream_new (version, "bar", "loki");

      g_assert_false (modulemd_module_stream_equals (stream_1, stream_2));
      g_clear_object (&stream_1);
      g_clear_object (&stream_2);

      /* Test equality with same arch */
      stream_1 = modulemd_module_stream_new (version, "bar", "thor");
      modulemd_module_stream_set_arch (stream_1, "x86_64");
      stream_2 = modulemd_module_stream_new (version, "bar", "thor");
      modulemd_module_stream_set_arch (stream_2, "x86_64");

      g_assert_true (modulemd_module_stream_equals (stream_1, stream_2));
      g_clear_object (&stream_1);
      g_clear_object (&stream_2);

      /* Test equality with different arch */
      stream_1 = modulemd_module_stream_new (version, "bar", "thor");
      modulemd_module_stream_set_arch (stream_1, "x86_64");
      stream_2 = modulemd_module_stream_new (version, "bar", "thor");
      modulemd_module_stream_set_arch (stream_2, "x86_25");

      g_assert_false (modulemd_module_stream_equals (stream_1, stream_2));
      g_clear_object (&stream_1);
      g_clear_object (&stream_2);
    }
}


G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static void
module_stream_test_nsvc (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *s_nsvc = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      // First test that nsvc is None for a module with no name
      stream = modulemd_module_stream_new (version, NULL, NULL);
      s_nsvc = modulemd_module_stream_get_nsvc_as_string (stream);
      g_assert_null (s_nsvc);
      g_clear_pointer (&s_nsvc, g_free);
      g_clear_object (&stream);

      // Now with valid module and stream names
      stream = modulemd_module_stream_new (version, "modulename", NULL);
      s_nsvc = modulemd_module_stream_get_nsvc_as_string (stream);
      g_assert_null (s_nsvc);
      g_clear_pointer (&s_nsvc, g_free);
      g_clear_object (&stream);

      // Now with valid module and stream names
      stream =
        modulemd_module_stream_new (version, "modulename", "streamname");
      s_nsvc = modulemd_module_stream_get_nsvc_as_string (stream);
      g_assert_cmpstr (s_nsvc, ==, "modulename:streamname:0");
      g_clear_pointer (&s_nsvc, g_free);

      //# Add a version number
      modulemd_module_stream_set_version (stream, 42);
      s_nsvc = modulemd_module_stream_get_nsvc_as_string (stream);
      g_assert_cmpstr (s_nsvc, ==, "modulename:streamname:42");
      g_clear_pointer (&s_nsvc, g_free);

      // Add a context
      modulemd_module_stream_set_context (stream, "deadbeef");
      s_nsvc = modulemd_module_stream_get_nsvc_as_string (stream);
      g_assert_cmpstr (s_nsvc, ==, "modulename:streamname:42:deadbeef");
      g_clear_pointer (&s_nsvc, g_free);
      g_clear_object (&stream);
    }
}
G_GNUC_END_IGNORE_DEPRECATIONS


static void
module_stream_test_nsvca (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *s_nsvca = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      // First test that NSVCA is None for a module with no name
      stream = modulemd_module_stream_new (version, NULL, NULL);
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_null (s_nsvca);
      g_clear_pointer (&s_nsvca, g_free);
      g_clear_object (&stream);

      // Now with valid module and stream names
      stream = modulemd_module_stream_new (version, "modulename", NULL);
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_cmpstr (s_nsvca, ==, "modulename");
      g_clear_pointer (&s_nsvca, g_free);
      g_clear_object (&stream);

      // Now with valid module and stream names
      stream =
        modulemd_module_stream_new (version, "modulename", "streamname");
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_cmpstr (s_nsvca, ==, "modulename:streamname");
      g_clear_pointer (&s_nsvca, g_free);

      //# Add a version number
      modulemd_module_stream_set_version (stream, 42);
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_cmpstr (s_nsvca, ==, "modulename:streamname:42");
      g_clear_pointer (&s_nsvca, g_free);

      // Add a context
      modulemd_module_stream_set_context (stream, "deadbeef");
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_cmpstr (s_nsvca, ==, "modulename:streamname:42:deadbeef");
      g_clear_pointer (&s_nsvca, g_free);

      // Add an architecture
      modulemd_module_stream_set_arch (stream, "x86_64");
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_cmpstr (
        s_nsvca, ==, "modulename:streamname:42:deadbeef:x86_64");
      g_clear_pointer (&s_nsvca, g_free);

      // Now try removing some of the bits in the middle
      modulemd_module_stream_set_context (stream, NULL);
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_cmpstr (s_nsvca, ==, "modulename:streamname:42::x86_64");
      g_clear_pointer (&s_nsvca, g_free);
      g_clear_object (&stream);

      stream = modulemd_module_stream_new (version, "modulename", NULL);
      modulemd_module_stream_set_arch (stream, "x86_64");
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_cmpstr (s_nsvca, ==, "modulename::::x86_64");
      g_clear_pointer (&s_nsvca, g_free);

      modulemd_module_stream_set_version (stream, 2019);
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_cmpstr (s_nsvca, ==, "modulename::2019::x86_64");
      g_clear_pointer (&s_nsvca, g_free);

      // Add a context
      modulemd_module_stream_set_context (stream, "feedfeed");
      s_nsvca = modulemd_module_stream_get_NSVCA_as_string (stream);
      g_assert_cmpstr (s_nsvca, ==, "modulename::2019:feedfeed:x86_64");
      g_clear_pointer (&s_nsvca, g_free);
      g_clear_object (&stream);
    }
}


static void
module_stream_v1_test_equals (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream_1 = NULL;
  g_autoptr (ModulemdModuleStreamV1) stream_2 = NULL;
  g_autoptr (ModulemdProfile) profile_1 = NULL;
  g_autoptr (ModulemdServiceLevel) servicelevel_1 = NULL;
  g_autoptr (ModulemdServiceLevel) servicelevel_2 = NULL;
  g_autoptr (ModulemdComponentModule) component_1 = NULL;
  g_autoptr (ModulemdComponentRpm) component_2 = NULL;

  /*Test equality of 2 streams with same string constants*/
  stream_1 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_set_community (stream_1, "community_1");
  modulemd_module_stream_v1_set_description (stream_1, "description_1");
  modulemd_module_stream_v1_set_documentation (stream_1, "documentation_1");
  modulemd_module_stream_v1_set_summary (stream_1, "summary_1");
  modulemd_module_stream_v1_set_tracker (stream_1, "tracker_1");

  stream_2 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_set_community (stream_2, "community_1");
  modulemd_module_stream_v1_set_description (stream_2, "description_1");
  modulemd_module_stream_v1_set_documentation (stream_2, "documentation_1");
  modulemd_module_stream_v1_set_summary (stream_2, "summary_1");
  modulemd_module_stream_v1_set_tracker (stream_2, "tracker_1");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with certain different string constants*/
  stream_1 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_set_community (stream_1, "community_1");
  modulemd_module_stream_v1_set_description (stream_1, "description_1");
  modulemd_module_stream_v1_set_documentation (stream_1, "documentation_1");
  modulemd_module_stream_v1_set_summary (stream_1, "summary_1");
  modulemd_module_stream_v1_set_tracker (stream_1, "tracker_1");

  stream_2 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_set_community (stream_2, "community_1");
  modulemd_module_stream_v1_set_description (stream_2, "description_2");
  modulemd_module_stream_v1_set_documentation (stream_2, "documentation_1");
  modulemd_module_stream_v1_set_summary (stream_2, "summary_2");
  modulemd_module_stream_v1_set_tracker (stream_2, "tracker_2");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with same hashtable sets*/
  stream_1 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_rpm_api (stream_1, "rpm_1");
  modulemd_module_stream_v1_add_rpm_api (stream_1, "rpm_2");
  modulemd_module_stream_v1_add_module_license (stream_1, "module_a");
  modulemd_module_stream_v1_add_module_license (stream_1, "module_b");
  modulemd_module_stream_v1_add_content_license (stream_1, "content_a");
  modulemd_module_stream_v1_add_content_license (stream_1, "content_b");
  modulemd_module_stream_v1_add_rpm_artifact (stream_1, "artifact_a");
  modulemd_module_stream_v1_add_rpm_artifact (stream_1, "artifact_b");
  modulemd_module_stream_v1_add_rpm_filter (stream_1, "filter_a");
  modulemd_module_stream_v1_add_rpm_filter (stream_1, "filter_b");

  stream_2 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_rpm_api (stream_2, "rpm_1");
  modulemd_module_stream_v1_add_rpm_api (stream_2, "rpm_2");
  modulemd_module_stream_v1_add_module_license (stream_2, "module_a");
  modulemd_module_stream_v1_add_module_license (stream_2, "module_b");
  modulemd_module_stream_v1_add_content_license (stream_2, "content_a");
  modulemd_module_stream_v1_add_content_license (stream_2, "content_b");
  modulemd_module_stream_v1_add_rpm_artifact (stream_2, "artifact_a");
  modulemd_module_stream_v1_add_rpm_artifact (stream_2, "artifact_b");
  modulemd_module_stream_v1_add_rpm_filter (stream_2, "filter_a");
  modulemd_module_stream_v1_add_rpm_filter (stream_2, "filter_b");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with different hashtable sets*/
  stream_1 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_rpm_api (stream_1, "rpm_1");
  modulemd_module_stream_v1_add_rpm_api (stream_1, "rpm_2");
  modulemd_module_stream_v1_add_module_license (stream_1, "module_a");
  modulemd_module_stream_v1_add_module_license (stream_1, "module_b");
  modulemd_module_stream_v1_add_content_license (stream_1, "content_a");
  modulemd_module_stream_v1_add_content_license (stream_1, "content_b");
  modulemd_module_stream_v1_add_rpm_artifact (stream_1, "artifact_a");
  modulemd_module_stream_v1_add_rpm_artifact (stream_1, "artifact_b");
  modulemd_module_stream_v1_add_rpm_artifact (stream_1, "artifact_c");
  modulemd_module_stream_v1_add_rpm_filter (stream_1, "filter_a");
  modulemd_module_stream_v1_add_rpm_filter (stream_1, "filter_b");

  stream_2 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_rpm_api (stream_2, "rpm_1");
  modulemd_module_stream_v1_add_module_license (stream_2, "module_a");
  modulemd_module_stream_v1_add_module_license (stream_2, "module_b");
  modulemd_module_stream_v1_add_content_license (stream_2, "content_a");
  modulemd_module_stream_v1_add_content_license (stream_2, "content_b");
  modulemd_module_stream_v1_add_rpm_artifact (stream_2, "artifact_a");
  modulemd_module_stream_v1_add_rpm_artifact (stream_2, "artifact_b");
  modulemd_module_stream_v1_add_rpm_filter (stream_2, "filter_a");
  modulemd_module_stream_v1_add_rpm_filter (stream_2, "filter_b");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with same dependencies*/
  stream_1 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_buildtime_requirement (
    stream_1, "testmodule", "stable");
  modulemd_module_stream_v1_add_runtime_requirement (
    stream_1, "testmodule", "latest");
  stream_2 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_buildtime_requirement (
    stream_2, "testmodule", "stable");
  modulemd_module_stream_v1_add_runtime_requirement (
    stream_2, "testmodule", "latest");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with different dependencies*/
  stream_1 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_buildtime_requirement (
    stream_1, "test", "stable");
  modulemd_module_stream_v1_add_runtime_requirement (
    stream_1, "testmodule", "latest");
  stream_2 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_buildtime_requirement (
    stream_2, "testmodule", "stable");
  modulemd_module_stream_v1_add_runtime_requirement (
    stream_2, "testmodule", "not_latest");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with same hashtables*/
  profile_1 = modulemd_profile_new ("testprofile");
  component_1 = modulemd_component_module_new ("testmodule");
  servicelevel_1 = modulemd_service_level_new ("foo");

  stream_1 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_profile (stream_1, profile_1);
  modulemd_module_stream_v1_add_component (stream_1,
                                           (ModulemdComponent *)component_1);
  modulemd_module_stream_v1_add_servicelevel (stream_1, servicelevel_1);
  stream_2 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_profile (stream_2, profile_1);
  modulemd_module_stream_v1_add_component (stream_2,
                                           (ModulemdComponent *)component_1);
  modulemd_module_stream_v1_add_servicelevel (stream_2, servicelevel_1);

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&profile_1);
  g_clear_object (&component_1);
  g_clear_object (&servicelevel_1);

  /*Test equality of 2 streams with different hashtables*/
  profile_1 = modulemd_profile_new ("testprofile");
  component_1 = modulemd_component_module_new ("testmodule");
  component_2 = modulemd_component_rpm_new ("something");
  servicelevel_1 = modulemd_service_level_new ("foo");
  servicelevel_2 = modulemd_service_level_new ("bar");

  stream_1 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_profile (stream_1, profile_1);
  modulemd_module_stream_v1_add_component (stream_1,
                                           (ModulemdComponent *)component_1);
  modulemd_module_stream_v1_add_servicelevel (stream_1, servicelevel_1);
  stream_2 = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_profile (stream_2, profile_1);
  modulemd_module_stream_v1_add_component (stream_2,
                                           (ModulemdComponent *)component_2);
  modulemd_module_stream_v1_add_servicelevel (stream_2, servicelevel_2);

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&profile_1);
  g_clear_object (&component_1);
  g_clear_object (&component_2);
  g_clear_object (&servicelevel_1);
  g_clear_object (&servicelevel_2);
}

static void
module_stream_v2_test_equals (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream_1 = NULL;
  g_autoptr (ModulemdModuleStreamV2) stream_2 = NULL;
  g_autoptr (ModulemdProfile) profile_1 = NULL;
  g_autoptr (ModulemdServiceLevel) servicelevel_1 = NULL;
  g_autoptr (ModulemdServiceLevel) servicelevel_2 = NULL;
  g_autoptr (ModulemdComponentModule) component_1 = NULL;
  g_autoptr (ModulemdComponentRpm) component_2 = NULL;
  g_autoptr (ModulemdDependencies) dep_1 = NULL;
  g_autoptr (ModulemdDependencies) dep_2 = NULL;
  g_autoptr (ModulemdRpmMapEntry) entry_1 = NULL;

  /*Test equality of 2 streams with same string constants*/
  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_set_community (stream_1, "community_1");
  modulemd_module_stream_v2_set_description (stream_1, "description_1");
  modulemd_module_stream_v2_set_documentation (stream_1, "documentation_1");
  modulemd_module_stream_v2_set_summary (stream_1, "summary_1");
  modulemd_module_stream_v2_set_tracker (stream_1, "tracker_1");

  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_set_community (stream_2, "community_1");
  modulemd_module_stream_v2_set_description (stream_2, "description_1");
  modulemd_module_stream_v2_set_documentation (stream_2, "documentation_1");
  modulemd_module_stream_v2_set_summary (stream_2, "summary_1");
  modulemd_module_stream_v2_set_tracker (stream_2, "tracker_1");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with certain different string constants*/
  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_set_community (stream_1, "community_1");
  modulemd_module_stream_v2_set_description (stream_1, "description_1");
  modulemd_module_stream_v2_set_documentation (stream_1, "documentation_1");
  modulemd_module_stream_v2_set_summary (stream_1, "summary_1");
  modulemd_module_stream_v2_set_tracker (stream_1, "tracker_1");

  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_set_community (stream_2, "community_1");
  modulemd_module_stream_v2_set_description (stream_2, "description_2");
  modulemd_module_stream_v2_set_documentation (stream_2, "documentation_1");
  modulemd_module_stream_v2_set_summary (stream_2, "summary_2");
  modulemd_module_stream_v2_set_tracker (stream_2, "tracker_2");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with same hashtable sets*/
  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_rpm_api (stream_1, "rpm_1");
  modulemd_module_stream_v2_add_rpm_api (stream_1, "rpm_2");
  modulemd_module_stream_v2_add_module_license (stream_1, "module_a");
  modulemd_module_stream_v2_add_module_license (stream_1, "module_b");
  modulemd_module_stream_v2_add_content_license (stream_1, "content_a");
  modulemd_module_stream_v2_add_content_license (stream_1, "content_b");
  modulemd_module_stream_v2_add_rpm_artifact (stream_1, "artifact_a");
  modulemd_module_stream_v2_add_rpm_artifact (stream_1, "artifact_b");
  modulemd_module_stream_v2_add_rpm_filter (stream_1, "filter_a");
  modulemd_module_stream_v2_add_rpm_filter (stream_1, "filter_b");

  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_rpm_api (stream_2, "rpm_1");
  modulemd_module_stream_v2_add_rpm_api (stream_2, "rpm_2");
  modulemd_module_stream_v2_add_module_license (stream_2, "module_a");
  modulemd_module_stream_v2_add_module_license (stream_2, "module_b");
  modulemd_module_stream_v2_add_content_license (stream_2, "content_a");
  modulemd_module_stream_v2_add_content_license (stream_2, "content_b");
  modulemd_module_stream_v2_add_rpm_artifact (stream_2, "artifact_a");
  modulemd_module_stream_v2_add_rpm_artifact (stream_2, "artifact_b");
  modulemd_module_stream_v2_add_rpm_filter (stream_2, "filter_a");
  modulemd_module_stream_v2_add_rpm_filter (stream_2, "filter_b");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with different hashtable sets*/
  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_rpm_api (stream_1, "rpm_1");
  modulemd_module_stream_v2_add_rpm_api (stream_1, "rpm_2");
  modulemd_module_stream_v2_add_module_license (stream_1, "module_a");
  modulemd_module_stream_v2_add_module_license (stream_1, "module_b");
  modulemd_module_stream_v2_add_content_license (stream_1, "content_a");
  modulemd_module_stream_v2_add_content_license (stream_1, "content_b");
  modulemd_module_stream_v2_add_rpm_artifact (stream_1, "artifact_a");
  modulemd_module_stream_v2_add_rpm_artifact (stream_1, "artifact_b");
  modulemd_module_stream_v2_add_rpm_artifact (stream_1, "artifact_c");
  modulemd_module_stream_v2_add_rpm_filter (stream_1, "filter_a");
  modulemd_module_stream_v2_add_rpm_filter (stream_1, "filter_b");

  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_rpm_api (stream_2, "rpm_1");
  modulemd_module_stream_v2_add_module_license (stream_2, "module_a");
  modulemd_module_stream_v2_add_module_license (stream_2, "module_b");
  modulemd_module_stream_v2_add_content_license (stream_2, "content_a");
  modulemd_module_stream_v2_add_content_license (stream_2, "content_b");
  modulemd_module_stream_v2_add_rpm_artifact (stream_2, "artifact_a");
  modulemd_module_stream_v2_add_rpm_artifact (stream_2, "artifact_b");
  modulemd_module_stream_v2_add_rpm_filter (stream_2, "filter_a");
  modulemd_module_stream_v2_add_rpm_filter (stream_2, "filter_b");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with same hashtables*/
  profile_1 = modulemd_profile_new ("testprofile");
  component_1 = modulemd_component_module_new ("testmodule");
  servicelevel_1 = modulemd_service_level_new ("foo");

  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_profile (stream_1, profile_1);
  modulemd_module_stream_v2_add_component (stream_1,
                                           (ModulemdComponent *)component_1);
  modulemd_module_stream_v2_add_servicelevel (stream_1, servicelevel_1);
  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_profile (stream_2, profile_1);
  modulemd_module_stream_v2_add_component (stream_2,
                                           (ModulemdComponent *)component_1);
  modulemd_module_stream_v2_add_servicelevel (stream_2, servicelevel_1);

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&profile_1);
  g_clear_object (&component_1);
  g_clear_object (&servicelevel_1);

  /*Test equality of 2 streams with different hashtables*/
  profile_1 = modulemd_profile_new ("testprofile");
  component_1 = modulemd_component_module_new ("testmodule");
  component_2 = modulemd_component_rpm_new ("something");
  servicelevel_1 = modulemd_service_level_new ("foo");
  servicelevel_2 = modulemd_service_level_new ("bar");

  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_profile (stream_1, profile_1);
  modulemd_module_stream_v2_add_component (stream_1,
                                           (ModulemdComponent *)component_1);
  modulemd_module_stream_v2_add_servicelevel (stream_1, servicelevel_1);
  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_profile (stream_2, profile_1);
  modulemd_module_stream_v2_add_component (stream_2,
                                           (ModulemdComponent *)component_2);
  modulemd_module_stream_v2_add_servicelevel (stream_2, servicelevel_2);

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&profile_1);
  g_clear_object (&component_1);
  g_clear_object (&component_2);
  g_clear_object (&servicelevel_1);
  g_clear_object (&servicelevel_2);

  /*Test equality of 2 streams with same dependencies*/
  dep_1 = modulemd_dependencies_new ();
  modulemd_dependencies_add_buildtime_stream (dep_1, "foo", "stable");

  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream_1, dep_1);
  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream_2, dep_1);

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&dep_1);

  /*Test equality of 2 streams with different dependencies*/
  dep_1 = modulemd_dependencies_new ();
  modulemd_dependencies_add_buildtime_stream (dep_1, "foo", "stable");
  dep_2 = modulemd_dependencies_new ();
  modulemd_dependencies_add_buildtime_stream (dep_2, "foo", "latest");

  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream_1, dep_1);
  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_add_dependencies (stream_2, dep_2);

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&dep_1);
  g_clear_object (&dep_2);

  /*Test equality of 2 streams with same rpm artifact map entry*/
  entry_1 = modulemd_rpm_map_entry_new (
    "bar", 0, "1.23", "1.module_deadbeef", "x86_64");

  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_set_rpm_artifact_map_entry (
    stream_1, entry_1, "sha256", "baddad");
  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_set_rpm_artifact_map_entry (
    stream_2, entry_1, "sha256", "baddad");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&entry_1);

  /*Test equality of 2 streams with different rpm artifact map entry*/
  entry_1 = modulemd_rpm_map_entry_new (
    "bar", 0, "1.23", "1.module_deadbeef", "x86_64");

  stream_1 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_set_rpm_artifact_map_entry (
    stream_1, entry_1, "sha256", "baddad");
  stream_2 = modulemd_module_stream_v2_new (NULL, NULL);
  modulemd_module_stream_v2_set_rpm_artifact_map_entry (
    stream_2, entry_1, "sha256", "badmom");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&entry_1);
}

static void
module_stream_v3_test_equals (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream_1 = NULL;
  g_autoptr (ModulemdModuleStreamV3) stream_2 = NULL;
  g_autoptr (ModulemdProfile) profile_1 = NULL;
  g_autoptr (ModulemdComponentModule) component_1 = NULL;
  g_autoptr (ModulemdComponentRpm) component_2 = NULL;
  g_autoptr (ModulemdRpmMapEntry) entry_1 = NULL;

  /*Test equality of 2 streams with same string constants*/
  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_community (stream_1, "community_1");
  modulemd_module_stream_v3_set_description (stream_1, "description_1");
  modulemd_module_stream_v3_set_documentation (stream_1, "documentation_1");
  modulemd_module_stream_v3_set_summary (stream_1, "summary_1");
  modulemd_module_stream_v3_set_tracker (stream_1, "tracker_1");

  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_community (stream_2, "community_1");
  modulemd_module_stream_v3_set_description (stream_2, "description_1");
  modulemd_module_stream_v3_set_documentation (stream_2, "documentation_1");
  modulemd_module_stream_v3_set_summary (stream_2, "summary_1");
  modulemd_module_stream_v3_set_tracker (stream_2, "tracker_1");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with certain different string constants*/
  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_community (stream_1, "community_1");
  modulemd_module_stream_v3_set_description (stream_1, "description_1");
  modulemd_module_stream_v3_set_documentation (stream_1, "documentation_1");
  modulemd_module_stream_v3_set_summary (stream_1, "summary_1");
  modulemd_module_stream_v3_set_tracker (stream_1, "tracker_1");

  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_community (stream_2, "community_1");
  modulemd_module_stream_v3_set_description (stream_2, "description_2");
  modulemd_module_stream_v3_set_documentation (stream_2, "documentation_1");
  modulemd_module_stream_v3_set_summary (stream_2, "summary_2");
  modulemd_module_stream_v3_set_tracker (stream_2, "tracker_2");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with same hashtable sets*/
  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_add_rpm_api (stream_1, "rpm_1");
  modulemd_module_stream_v3_add_rpm_api (stream_1, "rpm_2");
  modulemd_module_stream_v3_add_module_license (stream_1, "module_a");
  modulemd_module_stream_v3_add_module_license (stream_1, "module_b");
  modulemd_module_stream_v3_add_content_license (stream_1, "content_a");
  modulemd_module_stream_v3_add_content_license (stream_1, "content_b");
  modulemd_module_stream_v3_add_rpm_artifact (stream_1, "artifact_a");
  modulemd_module_stream_v3_add_rpm_artifact (stream_1, "artifact_b");
  modulemd_module_stream_v3_add_rpm_filter (stream_1, "filter_a");
  modulemd_module_stream_v3_add_rpm_filter (stream_1, "filter_b");

  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_add_rpm_api (stream_2, "rpm_1");
  modulemd_module_stream_v3_add_rpm_api (stream_2, "rpm_2");
  modulemd_module_stream_v3_add_module_license (stream_2, "module_a");
  modulemd_module_stream_v3_add_module_license (stream_2, "module_b");
  modulemd_module_stream_v3_add_content_license (stream_2, "content_a");
  modulemd_module_stream_v3_add_content_license (stream_2, "content_b");
  modulemd_module_stream_v3_add_rpm_artifact (stream_2, "artifact_a");
  modulemd_module_stream_v3_add_rpm_artifact (stream_2, "artifact_b");
  modulemd_module_stream_v3_add_rpm_filter (stream_2, "filter_a");
  modulemd_module_stream_v3_add_rpm_filter (stream_2, "filter_b");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with different hashtable sets*/
  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_add_rpm_api (stream_1, "rpm_1");
  modulemd_module_stream_v3_add_rpm_api (stream_1, "rpm_2");
  modulemd_module_stream_v3_add_module_license (stream_1, "module_a");
  modulemd_module_stream_v3_add_module_license (stream_1, "module_b");
  modulemd_module_stream_v3_add_content_license (stream_1, "content_a");
  modulemd_module_stream_v3_add_content_license (stream_1, "content_b");
  modulemd_module_stream_v3_add_rpm_artifact (stream_1, "artifact_a");
  modulemd_module_stream_v3_add_rpm_artifact (stream_1, "artifact_b");
  modulemd_module_stream_v3_add_rpm_artifact (stream_1, "artifact_c");
  modulemd_module_stream_v3_add_rpm_filter (stream_1, "filter_a");
  modulemd_module_stream_v3_add_rpm_filter (stream_1, "filter_b");

  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_add_rpm_api (stream_2, "rpm_1");
  modulemd_module_stream_v3_add_module_license (stream_2, "module_a");
  modulemd_module_stream_v3_add_module_license (stream_2, "module_b");
  modulemd_module_stream_v3_add_content_license (stream_2, "content_a");
  modulemd_module_stream_v3_add_content_license (stream_2, "content_b");
  modulemd_module_stream_v3_add_rpm_artifact (stream_2, "artifact_a");
  modulemd_module_stream_v3_add_rpm_artifact (stream_2, "artifact_b");
  modulemd_module_stream_v3_add_rpm_filter (stream_2, "filter_a");
  modulemd_module_stream_v3_add_rpm_filter (stream_2, "filter_b");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with same hashtables*/
  profile_1 = modulemd_profile_new ("testprofile");
  component_1 = modulemd_component_module_new ("testmodule");

  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_add_profile (stream_1, profile_1);
  modulemd_module_stream_v3_add_component (stream_1,
                                           (ModulemdComponent *)component_1);
  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_add_profile (stream_2, profile_1);
  modulemd_module_stream_v3_add_component (stream_2,
                                           (ModulemdComponent *)component_1);

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&profile_1);
  g_clear_object (&component_1);

  /*Test equality of 2 streams with different hashtables*/
  profile_1 = modulemd_profile_new ("testprofile");
  component_1 = modulemd_component_module_new ("testmodule");
  component_2 = modulemd_component_rpm_new ("something");

  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_add_profile (stream_1, profile_1);
  modulemd_module_stream_v3_add_component (stream_1,
                                           (ModulemdComponent *)component_1);
  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_add_profile (stream_2, profile_1);
  modulemd_module_stream_v3_add_component (stream_2,
                                           (ModulemdComponent *)component_2);

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&profile_1);
  g_clear_object (&component_1);
  g_clear_object (&component_2);

  /*Test equality of 2 streams with same dependencies*/
  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_platform (stream_1, "f30");
  modulemd_module_stream_v3_add_buildtime_requirement (
    stream_1, "testmodule", "stable");
  modulemd_module_stream_v3_add_runtime_requirement (
    stream_1, "testmodule", "latest");
  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_platform (stream_2, "f30");
  modulemd_module_stream_v3_add_buildtime_requirement (
    stream_2, "testmodule", "stable");
  modulemd_module_stream_v3_add_runtime_requirement (
    stream_2, "testmodule", "latest");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with different dependencies*/
  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_platform (stream_1, "f30");
  modulemd_module_stream_v3_add_buildtime_requirement (
    stream_1, "test", "stable");
  modulemd_module_stream_v3_add_runtime_requirement (
    stream_1, "testmodule", "latest");
  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_platform (stream_2, "f30");
  modulemd_module_stream_v3_add_buildtime_requirement (
    stream_2, "testmodule", "stable");
  modulemd_module_stream_v3_add_runtime_requirement (
    stream_2, "testmodule", "not_latest");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);

  /*Test equality of 2 streams with same rpm artifact map entry*/
  entry_1 = modulemd_rpm_map_entry_new (
    "bar", 0, "1.23", "1.module_deadbeef", "x86_64");

  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_rpm_artifact_map_entry (
    stream_1, entry_1, "sha256", "baddad");
  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_rpm_artifact_map_entry (
    stream_2, entry_1, "sha256", "baddad");

  g_assert_true (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&entry_1);

  /*Test equality of 2 streams with different rpm artifact map entry*/
  entry_1 = modulemd_rpm_map_entry_new (
    "bar", 0, "1.23", "1.module_deadbeef", "x86_64");

  stream_1 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_rpm_artifact_map_entry (
    stream_1, entry_1, "sha256", "baddad");
  stream_2 = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_set_rpm_artifact_map_entry (
    stream_2, entry_1, "sha256", "badmom");

  g_assert_false (modulemd_module_stream_equals (
    (ModulemdModuleStream *)stream_1, (ModulemdModuleStream *)stream_2));
  g_clear_object (&stream_1);
  g_clear_object (&stream_2);
  g_clear_object (&entry_1);
}


static void
module_stream_v1_test_dependencies (void)
{
  g_auto (GStrv) list = NULL;
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  stream = modulemd_module_stream_v1_new (NULL, NULL);
  modulemd_module_stream_v1_add_buildtime_requirement (
    stream, "testmodule", "stable");
  list = modulemd_module_stream_v1_get_buildtime_modules_as_strv (stream);
  g_assert_cmpint (g_strv_length (list), ==, 1);
  g_assert_cmpstr (list[0], ==, "testmodule");
  g_assert_cmpstr (modulemd_module_stream_v1_get_buildtime_requirement_stream (
                     stream, "testmodule"),
                   ==,
                   "stable");
  g_clear_pointer (&list, g_strfreev);

  modulemd_module_stream_v1_add_runtime_requirement (
    stream, "testmodule", "latest");
  list = modulemd_module_stream_v1_get_runtime_modules_as_strv (stream);
  g_assert_cmpint (g_strv_length (list), ==, 1);
  g_assert_cmpstr (list[0], ==, "testmodule");
  g_assert_cmpstr (modulemd_module_stream_v1_get_runtime_requirement_stream (
                     stream, "testmodule"),
                   ==,
                   "latest");
  g_clear_pointer (&list, g_strfreev);
  g_clear_object (&stream);
}

static void
module_stream_v2_test_dependencies (void)
{
  g_auto (GStrv) list = NULL;
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdDependencies) dep = NULL;
  stream = modulemd_module_stream_v2_new (NULL, NULL);
  dep = modulemd_dependencies_new ();
  modulemd_dependencies_add_buildtime_stream (dep, "foo", "stable");
  modulemd_dependencies_set_empty_runtime_dependencies_for_module (dep, "bar");
  modulemd_module_stream_v2_add_dependencies (stream, dep);
  GPtrArray *deps_list = modulemd_module_stream_v2_get_dependencies (stream);
  g_assert_cmpint (deps_list->len, ==, 1);

  list = modulemd_dependencies_get_buildtime_modules_as_strv (
    g_ptr_array_index (deps_list, 0));
  g_assert_cmpstr (list[0], ==, "foo");
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_dependencies_get_buildtime_streams_as_strv (
    g_ptr_array_index (deps_list, 0), "foo");
  g_assert_nonnull (list);
  g_assert_cmpstr (list[0], ==, "stable");
  g_assert_null (list[1]);
  g_clear_pointer (&list, g_strfreev);

  list = modulemd_dependencies_get_runtime_modules_as_strv (
    g_ptr_array_index (deps_list, 0));
  g_assert_nonnull (list);
  g_assert_cmpstr (list[0], ==, "bar");
  g_assert_null (list[1]);

  g_clear_pointer (&list, g_strfreev);
  g_clear_object (&dep);
  g_clear_object (&stream);
}

static void
module_stream_v3_test_dependencies (void)
{
  g_auto (GStrv) list = NULL;
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  stream = modulemd_module_stream_v3_new (NULL, NULL);
  modulemd_module_stream_v3_add_buildtime_requirement (
    stream, "testmodule", "stable");
  list = modulemd_module_stream_v3_get_buildtime_modules_as_strv (stream);
  g_assert_cmpint (g_strv_length (list), ==, 1);
  g_assert_cmpstr (list[0], ==, "testmodule");
  g_assert_cmpstr (modulemd_module_stream_v3_get_buildtime_requirement_stream (
                     stream, "testmodule"),
                   ==,
                   "stable");
  g_clear_pointer (&list, g_strfreev);

  modulemd_module_stream_v3_add_runtime_requirement (
    stream, "testmodule", "latest");
  list = modulemd_module_stream_v3_get_runtime_modules_as_strv (stream);
  g_assert_cmpint (g_strv_length (list), ==, 1);
  g_assert_cmpstr (list[0], ==, "testmodule");
  g_assert_cmpstr (modulemd_module_stream_v3_get_runtime_requirement_stream (
                     stream, "testmodule"),
                   ==,
                   "latest");
  g_clear_pointer (&list, g_strfreev);
  g_clear_object (&stream);
}


static void
module_stream_v1_test_parse_dump (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  g_autoptr (GError) error = NULL;
  gboolean ret;
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v1.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  /* First parse it */
  yaml_parser_set_input_file (&parser, yaml_stream);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_STREAM_START_EVENT);
  yaml_event_delete (&event);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_DOCUMENT_START_EVENT);
  yaml_event_delete (&event);

  subdoc = modulemd_yaml_parse_document_type (&parser);
  g_assert_nonnull (subdoc);
  g_assert_null (modulemd_subdocument_info_get_gerror (subdoc));

  g_assert_cmpint (modulemd_subdocument_info_get_doctype (subdoc),
                   ==,
                   MODULEMD_YAML_DOC_MODULESTREAM);
  g_assert_cmpint (modulemd_subdocument_info_get_mdversion (subdoc), ==, 1);
  g_assert_nonnull (modulemd_subdocument_info_get_yaml (subdoc));

  stream = modulemd_module_stream_v1_parse_yaml (subdoc, TRUE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (stream);

  /* Then dump it */
  g_debug ("Starting dumping");
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  ret = modulemd_module_stream_v1_emit_yaml (stream, &emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  ret = mmd_emitter_end_stream (&emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  g_assert_nonnull (yaml_string->str);

  g_assert_cmpstr (
    yaml_string->str,
    ==,
    "---\n"
    "document: modulemd\n"
    "version: 1\n"
    "data:\n"
    "  name: foo\n"
    "  stream: \"stream-name\"\n"
    "  version: 20160927144203\n"
    "  context: c0ffee43\n"
    "  arch: x86_64\n"
    "  summary: An example module\n"
    "  description: >-\n"
    "    A module for the demonstration of the metadata format. Also, the "
    "obligatory lorem\n"
    "    ipsum dolor sit amet goes right here.\n"
    "  servicelevels:\n"
    "    bug_fixes:\n"
    "      eol: 2077-10-23\n"
    "    rawhide:\n"
    "      eol: 2077-10-23\n"
    "    security_fixes:\n"
    "      eol: 2077-10-23\n"
    "    stable_api:\n"
    "      eol: 2077-10-23\n"
    "  license:\n"
    "    module:\n"
    "    - MIT\n"
    "    content:\n"
    "    - Beerware\n"
    "    - GPLv2+\n"
    "    - zlib\n"
    "  xmd:\n"
    "    some_key: some_data\n"
    "  dependencies:\n"
    "    buildrequires:\n"
    "      extra-build-env: and-its-stream-name-too\n"
    "      platform: and-its-stream-name\n"
    "    requires:\n"
    "      platform: and-its-stream-name\n"
    "  references:\n"
    "    community: http://www.example.com/\n"
    "    documentation: http://www.example.com/\n"
    "    tracker: http://www.example.com/\n"
    "  profiles:\n"
    "    buildroot:\n"
    "      rpms:\n"
    "      - bar-devel\n"
    "    container:\n"
    "      rpms:\n"
    "      - bar\n"
    "      - bar-devel\n"
    "    default:\n"
    "      rpms:\n"
    "      - bar\n"
    "      - bar-extras\n"
    "      - baz\n"
    "    minimal:\n"
    "      description: Minimal profile installing only the bar package.\n"
    "      rpms:\n"
    "      - bar\n"
    "    srpm-buildroot:\n"
    "      rpms:\n"
    "      - bar-extras\n"
    "  api:\n"
    "    rpms:\n"
    "    - bar\n"
    "    - bar-devel\n"
    "    - bar-extras\n"
    "    - baz\n"
    "    - xxx\n"
    "  filter:\n"
    "    rpms:\n"
    "    - baz-nonfoo\n"
    "  buildopts:\n"
    "    rpms:\n"
    "      macros: >\n"
    "        %demomacro 1\n"
    "\n"
    "        %demomacro2 %{demomacro}23\n"
    "  components:\n"
    "    rpms:\n"
    "      bar:\n"
    "        rationale: We need this to demonstrate stuff.\n"
    "        repository: https://pagure.io/bar.git\n"
    "        cache: https://example.com/cache\n"
    "        ref: 26ca0c0\n"
    "      baz:\n"
    "        rationale: This one is here to demonstrate other stuff.\n"
    "      xxx:\n"
    "        rationale: xxx demonstrates arches and multilib.\n"
    "        arches: [i686, x86_64]\n"
    "        multilib: [x86_64]\n"
    "      xyz:\n"
    "        rationale: xyz is a bundled dependency of xxx.\n"
    "        buildorder: 10\n"
    "    modules:\n"
    "      includedmodule:\n"
    "        rationale: Included in the stack, just because.\n"
    "        repository: https://pagure.io/includedmodule.git\n"
    "        ref: somecoolbranchname\n"
    "        buildorder: 100\n"
    "  artifacts:\n"
    "    rpms:\n"
    "    - bar-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-devel-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-extras-0:1.23-1.module_deadbeef.x86_64\n"
    "    - baz-0:42-42.module_deadbeef.x86_64\n"
    "    - xxx-0:1-1.module_deadbeef.i686\n"
    "    - xxx-0:1-1.module_deadbeef.x86_64\n"
    "    - xyz-0:1-1.module_deadbeef.x86_64\n"
    "...\n");
}

static void
module_stream_v2_test_parse_dump (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (GError) error = NULL;
  gboolean ret;
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v2.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  /* First parse it */
  yaml_parser_set_input_file (&parser, yaml_stream);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_STREAM_START_EVENT);
  yaml_event_delete (&event);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_DOCUMENT_START_EVENT);
  yaml_event_delete (&event);

  subdoc = modulemd_yaml_parse_document_type (&parser);
  g_assert_nonnull (subdoc);
  g_assert_null (modulemd_subdocument_info_get_gerror (subdoc));

  g_assert_cmpint (modulemd_subdocument_info_get_doctype (subdoc),
                   ==,
                   MODULEMD_YAML_DOC_MODULESTREAM);
  g_assert_cmpint (modulemd_subdocument_info_get_mdversion (subdoc), ==, 2);
  g_assert_nonnull (modulemd_subdocument_info_get_yaml (subdoc));

  stream = modulemd_module_stream_v2_parse_yaml (subdoc, TRUE, FALSE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (stream);

  /* Then dump it */
  g_debug ("Starting dumping");
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  ret = modulemd_module_stream_v2_emit_yaml (stream, &emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  ret = mmd_emitter_end_stream (&emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  g_assert_nonnull (yaml_string->str);

  g_assert_cmpstr (
    yaml_string->str,
    ==,
    "---\n"
    "document: modulemd\n"
    "version: 2\n"
    "data:\n"
    "  name: foo\n"
    "  stream: \"latest\"\n"
    "  version: 20160927144203\n"
    "  context: c0ffee43\n"
    "  arch: x86_64\n"
    "  summary: An example module\n"
    "  description: >-\n"
    "    A module for the demonstration of the metadata format. Also, the "
    "obligatory lorem\n"
    "    ipsum dolor sit amet goes right here.\n"
    "  servicelevels:\n"
    "    bug_fixes:\n"
    "      eol: 2077-10-23\n"
    "    rawhide:\n"
    "      eol: 2077-10-23\n"
    "    security_fixes:\n"
    "      eol: 2077-10-23\n"
    "    stable_api:\n"
    "      eol: 2077-10-23\n"
    "  license:\n"
    "    module:\n"
    "    - MIT\n"
    "    content:\n"
    "    - Beerware\n"
    "    - GPLv2+\n"
    "    - zlib\n"
    "  xmd:\n"
    "    some_key: some_data\n"
    "  dependencies:\n"
    "  - buildrequires:\n"
    "      platform: [-epel7, -f27, -f28]\n"
    "    requires:\n"
    "      platform: [-epel7, -f27, -f28]\n"
    "  - buildrequires:\n"
    "      buildtools: [v1, v2]\n"
    "      compatible: [v3]\n"
    "      platform: [f27]\n"
    "    requires:\n"
    "      compatible: [v3, v4]\n"
    "      platform: [f27]\n"
    "  - buildrequires:\n"
    "      platform: [f28]\n"
    "    requires:\n"
    "      platform: [f28]\n"
    "      runtime: [a, b]\n"
    "  - buildrequires:\n"
    "      extras: []\n"
    "      moreextras: [bar, foo]\n"
    "      platform: [epel7]\n"
    "    requires:\n"
    "      extras: []\n"
    "      moreextras: [bar, foo]\n"
    "      platform: [epel7]\n"
    "  references:\n"
    "    community: http://www.example.com/\n"
    "    documentation: http://www.example.com/\n"
    "    tracker: http://www.example.com/\n"
    "  profiles:\n"
    "    buildroot:\n"
    "      rpms:\n"
    "      - bar-devel\n"
    "    container:\n"
    "      rpms:\n"
    "      - bar\n"
    "      - bar-devel\n"
    "    minimal:\n"
    "      description: Minimal profile installing only the bar package.\n"
    "      rpms:\n"
    "      - bar\n"
    "    srpm-buildroot:\n"
    "      rpms:\n"
    "      - bar-extras\n"
    "  api:\n"
    "    rpms:\n"
    "    - bar\n"
    "    - bar-devel\n"
    "    - bar-extras\n"
    "    - baz\n"
    "    - xxx\n"
    "  filter:\n"
    "    rpms:\n"
    "    - baz-nonfoo\n"
    "  buildopts:\n"
    "    rpms:\n"
    "      macros: >\n"
    "        %demomacro 1\n"
    "\n"
    "        %demomacro2 %{demomacro}23\n"
    "      whitelist:\n"
    "      - fooscl-1-bar\n"
    "      - fooscl-1-baz\n"
    "      - xxx\n"
    "      - xyz\n"
    "    arches: [i686, x86_64]\n"
    "  components:\n"
    "    rpms:\n"
    "      bar:\n"
    "        rationale: We need this to demonstrate stuff.\n"
    "        name: bar-real\n"
    "        repository: https://pagure.io/bar.git\n"
    "        cache: https://example.com/cache\n"
    "        ref: 26ca0c0\n"
    "      baz:\n"
    "        rationale: Demonstrate updating the buildroot contents.\n"
    "        buildroot: true\n"
    "        srpm-buildroot: true\n"
    "        buildorder: -1\n"
    "      xxx:\n"
    "        rationale: xxx demonstrates arches and multilib.\n"
    "        arches: [i686, x86_64]\n"
    "        multilib: [x86_64]\n"
    "      xyz:\n"
    "        rationale: xyz is a bundled dependency of xxx.\n"
    "        buildorder: 10\n"
    "    modules:\n"
    "      includedmodule:\n"
    "        rationale: Included in the stack, just because.\n"
    "        repository: https://pagure.io/includedmodule.git\n"
    "        ref: somecoolbranchname\n"
    "        buildorder: 100\n"
    "  artifacts:\n"
    "    rpms:\n"
    "    - bar-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-devel-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-extras-0:1.23-1.module_deadbeef.x86_64\n"
    "    - baz-0:42-42.module_deadbeef.x86_64\n"
    "    - xxx-0:1-1.module_deadbeef.i686\n"
    "    - xxx-0:1-1.module_deadbeef.x86_64\n"
    "    - xyz-0:1-1.module_deadbeef.x86_64\n"
    "    rpm-map:\n"
    "      sha256:\n"
    "        "
    "ee47083ed80146eb2c84e9a94d0836393912185dcda62b9d93ee0c2ea5dc795b:\n"
    "          name: bar\n"
    "          epoch: 0\n"
    "          version: 1.23\n"
    "          release: 1.module_deadbeef\n"
    "          arch: x86_64\n"
    "          nevra: bar-0:1.23-1.module_deadbeef.x86_64\n"
    "...\n");
}

static void
module_stream_v3_test_parse_dump (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_autoptr (GError) error = NULL;
  gboolean ret;
  MMD_INIT_YAML_PARSER (parser);
  MMD_INIT_YAML_EVENT (event);
  MMD_INIT_YAML_EMITTER (emitter);
  MMD_INIT_YAML_STRING (&emitter, yaml_string);
  g_autofree gchar *yaml_path = NULL;
  g_autoptr (FILE) yaml_stream = NULL;
  g_autoptr (ModulemdSubdocumentInfo) subdoc = NULL;
  yaml_path = g_strdup_printf ("%s/yaml_specs/modulemd_stream_v3.yaml",
                               g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rbe");
  g_assert_nonnull (yaml_stream);

  /* First parse it */
  yaml_parser_set_input_file (&parser, yaml_stream);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_STREAM_START_EVENT);
  yaml_event_delete (&event);
  g_assert_true (yaml_parser_parse (&parser, &event));
  g_assert_cmpint (event.type, ==, YAML_DOCUMENT_START_EVENT);
  yaml_event_delete (&event);

  subdoc = modulemd_yaml_parse_document_type (&parser);
  g_assert_nonnull (subdoc);
  g_assert_null (modulemd_subdocument_info_get_gerror (subdoc));

  g_assert_cmpint (modulemd_subdocument_info_get_doctype (subdoc),
                   ==,
                   MODULEMD_YAML_DOC_MODULESTREAM);
  g_assert_cmpint (modulemd_subdocument_info_get_mdversion (subdoc), ==, 3);
  g_assert_nonnull (modulemd_subdocument_info_get_yaml (subdoc));

  stream = modulemd_module_stream_v3_parse_yaml (subdoc, TRUE, &error);
  g_assert_no_error (error);
  g_assert_nonnull (stream);

  /* Then dump it */
  g_debug ("Starting dumping");
  g_assert_true (mmd_emitter_start_stream (&emitter, &error));
  ret = modulemd_module_stream_v3_emit_yaml (stream, &emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  ret = mmd_emitter_end_stream (&emitter, &error);
  g_assert_no_error (error);
  g_assert_true (ret);
  g_assert_nonnull (yaml_string->str);

  g_assert_cmpstr (
    yaml_string->str,
    ==,
    "---\n"
    "document: modulemd-stream\n"
    "version: 3\n"
    "data:\n"
    "  name: foo\n"
    "  stream: \"latest\"\n"
    "  version: 20160927144203\n"
    "  context: CTX1\n"
    "  arch: x86_64\n"
    "  summary: An example module\n"
    "  description: >-\n"
    "    A module for the demonstration of the metadata format. Also, the "
    "obligatory lorem\n"
    "    ipsum dolor sit amet goes right here.\n"
    "  license:\n"
    "    module:\n"
    "    - MIT\n"
    "    content:\n"
    "    - Beerware\n"
    "    - GPLv2+\n"
    "    - zlib\n"
    "  xmd:\n"
    "    a_list:\n"
    "    - a\n"
    "    - b\n"
    "    some_key: some_data\n"
    "  dependencies:\n"
    "    platform: f32\n"
    "    buildrequires:\n"
    "      appframework: [v1]\n"
    "    requires:\n"
    "      appframework: [v1]\n"
    "  references:\n"
    "    community: http://www.example.com/\n"
    "    documentation: http://www.example.com/\n"
    "    tracker: http://www.example.com/\n"
    "  profiles:\n"
    "    buildroot:\n"
    "      rpms:\n"
    "      - bar-devel\n"
    "    container:\n"
    "      rpms:\n"
    "      - bar\n"
    "      - bar-devel\n"
    "    minimal:\n"
    "      description: Minimal profile installing only the bar package.\n"
    "      rpms:\n"
    "      - bar\n"
    "    srpm-buildroot:\n"
    "      rpms:\n"
    "      - bar-extras\n"
    "  api:\n"
    "    rpms:\n"
    "    - bar\n"
    "    - bar-devel\n"
    "    - bar-extras\n"
    "    - baz\n"
    "    - xxx\n"
    "  filter:\n"
    "    rpms:\n"
    "    - baz-nonfoo\n"
    "  buildopts:\n"
    "    rpms:\n"
    "      macros: >\n"
    "        %demomacro 1\n"
    "\n"
    "        %demomacro2 %{demomacro}23\n"
    "      whitelist:\n"
    "      - fooscl-1-bar\n"
    "      - fooscl-1-baz\n"
    "      - xxx\n"
    "      - xyz\n"
    "    arches: [i686, x86_64]\n"
    "  components:\n"
    "    rpms:\n"
    "      bar:\n"
    "        rationale: We need this to demonstrate stuff.\n"
    "        name: bar-real\n"
    "        repository: https://pagure.io/bar.git\n"
    "        cache: https://example.com/cache\n"
    "        ref: 26ca0c0\n"
    "      baz:\n"
    "        rationale: Demonstrate updating the buildroot contents.\n"
    "        buildroot: true\n"
    "        srpm-buildroot: true\n"
    "        buildorder: -1\n"
    "      xxx:\n"
    "        rationale: xxx demonstrates arches and multilib.\n"
    "        arches: [i686, x86_64]\n"
    "        multilib: [x86_64]\n"
    "      xyz:\n"
    "        rationale: xyz is a bundled dependency of xxx.\n"
    "        buildorder: 10\n"
    "    modules:\n"
    "      includedmodule:\n"
    "        rationale: Included in the stack, just because.\n"
    "        repository: https://pagure.io/includedmodule.git\n"
    "        ref: somecoolbranchname\n"
    "        buildorder: 100\n"
    "  artifacts:\n"
    "    rpms:\n"
    "    - bar-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-devel-0:1.23-1.module_deadbeef.x86_64\n"
    "    - bar-extras-0:1.23-1.module_deadbeef.x86_64\n"
    "    - baz-0:42-42.module_deadbeef.x86_64\n"
    "    - xxx-0:1-1.module_deadbeef.i686\n"
    "    - xxx-0:1-1.module_deadbeef.x86_64\n"
    "    - xyz-0:1-1.module_deadbeef.x86_64\n"
    "    rpm-map:\n"
    "      sha256:\n"
    "        "
    "ee47083ed80146eb2c84e9a94d0836393912185dcda62b9d93ee0c2ea5dc795b:\n"
    "          name: bar\n"
    "          epoch: 0\n"
    "          version: 1.23\n"
    "          release: 1.module_deadbeef\n"
    "          arch: x86_64\n"
    "          nevra: bar-0:1.23-1.module_deadbeef.x86_64\n"
    "...\n");
}


static void
module_stream_v1_test_depends_on_stream (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *module_name = NULL;
  g_autofree gchar *module_stream = NULL;

  path = g_strdup_printf ("%s/dependson_v1.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (path);
  stream = modulemd_module_stream_read_file (
    path, TRUE, module_name, module_stream, &error);
  g_assert_nonnull (stream);

  g_assert_true (
    modulemd_module_stream_depends_on_stream (stream, "platform", "f30"));
  g_assert_true (modulemd_module_stream_build_depends_on_stream (
    stream, "platform", "f30"));

  g_assert_false (
    modulemd_module_stream_depends_on_stream (stream, "platform", "f28"));
  g_assert_false (modulemd_module_stream_build_depends_on_stream (
    stream, "platform", "f28"));

  g_assert_false (
    modulemd_module_stream_depends_on_stream (stream, "base", "f30"));
  g_assert_false (
    modulemd_module_stream_build_depends_on_stream (stream, "base", "f30"));
  g_clear_object (&stream);
}

static void
module_stream_v2_test_depends_on_stream (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *module_name = NULL;
  g_autofree gchar *module_stream = NULL;

  path = g_strdup_printf ("%s/dependson_v2.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (path);
  stream = modulemd_module_stream_read_file (
    path, TRUE, module_name, module_stream, &error);
  g_assert_nonnull (stream);

  g_assert_true (
    modulemd_module_stream_depends_on_stream (stream, "platform", "f30"));
  g_assert_true (modulemd_module_stream_build_depends_on_stream (
    stream, "platform", "f30"));

  g_assert_false (
    modulemd_module_stream_depends_on_stream (stream, "platform", "f28"));
  g_assert_false (modulemd_module_stream_build_depends_on_stream (
    stream, "platform", "f28"));

  g_assert_false (
    modulemd_module_stream_depends_on_stream (stream, "base", "f30"));
  g_assert_false (
    modulemd_module_stream_build_depends_on_stream (stream, "base", "f30"));
  g_clear_object (&stream);
}

static void
module_stream_v3_test_depends_on_stream (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *module_name = NULL;
  g_autofree gchar *module_stream = NULL;

  path = g_strdup_printf ("%s/dependson_v3.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (path);
  stream = modulemd_module_stream_read_file (
    path, TRUE, module_name, module_stream, &error);
  g_assert_nonnull (stream);

  g_assert_true (
    modulemd_module_stream_depends_on_stream (stream, "runtime", "a"));
  g_assert_true (modulemd_module_stream_build_depends_on_stream (
    stream, "buildtools", "v1"));

  g_assert_false (
    modulemd_module_stream_depends_on_stream (stream, "buildtools", "v1"));
  g_assert_false (
    modulemd_module_stream_build_depends_on_stream (stream, "runtime", "a"));

  g_assert_false (
    modulemd_module_stream_depends_on_stream (stream, "base", "f30"));
  g_assert_false (
    modulemd_module_stream_build_depends_on_stream (stream, "base", "f30"));
  g_clear_object (&stream);
}


static void
module_stream_test_validate_buildafter (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  guint64 version;

  /* buildafter is supported starting with v2 */
  for (version = MD_MODULESTREAM_VERSION_TWO;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      /* Test a valid module stream with buildafter set */
      path =
        g_strdup_printf ("%s/buildafter/good_buildafter_v%" PRIu64 ".yaml",
                         g_getenv ("TEST_DATA_PATH"),
                         version);
      g_assert_nonnull (path);
      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_nonnull (stream);
      g_assert_null (error);
      g_clear_pointer (&path, g_free);
      g_clear_object (&stream);

      /* Should fail validation if both buildorder and buildafter are set for the
       * same component.
       */
      path =
        g_strdup_printf ("%s/buildafter/both_same_component_v%" PRIu64 ".yaml",
                         g_getenv ("TEST_DATA_PATH"),
                         version);
      g_assert_nonnull (path);
      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
      g_assert_null (stream);
      g_clear_error (&error);
      g_clear_pointer (&path, g_free);

      /* Should fail validation if both buildorder and buildafter are set in
       * different components of the same stream.
       */
      path =
        g_strdup_printf ("%s/buildafter/mixed_buildorder_v%" PRIu64 ".yaml",
                         g_getenv ("TEST_DATA_PATH"),
                         version);
      g_assert_nonnull (path);
      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
      g_assert_null (stream);
      g_clear_error (&error);
      g_clear_pointer (&path, g_free);

      /* Should fail if a key specified in a buildafter set does not exist for this
       * module stream.
       */
      path = g_strdup_printf ("%s/buildafter/invalid_key_v%" PRIu64 ".yaml",
                              g_getenv ("TEST_DATA_PATH"),
                              version);
      g_assert_nonnull (path);
      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
      g_assert_null (stream);
      g_clear_error (&error);
      g_clear_pointer (&path, g_free);
    }
}


static void
module_stream_test_validate_buildarches (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  guint64 version;

  /* skipping v1 because spec does not require build arch validation */
  for (version = MD_MODULESTREAM_VERSION_TWO;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      /* Test a valid module stream with no buildopts or component
       * rpm arches set.
       */
      path =
        g_strdup_printf ("%s/buildarches/good_no_arches_v%" PRIu64 ".yaml",
                         g_getenv ("TEST_DATA_PATH"),
                         version);
      g_assert_nonnull (path);
      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_nonnull (stream);
      g_assert_null (error);
      g_clear_pointer (&path, g_free);
      g_clear_object (&stream);

      /* Test a valid module stream with buildopts arches but no component rpm
       * arches set.
       */
      path =
        g_strdup_printf ("%s/buildarches/only_module_arches_v%" PRIu64 ".yaml",
                         g_getenv ("TEST_DATA_PATH"),
                         version);
      g_assert_nonnull (path);
      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_nonnull (stream);
      g_assert_null (error);
      g_clear_pointer (&path, g_free);
      g_clear_object (&stream);

      /* Test a valid module stream with component rpm arches but no buildopts
       * arches set.
       */
      path =
        g_strdup_printf ("%s/buildarches/only_rpm_arches_v%" PRIu64 ".yaml",
                         g_getenv ("TEST_DATA_PATH"),
                         version);
      g_assert_nonnull (path);
      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_nonnull (stream);
      g_assert_null (error);
      g_clear_pointer (&path, g_free);
      g_clear_object (&stream);

      /* Test a valid module stream with buildopts arches set and a component rpm
       * specified containing a subset of archs specified at the module level.
       */
      path =
        g_strdup_printf ("%s/buildarches/good_combo_arches_v%" PRIu64 ".yaml",
                         g_getenv ("TEST_DATA_PATH"),
                         version);
      g_assert_nonnull (path);
      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_nonnull (stream);
      g_assert_null (error);
      g_clear_pointer (&path, g_free);
      g_clear_object (&stream);

      /* Should fail validation if buildopts arches is set and a component rpm
       * specified an arch not specified at the module level.
       */
      path =
        g_strdup_printf ("%s/buildarches/bad_combo_arches_v%" PRIu64 ".yaml",
                         g_getenv ("TEST_DATA_PATH"),
                         version);
      g_assert_nonnull (path);
      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_error (error, MODULEMD_ERROR, MMD_ERROR_VALIDATE);
      g_assert_null (stream);
      g_clear_error (&error);
      g_clear_pointer (&path, g_free);
    }
}


static void
module_stream_v2_test_rpm_map (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  ModulemdRpmMapEntry *retrieved_entry = NULL;

  stream = modulemd_module_stream_v2_new ("foo", "bar");
  g_assert_nonnull (stream);

  entry = modulemd_rpm_map_entry_new (
    "bar", 0, "1.23", "1.module_deadbeef", "x86_64");
  g_assert_nonnull (entry);

  modulemd_module_stream_v2_set_rpm_artifact_map_entry (
    stream, entry, "sha256", "baddad");

  retrieved_entry = modulemd_module_stream_v2_get_rpm_artifact_map_entry (
    stream, "sha256", "baddad");
  g_assert_nonnull (retrieved_entry);

  g_assert_true (modulemd_rpm_map_entry_equals (entry, retrieved_entry));
}

static void
module_stream_v3_test_rpm_map (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_autoptr (ModulemdRpmMapEntry) entry = NULL;
  ModulemdRpmMapEntry *retrieved_entry = NULL;

  stream = modulemd_module_stream_v3_new ("foo", "bar");
  g_assert_nonnull (stream);

  entry = modulemd_rpm_map_entry_new (
    "bar", 0, "1.23", "1.module_deadbeef", "x86_64");
  g_assert_nonnull (entry);

  modulemd_module_stream_v3_set_rpm_artifact_map_entry (
    stream, entry, "sha256", "baddad");

  retrieved_entry = modulemd_module_stream_v3_get_rpm_artifact_map_entry (
    stream, "sha256", "baddad");
  g_assert_nonnull (retrieved_entry);

  g_assert_true (modulemd_rpm_map_entry_equals (entry, retrieved_entry));
}


static void
module_stream_test_unicode_desc (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
      /* Test a module stream with unicode in description */
      path = g_strdup_printf ("%s/stream_unicode_v%" PRIu64 ".yaml",
                              g_getenv ("TEST_DATA_PATH"),
                              version);
      g_assert_nonnull (path);

      stream =
        modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
      g_assert_nonnull (stream);
      g_assert_no_error (error);
      g_clear_object (&stream);
      g_clear_error (&error);
      g_clear_pointer (&path, g_free);
    }
}


static void
module_stream_v1_test_xmd_issue_274 (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  GVariant *xmd1 = NULL;
  GVariant *xmd2 = NULL;

  path =
    g_strdup_printf ("%s/stream_unicode_v1.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (path);

  stream = modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
  g_assert_nonnull (stream);
  g_assert_no_error (error);
  g_assert_cmpint (modulemd_module_stream_get_mdversion (stream),
                   ==,
                   MD_MODULESTREAM_VERSION_ONE);

  xmd1 =
    modulemd_module_stream_v1_get_xmd (MODULEMD_MODULE_STREAM_V1 (stream));
  xmd2 =
    modulemd_module_stream_v1_get_xmd (MODULEMD_MODULE_STREAM_V1 (stream));

  g_assert_true (xmd1 == xmd2);
}


static void
module_stream_v2_test_xmd_issue_290 (void)
{
  g_auto (GVariantBuilder) builder;
  g_autoptr (GVariant) xmd = NULL;
  GVariant *xmd_array = NULL;
  g_autoptr (GVariantDict) xmd_dict = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdModuleIndex) index = modulemd_module_index_new ();
  g_autofree gchar *yaml_str = NULL;

  g_autoptr (ModulemdModuleStreamV2) stream =
    modulemd_module_stream_v2_new ("foo", "bar");

  modulemd_module_stream_v2_set_summary (stream, "summary");
  modulemd_module_stream_v2_set_description (stream, "desc");
  modulemd_module_stream_v2_add_module_license (stream, "MIT");


  g_variant_builder_init (&builder, G_VARIANT_TYPE_ARRAY);

  g_variant_builder_add_value (&builder, g_variant_new_string ("foo"));
  g_variant_builder_add_value (&builder, g_variant_new_string ("bar"));

  xmd_array = g_variant_builder_end (&builder);

  xmd_dict = g_variant_dict_new (NULL);
  g_variant_dict_insert_value (
    xmd_dict, "something", g_steal_pointer (&xmd_array));
  xmd = g_variant_ref_sink (g_variant_dict_end (xmd_dict));


  modulemd_module_stream_v2_set_xmd (stream, xmd);

  g_assert_true (modulemd_module_index_add_module_stream (
    index, MODULEMD_MODULE_STREAM (stream), &error));
  g_assert_no_error (error);

  yaml_str = modulemd_module_index_dump_to_string (index, &error);

  // clang-format off
  g_assert_cmpstr (yaml_str, ==,
"---\n"
"document: modulemd\n"
"version: 2\n"
"data:\n"
"  name: foo\n"
"  stream: \"bar\"\n"
"  summary: summary\n"
"  description: >-\n"
"    desc\n"
"  license:\n"
"    module:\n"
"    - MIT\n"
"  xmd:\n"
"    something:\n"
"    - foo\n"
"    - bar\n"
"...\n");
  // clang-format on

  g_assert_no_error (error);
}


static void
module_stream_v2_test_xmd_issue_290_with_example (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (ModulemdModuleIndex) index = modulemd_module_index_new ();
  g_autofree gchar *output_yaml = NULL;
  g_autoptr (GVariant) xmd = NULL;

  path = g_strdup_printf ("%s/290.yaml", g_getenv ("TEST_DATA_PATH"));
  g_assert_nonnull (path);
  stream = modulemd_module_stream_read_file (path, TRUE, NULL, NULL, &error);
  g_assert_nonnull (stream);
  g_assert_no_error (error);

  xmd = modulemd_variant_deep_copy (
    modulemd_module_stream_v1_get_xmd (MODULEMD_MODULE_STREAM_V1 (stream)));
  modulemd_module_stream_v1_set_xmd (MODULEMD_MODULE_STREAM_V1 (stream), xmd);

  g_assert_true (
    modulemd_module_index_add_module_stream (index, stream, &error));
  g_assert_no_error (error);

  output_yaml = modulemd_module_index_dump_to_string (index, &error);
  g_assert_nonnull (output_yaml);
  g_assert_no_error (error);
}


static void
module_stream_v1_test_community (void)
{
  g_autoptr (ModulemdModuleStreamV1) stream = NULL;
  const gchar *community = NULL;
  g_autofree gchar *community_prop = NULL;

  stream = modulemd_module_stream_v1_new (NULL, NULL);

  // Check the defaults
  community = modulemd_module_stream_v1_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_null (community);
  g_assert_null (community_prop);

  g_clear_pointer (&community_prop, g_free);

  // Test property setting
  g_object_set (stream, MMD_TEST_COM_PROP, MMD_TEST_DOC_TEXT, NULL);

  community = modulemd_module_stream_v1_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_cmpstr (community_prop, ==, MMD_TEST_DOC_TEXT);
  g_assert_cmpstr (community, ==, MMD_TEST_DOC_TEXT);

  g_clear_pointer (&community_prop, g_free);

  // Test set_community()
  modulemd_module_stream_v1_set_community (stream, MMD_TEST_DOC_TEXT2);

  community = modulemd_module_stream_v1_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_cmpstr (community_prop, ==, MMD_TEST_DOC_TEXT2);
  g_assert_cmpstr (community, ==, MMD_TEST_DOC_TEXT2);

  g_clear_pointer (&community_prop, g_free);

  // Test setting to NULL
  g_object_set (stream, MMD_TEST_COM_PROP, NULL, NULL);

  community = modulemd_module_stream_v1_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_null (community);
  g_assert_null (community_prop);

  g_clear_pointer (&community_prop, g_free);
  g_clear_object (&stream);
}

static void
module_stream_v2_test_community (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  const gchar *community = NULL;
  g_autofree gchar *community_prop = NULL;

  stream = modulemd_module_stream_v2_new (NULL, NULL);

  // Check the defaults
  community = modulemd_module_stream_v2_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_null (community);
  g_assert_null (community_prop);

  g_clear_pointer (&community_prop, g_free);

  // Test property setting
  g_object_set (stream, MMD_TEST_COM_PROP, MMD_TEST_DOC_TEXT, NULL);

  community = modulemd_module_stream_v2_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_cmpstr (community_prop, ==, MMD_TEST_DOC_TEXT);
  g_assert_cmpstr (community, ==, MMD_TEST_DOC_TEXT);

  g_clear_pointer (&community_prop, g_free);

  // Test set_community()
  modulemd_module_stream_v2_set_community (stream, MMD_TEST_DOC_TEXT2);

  community = modulemd_module_stream_v2_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_cmpstr (community_prop, ==, MMD_TEST_DOC_TEXT2);
  g_assert_cmpstr (community, ==, MMD_TEST_DOC_TEXT2);

  g_clear_pointer (&community_prop, g_free);

  // Test setting to NULL
  g_object_set (stream, MMD_TEST_COM_PROP, NULL, NULL);

  community = modulemd_module_stream_v2_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_null (community);
  g_assert_null (community_prop);

  g_clear_pointer (&community_prop, g_free);
  g_clear_object (&stream);
}

static void
module_stream_v3_test_community (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  const gchar *community = NULL;
  g_autofree gchar *community_prop = NULL;

  stream = modulemd_module_stream_v3_new (NULL, NULL);

  // Check the defaults
  community = modulemd_module_stream_v3_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_null (community);
  g_assert_null (community_prop);

  g_clear_pointer (&community_prop, g_free);

  // Test property setting
  g_object_set (stream, MMD_TEST_COM_PROP, MMD_TEST_DOC_TEXT, NULL);

  community = modulemd_module_stream_v3_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_cmpstr (community_prop, ==, MMD_TEST_DOC_TEXT);
  g_assert_cmpstr (community, ==, MMD_TEST_DOC_TEXT);

  g_clear_pointer (&community_prop, g_free);

  // Test set_community()
  modulemd_module_stream_v3_set_community (stream, MMD_TEST_DOC_TEXT2);

  community = modulemd_module_stream_v3_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_cmpstr (community_prop, ==, MMD_TEST_DOC_TEXT2);
  g_assert_cmpstr (community, ==, MMD_TEST_DOC_TEXT2);

  g_clear_pointer (&community_prop, g_free);

  // Test setting to NULL
  g_object_set (stream, MMD_TEST_COM_PROP, NULL, NULL);

  community = modulemd_module_stream_v3_get_community (stream);
  g_object_get (stream, MMD_TEST_COM_PROP, &community_prop, NULL);
  g_assert_null (community);
  g_assert_null (community_prop);

  g_clear_pointer (&community_prop, g_free);
  g_clear_object (&stream);
}


/*
 * This is a regression test for a memory leak that occurred when reading a
 * v1 ModuleStream YAML document if the `data.license.content` field appeared
 * before the `data.license.module` field.
 */
static void
module_stream_v1_regression_content_license (void)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (GError) error = NULL;
  const char *content_first =
    "---\n"
    "document: modulemd\n"
    "version: 1\n"
    "data:\n"
    "  summary: summary\n"
    "  description: >-\n"
    "    desc\n"
    "  license:\n"
    "    content:\n"
    "    - BSD\n"
    "    module:\n"
    "    - MIT\n"
    "  xmd:\n"
    "    something:\n"
    "    - foo\n"
    "    - bar\n"
    "...\n";

  stream = modulemd_module_stream_read_string (
    content_first, TRUE, NULL, NULL, &error);
  g_assert_no_error (error);
  g_assert_nonnull (stream);
  g_assert_true (MODULEMD_IS_MODULE_STREAM_V1 (stream));
}


static void
module_stream_v2_test_obsoletes (void)
{
  g_autoptr (ModulemdModuleStreamV2) stream = NULL;
  g_autoptr (ModulemdObsoletes) o = NULL;

  stream = modulemd_module_stream_v2_new ("foo", "latest");
  g_assert_nonnull (stream);
  o = modulemd_obsoletes_new (1, 2, "testmodule", "teststream", "testmessage");
  g_assert_nonnull (o);

  g_assert_null (modulemd_module_stream_v2_get_obsoletes_resolved (stream));

  modulemd_module_stream_v2_associate_obsoletes (stream, o);
  g_clear_object (&o);

  o = modulemd_module_stream_v2_get_obsoletes_resolved (stream);
  g_assert_nonnull (o);
  g_assert_cmpstr (modulemd_obsoletes_get_module_name (o), ==, "testmodule");
  g_assert_cmpstr (modulemd_obsoletes_get_module_stream (o), ==, "teststream");
  g_assert_null (modulemd_obsoletes_get_module_context (o));

  o = modulemd_obsoletes_new (1, 2, "testmodule", "teststream", "testmessage");
  g_assert_nonnull (o);
  modulemd_obsoletes_set_reset (o, TRUE);
  modulemd_module_stream_v2_associate_obsoletes (stream, o);
  o = modulemd_module_stream_v2_get_obsoletes_resolved (stream);
  g_assert_null (o);
  g_clear_object (&o);

  o = modulemd_module_stream_v2_get_obsoletes (stream);
  g_assert_nonnull (o);
  g_assert_cmpstr (modulemd_obsoletes_get_module_name (o), ==, "testmodule");
  g_assert_cmpstr (modulemd_obsoletes_get_module_stream (o), ==, "teststream");
  g_assert_null (modulemd_obsoletes_get_module_context (o));

  g_clear_object (&o);
  g_clear_object (&stream);
}

static void
module_stream_v3_test_obsoletes (void)
{
  g_autoptr (ModulemdModuleStreamV3) stream = NULL;
  g_autoptr (ModulemdObsoletes) o = NULL;

  stream = modulemd_module_stream_v3_new ("foo", "latest");
  g_assert_nonnull (stream);
  o = modulemd_obsoletes_new (1, 2, "testmodule", "teststream", "testmessage");
  g_assert_nonnull (o);

  g_assert_null (modulemd_module_stream_v3_get_obsoletes_resolved (stream));

  modulemd_module_stream_v3_associate_obsoletes (stream, o);
  g_clear_object (&o);

  o = modulemd_module_stream_v3_get_obsoletes_resolved (stream);
  g_assert_nonnull (o);
  g_assert_cmpstr (modulemd_obsoletes_get_module_name (o), ==, "testmodule");
  g_assert_cmpstr (modulemd_obsoletes_get_module_stream (o), ==, "teststream");
  g_assert_null (modulemd_obsoletes_get_module_context (o));

  o = modulemd_obsoletes_new (1, 2, "testmodule", "teststream", "testmessage");
  g_assert_nonnull (o);
  modulemd_obsoletes_set_reset (o, TRUE);
  modulemd_module_stream_v3_associate_obsoletes (stream, o);
  o = modulemd_module_stream_v3_get_obsoletes_resolved (stream);
  g_assert_null (o);
  g_clear_object (&o);

  o = modulemd_module_stream_v3_get_obsoletes (stream);
  g_assert_nonnull (o);
  g_assert_cmpstr (modulemd_obsoletes_get_module_name (o), ==, "testmodule");
  g_assert_cmpstr (modulemd_obsoletes_get_module_stream (o), ==, "teststream");
  g_assert_null (modulemd_obsoletes_get_module_context (o));

  g_clear_object (&o);
  g_clear_object (&stream);
}


int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.o

  g_test_add_func ("/modulemd/v2/modulestream/construct",
                   module_stream_test_construct);

  g_test_add_func ("/modulemd/v2/modulestream/arch", module_stream_test_arch);

  g_test_add_func ("/modulemd/v2/modulestream/v1/documentation",
                   module_stream_v1_test_documentation);

  g_test_add_func ("/modulemd/v2/modulestream/v2/documentation",
                   module_stream_v2_test_documentation);

  g_test_add_func ("/modulemd/v2/modulestream/v3/documentation",
                   module_stream_v3_test_documentation);

  g_test_add_func ("/modulemd/v2/modulestream/v1/summary",
                   module_stream_v1_test_summary);

  g_test_add_func ("/modulemd/v2/modulestream/v2/summary",
                   module_stream_v2_test_summary);

  g_test_add_func ("/modulemd/v2/modulestream/v3/summary",
                   module_stream_v3_test_summary);

  g_test_add_func ("/modulemd/v2/modulestream/v1/description",
                   module_stream_v1_test_description);

  g_test_add_func ("/modulemd/v2/modulestream/v2/description",
                   module_stream_v2_test_description);

  g_test_add_func ("/modulemd/v2/modulestream/v3/description",
                   module_stream_v3_test_description);

  g_test_add_func ("/modulemd/v2/modulestream/v1/licenses",
                   module_stream_v1_test_licenses);

  g_test_add_func ("/modulemd/v2/modulestream/v2/licenses",
                   module_stream_v2_test_licenses);

  g_test_add_func ("/modulemd/v2/modulestream/v3/licenses",
                   module_stream_v3_test_licenses);

  g_test_add_func ("/modulemd/v2/modulestream/v1/tracker",
                   module_stream_v1_test_tracker);

  g_test_add_func ("/modulemd/v2/modulestream/v2/tracker",
                   module_stream_v2_test_tracker);

  g_test_add_func ("/modulemd/v2/modulestream/v3/tracker",
                   module_stream_v3_test_tracker);

  g_test_add_func ("/modulemd/v2/modulestream/v1/profiles",
                   module_stream_v1_test_profiles);

  g_test_add_func ("/modulemd/v2/modulestream/v2/profiles",
                   module_stream_v2_test_profiles);

  g_test_add_func ("/modulemd/v2/modulestream/v3/profiles",
                   module_stream_v3_test_profiles);

  g_test_add_func ("/modulemd/v2/modulestream/v1/rpm_api",
                   module_stream_v1_test_rpm_api);

  g_test_add_func ("/modulemd/v2/modulestream/v2/rpm_api",
                   module_stream_v2_test_rpm_api);

  g_test_add_func ("/modulemd/v2/modulestream/v3/rpm_api",
                   module_stream_v3_test_rpm_api);

  g_test_add_func ("/modulemd/v2/modulestream/v1/rpm_filters",
                   module_stream_v1_test_rpm_filters);

  g_test_add_func ("/modulemd/v2/modulestream/v2/rpm_filters",
                   module_stream_v2_test_rpm_filters);

  g_test_add_func ("/modulemd/v2/modulestream/v3/rpm_filters",
                   module_stream_v3_test_rpm_filters);

  g_test_add_func ("/modulemd/v2/modulestream/v2_yaml",
                   module_stream_test_v2_yaml);

  g_test_add_func ("/modulemd/v2/modulestream/v3_yaml",
                   module_stream_test_v3_yaml);

  g_test_add_func ("/modulemd/v2/packager/v2_sanity",
                   module_packager_v2_sanity);

  g_test_add_func ("/modulemd/v2/packager/v3_sanity",
                   module_packager_v3_sanity);

  g_test_add_func ("/modulemd/v2/modulestream/upgrade_v1_to_v2",
                   module_stream_test_upgrade_v1_to_v2);

  g_test_add_func ("/modulemd/v2/modulestream/upgrade_v2_to_v3",
                   module_stream_test_upgrade_v2_to_v3);

  g_test_add_func ("/modulemd/v2/modulestream/upgrade_v1_to_v3",
                   module_stream_test_upgrade_v1_to_v3);

  g_test_add_func ("/modulemd/v2/modulestream/stream_expansion_v2_to_v3",
                   module_stream_test_stream_deps_expansion_v2_to_v3);

  g_test_add_func (
    "/modulemd/v2/modulestream/stream_expansion_v2_to_v3/bad/no_streams",
    module_stream_test_stream_deps_expansion_v2_to_v3_no_streams);

  g_test_add_func (
    "/modulemd/v2/modulestream/stream_expansion_v2_to_v3/bad/exclusions",
    module_stream_test_stream_deps_expansion_v2_to_v3_exclusions);

  g_test_add_func (
    "/modulemd/v2/modulestream/stream_expansion_v2_to_v3/bad/no_platform",
    module_stream_test_stream_deps_expansion_v2_to_v3_no_platform);

  g_test_add_func (
    "/modulemd/v2/modulestream/stream_expansion_v2_to_v3/bad/"
    "conflicting_platforms",
    module_stream_test_stream_deps_expansion_v2_to_v3_conflicting_platforms);

  g_test_add_func ("/modulemd/v2/modulestream/v1/rpm_artifacts",
                   module_stream_v1_test_rpm_artifacts);

  g_test_add_func ("/modulemd/v2/modulestream/v2/rpm_artifacts",
                   module_stream_v2_test_rpm_artifacts);

  g_test_add_func ("/modulemd/v2/modulestream/v3/rpm_artifacts",
                   module_stream_v3_test_rpm_artifacts);

  g_test_add_func ("/modulemd/v2/modulestream/v1/servicelevels",
                   module_stream_v2_test_servicelevels);

  g_test_add_func ("/modulemd/v2/modulestream/v2/servicelevels",
                   module_stream_v1_test_servicelevels);

  g_test_add_func ("/modulemd/v2/modulestream/v1/components",
                   module_stream_v1_test_components);

  g_test_add_func ("/modulemd/v2/modulestream/v2/components",
                   module_stream_v2_test_components);

  g_test_add_func ("/modulemd/v2/modulestream/v3/components",
                   module_stream_v3_test_components);

  g_test_add_func ("/modulemd/v2/modulestream/copy", module_stream_test_copy);

  g_test_add_func ("/modulemd/v2/modulestream/equals",
                   module_stream_test_equals);

  g_test_add_func ("/modulemd/v2/modulestream/nsvc", module_stream_test_nsvc);


  g_test_add_func ("/modulemd/v2/modulestream/nsvca",
                   module_stream_test_nsvca);

  g_test_add_func ("/modulemd/v2/modulestream/v1/equals",
                   module_stream_v1_test_equals);

  g_test_add_func ("/modulemd/v2/modulestream/v2/equals",
                   module_stream_v2_test_equals);

  g_test_add_func ("/modulemd/v2/modulestream/v3/equals",
                   module_stream_v3_test_equals);

  g_test_add_func ("/modulemd/v2/modulestream/v1/dependencies",
                   module_stream_v1_test_dependencies);

  g_test_add_func ("/modulemd/v2/modulestream/v2/dependencies",
                   module_stream_v2_test_dependencies);

  g_test_add_func ("/modulemd/v2/modulestream/v3/dependencies",
                   module_stream_v3_test_dependencies);

  g_test_add_func ("/modulemd/v2/modulestream/v1/parse_dump",
                   module_stream_v1_test_parse_dump);

  g_test_add_func ("/modulemd/v2/modulestream/v2/parse_dump",
                   module_stream_v2_test_parse_dump);

  g_test_add_func ("/modulemd/v2/modulestream/v3/parse_dump",
                   module_stream_v3_test_parse_dump);

  g_test_add_func ("/modulemd/v2/modulestream/v1/depends_on_stream",
                   module_stream_v1_test_depends_on_stream);

  g_test_add_func ("/modulemd/v2/modulestream/v2/depends_on_stream",
                   module_stream_v2_test_depends_on_stream);

  g_test_add_func ("/modulemd/v2/modulestream/v3/depends_on_stream",
                   module_stream_v3_test_depends_on_stream);

  g_test_add_func ("/modulemd/v2/modulestream/validate/buildafter",
                   module_stream_test_validate_buildafter);

  g_test_add_func ("/modulemd/v2/modulestream/validate/buildarches",
                   module_stream_test_validate_buildarches);

  g_test_add_func ("/modulemd/v2/modulestream/v2/rpm_map",
                   module_stream_v2_test_rpm_map);

  g_test_add_func ("/modulemd/v2/modulestream/v3/rpm_map",
                   module_stream_v3_test_rpm_map);

  g_test_add_func ("/modulemd/v2/modulestream/v1/community",
                   module_stream_v1_test_community);

  g_test_add_func ("/modulemd/v2/modulestream/v2/community",
                   module_stream_v2_test_community);

  g_test_add_func ("/modulemd/v2/modulestream/v3/community",
                   module_stream_v3_test_community);

  g_test_add_func ("/modulemd/v2/modulestream/unicode/description",
                   module_stream_test_unicode_desc);

  g_test_add_func ("/modulemd/v2/modulestream/v1/xmd/issue274",
                   module_stream_v1_test_xmd_issue_274);

  g_test_add_func ("/modulemd/v2/modulestream/v2/xmd/issue290",
                   module_stream_v2_test_xmd_issue_290);

  g_test_add_func ("/modulemd/v2/modulestream/v2/xmd/issue290plus",
                   module_stream_v2_test_xmd_issue_290_with_example);

  g_test_add_func ("/modulemd/v2/modulestream/regression/memleak/v1_licenses",
                   module_stream_v1_regression_content_license);

  g_test_add_func ("/modulemd/v2/modulestream/v2/obsoletes",
                   module_stream_v2_test_obsoletes);

  g_test_add_func ("/modulemd/v2/modulestream/v3/obsoletes",
                   module_stream_v3_test_obsoletes);

  return g_test_run ();
}
