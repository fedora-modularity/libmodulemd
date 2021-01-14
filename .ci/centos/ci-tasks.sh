#!/usr/bin/env bash

#Exit on failures
set -e
set -x

pushd /builddir/

# Build the code under GCC and run standard tests
meson --buildtype=debugoptimized \
      -Dpython_name=python3.6 \
      ci

ninja-build -C ci test

popd #builddir
