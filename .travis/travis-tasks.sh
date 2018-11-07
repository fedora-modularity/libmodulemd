#!/bin/bash

#Exit on failures
set -e

pushd /builddir/

meson --buildtype=debug -Dbuild_api_v1=true -Dbuild_api_v2=true -Dtest_dirty_git=${DIRTY_REPO_CHECK:-true} travis

ninja -C travis test
if [ $? != 0 ]; then
    cat /builddir/travis/meson-logs/testlog.txt
fi

meson --buildtype=debug -Dbuild_api_v1=true -Dbuild_api_v2=true coverity
pushd coverity

# The coverity scan script returns an error despite succeeding...
 TRAVIS_BRANCH="${TRAVIS_BRANCH:-master}" \
 COVERITY_SCAN_PROJECT_NAME="${COVERITY_SCAN_PROJECT_NAME:-sgallagher/libmodulemd}" \
 COVERITY_SCAN_NOTIFICATION_EMAIL="${COVERITY_SCAN_NOTIFICATION_EMAIL:-sgallagh@redhat.com}" \
 COVERITY_SCAN_BUILD_COMMAND="${COVERITY_SCAN_BUILD_COMMAND:-ninja}" \
 COVERITY_SCAN_BRANCH_PATTERN=${COVERITY_SCAN_BRANCH_PATTERN:-master} \
 /usr/bin/travisci_build_coverity_scan.sh ||:

popd #coverity


# Always install and run the installed RPM tests last so we don't pollute the
# testing environment above.

meson --buildtype=debug -Dbuild_api_v1=true build_rpm
pushd build_rpm

ninja
./make_rpms.sh
dnf -y install rpmbuild/RPMS/*/*.rpm

popd #build_rpm


meson --buildtype=debug -Dbuild_api_v1=true -Dtest_installed_lib=true installed_lib_tests
pushd installed_lib_tests

# Run the tests against the installed RPMs
ninja test

popd #installed_lib_tests

popd #builddir
