#!/bin/bash

#Exit on failures
set -e
set -x

PROCESSORS=$(/usr/bin/getconf _NPROCESSORS_ONLN)
MESON_DIRTY_REPO_ARGS="-Dtest_dirty_git=${DIRTY_REPO_CHECK:-false}"
RETRY_CMD=/builddir/.travis/retry-command.sh

pushd /builddir/

valgrind_cmd='
    valgrind --error-exitcode=1
             --errors-for-leak-kinds=definite
             --leak-check=full
             --show-leak-kinds=definite
             --suppressions=/usr/share/glib-2.0/valgrind/glib.supp
             --suppressions=/builddir/contrib/valgrind/libmodulemd-python.supp
'

# Build the code under GCC and run standard tests
meson --buildtype=debug \
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
           --wrap="$valgrind_cmd" \
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

meson --buildtype=debug \
      $COMMON_MESON_ARGS \
      build_rpm

pushd build_rpm

ninja
./make_rpms.sh

createrepo_c rpmbuild/RPMS/

$RETRY_CMD dnf -y install --nogpgcheck \
               --allowerasing \
               --repofrompath libmodulemd-travis,rpmbuild/RPMS \
               python3-libmodulemd \
               "libmodulemd-devel > 2"

# Also install the python2-libmodulemd if it was built for this release
# the ||: at the end instructs bash to consider this a pass either way.
$RETRY_CMD dnf -y install --nogpgcheck \
               --allowerasing \
               --repofrompath libmodulemd-travis,rpmbuild/RPMS \
               python2-libmodulemd ||:
popd #build_rpm

meson --buildtype=debug \
      -Dtest_installed_lib=true \
      installed_lib_tests

# Run the tests against the installed RPMs
meson test --suite ci \
           -C installed_lib_tests \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 5

popd #builddir
