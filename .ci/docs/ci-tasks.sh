#!/usr/bin/env bash

#Exit on failures
set -e
set -x

RETRY_CMD=/builddir/.ci/retry-command.sh

pushd /builddir/

# Build the code under GCC and run documentation generation
meson --buildtype=debug \
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

MODULEMD_VERSION=${MODULEMD_VERSION:-latest}

mkdir -p /builddir/fedora-modularity.github.io/libmodulemd/$MODULEMD_VERSION
rsync -avh --delete-before --no-perms --omit-dir-times \
    /builddir/doc-generation/modulemd/html/* \
    /builddir/fedora-modularity.github.io/libmodulemd/$MODULEMD_VERSION

popd #builddir
