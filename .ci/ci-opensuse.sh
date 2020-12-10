#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/ci-common.inc

set -e
set -x

release=${1:-tumbleweed}

mmd_run_docker_tests \
    os=opensuse \
    release=$release \
    repository=registry.opensuse.org \
    image=opensuse/$release

popd # $SCRIPT_DIR
