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

Install the `python2-libmodulemd` or `python3-libmodulemd` package for your
system depending on the version of python with which you are working.
These packages provide the appropriate language bindings.

```python
import gi
gi.require_version('Modulemd', '1.0')
from gi.repository import Modulemd
```

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
sudo systemctl enable --now docker.service
```
and to make sure that your user has privilege to run `sudo docker` (see the documentation for the `/etc/sudoers` file to figure this out).

## Forking and cloning the sources
The libmodulemd project follows the [Github Fork-and-Pull](https://reflectoring.io/github-fork-and-pull/) model of development. To get started, create a fork of the upstream libmodulemd sources, clone those locally and create branches on your fork to make changes. When they are ready for review or feedback, create a pull-request.

## Building the sources
This guideline will describe the procedure for working with the 1.0 API. (Note that the 1.0 API is deprecated and will be removed soon)

Projects built with the meson build-system require a separate build directory from the source path. The `meson` command will generate this directory for you.
```
meson --buildtype=debug -Db_coverage=true build
```
The above command (run from the root of the source checkout) will create a new subdirectory - `build` - configured to compile with debug symbols and `gcov` symbols to measure test coverage.

To build the sources, `chdir()` into the `build` directory and run
```
ninja
```

To build and run the in-tree tests, use
```
ninja test
```

To generate HTML documentation, you can run
```
ninja modulemd-1.0-doc
```
(Be aware that the GLib documentation module in meson has some strange quirks and won't recognize newly-added pages without deleting and re-creating the build directory first.)


To run the docker-based tests, you can run (from the source root and with `sudo` privilege to run `docker`):
```
./.travis/travis-fedora.sh
```
(Optionally setting the environment variable `TRAVIS_JOB_NAME` to `Fedora 28`, `Fedora 29`, etc. to switch to building against those releases rather than Fedora Rawhide).


## Tips and tricks

### Running tests in debug mode

The libmodulemd library is built atop
[GObject](https://developer.gnome.org/gobject/stable/). It provides a debug
mode that is configurable by an environment variable. In general, it is highly
recommended that you run all tests with
`G_DEBUG='fatal-warnings,fatal-criticals'` set in the environment. This will
cause the application to `abort()` on programming errors that would be logged
and ignored at runtime.


### Skipping the valgrind tests

If you are trying to iterate quickly, you can temporarily skip the valgrind
memory tests by running the test suite with:
```
MMD_SKIP_VALGRIND=True ninja test
```

The automated CI tests will always run with valgrind on all platforms where it
is supported.

# Authors:
* Stephen Gallagher <sgallagh@redhat.com>
* Igor Gnatenko <ignatenkobrain@fedoraproject.org>
