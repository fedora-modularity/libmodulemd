#!/bin/bash

#Exit on failures
set -e

set -x

JOB_NAME=${TRAVIS_JOB_NAME:-CentOS 7}

arr=($JOB_NAME)
os_name=${arr[0]:-CentOS}
release=${arr[1]:-7}

# CentOS 7 doesn't have autopep8, so we'll drop the requirement for it
# This implementation will still allow it to occur if autopep8 still shows
# up later.
COMMON_MESON_ARGS="-Dtest_dirty_git=false -Ddeveloper_build=false -Dpython_name=python3.6"

pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debug \
      $COMMON_MESON_ARGS \
      travis

set +e
ninja-build -C travis test
ret=$?
if [ $ret != 0 ]; then
    cat travis/meson-logs/testlog.txt
    exit $ret
fi
set -e

popd #builddir
