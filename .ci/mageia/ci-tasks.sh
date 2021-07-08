#!/usr/bin/env bash

#Exit on failures
set -e
set -x

PROCESSORS=$(/usr/bin/getconf _NPROCESSORS_ONLN)


pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debugoptimized \
      ci

meson test --suite ci \
           -C ci \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 5

meson --buildtype=debugoptimized \
      -Dverbose_tests=false \
      ci-valgrind
meson test --suite ci_valgrind \
           --wrap=/builddir/contrib/valgrind/valgrind_wrapper.sh \
           -C ci-valgrind \
           --num-processes=$PROCESSORS \
           --print-errorlogs \
           -t 10

# Test the code with clang-analyzer
meson --buildtype=debug \
      -Dskip_introspection=true \
      -Dwith_py3=false \
      $COMMON_MESON_ARGS \
      ci_scanbuild

pushd ci_scanbuild
/builddir/.ci/scanbuild.sh
popd #ci_scanbuild

