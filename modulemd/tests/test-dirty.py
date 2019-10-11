#!/usr/bin/python3

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

import os
import sys
import git
import subprocess

script_dir = os.path.dirname(os.path.realpath(__file__))

# Get the repo we're running in
repo = git.Repo(script_dir, search_parent_directories=True)

# When running in CI, the only reason the git repo could
# become "dirty" (files differ from their checkout) is if
# the autoformatter made changes. This should be reported
# back so the submitter can fix this.
if repo.is_dirty():
    print(
        "Autoformatter was not run before submitting. Please run "
        "`ninja test`, amend the commit and resubmit this pull request."
    )
    res = subprocess.run(["git", "diff"], capture_output=True, text=True)
    print(res.stdout, file=sys.stderr)
    sys.exit(os.EX_USAGE)

sys.exit(os.EX_OK)
