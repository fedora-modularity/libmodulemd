#!/bin/bash

#Exit on failures
set -e

set -x

JOB_NAME=${TRAVIS_JOB_NAME:-Arch Linux}

os_name='Arch Linux'
release=base

COMMON_MESON_ARGS="-Dtest_dirty_git=false -Ddeveloper_build=false -Dpython_name=python3"


pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debug \
      $COMMON_MESON_ARGS \
      travis

ninja -C travis test

popd #builddir
