#!/bin/bash

#Exit on failures
set -e

set -x

os_name=Fedora
release=rawhide

pushd /builddir/

meson --buildtype=debug \
      $COMMON_MESON_ARGS \
      coverity

pushd coverity

# The coverity scan script returns an error despite succeeding...
 TRAVIS_BRANCH="${TRAVIS_BRANCH:-master}" \
 COVERITY_SCAN_PROJECT_NAME="${COVERITY_SCAN_PROJECT_NAME:-sgallagher/libmodulemd}" \
 COVERITY_SCAN_NOTIFICATION_EMAIL="${COVERITY_SCAN_NOTIFICATION_EMAIL:-sgallagh@redhat.com}" \
 COVERITY_SCAN_BUILD_COMMAND="${COVERITY_SCAN_BUILD_COMMAND:-ninja}" \
 COVERITY_SCAN_BRANCH_PATTERN=${COVERITY_SCAN_BRANCH_PATTERN:-master} \
 /usr/bin/travisci_build_coverity_scan.sh ||:

 cat /builddir/coverity/cov-int/build-log.txt

popd #coverity

popd #builddir
