#include <glib.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <signal.h>

#include "modulemd-module-stream.h"
#include "private/glib-extensions.h"
#include "private/modulemd-module-stream-private.h"
#include "private/modulemd-module-stream-v1-private.h"
#include "private/modulemd-module-stream-v2-private.h"
#include "private/modulemd-subdocument-info-private.h"
#include "private/modulemd-yaml.h"
#include "private/test-utils.h"

typedef struct _ModuleStreamFixture
{
} ModuleStreamFixture;


static void
module_stream_test_construct (ModuleStreamFixture *fixture,
                              gconstpointer user_data)
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
      
      /*Test with no module name*/
      stream=modulemd_module_stream_new(version,NULL,'latest')
      g_assert_nonnull(stream);
      g_assert_true(MODULEMD_IS_MODULE_STREAM(stream));
          
      g_assert_cmpint (
        modulemd_module_stream_get_mdversion (stream), ==, version);
      g_assert_cmpstr (
        modulemd_module_stream_get_module_name (stream));
      g_assert_null (modulemd_module_stream_get_stream_name (stream,==,'latest'));

      g_clear_object (&stream);


      /* Test with no module or stream name */
      stream = modulemd_module_stream_new (version, NULL, NULL);
      g_assert_nonnull (stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (stream));

      g_assert_cmpint (
        modulemd_module_stream_get_mdversion (stream), ==, version);
      g_assert_null (modulemd_module_stream_get_module_name (stream));
      g_assert_null (modulemd_module_stream_get_stream_name (stream));

      g_clear_object (&stream);
    
      /* Test  */
      
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
module_stream_test_arch (ModuleStreamFixture *fixture, gconstpointer user_data)
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
module_stream_test_copy (ModuleStreamFixture *fixture, gconstpointer user_data)
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
      
      /* Test that copying a string and changing the string works */
      stream = modulemd_module_stream_new (version, "foo", "stable");
      copied_stream = modulemd_module_stream_copy (stream, NULL, "latest");
      g_assert_nonnull (copied_stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_mdversion(stream),
                       ==,
                       modulemd_module_stream_get_mdversion (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_stream_name(stream),
                       ==,
                       "stable");
      g_assert_cmpstr (modulemd_module_stream_get_stream_name (copied_stream),
                       ==,
                       "latest");
      g_clear_object (&stream);
      g_clear_object (&copied_stream);
    
      /* Test that copying a stream without a module name works */
      stream = modulemd_module_stream_new (version, None,"stable");
      copied_stream = modulemd_module_stream_copy (stream, NULL, NULL);
      g_assert_nonnull (copied_stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_module_name (stream),
                       ==,
                       modulemd_module_stream_get_module_name (copied_stream));
      g_clear_object (&stream);
      g_clear_object (&copied_stream);
    
      /* Test that copying a stream and changing the module name works*/
      stream = modulemd_module_stream_new (version, "foo", "stable");
      copied_stream = modulemd_module_stream_copy (stream, "bar", NULL);
      g_assert_nonnull (copied_stream);
      g_assert_true (MODULEMD_IS_MODULE_STREAM (copied_stream));
      g_assert_cmpstr (modulemd_module_stream_get_module_name(stream),
                       ==,
                       "foo");
      g_assert_cmpstr (modulemd_module_stream_get_module_name (copied_stream),
                       ==,
                       "bar");
      g_clear_object (&stream);
      g_clear_object (&copied_stream);
    
      /* Test the version and context */
      stream = modulemd_module_stream_new(version,"foo","latest");
      copied_stream = modulemd_module_stream_copy(0,None,NULL);
      g_assert_cmpint (modulemd_module_stream_get_mdversion (stream), 
                       ==, 
                       modulemd_module_stream_get_mdversion(copied_stream));
      g_assert_cmpstr(modulemd_module_stream_get_module_name(stream),
                      ==,
                      "foo");
      g_assert_null(modulemd_module_stream_get_module_name(copied_stream));
      g_clear_object (&stream);
      g_clear_object (&copied_stream);
      
    /* Set a version and check the copy */
      stream = modulemd_module_stream_new(version,"foo","latest");
      copied_stream = modulemd_module_stream_copy(42,NULL,NULL);
      g_assert_cmpint(modulemd_module_stream_get_mdversion(stream),
                      ==,
                      version);
      g_assert_cmpint(modulemd_module_stream_get_mdversion(copied_stream),
                      ==,
                      42);
      g_clear_object (&stream);
      g_clear_object (&copied_stream);
    
    /* Set the context and check the copy */
      stream = modulemd_module_stream_new(stream,"foo","latest");
      copied_stream = modulemd_module_stream_copy(version,"cOffee42",NULL);
      g_asser_cmpstr(modulemd_module_stream_get_module_name(stream),
                     ==,
                     "foo");
      g_assert_cmpstr(modulemd_module_stream_get_module_name(copied_stream),
                      ==,
                      "cOffee42");
      g_clear_object (&stream);
      g_clear_object (&copied_stream);
    }
}

