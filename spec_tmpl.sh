#!/usr/bin/sh

version=$1
template=$2
datetime=$(date +%Y%m%d%H%M%S)

sed -e "s/@VERSION@/$1/" -e "s/@DATETIME@/$datetime/" $2
