#!/bin/bash

#Exit on failures
set -e
set -x

RETRY_CMD=/builddir/.ci/retry-command.sh

pushd /builddir/

# Build the code under GCC and run documentation generation
meson --buildtype=debug \
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

# Fix external references for publishing on the web
pushd doc-generation/modulemd/html
/builddir/contrib/doc-tools/fix-xref.sh
popd

$RETRY_CMD git clone https://sgallagher:$DOC_TOKEN@github.com/fedora-modularity/fedora-modularity.github.io
rsync -avh --delete-before --no-perms --omit-dir-times /builddir/doc-generation/modulemd/html/* fedora-modularity.github.io/libmodulemd/latest

pushd fedora-modularity.github.io

git add libmodulemd/latest

# Check to see if there are any changes
set +e
git commit -m "Updating libmodulemd docs for $GITHUB_SHA" --dry-run
err=$?
if [ $err = 0 ]; then
    set -e
    git config user.name "Libmodulemd CI"
    git config user.email "sgallagh@redhat.com"
    git commit -m "Updating libmodulemd docs for $GITHUB_SHA"
    $RETRY_CMD git push origin master
fi
set -e

popd #fedora-modularity.github.io

popd #builddir