static void
module_stream_test_nsvc (ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdModuleStream) copied_stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
  {
      /*First test that NSVC is None for a module with no name*/
      stream=modulemd_module_stream_new(version,None,None);
      g_assert_null(modulemd_module_stream_get_NSVC(stream));
      g_clear_object (&stream);
    
     /*Next, test for no stream name*/
     stream = modulemd_module_stream_new(version,"modulename",NULL);
     g_assert_null(modulemd_module_stream_get_NSVC(stream));
    
     /*Now with valid module and stream names*/
     stream=modulemd_module_stream_new(version,"modulename","streamname")
     g_assert_cmpstr(modulemd_module_stream_get_NSVC(stream),
                     ==,
                     'modulename:streamname:0');
    
     /*Add a version number */
     stream = modulemd_module_stream_new(42,NULL,NULL)
     g_assert_addint(modulemd_module_stream_get_NSVC(stream),
                     ==,
                     'modulename:streamname:42');
    
     /* Add a context */
     stream = modulemd_module_stream_new (42,"modulename","streamname")
     g_assert_addstr(modulemd_module_stream_get_NSCV(stream),
                     ==,
                     'modulename:streamname:42:deadbeef')
  } 
}
static void
module_stream_test_nsvca (ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autoptr (ModulemdModuleStream) copied_stream = NULL;
  guint64 version;
 

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
  {
      /*First test that NSVCA is None for a module with no name*/
      stream = modulemd_module_stream_new (version,None,None);
      g_assert_cmpstr (modulemd_module_stream_get_NSVCA(stream));
      g_clear_object (&stream);
    
     /*Next, test for no stream name*/
     stream = modulemd_module_stream_new (version,"modulename",NULL);
     g_assert_Equal(modulemd_module_stream_get_NSVCA(stream),
                     =,
                     "modulename");
    
     /*Now with valid module and stream names*/
     stream=modulemd_module_stream_new(version,"modulename","streamname")
     g_assert_Equal(modulemd_module_stream_get_NSVCA(stream),
                     =,
                     'modulename:streamname');
    
     /*Add a version number */
     stream = modulemd_module_stream_new(42,NULL,NULL)
     g_assert_addint(modulemd_module_stream_get_NSVCA(stream),
                     ==,
                     'modulename:streamname:42');
     
     /* Add a context */
     stream = modulemd_module_stream_new (42,"modulename","streamname")
     g_assert_addstr(modulemd_module_stream_get_NSCV(stream),
                     ==,
                     'modulename:streamname:42:deadbeef')
     
     /*Add an architecture*/
     stream = modulemd_module_stream_new (version, "foo", "latest");
     g_assert_nonnull (stream);
     g_assert_nonnull (modulemd_module_stream_get_arch(stream),
                      =,
                      'x86_64');
     g_assert_nonnull (modulemd_module_stream_get_NSVCA(stream),
                      =,
                      'modulename:streamname:42:deadbeef:x86_64');
    
     /* Now try removing some of the bits in the middle */
     g_assert_Equal(modulemd_module_stream_get_NSVCA,
                    =,
                   'modulename:streamname:42::x86_64')
     stream = modulemd_module_stream_new (version, "modulename", NULL);
     g_assert_nonnull (modulemd_module_stream_get_arch(stream),
                      =,
                      'x86_64');
     g_assert_nonnull (modulemd_module_stream_get_NSVCA(stream),
                      =,
                      'modulename::::x86_64');
     g_assert_nonnull (modulemd_module_stream_mdversion(stream),
                      =,
                      '2019');
     g_assert_nonnull (modulemd_module_stream_get_NSVCA(stream),
                       =,
                       'modulename::2019::x86_64');
    
    /* Add a context */
    g_assert_Equal(modulemd_module_stream_get_NSVCA(stream),
                  =,
                  'modulename::2019:feedfeed:x86_64');
  } 
}

