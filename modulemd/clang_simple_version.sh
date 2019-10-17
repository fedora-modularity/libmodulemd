#!/usr/bin/sh

$1 --version | awk '/LLVM version/ {print $NF}' -

