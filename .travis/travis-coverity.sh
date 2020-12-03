#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/travis-common.inc

set -e
set -x

os=centos
release=8
repository=docker.io

# Override the standard tests with the Coverity scan
mmd_run_docker_tests \
    os=$os \
    release=$release \
    repository=$repository \
    test_template="coverity/Dockerfile.tmpl" \
    test_image="libmodulemd-coverity" \
    oci_extra_args="
        -e COVERITY_SCAN_TOKEN=$COVERITY_SCAN_TOKEN
        -e TRAVIS=$TRAVIS
        -e TRAVIS_COMMIT='$TRAVIS_COMMIT'
    "

popd # $SCRIPT_DIR
