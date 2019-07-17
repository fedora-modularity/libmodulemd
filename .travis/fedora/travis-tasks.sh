#!/bin/bash

#Exit on failures
set -e

set -x

JOB_NAME=${TRAVIS_JOB_NAME:-Fedora rawhide}

arr=($JOB_NAME)
os_name=${arr[0]:-Fedora}
release=${arr[1]:-rawhide}

COMMON_MESON_ARGS="-Dtest_dirty_git=${DIRTY_REPO_CHECK:-true}"


pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debug \
      $COMMON_MESON_ARGS \
      travis

set +e
ninja -C travis test
ret=$?
if [ $ret != 0 ]; then
    cat travis/meson-logs/testlog.txt
    exit $ret
fi
set -e


# Always install and run the installed RPM tests last so we don't pollute the
# testing environment above.

meson --buildtype=debug \
      $COMMON_MESON_ARGS \
      build_rpm

pushd build_rpm

ninja
./make_rpms.sh

createrepo_c rpmbuild/RPMS/

dnf -y install --nogpgcheck \
               --repofrompath libmodulemd-travis,rpmbuild/RPMS \
               python3-libmodulemd1 \
               libmodulemd1-devel \
               --exclude libmodulemd

# Also install the python2-libmodulemd1 if it was built for this release
# the ||: at the end instructs bash to consider this a pass either way.
dnf -y install --nogpgcheck \
               --repofrompath libmodulemd-travis,rpmbuild/RPMS \
               python2-libmodulemd1 \
               --exclude libmodulemd ||:

popd #build_rpm

meson --buildtype=debug \
      -Dtest_installed_lib=true \
      $COMMON_MESON_ARGS \
      installed_lib_tests

pushd installed_lib_tests

# Run the tests against the installed RPMs
ninja test

popd #installed_lib_tests

popd #builddir
