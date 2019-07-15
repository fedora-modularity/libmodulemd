#!/bin/bash
# This file is part of libmodulemd
# Copyright (C) 2017-2018 Stephen Gallagher
#
# Fedora-License-Identifier: MIT
# SPDX-2.0-License-Identifier: MIT
# SPDX-3.0-License-Identifier: MIT
#
# This program is free software.
# For more information on the license, see COPYING.
# For more information on free software, see
# <https://www.gnu.org/philosophy/free-sw.en.html>.


set -e

tempdir=`mktemp -d`
pushd $tempdir

for arg; do
  header=`basename $arg`
  cat << EOF > $header.c
#include "$header"

int main (int argc, char **argv)
{
    return 0;
}
EOF

  cat $header.c

  echo "gcc \`pkg-config --cflags gobject-2.0\` \
            \`pkg-config --cflags yaml-0.1\` \
            -I `dirname $arg` \
            -o $header.out \
            $header.c"
  gcc `pkg-config --cflags gobject-2.0` \
      `pkg-config --cflags yaml-0.1` \
      -I `dirname $arg` \
      -o $header.out \
      $header.c

  echo "g++ \`pkg-config --cflags gobject-2.0\` \
            \`pkg-config --cflags yaml-0.1\` \
            -I `dirname $arg` \
            -o $header.out \
            $header.c"
  g++ `pkg-config --cflags gobject-2.0` \
      `pkg-config --cflags yaml-0.1` \
      -I `dirname $arg` \
      -o $header.out \
      $header.c
done

popd
rm -Rf $tempdir