![Continuous Integration](https://github.com/fedora-modularity/libmodulemd/workflows/Continuous%20Integration/badge.svg)
[![Travis](https://img.shields.io/coverity/scan/13739.svg?style=plastic)](https://scan.coverity.com/projects/sgallagher-libmodulemd)

# libmodulemd
C Library for manipulating module metadata files

Full details can be found in the
[API Documentation](https://fedora-modularity.github.io/libmodulemd/latest/)

# Using libmodulemd from Python
Using libmodulemd from Python is possible thanks to the gobject-introspection
project. To use libmodulemd from Python, include the following at the top of
your sources.

Install the `python2-libmodulemd` or `python3-libmodulemd` package for your
system depending on the version of python with which you are working.
These packages provide the appropriate language bindings.

```python
import gi
gi.require_version("Modulemd", "2.0")
from gi.repository import Modulemd
```

# Working with repodata (DNF use-case)
The libmodulemd API provides a number of convenience tools for interacting
with repodata (that is, streams of YAML that contains information on multiple
streams, default data and translations). The documentation will use two
repositories, called "fedora" and "updates" for demonstrative purposes. It
will assume that the content of the YAML module metadata from those two
repositories have been loaded into string variables `fedora_yaml` and
`updates_yaml`, respectively.

Tools such as DNF that are consuming data from a repository should always
set `strict=False`, so that it can safely handle minor,
backwards-compatible changes to the modulemd format.

First step is to load the metadata from these two repositories into
ModulemdModuleIndex objects. This is done as follows:

## C
```C
ModulemdModuleIndex *fedora_index = modulemd_module_index_new ();
gboolean ret = modulemd_module_index_update_from_string (
  fedora_index, fedora_yaml, FALSE, &failures, &error);

ModulemdModuleIndex *updates_index = modulemd_module_index_new ();
gboolean ret2 = modulemd_module_index_update_from_string (
  updates_index, updates_yaml, FALSE, &failures, &error);
```

## Python
```python
fedora_index = Modulemd.ModuleIndex.new()
ret, failures = fedora_index.update_from_string(fedora_yaml, False)

updates_index = Modulemd.ModuleIndex.new()
ret, failures = updates_index.update_from_string(updates_yaml, False)
```

The `failures` are a list of subdocuments in the YAML that failed parsing,
along with the reason they failed. Hence, by checking the return value of
failures we will know if the YAML parsing was successful or not.

Since it doesn't really make sense to view the contents from separate
repositories in isolation (in most cases), the next step is to merge the two
indexes into a combined one:

## C
```C
ModulemdModuleIndexMerger *merger = modulemd_module_index_merger_new ();

modulemd_module_index_merger_associate_index (merger, fedora_index, 0);
modulemd_module_index_merger_associate_index (merger, updates_index, 0);

ModulemdModuleIndex *merged_index =
  modulemd_module_index_merger_resolve (merger, &error);
```

## Python
```python
merger = Modulemd.ModuleIndexMerger.new()

merger.associate_index(fedora_index, 0)
merger.associate_index(updates_index, 0)

merged_index = merger.resolve()
```

At this point, you now have either a complete view of the merged repodata,
or else have received an error describing why the merge was unable to
complete successfully. Additionally, it should be noted that the combined
metadata in any ModulemdModuleIndex will have all of its component parts
upgraded to match the highest version of those objects seen. So for example
if the repodata has a mix of v1 and v2 ModulemdModuleStream objects, the
index will contain only v2 objects (with the v1 objects automatically
upgraded internally).

Now, we can start operating on the retrieved data. This guide will
give only a brief overview of the most common operations. See the API
specification for a full list of information that can be retrieved.

## Discover the default stream for a particular module.

## C
```C
ModulemdModule *module =
  modulemd_module_index_get_module (merged_index, "modulename");
ModulemdDefaults *defaults = modulemd_module_get_defaults (module);
if (defaults)
  {
    printf ("Default stream for modulename is %s\n",
            modulemd_defaults_v1_get_default_stream (
              MODULEMD_DEFAULTS_V1 (defaults), NULL));
  }
```

## Python
```python
module = merged_index.get_module("modulename")
defaults = module.get_defaults()
print("Default stream for modulename is %s" % defaults.get_default_stream())
```

## Get the list of RPMs defining the public API for a particular module NSVCA
First, query the ModulemdModuleIndex for the module with a given name.

## C
```C
ModulemdModule *module =
  modulemd_module_index_get_module (merged_index, "modulename");
```

## Python
```python
module = merged_index.get_module("modulename")
```

Then, query the ModulemdModule for the ModulemdModuleStream associated with the
provided NSVCA (name-stream-version-context-architecture identifier).

## C
```C
ModulemdModuleStream *stream = modulemd_module_get_stream_by_NSVCA (
  module, "modulestream", 0, "deadbeef", "coolarch", &error);
```

## Python
```python
stream = module.get_stream_by_NSVCA("modulestream", 0, "deadbeef", "coolarch")
```

Lastly, read the RPM API from the ModulemdModuleStream. Here, `api_list` is
a list of strings containing package names.

## C
```C
GStrv api_list = modulemd_module_stream_v2_get_rpm_api_as_strv (
  MODULEMD_MODULE_STREAM_V2 (stream));
```

## Python
```python
api_list = stream.get_rpm_api()
```

Also note that in addition to accessor API methods, many objects also have
properties that can be accessed directly.

## C
```C
printf ("Documentation for module stream is at %s\n",
        modulemd_module_stream_v2_get_documentation (
          MODULEMD_MODULE_STREAM_V2 (stream)));
g_autofree gchar *doc;
g_object_get (MODULEMD_MODULE_STREAM_V2 (stream), "documentation", &doc, NULL);
printf ("Documentation for module stream is at %s\n", doc);
```

## Python
```python
print("Documentation for module stream is at %s" % stream.get_documentation())
print("Documentation for module stream is at %s" % stream.props.documentation)
```

## Retrieve the modular runtime dependencies for a particular module NSVCA

## C
```C
ModulemdModule *module =
  modulemd_module_index_get_module (merged_index, "modulename");
ModulemdModuleStream *stream = modulemd_module_get_stream_by_NSVCA (
  module, "modulestream", 0, "deadbeef", "coolarch", &error);
GPtrArray *deps_list = modulemd_module_stream_v2_get_dependencies (
  MODULEMD_MODULE_STREAM_V2 (stream));

for (gint i = 0; i < deps_list->len; i++)
  {
    GStrv depmodules_list =
     modulemd_dependencies_get_runtime_modules_as_strv (
       g_ptr_array_index (deps_list, i));

    for (gint j = 0; j < g_strv_length (depmodules_list); j++)
      {
        GStrv depstreams_list =
          modulemd_dependencies_get_runtime_streams_as_strv (
            g_ptr_array_index (deps_list, i), depmodules_list[j]);

        for (gint k = 0; k < g_strv_length (depstreams_list); k++)
          {
            // do stuff with depmodules_list[j], depstreams_list[k]
          }
      }
  }
```

## Python
```python
module = merged_index.get_module("modulename")
stream = module.get_stream_by_NSVCA("modulestream", 0, "deadbeef", "coolarch")
deps_list = stream.get_dependencies()
for dep in deps_list:
    depmodules_list = dep.get_runtime_modules()
    for depmod in depmodules_list:
        depstream_list = dep.get_runtime_streams(depmod)
        for depstream in depstream_list:
            # do stuff with depmod, depstream
```

# Working with a single module stream (Packager/MBS use-case)
One limitation of the ModulemdModuleIndex format is that it requires that
all module streams loaded into it have both a name and a stream name.
This however is not possible when dealing with streams such as a packager
would be using (since the build-system auto-generates the module name and
stream name from the git repository information. In this case, we need to
work with a single module stream document at a time. For this, we will
use the ModulemdModuleStream interface.

This example will assume that the module name and stream name have
already been determined from the repodata and that they are stored in
string variables named `module_name` and `stream_name`, respectively.

## Python
```python
stream = Modulemd.ModuleStream.read_file(
    "/path/to/module_name.yaml", True, module_name, stream_name
)
v2_stream = stream.upgrade(Modulemd.ModuleStreamVersionEnum.TWO)
v2_stream.validate()
```
In the example above, we upgraded the stream to v2, in case we were reading
from v1 metadata. This will allow us to avoid having to manage multiple
code-paths and support only the latest we understand. After that, it calls
validate() to ensure that the content that was read in was valid both
syntactically and referentially.

Also available is `Modulemd.ModuleStreamVersionEnum.LATEST` which will
always represent the highest-supported version of the
ModulemdModuleStream metadata format. This may change at any time.

# Getting started with developing

## Forking and cloning the sources
The libmodulemd project follows the
[Github Fork-and-Pull](https://reflectoring.io/github-fork-and-pull/) model of
development. To get started, create a fork of the upstream libmodulemd sources,
clone those locally and create branches on your fork to make changes. When they
are ready for review or feedback, create a pull-request.

## Prerequisites
* A development system with either [Podman](https://podman.io/) and
[Buildah](https://buildah.io/) (preferred) or
[Docker](https://www.docker.com/) installed.

## Preparing a build environment
Create a container for building the libmodulemd sources. Run the handy setup
script in the root of the checkout.
```
./setup_dev_container.sh [<Fedora Release>]
```
If unspecified, it will default to "Fedora Rawhide". To select a different
Fedora release for the base image, add the release number as an argument to the
command. For example:
```
./setup_dev_container.sh 33
```

This will automatically pull down a Fedora base image, install all of the
packages needed for development and testing and then provide you with a shell
inside this environment.

## Building the sources
Projects built with the meson build-system require a separate build directory from
the source path. The `meson` command will generate this directory for you.
```
meson --buildtype=debug -Db_coverage=true debugbuild
```
The above command (run from the root of the source checkout) will create a new
subdirectory - `debugbuild` - configured to compile with debug symbols and
`gcov` symbols to measure test coverage.

To build the sources, `chdir()` into the `debugbuild` directory and run
```
ninja
```

To build and run the in-tree tests, use
```
ninja test
```

To generate HTML documentation, you can run
```
ninja modulemd-2.0-doc
```
(Be aware that the GLib documentation module in meson has some strange quirks
and won't recognize newly-added pages without deleting and re-creating the
build directory first.)


## Running more advanced test suites
In addition to the basic `ninja test` set of tests, libmodulemd also has a
suite of tests performed in the Github Actions environment. These are also
containerized and can be run locally on the host. (You will not be able to run
them from within the development container shell, as nested containers are not
supported.)

To run the container-based tests, you can run the following from the source
root:
```
./.ci/ci-fedora.sh
```
(Optionally, you can pass the release number as an argument to switch to
building and testing against that release rather than Fedora Rawhide).

Support for running the tests on other OSes is ongoing. See the `.ci`
directory for available suites. All supported OSes and release versions are
tested as part of each pull request.


## Tips and tricks

### Running tests in debug mode

The libmodulemd library is built atop
[GObject](https://developer.gnome.org/gobject/stable/). It provides a debug
mode that is configurable by an environment variable. In general, it is highly
recommended that you run all tests with
`G_DEBUG='fatal-warnings,fatal-criticals'` set in the environment. This will
cause the application to `abort()` on programming errors that would be logged
and ignored at runtime.


### Running tests with valgrind
Assuming your current working directory is `debugbuild` as described above:
```
meson test --suite=ci_valgrind --wrap=../contrib/valgrind/valgrind_wrapper.sh
```

If not, you may need to adjust the path to libmodulemd-python.supp.

You can also specify individual tests to run against. See `meson test --list`
for the available tests.

The automated CI tests will always run with valgrind on all platforms where it
is supported.

# Authors:
* Stephen Gallagher <sgallagh@redhat.com>
* Merlin Mathesius <mmathesi@redhat.com>
* Igor Gnatenko <ignatenkobrain@fedoraproject.org>
