#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/travis-common.inc

set -e
set -x

os=fedora
release=32
repository=quay.io
image=fedora/fedora:${release}-$(uname -m)

# Override the standard tests with the doc generation template
mmd_run_docker_tests \
    os=$os \
    release=$release \
    repository=$repository \
    image=$image \
    test_template="docs/Dockerfile.tmpl" \
    test_image="libmodulemd-docs-$os:$release" \
    oci_extra_args="
        -e TRAVIS=$TRAVIS
        -e TRAVIS_COMMIT='$TRAVIS_COMMIT'
        -e DOC_TOKEN='$DOC_TOKEN'
    "

popd # $SCRIPT_DIR
