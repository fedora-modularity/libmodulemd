#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/ci-common.inc

set -e
set -x

release=${1:-7}

mmd_run_docker_tests \
    os=mageia \
    release=$release \
    repository=docker.io

popd # $SCRIPT_DIR
