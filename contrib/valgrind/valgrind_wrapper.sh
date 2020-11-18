#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set -e
set -x

valgrind \
    --error-exitcode=1 \
    --errors-for-leak-kinds=definite \
    --leak-check=full \
    --show-leak-kinds=definite \
    --suppressions=/usr/share/glib-2.0/valgrind/glib.supp \
    --suppressions=$SCRIPT_DIR/libmodulemd-python.supp \
    "$@"