static void
module_stream_test_arch (ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
    stream=modulemd_module_stream_new (version, "foo", "latest");
    
    /*Check the defaults*/
    g_assert_null (modulemd_module_stream_get_arch(stream));
    /*Test property settings*/
    g_assert_cmpstr (modulemd_module_stream_get_arch(stream),
                      ==,
                     'x86_64');
    /* Test set_arch */
    g_assert_cmpstr (modulemd_module_stream_set_arch(stream),
                      ==,
                     'ppc64le');
    /* Test setting it to None */
    g_assert_null (modulemd_module_stream_set_arch(stream));
  }
} 
static void
module_stream_test_buildopts(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
    stream=modulemd_module_stream_new (version, "foo", "latest");
    
    /*Check the defaults*/
    g_assert_null (modulemd_module_stream_get_buildopts(stream));
    buildopts=Modulemd_Buildopts();
    g_assert_nonnull (modulemd_buildopts_get_rpm_macros(buildopts),
                      =,
                     '%demomacro 1');
    g_assert_nonnull(modulemd_buildtops_set_rpm_macros(buildtops))
    g_assert_nonnull (modulemd_module_stream_get_buildopts(stream),
                      !=,
                      None);
    g_assert_cmpstr (modulemd_buildopts_get_rpm_macros(buildopts),
                      ==,
                      '%demomacro 1');
   } 
}

static void
module_stream_test_community(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
    stream=modulemd_module_stream_new (version, "foo", "latest");
    
    /*Check the defaults*/
    g_assert_null (modulemd_module_stream_get_community(stream));
    /*Test property settings*/
    g_assert_cmpstr (modulemd_module_stream_get_community(stream),
                      ==,
                     'http://example.com');
    /* Test set_community*/
    g_assert_cmpstr (modulemd_module_stream_set_community(stream),
                      ==,
                     'http://redhat.com');
    /* Test setting it to None */
    g_assert_null (modulemd_module_stream_get_community(stream));
   } 
}

static void
module_stream_test_description(moduleStreamFixture*fixture,gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
    stream=modulemd_module_stream_new (version, "foo", "latest");
    
    /*Check the defaults*/
    g_assert_null (modulemd_module_stream_get_description(stream,locale="C"));
   
    /* Test set_description*/
    g_assert_nonnull (modulemd_module_stream_set_description(stream),
                      =,
                     'A Different description');
    g_assert_cmpstr(modulemd_module_stream_get_description(stream,locale="C"),
                     ==,
                     'A Different description');
                     
    /* Test setting it to None */
    g_assert_null (modulemd_module_stream_set_description(stream));
    g_assert_null (modulemd_module_stream_get_description(stream,locale="C"));
   } 
}

static void
module_stream_test_documentation(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
    stream=modulemd_module_stream_new (version, "foo", "latest");
    
    /*Check the defaults*/
    g_assert_null (modulemd_module_stream_get_documentation(stream));
    /*Test property settings*/
    g_assert_nonnull (modulemd_module_stream_set_documentation(stream),
                      =,
                     'http://example.com');
    g_assert_cmpstr (modulemd_module_stream_get_documentation(stream),
                      ==,
                      'http://example.com');
    /* Test set_documentation*/
    g_assert_Equal (modulemd_module_stream_set_documentation(stream),
                      =,
                     'http://redhat.com');
    
    g_assert_cmpstr (modulemd_module_stream_get_documentation(stream),
                      ==,
                     'http://redhat.com');
    /* Test setting it to None */
    g_assert_null (modulemd_module_stream_get_documentation(stream));
   } 
}

static void
module_stream_test_summary(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
    stream=modulemd_module_stream_new (version, "foo", "latest");
    
    /*Check the defaults*/
    g_assert_null (modulemd_module_stream_get_summary(stream,locale="C"));
    
    /* Test set_summary()*/
    g_assert_nonnull (modulemd_module_stream_set_summary(stream),
                      =,
                     'A different summary');
    
    g_assert_cmpstr (modulemd_module_stream_get_summary(stream),
                      ==,
                     'A different summary');
    /* Test setting it to None */
    g_assert_null (modulemd_module_stream_set_summary(stream));
    g_assert_null (modulemd_module_stream_get_summary(stream,locale="C"));
   } 
}

