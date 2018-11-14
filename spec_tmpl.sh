#!/usr/bin/sh

version=$1
libmodulemd_v1_version=$2
template=$3
datetime=$(date +%Y%m%d%H%M%S)

sed -e "s/@VERSION@/$version/" \
    -e "s/@V1_VERSION@/$libmodulemd_v1_version/" \
    -e "s/@DATETIME@/$datetime/" $template
