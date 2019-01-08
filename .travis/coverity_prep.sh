#!/usr/bin/bash

echo -n | openssl s_client -connect scan.coverity.com:443 | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' | sudo tee -a /etc/ssl/certs/ca-

curl -s https://scan.coverity.com/scripts/travisci_build_coverity_scan.sh -o /usr/bin/travisci_build_coverity_scan.sh

if [ "$(file /usr/bin/travisci_build_coverity_scan.sh)" == "text/x-shellscript" ]; then
    chmod a+x /usr/bin/travisci_build_coverity_scan.sh
else
    echo "Warning: Coverity not detected!"
    # Disable the scanning tool
    ln -sf /usr/bin/true /usr/bin/travisci_build_coverity_scan.sh
fi
