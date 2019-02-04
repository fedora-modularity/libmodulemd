#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR/../..

#Exit on failures
set -e

set -x

JOB_NAME=${TRAVIS_JOB_NAME:-Fedora Rawhide}

arr=($JOB_NAME)
os_name=${arr[0]:-Fedora}
release=${arr[1]:-Rawhide}

COMMON_MESON_ARGS="-Dtest_dirty_git=${DIRTY_REPO_CHECK:-true}"

# Build the v1 and v2 code under GCC and run standard tests
meson --buildtype=debug \
      -Dbuild_api_v1=true \
      -Dbuild_api_v2=true \
      $COMMON_MESON_ARGS \
      travis

set +e
ninja-build -C travis test
if [ $? != 0 ]; then
    cat travis/meson-logs/testlog.txt
fi
set -e

popd
