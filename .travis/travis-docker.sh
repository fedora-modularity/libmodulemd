#!/bin/bash

set -e
set -x

JOB_NAME=${TRAVIS_JOB_NAME:-Fedora rawhide}

arr=($JOB_NAME)
os_name=${arr[0]:-Fedora}
release=${arr[1]:-rawhide}

# Create an archive of the current checkout
TARBALL=`mktemp -p . tarball-XXXXXX.tar.bz2`
git ls-files |xargs tar cfj $TARBALL .git

if [ $os_name = "Fedora" ]; then
    repository="registry.fedoraproject.org"
    os="fedora"
    pkgmgr="dnf"
    extra_pre=""
else
    repository="registry.centos.org"
    os="centos"
    pkgmgr="yum"
    extra_pre="RUN yum -y --setopt=install_weak_deps=False install epel-release"
fi

sed -e "s/@IMAGE@/$repository\/$os:$release/" \
    -e "s/@PKGMGR@/$pkgmgr/" \
    -e "s/@EXTRA_PRE_STEP@/$extra_pre/" \
    .travis/Dockerfile.deps.tmpl > .travis/Dockerfile.deps.$release
sed -e "s/@RELEASE@/$release/" .travis/Dockerfile.tmpl > .travis/Dockerfile-$release

sudo docker build -f .travis/Dockerfile.deps.$release -t fedora-modularity/libmodulemd-deps-$release .
sudo docker build -f .travis/Dockerfile-$release -t fedora-modularity/libmodulemd:$release --build-arg TARBALL=$TARBALL .

if [ $release != "rawhide" ]; then
  # Only run Coverity scan on Rawhide since we have a limited number of scans per week.
  unset COVERITY_SCAN_TOKEN
fi

docker run -e COVERITY_SCAN_TOKEN=$COVERITY_SCAN_TOKEN -e TRAVIS=$TRAVIS -eTRAVIS_JOB_NAME="$TRAVIS_JOB_NAME" --rm fedora-modularity/libmodulemd:$release

rm -f $TARBALL .travis/Dockerfile.deps.$release .travis/Dockerfile-$release

exit 0
