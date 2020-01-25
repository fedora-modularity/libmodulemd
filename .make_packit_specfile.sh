#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

pushd $SCRIPT_DIR

# The first version pattern appearing in meson.build must always be libmodulemd's version
MODULEMD_VERSION=$(grep -E -o "[[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+" meson.build |head -n1)

./spec_tmpl.sh $MODULEMD_VERSION libmodulemd.spec.in > libmodulemd.spec

popd
