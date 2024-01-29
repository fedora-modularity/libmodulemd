#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/ci-common.inc

set -e
set -x

modulemd_version=${1:-latest}
os=fedora
# glib-2.79.0 available since Fedora ≥ 40 changed a documentation format
# references in libmodulemd documentation do not resolve to glib
# documentation. Use older Fedora release to have an on-line documentation
# with the hyperlinks. See
# <https://github.com/fedora-modularity/libmodulemd/pull/612>.
#release=latest
release=39
repository=quay.io
image=fedora/fedora:${release}

# Override the standard tests with the doc generation template
mmd_run_docker_tests \
    os=$os \
    release=$release \
    repository=$repository \
    image=$image \
    test_template="docs/Dockerfile.tmpl" \
    test_image="libmodulemd-docs-$os:$release" \
    oci_extra_args="
        --env MODULEMD_VERSION=$modulemd_version
        --volume=$GITHUB_WORKSPACE:/builddir
    "

popd # $SCRIPT_DIR 
