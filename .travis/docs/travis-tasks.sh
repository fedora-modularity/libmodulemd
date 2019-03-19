#!/bin/bash

#Exit on failures
set -e

set -x

os_name=Fedora}
release=rawhide}

pushd /builddir/

# Build the v1 and v2 code under GCC and run standard tests
meson --buildtype=debug \
      -Dbuild_api_v1=false \
      -Dbuild_api_v2=true \
      $COMMON_MESON_ARGS \
      doc-generation

set +e
ninja -C doc-generation modulemd-2.0-doc
err=$?
if [ $err != 0 ]; then
    cat doc-generation/meson-logs/testlog.txt
    exit $err
fi
set -e

git clone https://sgallagher:$DOC_TOKEN@github.com/fedora-modularity/fedora-modularity.github.io
rsync -avh --delete-before --no-perms --omit-dir-times /builddir/doc-generation/modulemd/v2/html/* fedora-modularity.github.io/libmodulemd/latest

pushd fedora-modularity.github.io

git add libmodulemd/latest

# Check to see if there are any changes
set +e
git commit -m "Updating libmodulemd docs for $TRAVIS_COMMIT" --dry-run
err=$?
if [ $err = 0 ]; then
    set -e
    git config user.name "Travis CI"
    git config user.email "sgallagh@redhat.com"
    git commit -m "Updating libmodulemd docs for $TRAVIS_COMMIT"
    git push origin master
fi
set -e

popd #fedora-modularity.github.io

popd #builddir