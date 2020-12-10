#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/ci-common.inc

set -e
set -x

release=${1:-7}
repository=docker.io

mmd_run_docker_tests \
    os=centos \
    release=$release \
    repository=$repository

popd # $SCRIPT_DIR
