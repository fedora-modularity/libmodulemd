#!/bin/bash

#Exit on failures
set -e
set -x


COMMON_MESON_ARGS="-Dtest_dirty_git=false -Ddeveloper_build=false -Dpython_name=python3"


pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debugoptimized \
      $COMMON_MESON_ARGS \
      ci

MMD_SKIP_VALGRIND=TRUE ninja -C ci test

popd #builddir
