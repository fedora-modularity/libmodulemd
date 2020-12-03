#!/usr/bin/env bash
# This file is part of libmodulemd
# Copyright (C) 2018 Stephen Gallagher
#
# Fedora-License-Identifier: MIT
# SPDX-2.0-License-Identifier: MIT
# SPDX-3.0-License-Identifier: MIT
#
# This program is free software.
# For more information on the license, see COPYING.
# For more information on free software, see
# <https://www.gnu.org/philosophy/free-sw.en.html>.

ninja scan-build
if [ $? -eq 0 ]; then
  exit 0
else
  elinks -dump meson-logs/scanbuild/*/index.html
  exit 1
fi
