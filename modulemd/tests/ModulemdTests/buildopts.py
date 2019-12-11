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


class TestBuildopts(TestBase):
    def test_constructor(self):
        # Test that the new() function works
        b = Modulemd.Buildopts.new()
        assert b
        assert b.props.rpm_macros is None
        assert b.get_rpm_macros() is None
        assert b.get_rpm_whitelist() == []
        assert b.get_arches() == []

        # Test that init works with rpm_macros
        b = Modulemd.Buildopts(rpm_macros="Test macros")
        assert b
        assert b.props.rpm_macros == "Test macros"
        assert b.get_rpm_macros() == "Test macros"
        assert b.get_rpm_whitelist() == []
        assert b.get_arches() == []

    def test_copy(self):
        b_orig = Modulemd.Buildopts()
        b = b_orig.copy()
        assert b
        assert b.props.rpm_macros is None
        assert b.get_rpm_macros() is None
        assert b.get_rpm_whitelist() == []
        assert b.get_arches() == []

        b.add_rpm_to_whitelist("test2")
        b.add_rpm_to_whitelist("test3")
        b.add_rpm_to_whitelist("test1")
        b.add_arch("x86_64")
        b.add_arch("ppc64le")

        b = b_orig.copy()
        assert b
        # make sure lists added to b above got clobbered by copy
        assert b.get_rpm_whitelist() == []
        assert b.get_arches() == []

        b_orig.set_rpm_macros("Test macros")
        b_orig.add_rpm_to_whitelist("test2")
        b_orig.add_rpm_to_whitelist("test3")
        b_orig.add_rpm_to_whitelist("test1")
        b_orig.add_arch("x86_64")
        b_orig.add_arch("ppc64le")

        b = b_orig.copy()
        assert b
        assert b.props.rpm_macros == "Test macros"
        assert b.get_rpm_macros() == "Test macros"
        assert b.get_rpm_whitelist() == ["test1", "test2", "test3"]
        assert b.get_arches() == ["ppc64le", "x86_64"]

    def test_get_set_rpm_macros(self):
        b = Modulemd.Buildopts()

        assert b.props.rpm_macros is None
        assert b.get_rpm_macros() is None

        b.set_rpm_macros("foobar")
        assert b.props.rpm_macros == "foobar"
        assert b.get_rpm_macros() == "foobar"

        b.props.rpm_macros = "barfoo"
        assert b.props.rpm_macros == "barfoo"
        assert b.get_rpm_macros() == "barfoo"

        b.props.rpm_macros = None
        assert b.props.rpm_macros is None
        assert b.get_rpm_macros() is None

    def test_whitelist(self):
        b = Modulemd.Buildopts()

        assert b.get_rpm_whitelist() == []

        b.add_rpm_to_whitelist("test2")
        assert b.get_rpm_whitelist() == ["test2"]

        b.add_rpm_to_whitelist("test3")
        b.add_rpm_to_whitelist("test1")
        assert b.get_rpm_whitelist() == ["test1", "test2", "test3"]

        b.add_rpm_to_whitelist("test2")
        assert b.get_rpm_whitelist() == ["test1", "test2", "test3"]

        b.remove_rpm_from_whitelist("test1")
        assert b.get_rpm_whitelist() == ["test2", "test3"]

        b.clear_rpm_whitelist()
        assert b.get_rpm_whitelist() == []

    def test_arches(self):
        b = Modulemd.Buildopts()

        assert b.get_arches() == []

        b.add_arch("s390x")
        assert b.get_arches() == ["s390x"]

        b.add_arch("x86_64")
        b.add_arch("ppc64le")
        assert b.get_arches() == ["ppc64le", "s390x", "x86_64"]

        b.add_arch("s390x")
        assert b.get_arches() == ["ppc64le", "s390x", "x86_64"]

        b.remove_arch("ppc64le")
        assert b.get_arches() == ["s390x", "x86_64"]

        b.clear_arches()
        assert b.get_arches() == []


if __name__ == "__main__":
    unittest.main()
