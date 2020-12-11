#!/usr/bin/env bash

set -e
set -x

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $SCRIPT_DIR

dnf -y --setopt=install_weak_deps=False --setopt=tsflags='' \
       --nogpgcheck --skip-broken --quiet install \
    python3-black \
    clang \
    clang-analyzer \
    clang-tools-extra \
    createrepo_c \
    "libmodulemd >= 2.3" \
    curl \
    elinks \
    file-devel \
    gcc \
    gcc-c++ \
    git-core \
    glib2-devel \
    glib2-doc \
    gobject-introspection-devel \
    gtk-doc \
    help2man \
    jq \
    libyaml-devel \
    meson \
    ninja-build \
    openssl \
    packit \
    pkgconf \
    popt-devel \
    python2-devel \
    python2-six \
    python2-gobject-base \
    python3-autopep8 \
    python3-devel \
    python3-GitPython \
    python3-gobject-base \
    python3-koji \
    python3-pycodestyle \
    python3-rpm-macros \
    redhat-rpm-config \
    rpm-build \
    rpm-devel \
    rpmdevtools \
    ruby \
    "rubygem(json)" \
    rubygems \
    sudo \
    valgrind \
    wget

dnf -y clean all

ln -sf /builddir/bindings/python/gi/overrides/Modulemd.py $(python3 -c "import gi; print(gi._overridesdir)")/Modulemd.py

popd
