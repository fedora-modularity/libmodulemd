#!/bin/bash

#Exit on failures
set -e

set -x


JOB_NAME=${TRAVIS_JOB_NAME:- Mageia 7}

arr=($JOB_NAME)
os_name=${arr[0]:-Mageia}
release=${arr[1]:-7}

COMMON_MESON_ARGS="-Dtest_dirty_git=${DIRTY_REPO_CHECK:-true}"


pushd /builddir/

# Build the v1 and v2 code under GCC and run standard tests
meson --buildtype=debug \
      -Dbuild_api_v1=true \
      -Dbuild_api_v2=true \
      $COMMON_MESON_ARGS \
      travis

set +e
ninja -C travis test
if [ $? != 0 ]; then
    if [ "x$TRAVIS_JOB_NAME" != "x" ]; then
        cat travis/meson-logs/testlog.txt
    fi
    exit $ret
fi
set -e

# Test the v2 code with clang-analyzer
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
          -Dbuild_api_v1=false \
          -Dbuild_api_v2=true \
          -Dskip_introspection=true \
          $COMMON_MESON_ARGS \
          travis_scanbuild

    pushd travis_scanbuild
    /builddir/.travis/scanbuild.sh
    popd #travis_scanbuild
fi

meson --buildtype=debug \
      -Dbuild_api_v1=true \
      -Dbuild_api_v2=true \
      $COMMON_MESON_ARGS \
      coverity

