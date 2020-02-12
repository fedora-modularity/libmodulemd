#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

pushd $SCRIPT_DIR

date=$(date +%Y%m%d)
version_desc=$(git describe --tags --match "*.*")
version=$(echo $version_desc | cut -d '-' -f2)
patch_count=$(echo $version_desc | cut -d '-' -f3)
hash=$(echo $version_desc | cut -d '-' -f4)

if [ x$patch_count != x ]; then
    release=0.${date}.${patch_count}git${hash}%{?dist}
else
    release=0.${date}%{?dist}
fi

./spec_tmpl.sh version=$version release=$release template=libmodulemd.spec.in > libmodulemd.spec

popd
