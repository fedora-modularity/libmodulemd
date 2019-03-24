[![Travis](https://img.shields.io/travis/fedora-modularity/libmodulemd.svg?style=plastic)](https://travis-ci.org/fedora-modularity/libmodulemd)
[![Travis](https://img.shields.io/coverity/scan/13739.svg?style=plastic)](https://scan.coverity.com/projects/sgallagher-libmodulemd)

# libmodulemd
C Library for manipulating module metadata files

Full details can be found in the
[API Documentation](https://fedora-modularity.github.io/libmodulemd/latest/)

# Using libmodulemd from Python

Using libmodulemd from Python is possible thanks to the gobject-introspection
project. To use libmodulemd from Python, include the following at the top of
your sources.
```python
import gi
gi.require_version('Modulemd', '2.0')
from gi.repository import Modulemd
```

# Working with repodata (DNF use-case)
The libmodulemd API provides a number of convenience tools for interacting
with repodata (that is, streams of YAML that contains information on multiple
streams, default data and translations). The documentation will use two
repositories, called "fedora" and "updates" for demonstrative purposes. It
will assume that the content of the YAML module metadata from those two
repositories have been loaded into string variables `"fedora_yaml"` and
`"updates_yaml"`, respectively.

First step is to load the metadata from these two repositories into
ModulemdModuleIndex objects. This is done as follows:

## C
```C
ModulemdModuleIndex * fedora_index = modulemd_module_index_new();
gboolean ret = modulemd_module_index_update_from_string(fedora_index,
                                               fedora_yaml,
                                               &failures,
                                               &error);

ModulemdModuleIndex * updates_index = modulemd_module_index_new();
gboolean ret = modulemd_module_index_update_from_string(updates_index,
                                               updates_yaml,
                                               &failures,
                                               &error);
```

## Python
```python
fedora_index = Modulemd.ModuleIndex.new()
ret, failures = fedora_index.update_from_string(fedora_yaml)

fedora_index = Modulemd.ModuleIndex.new()
ret, failures = updates_index.update_from_string(updates_yaml)
```

The `failures` are a list of subdocuments in the YAML that failed parsing, along with the reason they failed. Hence, by checking the return value of failures we will know if the YAML parsing was successful or not.

Since it doesn't really make sense to view the contents from separate
repositories in isolation (in most cases), the next step is to merge the two
indexes into a combined one:

## C
```C
ModulemdModuleIndexMerger * merger = modulemd_module_index_merger_new();

modulemd_module_index_merger_associate_index (merger, fedora_index, 0);
modulemd_module_index_merger_associate_index (merger, updates_index, 0);

ModulemdModuleIndex * merged_index = modulemd_module_index_merger_resolve (merger, &error);
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
upgraded to match the highest version of those objects seen. So for
example if the repodata has a mix of v1 and v2 ModuleStream objects, the
index will contain only v2 objects (with the v1 objects automatically
upgraded internally).

Now, we can start operating on the retrieved data. This guide will
give only a brief overview of the most common operations. See the API
specification for a full list of information that can be retrieved.

## Discover the default stream for a particular module.
## C
```C
 ModulemdModule * module =  modulemd_module_index_get_module(merged_index, "modulename");
 ModulemdDefaults * defaults = modulemd_module_get_defaults (module);
 printf ("Default stream for modulename is %s\n", 
       modulemd_defaults_get_module_name (defaults));
  ```

## Python
```python
 module = merged_index.get_module ('modulename')
 defaults = module.get_defaults()
 print ('Default stream for modulename is %s' % (
       defaults.get_default_stream())
  ```
 
## Get the list of RPMs defining the public API for a particular module NSVCA
First, query the Modulemd.ModuleIndex for the module with a given name.
## C
```C
ModulemdModule * module = modulemd_module_index_get_module (merged_index, "modulename");
```

## Python
```python
 module = merged_index.get_module ('modulename')
```

Then, query the Modulemd.Module for the Modulemd.ModuleStream associated with the provided NSVCA
(name-stream-version-context-architechture identifier).
## C
```C
ModulemdModuleStream * stream = modulemd_module_get_stream_by_NSVCA(module, "modulestream", 0, "deadbeef", "coolarch", &error);
```

## Python
```python
 stream = module.get_stream_by_NSVCA('modulestream', 0, 'deadbeef', 'coolarch')
```

Lastly, read the RPM API from the Modulemd.ModuleStream. Here, `api_list` is a list of strings containing package names.
## C
```C
GStrv api_list = modulemd_module_stream_v2_get_rpm_api_as_strv(stream); 
```

## Python
```python
 api_list = stream.get_rpm_api()
```

## Retrieve the modular runtime dependencies for a particular module NSVCA
## C
```C
ModulemdModule * module = modulemd_module_index_get_module (merged_index, "modulename");
ModulemdModuleStream * stream = modulemd_module_get_stream_by_NSVCA (module, "modulestream", 0, "deadbeef", "coolarch", &error);
GPtrArray * deps_list = modulemd_module_stream_v2_get_dependencies (stream);

for (gint i = 0; i < deps_list->len; i++)
  {
    stuff with g_ptr_array_index(deps_list, i);
  }
```

## Python
```python
 module = merged_index.get_module ('modulename')
 stream = module.get_stream_by_NSVCA('modulestream', 0, 'deadbeef', 'coolarch')
 deps_list = stream.get_dependencies()
 for dep in deps_list:
    depstream_list = dep.get_runtime_streams('depstreamname')
    <do_stuff>
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
string variables named 'module_name' and 'stream_name', respectively.

## Python
```python
stream = Modulemd.ModuleStream.read_file ('/path/to/module_name.yaml',
                                          module_name,
                                          stream_name)
v2_stream = stream.upgrade(Modulemd.ModuleStreamVersion.TWO)
v2_stream.validate()
```
In the example above, we upgraded the stream to v2, in case we were reading from v1 metadata. This will allow us to avoid having to manage multiple code-paths and support only the latest we understand. After that, it calls validate() to ensure that the content that was read in was valid both syntactically and referentially.

# Getting started with developing
## Prerequisites
* A Fedora development environment (physical or virtual)

To install all of the dependencies needed to build libmodulemd, the following command will work on Fedora 28+ (run as root or with sudo):
```
dnf -y install clang git-core python3-pycodestyle python3-autopep8 redhat-rpm-config "dnf-command(builddep)"
dnf -y builddep libmodulemd
```

To install the tools needed to run the docker-based tests, you will also need:
```
dnf -y install docker
```
and to make sure that your user has privilege to run `sudo docker` (see the documentation for the `/etc/sudoers` file to figure this out).

## Forking and cloning the sources
The libmodulemd project follows the [Github Fork-and-Pull](https://reflectoring.io/github-fork-and-pull/) model of development. To get started, create a fork of the upstream libmodulemd sources, clone those locally and create branches on your fork to make changes. When they are ready for review or feedback, create a pull-request.

## Building the sources
First, decide if you are planning to build the sources for the 1.0 API or the 2.0 API. (Note that the 1.0 API is deprecated and will be removed soon). This guideline will describe the procedure for working with the 2.0 API.

Projects built with the meson build-system require a separate build directory from the source path. The `meson` command will generate this directory for you.
```
meson --buildtype=debug -Db_coverage=true -Dbuild_api_v1=false -Dbuild_api_v2=true  api2
```
The above command (run from the root of the source checkout) will create a new subdirectory - `api2` - configured to compile with debug symbols and `gcov` symbols to measure test coverage. It enables building the 2.0 API and skips the deprecated 1.0 API.

To build the sources, `chdir()` into the `api2` directory and run
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
(Be aware that the GLib documentation module in meson has some strange quirks and won't recognize newly-added pages without deleting and re-creating the build directory first.)


To run the docker-based tests, you can run (from the source root and with `sudo` privilege to run `docker`):
```
./.travis/travis-fedora.sh
```
(Optionally setting the environment variable `TRAVIS_JOB_NAME` to `Fedora 28`, `Fedora 29`, etc. to switch to building against those releases rather than Fedora Rawhide).


# Authors:
* Stephen Gallagher <sgallagh@redhat.com>
* Igor Gnatenko <ignatenkobrain@fedoraproject.org>
