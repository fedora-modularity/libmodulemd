#!/bin/bash

#Exit on failures
set -e
set -x

PROCESSORS=$(/usr/bin/getconf _NPROCESSORS_ONLN)
COMMON_MESON_ARGS="-Dtest_dirty_git=false -Ddeveloper_build=false -Dwith_docs=false"

pushd /builddir/

# Build the code under LLVM/clang and run standard tests
CC=clang CXX=clang++ meson --buildtype=debugoptimized \
      -Dverbose_tests=false \
      $COMMON_MESON_ARGS \
      ci

meson test --suite ci \
           -C ci \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 5

meson test --suite ci_valgrind \
           --wrap=/builddir/contrib/valgrind/valgrind_wrapper.sh \
           -C ci \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 10

# Test the code with clang-analyzer
# This requires meson 0.49.0 or later
set +e
rpmdev-vercmp $(meson --version) 0.49.0
if [ $? -eq 12 ]; then
    # Meson was older than 0.49.0, skip this step
    printf '%s\n' 'Meson is too old to run scan-build'
    set -e
else
    set -e
CC=clang CXX=clang++ meson --buildtype=debug \
          -Dskip_introspection=true \
          $COMMON_MESON_ARGS \
          ci_scanbuild

    pushd ci_scanbuild
    /builddir/.ci/scanbuild.sh
    popd #ci_scanbuild
fi
