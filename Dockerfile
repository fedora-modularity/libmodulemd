FROM fedora:rawhide

MAINTAINER Stephen Gallagher <sgallagh@redhat.com>

ARG TARBALL

RUN dnf -y --setopt=install_weak_deps=False install git-core meson gcc ninja-build wget curl openssl popt-devel sudo pkgconf glib2-devel gobject-introspection-devel python3-gobject-base libyaml-devel gtk-doc redhat-rpm-config ruby rubygems && dnf -y clean all

ADD $TARBALL /builddir/

ENTRYPOINT /builddir/.travis/travis-tasks.sh
