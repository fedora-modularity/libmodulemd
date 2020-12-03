#!/bin/bash

SOURCE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SCRIPT_DIR="$SOURCE_DIR/.travis"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/travis-common.inc

set -e
set -x

HOME=${HOME-/}

release=${1:-$($SCRIPT_DIR/get_rawhide_version.py)}
image=fedora/fedora:$release-$(uname -m)


mmd_setup_container \
    os=fedora \
    release=$release \
    repository=quay.io \
    image=$image \
    oci_extra_commands='RUN ln -sf /builddir/bindings/python/gi/overrides/Modulemd.py $(python3 -c "import gi; print(gi._overridesdir)")/Modulemd.py' \
    deps_image=libmodulemd-dev-$release


# Create a home directory to log into
homedir=$SCRIPT_DIR/.home_fedora
if [ ! -d $homedir ]; then
    cp -a /etc/skel $homedir
fi

eval $MMD_OCI run \
     --rm \
     --tty \
     --interactive \
     --name libmodulemd-dev-$release \
     --hostname libmodulemd-dev-$release \
     --userns keep-id \
     --volume=$homedir:$HOME:Z \
     --volume=$SOURCE_DIR:/builddir:Z \
     --workdir=/builddir \
     --env HOME=$HOME \
     fedora-modularity/libmodulemd-dev-$release \
     /usr/bin/bash

set +x
