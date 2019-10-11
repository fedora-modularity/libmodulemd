#!/bin/bash

#Exit on failures
set -e
set -x


COMMON_MESON_ARGS="-Dtest_dirty_git=${DIRTY_REPO_CHECK:-true} -Ddeveloper_build=false"


pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debug \
      $COMMON_MESON_ARGS \
      travis

ninja -C travis test

# Test the code with clang-analyzer
# This requires meson 0.49.0 or later
set +e
rpmdev-vercmp `meson --version` 0.49.0
if [ $? -eq 12 ]; then
    # Meson was older than 0.49.0, skip this step
    echo "Meson is too old to run scan-build"
    set -e
else
    set -e
    meson --buildtype=debug \
          -Dskip_introspection=true \
          $COMMON_MESON_ARGS \
          travis_scanbuild

    pushd travis_scanbuild
    /builddir/.travis/scanbuild.sh
    popd #travis_scanbuild
fi

