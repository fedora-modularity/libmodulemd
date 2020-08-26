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
        assert o.version == 1
        assert o.modified == 2
        assert o.module_name == "testmodule"
        assert o.module_stream == "teststream"
        assert o.message == "testmsg"

        # Test that keywords are accepted
        o = Modulemd.Obsoletes(
            version=1,
            modified=42,
            module_name="testmodule2",
            module_stream="teststream2",
            message="testmessage2",
        )
        assert o
        assert o.validate()
        assert o.version == 1
        assert o.modified == 42
        assert o.module_name == "testmodule2"
        assert o.module_stream == "teststream2"
        assert o.message == "testmsg2"

    def test_copy(self):
        o = Modulemd.Obsoletes.new(1, 2, "testmodule", "teststream", "testmsg")
        o_copy = o.copy()
        assert o_copy
        assert o_copy.validate()
        assert o_copy.version == 1
        assert o_copy.modified == 2
        assert o_copy.module_name == "testmodule"
        assert o_copy.module_stream == "teststream"
        assert o_copy.message == "testmsg"


if __name__ == "__main__":
    unittest.main()
