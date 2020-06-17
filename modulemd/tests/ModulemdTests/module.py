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

        self.assertEqual(len(module.search_streams("8", 0)), 1)
        self.assertEqual(len(module.search_streams("10", 0)), 1)

    def test_copy_with_obsoletes(self):
        idx = Modulemd.ModuleIndex.new()
        e = Modulemd.Obsoletes.new(1, 2, "testmodule", "teststream", "testmsg")
        e.set_obsoleted_by("module_obsoleter", "stream_obsoleter")
        idx.add_obsoletes(e)

        m = idx.get_module("testmodule")
        assert m
        assert m.get_module_name() == "testmodule"
        obsoletes_from_orig = m.get_newest_active_obsoletes("teststream", None)
        assert (
            obsoletes_from_orig.get_obsoleted_by_module_name()
            == "module_obsoleter"
        )

        m_copy = m.copy()
        assert m_copy.get_module_name() == "testmodule"
        obsoletes_from_copy = m_copy.get_newest_active_obsoletes(
            "teststream", None
        )
        assert (
            obsoletes_from_copy.get_obsoleted_by_module_name()
            == "module_obsoleter"
        )

    def test_adding_obsoletes_is_order_independent(self):
        obsoletes_without_context = """
---
document: modulemd-obsoletes
version: 1
data:
  module: nodejs
  stream: 10
  context: deadbeef
  modified: 2019-07-27T00:00Z
  message: test message
  obsoleted_by:
    module: nodejs
    stream: 12
...
"""
        obsoletes_with_context = """
---
document: modulemd-obsoletes
version: 1
data:
  module: nodejs
  stream: 10
  modified: 2019-09-27T00:00Z
  message: test message
  obsoleted_by:
    module: nodejs
    stream: 14
...
"""

        for ordered_yaml in [
            obsoletes_without_context + obsoletes_with_context,
            obsoletes_with_context + obsoletes_without_context,
        ]:
            idx = ModuleIndex.new()
            stream = Modulemd.ModuleStream.new(2, "nodejs", "10")
            stream.props.context = "deadbeef"
            res = idx.add_module_stream(stream)
            res, failures = idx.update_from_string(ordered_yaml, True)
            m = idx.get_module("nodejs")
            streams = m.get_all_streams()
            s = streams[0]
            assert (
                s.get_obsoletes_resolved().get_obsoleted_by_module_stream()
                == "14"
            )


if __name__ == "__main__":
    unittest.main()
