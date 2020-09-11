#!/usr/bin/bash

set -x

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $MESON_BUILD_ROOT

function error_out {
    local code message
    local "${@}"

    echo $message 1>&2
    exit $code
}

function common_finalize {
    exitcode=$?

    return $exitcode
}

trap common_finalize EXIT

jq --version >/dev/null || error_out code=127 message="Install 'jq' to use this script"

# Get the previous tag for this branch
OLDTAG=$(git describe --first-parent --abbrev=0)

# Get the version that will be tagged
NEWVERSION=$(meson introspect $MESON_BUILD_ROOT --projectinfo |jq -r .version)
NEWTAG=libmodulemd-$NEWVERSION

if [ "$NEWTAG" = "$OLDTAG" ]; then
    error_out code=2 message="Version is already tagged. Update meson.build with the new version."
fi

