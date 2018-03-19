FROM fedora:rawhide

MAINTAINER Stephen Gallagher <sgallagh@redhat.com>

ARG TARBALL

RUN dnf -y --setopt=install_weak_deps=False install git-core meson gcc ninja-build wget curl openssl popt-devel sudo pkgconf glib2-devel gobject-introspection-devel python3-gobject-base libyaml-devel gtk-doc redhat-rpm-config ruby rubygems "rubygem(json)" && dnf -y clean all

RUN  echo -n | openssl s_client -connect scan.coverity.com:443 | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' | sudo tee -a /etc/ssl/certs/ca- && curl -s https://scan.coverity.com/scripts/travisci_build_coverity_scan.sh -o /usr/bin/travisci_build_coverity_scan.sh && chmod a+x /usr/bin/travisci_build_coverity_scan.sh

ADD $TARBALL /builddir/

ENTRYPOINT /builddir/.travis/travis-tasks.sh
