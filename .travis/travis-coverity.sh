#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

set -e
set -x

JOB_NAME=${TRAVIS_JOB_NAME:-Fedora rawhide}

# Always run the Coverity scan on Fedora Rawhide
os_name=Fedora

# Temporarily switch to 30 since it's breaking on Rawhide
# Make sure to change this in coverity/Dockerfile as well
release=30


# Create an archive of the current checkout
TARBALL_PATH=`mktemp -p $SCRIPT_DIR tarball-XXXXXX.tar.bz2`
TARBALL=`basename $TARBALL_PATH`

pushd $SCRIPT_DIR/..
git ls-files |xargs tar cfj $TARBALL_PATH .git
popd

repository="registry.fedoraproject.org"
os="fedora"

sed -e "s/@IMAGE@/$repository\/$os:$release/" \
    $SCRIPT_DIR/fedora/Dockerfile.deps.tmpl > $SCRIPT_DIR/fedora/Dockerfile.deps.$release

sudo docker build \
    -f $SCRIPT_DIR/fedora/Dockerfile.deps.$release \
    -t fedora-modularity/libmodulemd-deps-$release .

sudo docker build \
    -f $SCRIPT_DIR/coverity/Dockerfile \
    -t fedora-modularity/libmodulemd-coverity \
    --build-arg TARBALL=$TARBALL .

rm -f $TARBALL_PATH $SCRIPT_DIR/fedora/Dockerfile.deps.$release $SCRIPT_DIR/fedora/Dockerfile-$release

# Override the standard tasks with the Coverity scan
docker run \
    -e COVERITY_SCAN_TOKEN=$COVERITY_SCAN_TOKEN \
    -e TRAVIS=$TRAVIS \
    -e TRAVIS_JOB_NAME="$TRAVIS_JOB_NAME" \
    -e TRAVIS_COMMIT="$TRAVIS_COMMIT" \
    --rm fedora-modularity/libmodulemd-coverity

popd
exit 0
