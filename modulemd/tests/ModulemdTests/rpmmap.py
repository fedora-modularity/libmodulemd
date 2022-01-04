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

try:
    import unittest
    import gi

    gi.require_version("Modulemd", "2.0")
    from gi.repository import GLib
    from gi.repository import Modulemd
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase


class TestRpmMapEntry(TestBase):
    def test_basic(self):
        # Test that the new() function works
        entry = Modulemd.RpmMapEntry.new(
            "bar", 0, "1.23", "1.module_deadbeef", "x86_64"
        )

        self.assertIsNotNone(entry)

        self.assertEqual(entry.props.name, "bar")
        self.assertEqual(entry.props.epoch, 0)
        self.assertEqual(entry.props.version, "1.23")
        self.assertEqual(entry.props.release, "1.module_deadbeef")
        self.assertEqual(entry.props.arch, "x86_64")
        self.assertEqual(
            entry.props.nevra, "bar-0:1.23-1.module_deadbeef.x86_64"
        )

        # Test that object instantiation with attributes works
        entry2 = Modulemd.RpmMapEntry(
            name="bar",
            version="1.23",
            release="1.module_deadbeef",
            arch="x86_64",
        )
        self.assertEqual(
            entry2.props.nevra, "bar-0:1.23-1.module_deadbeef.x86_64"
        )

        # Test that nevra returns NULL if attributes are missing

        # Remove name
        entry2.props.name = None
        with self.assertRaisesRegex(
            gi.repository.GLib.GError, "Missing name attribute"
        ):
            entry2.validate()
        self.assertIsNone(entry2.props.nevra)
        entry2.props.name = "bar"

        # Remove the version
        entry2.props.version = None
        with self.assertRaisesRegex(
            gi.repository.GLib.GError, "Missing version attribute"
        ):
            entry2.validate()
        self.assertIsNone(entry2.props.nevra)
        entry2.props.version = "1.23"

        # Remove the release
        entry2.props.release = None
        with self.assertRaisesRegex(
            gi.repository.GLib.GError, "Missing release attribute"
        ):
            entry2.validate()
        self.assertIsNone(entry2.props.nevra)
        entry2.props.release = "1.module_deadbeef"

        # Remove the arch
        entry2.props.arch = None
        with self.assertRaisesRegex(
            gi.repository.GLib.GError, "Missing arch attribute"
        ):
            entry2.validate()
        self.assertIsNone(entry2.props.nevra)
        entry2.props.arch = "x86_64"

        self.assertEqual(
            entry2.props.nevra, "bar-0:1.23-1.module_deadbeef.x86_64"
        )

    def test_compare(self):
        entry = Modulemd.RpmMapEntry.new(
            "bar", 0, "1.23", "1.module_deadbeef", "x86_64"
        )
        self.assertIsNotNone(entry)

        entry2 = Modulemd.RpmMapEntry.new(
            "bar", 0, "1.23", "1.module_deadbeef", "x86_64"
        )
        self.assertIsNotNone(entry2)

        entry3 = Modulemd.RpmMapEntry.new(
            "foo", 0, "1.23", "1.module_deadbeef", "x86_64"
        )
        self.assertIsNotNone(entry3)

        self.assertTrue(entry.equals(entry))
        self.assertTrue(entry.equals(entry2))
        self.assertFalse(entry.equals(entry3))


if __name__ == "__main__":
    unittest.main()
