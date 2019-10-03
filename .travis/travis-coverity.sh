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

# Always run the Coverity scan on Fedora Rawhide
MMD_OS=fedora

# Temporarily switch to 30 since it's breaking on Rawhide
# Make sure to change this in coverity/Dockerfile as well
MMD_RELEASE=30

repository="registry.fedoraproject.org"

# Create an archive of the current checkout
MMD_TARBALL_PATH=`mktemp -p $SCRIPT_DIR tarball-XXXXXX.tar.bz2`
TARBALL=`basename $TARBALL_PATH`

pushd $SCRIPT_DIR/..
git ls-files |xargs tar cfj $TARBALL_PATH .git
popd

sed -e "s/@IMAGE@/$repository\/$MMD_OS:$MMD_RELEASE/" \
    $SCRIPT_DIR/fedora/Dockerfile.deps.tmpl > $SCRIPT_DIR/fedora/Dockerfile.deps.$MMD_RELEASE

sudo docker build \
    -f $SCRIPT_DIR/fedora/Dockerfile.deps.$MMD_RELEASE \
    -t fedora-modularity/libmodulemd-deps-$MMD_OS:$MMD_RELEASE .

sudo docker build \
    -f $SCRIPT_DIR/coverity/Dockerfile \
    -t fedora-modularity/libmodulemd-coverity \
    --build-arg TARBALL=$TARBALL .

rm -f $TARBALL_PATH $SCRIPT_DIR/fedora/Dockerfile.deps.$MMD_RELEASE $SCRIPT_DIR/fedora/Dockerfile-$MMD_RELEASE

# Override the standard tasks with the Coverity scan
docker run \
    -e COVERITY_SCAN_TOKEN=$COVERITY_SCAN_TOKEN \
    -e TRAVIS=$TRAVIS \
    -e TRAVIS_COMMIT="$TRAVIS_COMMIT" \
    --rm fedora-modularity/libmodulemd-coverity

popd

