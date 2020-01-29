#!/bin/bash

function retry_command {
    local usage="Usage: ${FUNCNAME[0]} [-n numtries] [-d delay]"
    local OPTIND OPTION
    local numtries=3 delay=2

    while getopts ":n:d:" OPTION; do
        case "${OPTION}" in
        n)
            numtries=${OPTARG}
            ;;
        d)
            delay=${OPTARG}
            ;;
        *)
            echo "$usage" 1>&2
            return 1
            ;;
        esac
    done
    shift $((OPTIND-1))

    exitcode=0
    while (( numtries > 0 )) ; do
        eval "$@"
        exitcode=$?
        (( exitcode == 0 )) && break
        (( --numtries > 0 )) && sleep $delay
    done

    return $exitcode
}

retry_command "$@"
