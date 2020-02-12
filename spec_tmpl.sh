#!/bin/bash

function main {
    local template version release
    local "${@}"

    datetime=$(date +%Y%m%d%H%M%S)
    release=${release:-0.$datetime%\{?dist\}}

    sed -e "s/@VERSION@/$version/" \
        -e "s/@RELEASE@/$release/" $template
}

main "${@}"
