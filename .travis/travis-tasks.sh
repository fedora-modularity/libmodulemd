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

createrepo_c rpmbuild/RPMS/

dnf -y install --nogpgcheck \
               --repofrompath libmodulemd-travis,rpmbuild/RPMS \
               python3-compat-libmodulemd1 \
               compat-libmodulemd1-devel \
               --exclude libmodulemd

popd #build_rpm

meson --buildtype=debug -Dbuild_api_v1=true -Dbuild_api_v2=false -Dtest_installed_lib=true installed_lib_tests_v1
pushd installed_lib_tests_v1

# Run the tests against the installed RPMs
ninja test

popd #installed_lib_tests_v1


pushd build_rpm
dnf -y install --nogpgcheck \
               --allowerasing \
               --repofrompath libmodulemd-travis,rpmbuild/RPMS \
               python3-libmodulemd \
               "libmodulemd-devel > 2"
popd

meson --buildtype=debug -Dbuild_api_v1=false -Dbuild_api_v2=true -Dtest_installed_lib=true installed_lib_tests_v2
pushd installed_lib_tests_v2

# Run the tests against the installed RPMs
ninja test

popd #installed_lib_tests_v2

popd #builddir
