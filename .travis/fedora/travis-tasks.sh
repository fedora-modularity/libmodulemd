#!/bin/bash

#Exit on failures
set -e
set -x

PROCESSORS=$(/usr/bin/getconf _NPROCESSORS_ONLN)
MESON_DIRTY_REPO_ARGS="-Dtest_dirty_git=${DIRTY_REPO_CHECK:-false}"
RETRY_CMD=/builddir/.travis/retry-command.sh

override_dir=`python3 -c 'import gi; print(gi._overridesdir)'`

pushd /builddir/

# Ensure that the python 3 overrides are always in place or else some of those
# tests may fail if they are modified.
ln -sf /builddir/bindings/python/gi/Modulemd.py $override_dir/

# Build the code under GCC and run standard tests
meson --buildtype=debugoptimized \
      -Dverbose_tests=false \
      $MESON_DIRTY_REPO_ARGS \
      travis

meson test --suite formatters \
           -C travis \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 5

meson test --suite ci \
           -C travis \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 5

meson test --suite ci_valgrind \
           --wrap=/builddir/contrib/valgrind/valgrind_wrapper.sh \
           -C travis \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 10

# Test the code with clang-analyzer
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
          -Dskip_introspection=true \
          $COMMON_MESON_ARGS \
          travis_scanbuild

    pushd travis_scanbuild
    /builddir/.travis/scanbuild.sh
    popd #travis_scanbuild
fi


# Always install and run the installed RPM tests last so we don't pollute the
# testing environment above.

arch=$(uname -m)
mkdir -p rpmbuild/RPMS/
pushd rpmbuild/RPMS/
packit local-build ../..
createrepo_c $arch

$RETRY_CMD dnf -y install --nogpgcheck \
               --allowerasing \
               --repofrompath libmodulemd-travis,$arch \
               $arch/python3-libmodulemd*.rpm \
               $arch/libmodulemd-devel*.rpm

# Also install the python2-libmodulemd if it was built for this release
# the ||: at the end instructs bash to consider this a pass either way.
dnf -y install --nogpgcheck \
               --allowerasing \
               --repofrompath libmodulemd-travis,$arch \
               $arch/python2-libmodulemd*.rpm ||:
popd #build_rpm

meson --buildtype=release \
      -Dtest_installed_lib=true \
      installed_lib_tests

# Run the tests against the installed RPMs
meson test --suite ci \
           -C installed_lib_tests \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 5

popd #builddir
