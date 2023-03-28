#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/ci-common.inc

set -e
set -x

os=fedora
release=latest
repository=quay.io
image=fedora/fedora:${release}

# Override the standard tests with auto-formatting
mmd_run_docker_tests \
    os=$os \
    release=$release \
    repository=$repository \
    image=$image \
    test_template="autoformat/Dockerfile.tmpl" \
    test_image="libmodulemd-autoformat-$os:$release" \
    oci_extra_args="
        -e FORMAT_DEST_DIR=/modulemd-formatted
        --volume '${SCRIPT_DIR}/../modulemd:/modulemd-formatted:Z'
    "

popd # $SCRIPT_DIR
