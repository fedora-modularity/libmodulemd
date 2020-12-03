#!/usr/bin/env bash

#Exit on failures
set -e
set -x


pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debugoptimized \
      -Dtest_dirty_git=false \
      -Ddeveloper_build=false \
      -Dpython_name=python3 \
      -Drpmio=disabled \
      travis

ninja -C travis test

popd #builddir
