#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/travis-common.inc

set -e
set -x

function coverity_finalize {
    exitcode=$?

    # Make sure to delete the Dockerfile.deps from fedora
    rm -f $SCRIPT_DIR/$MMD_OS/Dockerfile.deps.$MMD_RELEASE

    common_finalize

    return $exitcode
}

trap coverity_finalize EXIT

# Always run the Coverity scan on the oldest supported Fedora
# because it generally lags behind with GCC compatibility.
MMD_OS=fedora
MMD_RELEASE=30
MMD_IMAGE=fedora/fedora:${MMD_RELEASE}-$(uname -m)
repository="quay.io"

# Create an archive of the current checkout
MMD_TARBALL_PATH=`mktemp -p $SCRIPT_DIR tarball-XXXXXX.tar.bz2`
TARBALL=`basename $MMD_TARBALL_PATH`

pushd $SCRIPT_DIR/..
git ls-files |xargs tar cfj $MMD_TARBALL_PATH .git
popd

sed -e "s#@IMAGE@#$repository/${MMD_IMAGE}#" \
    $SCRIPT_DIR/fedora/Dockerfile.deps.tmpl > $SCRIPT_DIR/coverity/Dockerfile.deps.$MMD_RELEASE

sed -e "s#@RELEASE@#${MMD_RELEASE}#" $SCRIPT_DIR/coverity/Dockerfile.tmpl \
    | m4 -D_RELEASE_=$release \
    > $SCRIPT_DIR/coverity/Dockerfile-$MMD_RELEASE

$RETRY_CMD $MMD_BUILDAH $MMD_LAYERS_TRUE \
    -f $SCRIPT_DIR/coverity/Dockerfile.deps.$MMD_RELEASE \
    -t fedora-modularity/libmodulemd-deps-$MMD_OS:$MMD_RELEASE .

$RETRY_CMD $MMD_BUILDAH $MMD_LAYERS_FALSE \
    -f $SCRIPT_DIR/coverity/Dockerfile-$MMD_RELEASE \
    -t fedora-modularity/libmodulemd-coverity \
    --build-arg TARBALL=$TARBALL .

rm -f $MMD_TARBALL_PATH $SCRIPT_DIR/fedora/Dockerfile.deps.$MMD_RELEASE $SCRIPT_DIR/fedora/Dockerfile-$MMD_RELEASE

# Override the standard tasks with the Coverity scan
$RETRY_CMD $MMD_OCI run \
    -e COVERITY_SCAN_TOKEN=$COVERITY_SCAN_TOKEN \
    -e TRAVIS=$TRAVIS \
    -e TRAVIS_COMMIT="$TRAVIS_COMMIT" \
    --rm fedora-modularity/libmodulemd-coverity

popd

