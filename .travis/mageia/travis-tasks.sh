#!/usr/bin/env bash

#Exit on failures
set -e
set -x

PROCESSORS=$(/usr/bin/getconf _NPROCESSORS_ONLN)
COMMON_MESON_ARGS="-Dtest_dirty_git=${DIRTY_REPO_CHECK:-false} -Ddeveloper_build=false"


pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debugoptimized \
      -Dverbose_tests=false \
      $COMMON_MESON_ARGS \
      travis

meson test --suite ci \
           -C travis \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 5

meson test --suite ci_valgrind \
           --wrap=/builddir/contrib/valgrind/valgrind_wrapper.sh \
           -C travis \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 10

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

