#!/usr/bin/sh

version=$1
template=$2
datetime=$(date +%Y%m%d%H%M%S)

sed -e "s/@VERSION@/$version/" \
    -e "s/@DATETIME@/$datetime/" $template
