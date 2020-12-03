#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/travis-common.inc

set -e
set -x


# There is only one release of archlinux since it's a rolling release
# distribution, so we can hard-code these values.
mmd_run_docker_tests \
    os=archlinux \
    release=base \
    repository=docker.io \
    image=archlinux/base

popd # $SCRIPT_DIR
