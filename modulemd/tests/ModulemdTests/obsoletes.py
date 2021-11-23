#!/usr/bin/python3

# This file is part of libmodulemd
# Copyright (C) 2020 Red Hat, Inc.
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


class TestObsoletes(TestBase):
    def test_constructors(self):

        # Test that the new() function works
        o = Modulemd.Obsoletes.new(1, 2, "testmodule", "teststream", "testmsg")
        assert o
        assert o.props.mdversion == 1
        assert o.props.modified == 2
        assert o.props.module_name == "testmodule"
        assert o.props.module_stream == "teststream"
        assert o.props.message == "testmsg"

        # Test that keywords are accepted
        o = Modulemd.Obsoletes(
            mdversion=1,
            modified=42,
            module_name="testmodule2",
            module_stream="teststream2",
            message="testmessage2",
        )
        assert o
        assert o.validate()
        assert o.props.mdversion == 1
        assert o.props.modified == 42
        assert o.props.module_name == "testmodule2"
        assert o.props.module_stream == "teststream2"
        assert o.props.message == "testmessage2"

    def test_copy(self):
        o = Modulemd.Obsoletes.new(1, 2, "testmodule", "teststream", "testmsg")
        o_copy = o.copy()
        assert o_copy
        assert o_copy.validate()
        assert o_copy.props.mdversion == 1
        assert o_copy.props.modified == 2
        assert o_copy.props.module_name == "testmodule"
        assert o_copy.props.module_stream == "teststream"
        assert o_copy.props.message == "testmsg"


if __name__ == "__main__":
    unittest.main()