static void
module_stream_test_tracker(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
    stream=modulemd_module_stream_new (version, "foo", "latest");
    
    /*Check the defaults*/
    g_assert_null (modulemd_module_stream_get_tracker(stream));
    /*Test property settings*/
    g_assert_Equal (modulemd_module_stream_set_tracker(stream),
                      =,
                     'http://example.com');
    g_assert_cmpstr (modulemd_module_stream_get_tracker(stream),
                      ==,
                      'http://example.com');
    /* Test set_tracker*/
    g_assert_Equal (modulemd_module_stream_set_tracker(stream),
                      =,
                     'http://redhat.com');
    
    g_assert_cmpstr (modulemd_module_stream_get_tracker(stream),
                      ==,
                     'http://redhat.com');
    /* Test setting it to None */
    g_assert_null (modulemd_module_stream_get_tracker(stream));
   } 
}

static void
module_stream_test_components(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
    stream=modulemd_module_stream_new (version, "foo", "latest");
    /* Add an RPM Component to a stream */
    rpm_comp = modulemd_component_rpm_new('rpmcomponent');
    stream=modulemd_module_stream_add_component(rpm_comp);
    g_assert_nonnull(modulemd_module_get_rpm_components_name(stream),
                     =
                     'rpmcomponent');
    retrieved_comp = modulemd_module_get_rpm_component(stream,'rpmcomponent');
    g_assert_nonnull(retrieved_comp);
    g_assert_cmpstr(modulemd_module_get_module_name(retrieved_comp),
                    ==,
                    'rpmcomponent');
    /* Add a jmodule component to a stream */
    mod_comp=modulemd_component_rpm_new('modulecomponent');
    stream=modulemd_module_stream_add_component(mod_comp);
    g_assert_nonnull(modulemd_module_get_module_component_name(stream),
                     =
                     'modulecomponent');
    retrieved_comp = modulemd_module_get_module_component(stream,'modulecomponent');
    g_assert_nonnull(retrieved_comp);
    g_assert_cmpstr(modulemd_module_get_module_name(retrieved_comp),
                    ==,
                    'modulecomponent');
    
    /* Remove an rpm component from a stream */
    stream=modulemd_module_stream_remove_rpm_component('rpmcomponent');
    
    /* Remove a module component from a stream*/
    stream = modulemd_module_stream_remove_module_component('modulecomponent');
  }
} 
static void
module_stream_test_licenses(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
    {
    stream=modulemd_module_stream_new (version, "foo", "latest");
    
    stream=modulemd_module_add_content_license('GPLv2+');
    g_assert_cmpstr(modulemd_module_get_content_license(stream),
                    ==,
                    'GPLv2+');
    stream=modulemd_module_add_module_license('MIT');
    g_assert_cmpstr(modulemd_module_get_module_license(stream),
                    ==,
                    'MIT');
    stream=modulemd_module_stream_remove_content_license('GPLv2+');
    stream=modulemd_module_stream_remove_module_license('MIT');
  }
}     

static void
module_stream_test_profiles(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
  {
    stream=modulemd_module_stream_new (version, 'sssd');
    profile = modulemd_profile_new('client');
    profile = modulemd_profile_add_rpm('sssd-client');

    stream=modulemd_profile_copy(profile);
    g_assert_cmpint(len(modulemd_get_profile_names(stream)),
                 ==,
                 1);
    g_assert_cmpstr(modulemd_get_profile_names(stream),
                    ==,
                    'client');
    
    g_assert_cmpstr(modulemd_profile_get_rpms(stream),
                    ==,
                    'sssd-client');
    stream = modulemd_clear_profiles();
    g_assert_cmpint(len(modulemd_get_profile_names(stream)),
                    ==,
                    0);
  }
} 


static void
module_stream_test_rpm_api(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
  {
    stream=modulemd_module_stream_new (version, 'sssd');
    stream=modulemd_profile_add_rpm_api('sssd-common');
    g_assert_cmpstr(modulemd_profile_get_rpm_api(stream),
                    ==,
                    'sssd-common');
    stream=modulemd_profile_remove_rpm_api('sssd-common');
    g_assert_cmpstr(len(modulemd_profile_get_rpm_api(stream)),
                    ==,
                    0);
  }
}

