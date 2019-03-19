FROM fedora-modularity/libmodulemd-deps-rawhide

MAINTAINER Stephen Gallagher <sgallagh@redhat.com>

RUN dnf -y --setopt=install_weak_deps=False install rsync \
    && dnf -y clean all

ARG TARBALL

ADD $TARBALL /builddir/

ENTRYPOINT /builddir/.travis/docs/travis-tasks.sh
