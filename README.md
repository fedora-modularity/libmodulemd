[![Travis](https://img.shields.io/travis/fedora-modularity/libmodulemd.svg?style=plastic)](https://travis-ci.org/fedora-modularity/libmodulemd)
[![Travis](https://img.shields.io/coverity/scan/13739.svg?style=plastic)](https://scan.coverity.com/projects/sgallagher-libmodulemd)

# libmodulemd
C Library for manipulating module metadata files

Full details can be found in the
[API Documentation](https://sgallagh.fedorapeople.org/docs/libmodulemd/latest/)

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
repositories have been loaded into string variables "fedora_yaml" and
"updates_yaml", respectively.

First step is to load the metadata from these two repositories into
ModulemdModuleIndex objects. This is done as follows:

## C
```C
fedora_index = modulemd_module_index_new();
ret = modulemd_module_index_update_from_string(fedora_index,
                                               fedora_yaml,
                                               &failures,
                                               &error);

updates_index = modulemd_module_index_new();
ret = modulemd_module_index_update_from_string(updates_index,
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

Since it doesn't really make sense to view the contents from separate
repositories in isolation (in most cases), the next step is to merge the two
indexes into a combined one:

## C
```C
merger = modulemd_module_index_merger_new()

modulemd_module_index_merger_associate_index (merger, fedora_index, 0);
modulemd_module_index_merger_associate_index (merger, updates_index, 0);

merged_index = modulemd_module_index_merger_resolve (merger, &error);
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
dnf -y install git-core "dnf-command(builddep)" && dnf -y builddep libmodulemd
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