static void
module_stream_test_rpm_artifacts(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
  {
    stream=modulemd_module_stream_new (version);
    stream=modulemd_profile_add_rpm_artifact('bar-0:1.23-1.module_deadbeef.x86_64');
    g_assert_cmpstr(modulemd_profile_get_rpm_artifacts(stream),
                    ==,
                    'bar-0:1.23-1.module_deadbeef.x86_64');
    stream=modulemd_profile_remove_rpm_artifact('bar-0:1.23-1.module_deadbeef.x86_64');
    g_assert_cmpstr(len(modulemd_profile_get_rpm_artifacts(stream)),
                    ==,
                    0);
  }
}

static void
module_stream_test_rpm_filters(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
  {
    stream=modulemd_module_stream_new (version);
    stream=modulemd_profile_add_rpm_filter('bar');
    g_assert_cmpstr(modulemd_profile_get_rpm_filters(stream),
                    ==,
                    'bar');
    stream=modulemd_profile_remove_rpm_filter('bar');
    g_assert_cmpstr(len(modulemd_profile_get_rpm_filters(stream)),
                    ==,
                    0);
  }
}

static void
module_stream_test_servicelevels(ModuleStreamFixture *fixture, gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  guint64 version;

  for (version = MD_MODULESTREAM_VERSION_ONE;
       version <= MD_MODULESTREAM_VERSION_LATEST;
       version++)
  {
    stream=modulemd_module_stream_new (version);
    s1=modulemd_service_level_new(version);
    s1=modulemd_service_level_set_eol_ymd(1980, 3, 2);
    
    stream=modulemd_module_add_service_level(s1);
    
    g_assert_cmpstr(modulemd_get_service_level_names(stream),
                    ==,
                    'rawhide');
    retrieved_s1=modulemd_module_stream_get_service_level('rawhide');
    g_assert_nonnull(modulemd_service_level_get_name(retrieved_s1),
                     ==,
                     'rawhide');
    g_assert_nonnull(modulemd_service_level_get_eol_as_string(retrieved_s1),
                     ==,
                     '1980-03-02');
  }
}

static void
module_stream_test_v1_eol(ModuleStreamFixture *fixture, gconstpointer user_data)
{
 
    stream=modulemd_module_stream_v1_new ();
    eol=GLib_Date_new_dmy(3, 2, 1998);
    stream=modulemd_module_stream_v1_set_eol(eol);
    retrieved_eol=modulemd_module_stream_v1_get_eol(stream);
    g_assert_cmpint(modulemd_module_stream_v1_get_day(retrieved_eol),
                   ==,
                   3);
    g_assert_cmpint(modulemd_module_stream_v1_get_month(retrieved_eol),
                   ==,
                   2);
    g_assert_cmpint(modulemd_module_stream_v1_get_year(retrieved_eol),
                   ==,
                   1998);
    
    s1=modulemd_module_get_servicelevel(stream,'rawhide');
    g_assert_cmpstr(modulemd_module_stream_v1_get_eol_as_string(s1),
                    ==,
                    '1998-02-03');
 }
  
static void
module_stream_test_v1_dependencies(ModuleStreamFixture *fixture, gconstpointer user_data)
{
 
    stream=modulemd_module_stream_v1_new ();
    stream=modulemd_module_stream_v1_add_buildtime_requirement ('testmodule', 'stable');
    g_assert_cmpint(len(modulemd_module_stream_v1_get_buildtime_modules(stream)),
                   ==,
                   1);
    g_assert_cmpstr(modulemd_module_stream_v1_get_buildtime_modules(stream),
                  ==,
                  'testmodule');
    g_assert_cmpstr(modulemd_module_stream_v1_get_buildtime_requirement_stream('testmodule'),
                  ==,
                  'stable');
    stream = modulemd_module_stream_v1_add_runtime_requirement('testmodule', 'latest');
    g_assert_cmpint (len(modulemd_module_stream_v1_get_runtime_modules(stream)),
                     == ,
                     1);
    g_assert_cmpstr(modulemd_module_stream_v1_get_runtime_modules(stream),
                     ==,
                     'testmodule');
    g_assert_cmpstr(modulemd_module_stream_v1_get_runtime_requirement_stream('testmodule'),
                    == ,
                    'latest');
}

