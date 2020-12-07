#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/ci-common.inc

set -e
set -x

release=${1:-rawhide}

if [ $release = rawhide ]; then
    release=$($SCRIPT_DIR/get_rawhide_version.py)
fi

mmd_run_docker_tests \
    os=fedora \
    release=$release \
    repository=quay.io \
    image=fedora/fedora:$release-$(uname -m)

popd # $SCRIPT_DIR
