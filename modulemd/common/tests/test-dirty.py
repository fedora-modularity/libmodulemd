#!/usr/bin/python3

import os
import sys
import git

script_dir = os.path.dirname(os.path.realpath(__file__))

# Get the repo we're running in
repo = git.Repo(script_dir,
                search_parent_directories=True)

# When running in CI, the only reason the git repo could
# become "dirty" (files differ from their checkout) is if
# the autoformatted made changes. This should be reported
# back so the submitter can fix this.
if (repo.is_dirty()):
    print("Autoformatter was not run before submitting. Please run "
          "`ninja test`, amend the commit and resubmit this pull request.")
    sys.exit(os.EX_USAGE)

sys.exit(os.EX_OK)
