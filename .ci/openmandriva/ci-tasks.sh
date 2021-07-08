#!/usr/bin/env bash

#Exit on failures
set -e
set -x

PROCESSORS=$(/usr/bin/getconf _NPROCESSORS_ONLN)
COMMON_MESON_ARGS="-Dwith_docs=false"

pushd /builddir/

# Build the code under LLVM/clang and run standard tests
CC=clang CXX=clang++ meson --buildtype=debugoptimized \
      $COMMON_MESON_ARGS \
      ci

meson test --suite ci \
           -C ci \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 5

# Disable the verbose tests and run the supported tests through valgrind
CC=clang CXX=clang++ meson --buildtype=debugoptimized \
      -Dverbose_tests=false \
      $COMMON_MESON_ARGS \
      ci-valgrind

meson test --suite ci_valgrind \
           --wrap=/builddir/contrib/valgrind/valgrind_wrapper.sh \
           -C ci-valgrind \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 10

# Test the code with clang-analyzer
CC=clang CXX=clang++ meson --buildtype=debug \
          -Dskip_introspection=true \
          -Dwith_py3=false \
          $COMMON_MESON_ARGS \
          ci_scanbuild

pushd ci_scanbuild
/builddir/.ci/scanbuild.sh
popd #ci_scanbuild
