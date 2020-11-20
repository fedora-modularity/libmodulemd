#!/bin/bash

#Exit on failures
set -e
set -x


# CentOS 7 doesn't have autopep8, so we'll drop the requirement for it
# This implementation will still allow it to occur if autopep8 still shows
# up later.
COMMON_MESON_ARGS="-Dtest_dirty_git=false -Ddeveloper_build=false -Dpython_name=python3.6"

pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debugoptimized \
      $COMMON_MESON_ARGS \
      travis

ninja-build -C travis test

popd #builddir
