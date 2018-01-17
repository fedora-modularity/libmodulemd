#!/usr/bin/bash

set -e

# Create an archive of the current checkout
TARBALL=`mktemp -p . tarball-XXXXXX.tar.bz2`
git ls-files |xargs tar cfj $TARBALL .git

# The coverity scan script returns an error despite succeeding...
sudo docker build -t fedora-modularity/libmodulemd --build-arg TARBALL=$TARBALL .

rm -f $TARBALL

exit 0
