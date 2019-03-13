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
    cat travis_v1/meson-logs/testlog.txt
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

pushd coverity

# Always install and run the installed RPM tests last so we don't pollute the
# testing environment above.

meson --buildtype=debug \
      -Dbuild_api_v1=true \
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

popd #build_rpm

meson --buildtype=debug \
      -Dbuild_api_v1=true \
      -Dbuild_api_v2=false \
      -Dtest_installed_lib=true \
      $COMMON_MESON_ARGS \
      installed_lib_tests_v1

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

meson --buildtype=debug \
      -Dbuild_api_v1=false \
      -Dbuild_api_v2=true \
      -Dtest_installed_lib=true \
      $COMMON_MESON_ARGS \
      installed_lib_tests_v2

pushd installed_lib_tests_v2
# Run the tests against the installed RPMs
ninja test

popd #installed_lib_tests_v2

popd #builddir
