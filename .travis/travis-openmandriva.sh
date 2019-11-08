#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

source $SCRIPT_DIR/travis-common.inc

set -e
set -x

JOB_NAME=${TRAVIS_JOB_NAME:-OpenMandriva Lx cooker }

arr=($JOB_NAME)
release=${arr[1]:-cooker}

mmd_run_docker_tests \
    os=openmandriva \
    release=$release \
    repository=docker.io

