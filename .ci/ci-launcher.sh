#!/usr/bin/env bash

set -e

SOURCE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

arr=(${DISTRO:-Fedora rawhide})
os=${arr[0]:-Fedora}
release=${arr[1]:-rawhide}

case $os in

  Fedora)
    $SOURCE_DIR/ci-fedora.sh $release
    ;;

  CentOS)
    $SOURCE_DIR/ci-centos.sh $release
    ;;

  Archlinux)
    $SOURCE_DIR/ci-archlinux.sh
    ;;

  Mageia)
    $SOURCE_DIR/ci-mageia.sh $release
    ;;

  OpenMandriva)
    $SOURCE_DIR/ci-openmandriva.sh $release
    ;;

  openSUSE)
    $SOURCE_DIR/ci-opensuse.sh $release
    ;;

  *)
    echo "Unknown OS: $os"
    exit 1
    ;;

esac
