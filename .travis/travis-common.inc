#!/bin/bash

set -x

# Global variables to support finalize()
MMD_OS=
MMD_RELEASE=
MMD_TARBALL_PATH=

function common_finalize {
    # If any specific test launcher needs to do its own cleanup as well,
    # it should set the EXIT trap and make sure to also call this functino
    # internally.
    exitcode=$?

    rm -f $MMD_TARBALL_PATH \
          $SCRIPT_DIR/$MMD_OS/Dockerfile.deps.$MMD_RELEASE \
          $SCRIPT_DIR/$MMD_OS/Dockerfile-$MMD_RELEASE

    return $exitcode
}

trap common_finalize EXIT


function mmd_run_docker_tests {
    local os release repository image
    local "${@}"

    if [ -z $SCRIPT_DIR ]; then
        >&2 echo "Programming error: \$SCRIPT_DIR must be set"
        exit 1
    fi

    os=${os-fedora}
    release=${release-rawhide}
    repository=${repository-registry.fedoraproject.org}


    # Lower-case the os and release for the container registry
    MMD_OS=${os,,}
    MMD_RELEASE=${release,,}

    image=${image-$MMD_OS:$MMD_RELEASE}

    # Create an archive of the current checkout
    MMD_TARBALL_PATH=`mktemp -p $SCRIPT_DIR tarball-XXXXXX.tar.bz2`
    TARBALL=`basename $MMD_TARBALL_PATH`

    pushd $SCRIPT_DIR/..
    git ls-files |xargs tar cfj $MMD_TARBALL_PATH .git
    popd

    sed -e "s#@IMAGE@#$repository/$image#" \
        $SCRIPT_DIR/$MMD_OS/Dockerfile.deps.tmpl > $SCRIPT_DIR/$MMD_OS/Dockerfile.deps.$MMD_RELEASE
    sed -e "s/@RELEASE@/$MMD_OS:$MMD_RELEASE/" $SCRIPT_DIR/$MMD_OS/Dockerfile.tmpl > $SCRIPT_DIR/$MMD_OS/Dockerfile-$MMD_RELEASE

    sudo docker build \
        -f $SCRIPT_DIR/$MMD_OS/Dockerfile.deps.$MMD_RELEASE \
        -t fedora-modularity/libmodulemd-deps-$MMD_OS:$MMD_RELEASE .

    sudo docker build \
        -f $SCRIPT_DIR/$MMD_OS/Dockerfile-$MMD_RELEASE \
        -t fedora-modularity/libmodulemd-$MMD_OS:$MMD_RELEASE \
        --build-arg TARBALL=${TARBALL} .

    docker run --rm fedora-modularity/libmodulemd-$MMD_OS:$MMD_RELEASE

    popd

}

set +x
