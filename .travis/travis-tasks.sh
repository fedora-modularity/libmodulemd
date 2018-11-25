#!/bin/bash

#Exit on failures
set -e

set -x

JOB_NAME=${TRAVIS_JOB_NAME:-Fedora rawhide}

arr=($JOB_NAME)
os_name=${arr[0]:-Fedora}
release=${arr[1]:-rawhide}

if [ $os_name = "Fedora" ]; then
    COMMON_MESON_ARGS="-Dtest_dirty_git=${DIRTY_REPO_CHECK:-true}"
    NINJA="ninja"
else
    # CentOS 7 doesn't have autopep8, so we'll drop the requirement for it
    # This implementation will still allow it to occur if autopep8 still shows
    # up later.
    COMMON_MESON_ARGS="-Dtest_dirty_git=${DIRTY_REPO_CHECK:-true} -Ddeveloper_build=false"
    NINJA="ninja-build"
    EPEL_HACK="sed -r -i -e /g-ir-scanner/s/-l(gobject-2.0|glib-2.0|yaml)//g"
fi



pushd /builddir/

meson --buildtype=debug -Dbuild_api_v1=true -Dbuild_api_v2=true $COMMON_MESON_ARGS travis

if [ $os_name = "CentOS" ]; then
    $EPEL_HACK ./travis/build.ninja
fi

$NINJA -C travis test
if [ $? != 0 ]; then
    cat /builddir/travis/meson-logs/testlog.txt
fi

meson --buildtype=debug -Dbuild_api_v1=true -Dbuild_api_v2=true $COMMON_MESON_ARGS coverity
pushd coverity

if [ $os_name = "CentOS" ]; then
    $EPEL_HACK ./build.ninja
fi

# The coverity scan script returns an error despite succeeding...
 TRAVIS_BRANCH="${TRAVIS_BRANCH:-master}" \
 COVERITY_SCAN_PROJECT_NAME="${COVERITY_SCAN_PROJECT_NAME:-sgallagher/libmodulemd}" \
 COVERITY_SCAN_NOTIFICATION_EMAIL="${COVERITY_SCAN_NOTIFICATION_EMAIL:-sgallagh@redhat.com}" \
 COVERITY_SCAN_BUILD_COMMAND="${COVERITY_SCAN_BUILD_COMMAND:-$NINJA}" \
 COVERITY_SCAN_BRANCH_PATTERN=${COVERITY_SCAN_BRANCH_PATTERN:-master} \
 /usr/bin/travisci_build_coverity_scan.sh ||:

popd #coverity


# Always install and run the installed RPM tests last so we don't pollute the
# testing environment above.

meson --buildtype=debug -Dbuild_api_v1=true $COMMON_MESON_ARGS build_rpm
pushd build_rpm

if [ $os_name = "CentOS" ]; then
    $EPEL_HACK ./build.ninja
fi

$NINJA
./make_rpms.sh

createrepo_c rpmbuild/RPMS/

dnf -y install --nogpgcheck \
               --repofrompath libmodulemd-travis,rpmbuild/RPMS \
               python3-compat-libmodulemd1 \
               compat-libmodulemd1-devel \
               --exclude libmodulemd

popd #build_rpm

meson --buildtype=debug -Dbuild_api_v1=true -Dbuild_api_v2=false -Dtest_installed_lib=true $COMMON_MESON_ARGS installed_lib_tests_v1
pushd installed_lib_tests_v1

if [ $os_name = "CentOS" ]; then
    $EPEL_HACK ./build.ninja
fi

# Run the tests against the installed RPMs
$NINJA test

popd #installed_lib_tests_v1


pushd build_rpm

dnf -y install --nogpgcheck \
               --allowerasing \
               --repofrompath libmodulemd-travis,rpmbuild/RPMS \
               python3-libmodulemd \
               "libmodulemd-devel > 2"
popd

meson --buildtype=debug -Dbuild_api_v1=false -Dbuild_api_v2=true -Dtest_installed_lib=true $COMMON_MESON_ARGS installed_lib_tests_v2
pushd installed_lib_tests_v2

if [ $os_name = "CentOS" ]; then
    $EPEL_HACK ./build.ninja
fi

# Run the tests against the installed RPMs
$NINJA test

popd #installed_lib_tests_v2

popd #builddir