static void
module_stream_test_v2_dependencies(ModuleStreamFixture *fixture, gconstpointer user_data)
{
 
    stream=modulemd_module_stream_v2_new ();
    deps=modulemd_module_stream_v2_get_dependencies();
    deps=modulemd_module_stream_v2_add_buildtime_stream ('foo', 'stable');
    deps=modulemd_module_stream_v2_set_empty_runtime_dependencies_for_module('bar');
    stream=modulemd_module_stream_v2_add_dependencies(deps);
    g_assert_cmpint(len(modulemd_module_stream_v2_get_dependencies(stream)),
                   ==,
                   1)
    g_assert_cmpstr(modulemd_module_stream_v2_get_dependencies_get_buildtime_modules(stream),
                    ==,
                    'foo');
    g_assert_cmpstr(modulemd_module_stream_v2_get_dependencies_get_buildtime_modules('foo'),
                     ==,
                     'stable');
    g_assert_cmpstr(modulemd_module_stream_v2_get_dependencies_get_runtime_modules(stream),
                     ==,
                     'bar');
}
static void
module_stream_v1_test_parse_dump (ModuleStreamFixture *fixture,
                                  gconstpointer user_data)
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
  yaml_path =
    g_strdup_printf ("%s/spec.v1.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rb");
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
    "  stream: stream-name\n"
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
module_stream_v2_test_parse_dump (ModuleStreamFixture *fixture,
                                  gconstpointer user_data)
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
  yaml_path =
    g_strdup_printf ("%s/spec.v2.yaml", g_getenv ("MESON_SOURCE_ROOT"));
  g_assert_nonnull (yaml_path);

  yaml_stream = g_fopen (yaml_path, "rb");
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

  stream = modulemd_module_stream_v2_parse_yaml (subdoc, TRUE, &error);
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
    "  stream: latest\n"
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
    "      whitelist:\n"
    "      - fooscl-1-bar\n"
    "      - fooscl-1-baz\n"
    "      - xxx\n"
    "      - xyz\n"
    "  components:\n"
    "    rpms:\n"
    "      bar:\n"
    "        rationale: We need this to demonstrate stuff.\n"
    "        name: bar-real\n"
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
module_stream_v1_test_depends_on_stream (ModuleStreamFixture *fixture,
                                         gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *module_name = NULL;
  g_autofree gchar *module_stream = NULL;

  path = g_strdup_printf ("%s/modulemd/v2/tests/test_data/dependson_v1.yaml",
                          g_getenv ("MESON_SOURCE_ROOT"));
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
module_stream_v2_test_depends_on_stream (ModuleStreamFixture *fixture,
                                         gconstpointer user_data)
{
  g_autoptr (ModulemdModuleStream) stream = NULL;
  g_autofree gchar *path = NULL;
  g_autoptr (GError) error = NULL;
  g_autofree gchar *module_name = NULL;
  g_autofree gchar *module_stream = NULL;

  path = g_strdup_printf ("%s/modulemd/v2/tests/test_data/dependson_v2.yaml",
                          g_getenv ("MESON_SOURCE_ROOT"));
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

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  g_test_init (&argc, &argv, NULL);
  g_test_bug_base ("https://bugzilla.redhat.com/show_bug.cgi?id=");

  // Define the tests.o

  g_test_add ("/modulemd/v2/modulestream/construct",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_test_construct,
              NULL);

  g_test_add ("/modulemd/v2/modulestream/arch",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_test_arch,
              NULL);

  g_test_add ("/modulemd/v2/modulestream/copy",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_test_copy,
              NULL);

  g_test_add ("/modulemd/v2/modulestream/v1/parse_dump",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_v1_test_parse_dump,
              NULL);

  g_test_add ("/modulemd/v2/modulestream/v2/parse_dump",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_v2_test_parse_dump,
              NULL);

  g_test_add ("/modulemd/v2/modulestream/v1/depends_on_stream",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_v1_test_depends_on_stream,
              NULL);
  g_test_add ("/modulemd/v2/modulestream/v2/depends_on_stream",
              ModuleStreamFixture,
              NULL,
              NULL,
              module_stream_v2_test_depends_on_stream,
              NULL);


  return g_test_run ();
}
