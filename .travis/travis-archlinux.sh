#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

set -e
set -x

JOB_NAME=${TRAVIS_JOB_NAME:-Arch Linux}

os_name='Arch Linux'
release=base

# Create an archive of the current checkout
TARBALL_PATH=`mktemp -p $SCRIPT_DIR tarball-XXXXXX.tar.bz2`
TARBALL=`basename $TARBALL_PATH`

pushd $SCRIPT_DIR/..
git ls-files |xargs tar cfj $TARBALL_PATH .git
popd

repository="docker.io"
os="archlinux"

sed -e "s/@IMAGE@/$repository\/$os\/$release/" \
    $SCRIPT_DIR/archlinux/Dockerfile.deps.tmpl > $SCRIPT_DIR/archlinux/Dockerfile.deps.$release
sed -e "s/@RELEASE@/$release/" $SCRIPT_DIR/archlinux/Dockerfile.tmpl > $SCRIPT_DIR/archlinux/Dockerfile-$release

sudo docker build -f $SCRIPT_DIR/archlinux/Dockerfile.deps.$release -t fedora-modularity/libmodulemd-deps-$release .
sudo docker build -f $SCRIPT_DIR/archlinux/Dockerfile-$release -t fedora-modularity/libmodulemd:$release --build-arg TARBALL=$TARBALL .

rm -f $TARBALL_PATH $SCRIPT_DIR/archlinux/Dockerfile.deps.$release $SCRIPT_DIR/archlinux/Dockerfile-$release

docker run -e TRAVIS=$TRAVIS -eTRAVIS_JOB_NAME="$TRAVIS_JOB_NAME" --rm fedora-modularity/libmodulemd:$release
popd
exit 0
