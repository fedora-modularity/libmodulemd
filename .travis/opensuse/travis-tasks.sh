#!/bin/bash

#Exit on failures
set -e

set -x

JOB_NAME=${TRAVIS_JOB_NAME:-openSUSE tumbleweed}

arr=($JOB_NAME)
os_name=${arr[0]:-openSUSE}
release=${arr[1]:-tumbleweed}

COMMON_MESON_ARGS="-Dtest_dirty_git=false -Ddeveloper_build=false -Dpython_name=python3"


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
    cat travis/meson-logs/testlog.txt
fi
set -e

popd #builddir
