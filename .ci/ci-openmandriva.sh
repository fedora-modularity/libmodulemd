#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

source $SCRIPT_DIR/ci-common.inc

set -e
set -x

release=${1:-cooker}

mmd_run_docker_tests \
    os=openmandriva \
    release=$release \
    repository=docker.io \
    image=openmandriva/$release
