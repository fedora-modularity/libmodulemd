#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

pushd $SCRIPT_DIR

./.travis/travis-docker-build.sh
sudo docker run -e DIRTY_REPO_CHECK=false --rm fedora-modularity/libmodulemd

popd
