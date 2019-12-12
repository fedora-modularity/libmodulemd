#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/travis-common.inc

set -e
set -x

JOB_NAME=${TRAVIS_JOB_NAME:-Fedora rawhide}

arr=($JOB_NAME)
release=${arr[1]:-rawhide}

mmd_run_docker_tests \
    os=fedora \
    release=$release \
    repository=registry.fedoraproject.org \
    use_ccache=true

popd # $SCRIPT_DIR
