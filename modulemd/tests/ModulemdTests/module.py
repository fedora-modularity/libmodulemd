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

from os import path
import sys

try:
    import unittest
    import gi

    gi.require_version("Modulemd", "2.0")
    from gi.repository import Modulemd
    from gi.repository.Modulemd import ModuleIndex
    from gi.repository import GLib
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase


class TestModule(TestBase):
    def test_search_streams(self):
        idx = Modulemd.ModuleIndex.new()
        idx.update_from_file(path.join(self.test_data_path, "f29.yaml"), True)
        module = idx.get_module("nodejs")

        self.assertEquals(len(module.search_streams("8", 0)), 1)
        self.assertEquals(len(module.search_streams("10", 0)), 1)


if __name__ == "__main__":
    unittest.main()
