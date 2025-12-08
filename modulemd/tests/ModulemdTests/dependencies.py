#!/usr/bin/python3

# This file is part of libmodulemd
# Copyright (C) 2018 Red Hat, Inc.
#
# Fedora-License-Identifier: MIT
# SPDX-2.0-License-Identifier: MIT
# SPDX-3.0-License-Identifier: MIT
#
# This program is free software.
# For more information on the license, see COPYING.
# For more information on free software, see
# <https://www.gnu.org/philosophy/free-sw.en.html>.

import sys

try:
    import unittest
    import gi

    gi.require_version("Modulemd", "2.0")
    from gi.repository import Modulemd
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase


def _get_buildtime_streams(modulemd_dependecies, stream_name):
    modulemd_dependecies.get_buildtime_streams(stream_name)


def _get_runtime_streams(modulemd_dependecies, stream_name):
    modulemd_dependecies.get_runtime_streams(stream_name)


class TestDependencies(TestBase):
    def test_constructor(self):
        # Test that the new() function works
        d = Modulemd.Dependencies.new()
        assert d
        assert d.get_buildtime_modules() == []
        self.assertProcessFailure(_get_buildtime_streams, d, "foobar123")
        assert d.get_runtime_modules() == []
        self.assertProcessFailure(_get_runtime_streams, d, "foobar123")

        # Test that keyword name is accepted
        d = Modulemd.Dependencies()
        assert d
        assert d.get_buildtime_modules() == []
        self.assertProcessFailure(_get_buildtime_streams, d, "foobar123")
        assert d.get_runtime_modules() == []
        self.assertProcessFailure(_get_runtime_streams, d, "foobar123")

    def test_copy(self):
        d_orig = Modulemd.Dependencies()
        d = d_orig.copy()
        assert d
        assert d.get_buildtime_modules() == []
        self.assertProcessFailure(_get_buildtime_streams, d, "foobar123")
        assert d.get_runtime_modules() == []
        self.assertProcessFailure(_get_runtime_streams, d, "foobar123")

        d_orig.add_buildtime_stream("buildmod1", "stream2")
        d_orig.add_buildtime_stream("buildmod1", "stream1")
        d_orig.set_empty_buildtime_dependencies_for_module("builddef")
        d_orig.add_runtime_stream("runmod1", "stream3")
        d_orig.add_runtime_stream("runmod1", "stream4")
        d_orig.set_empty_runtime_dependencies_for_module("rundef")

        d = d_orig.copy()
        assert d
        assert d.get_buildtime_modules() == ["builddef", "buildmod1"]
        assert d.get_buildtime_streams("builddef") == []
        assert d.get_buildtime_streams("buildmod1") == ["stream1", "stream2"]
        assert d.get_runtime_modules() == ["rundef", "runmod1"]
        assert d.get_runtime_streams("rundef") == []
        assert d.get_runtime_streams("runmod1") == ["stream3", "stream4"]


if __name__ == "__main__":
    unittest.main()
