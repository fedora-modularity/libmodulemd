#!/bin/bash

#Exit on failures
set -e
set -x

COMMON_MESON_ARGS="-Dtest_dirty_git=false -Ddeveloper_build=false -Dskip_clang_tidy=false -Dwith_docs=false"

pushd /builddir/

# Build the code under LLVM/clang and run standard tests
CC=clang CXX=clang++ meson --buildtype=debug \
      $COMMON_MESON_ARGS \
      travis

# (tpg) skip valgrind tests taking too much time
MMD_SKIP_VALGRIND=True ninja -C travis test

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
          travis_scanbuild

    pushd travis_scanbuild
    /builddir/.travis/scanbuild.sh
    popd #travis_scanbuild
fi
