#!/bin/bash

#Exit on failures
set -e

pushd /builddir/

meson --buildtype=debug travis

ninja -C travis test
cat /builddir/travis/meson-logs/testlog.txt

meson --buildtype=debug coverity
pushd coverity
echo -n | openssl s_client -connect scan.coverity.com:443 | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' | sudo tee -a /etc/ssl/certs/ca-

# The coverity scan script returns an error despite succeeding...
curl -s https://scan.coverity.com/scripts/travisci_build_coverity_scan.sh | \
 TRAVIS_BRANCH="${TRAVIS_BRANCH:-master}" \
 COVERITY_SCAN_PROJECT_NAME="${COVERITY_SCAN_PROJECT_NAME:-sgallagher/libmodulemd}" \
 COVERITY_SCAN_NOTIFICATION_EMAIL="${COVERITY_SCAN_NOTIFICATION_EMAIL:-sgallagh@redhat.com}" \
 COVERITY_SCAN_BUILD_COMMAND="${COVERITY_SCAN_BUILD_COMMAND:-ninja}" \
 COVERITY_SCAN_BRANCH_PATTERN=${COVERITY_SCAN_BRANCH_PATTERN:-master} \
 bash ||:

popd #coverity

popd #builddir
