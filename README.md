[![Travis](https://img.shields.io/travis/fedora-modularity/libmodulemd.svg?style=plastic)](https://travis-ci.org/fedora-modularity/libmodulemd)
[![Travis](https://img.shields.io/coverity/scan/13739.svg?style=plastic)](https://scan.coverity.com/projects/sgallagher-libmodulemd)

# libmodulemd
C Library for manipulating module metadata files

[API Documentation](https://sgallagh.fedorapeople.org/docs/libmodulemd/latest/)

## Using libmodulemd from Python

Using libmodulemd from Python is possible thanks to gobject-introspection
project. Following is example code how to do that:

```
import gi
gi.require_version('Modulemd', '2.0')
from gi.repository import Modulemd
help(Modulemd.Module)
```

# Authors:
* Stephen Gallagher <sgallagh@redhat.com>
* Igor Gnatenko <ignatenkobrain@fedoraproject.org>
