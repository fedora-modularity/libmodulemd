#!/usr/bin/sh

# optionally call this function with --nocheck to skip the test suite while
# building RPMs.

set -e

echo "Creating tarball. This may take a while."

MMD_SKIP_VALGRIND=True ninja dist > /dev/null

ln -sf ../meson-dist/ ./rpmbuild/SOURCES

rpmbuild -bb $1 libmodulemd.spec --define "_topdir $(pwd)/rpmbuild"
