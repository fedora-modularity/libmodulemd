#!/bin/bash

#Exit on failures
set -e
set -x

if [ -e /usr/lib/os-release ]; then
    source /usr/lib/os-release
    case "${ID-unknown} ${VERSION_ID-unknown}" in
    "centos 7")
        COMMON_MESON_ARGS="-Dtest_dirty_git=false -Ddeveloper_build=false -Dpython_name=python3.6"
        ;;
    "centos "*)
        COMMON_MESON_ARGS="-Dtest_dirty_git=false -Ddeveloper_build=false"
        ;;
    esac
fi

pushd /builddir/

meson --buildtype=debug \
      $COMMON_MESON_ARGS \
      coverity

pushd coverity

# The coverity scan script returns an error despite succeeding...
 TRAVIS_BRANCH="${TRAVIS_BRANCH:-main}" \
 COVERITY_SCAN_PROJECT_NAME="${COVERITY_SCAN_PROJECT_NAME:-sgallagher/libmodulemd}" \
 COVERITY_SCAN_NOTIFICATION_EMAIL="${COVERITY_SCAN_NOTIFICATION_EMAIL:-sgallagh@redhat.com}" \
 COVERITY_SCAN_BUILD_COMMAND="${COVERITY_SCAN_BUILD_COMMAND:-ninja}" \
 COVERITY_SCAN_BRANCH_PATTERN=${COVERITY_SCAN_BRANCH_PATTERN:-main} \
 /usr/bin/travisci_build_coverity_scan.sh ||:

popd #coverity

popd #builddir
