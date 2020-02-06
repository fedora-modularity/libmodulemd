#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/travis-common.inc

set -e
set -x

function docs_finalize {
    exitcode=$?

    # Make sure to delete the Dockerfile.deps from fedora
    rm -f $SCRIPT_DIR/$MMD_OS/Dockerfile.deps.$MMD_RELEASE

    common_finalize

    return $exitcode
}

trap docs_finalize EXIT


# Always generate the docs on Fedora Rawhide
MMD_OS=fedora
MMD_RELEASE=$($SCRIPT_DIR/get_rawhide_version.py)
MMD_IMAGE=fedora/fedora:${MMD_RELEASE}-$(uname -m)
repository="quay.io"

# Create an archive of the current checkout
MMD_TARBALL_PATH=`mktemp -p $SCRIPT_DIR tarball-XXXXXX.tar.bz2`
TARBALL=`basename $MMD_TARBALL_PATH`

pushd $SCRIPT_DIR/..
git ls-files |xargs tar cfj $MMD_TARBALL_PATH .git
popd

sed -e "s#@IMAGE@#$repository/${MMD_IMAGE}#" \
    $SCRIPT_DIR/fedora/Dockerfile.deps.tmpl > $SCRIPT_DIR/fedora/Dockerfile.deps.$MMD_RELEASE

$RETRY_CMD sudo docker build \
    -f $SCRIPT_DIR/$MMD_OS/Dockerfile.deps.$MMD_RELEASE \
    -t fedora-modularity/libmodulemd-deps-$MMD_OS:$MMD_RELEASE .

$RETRY_CMD sudo docker build \
    -f $SCRIPT_DIR/docs/Dockerfile \
    -t fedora-modularity/libmodulemd-docs-$MMD_OS:$MMD_RELEASE \
    --build-arg TARBALL=$TARBALL .


# Override the standard tasks with the doc-generation
docker run \
    -e TRAVIS=$TRAVIS \
    -e TRAVIS_COMMIT="$TRAVIS_COMMIT" \
    -e DOC_TOKEN="$DOC_TOKEN" \
    --rm fedora-modularity/libmodulemd-docs-$MMD_OS:$MMD_RELEASE

popd

