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
    gi.require_version('Modulemd', '2.0')
    from gi.repository import Modulemd
    from gi.repository.Modulemd import ModuleIndex
    from gi.repository import GLib
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase


class TestModuleIndex(TestBase):

    def test_constructors(self):
        # Test that the new() function works
        idx = ModuleIndex.new()
        self.assertIsNotNone(idx)
        self.assertListEqual(idx.get_module_names(), [])
        self.assertIsNone(idx.get_module("foo"))

    def test_read(self):
        idx = ModuleIndex.new()

        with open(path.join(self.source_root, "spec.v1.yaml"), 'r') as v1:
            res, failures = idx.update_from_string(v1.read())
            self.assertTrue(res)
            self.assertListEqual(failures, [])

        for fname in [
                "spec.v2.yaml",
                "translations/spec.v1.yaml",
                "mod-defaults/spec.v1.yaml"]:
            res, failures = idx.update_from_file(
                path.join(self.source_root, fname))
            self.assertTrue(res)
            self.assertListEqual(failures, [])

        res, failures = idx.update_from_file(
            path.join(self.source_root, "modulemd/v2/tests/test_data/te.yaml"))
        self.assertTrue(res)
        self.assertEqual(len(failures), 1)
        self.assertIn(
            "No document type specified", str(
                failures[0].get_gerror()))
        self.assertMultiLineEqual(failures[0].get_yaml(), """---
summary: An example module
description: An example module.
profiles:
  profile_a: An example profile
...
""")

        self.assertListEqual(idx.get_module_names(), ["foo"])

        mod_foo = idx.get_module("foo")
        self.assertTrue(mod_foo.validate())
        self.assertEqual(mod_foo.get_module_name(), "foo")
        self.assertIsNone(mod_foo.get_stream_by_NSVC("a", 5, "c"))
        self.assertEqual(len(mod_foo.get_all_streams()), 2)
        self.assertIsNotNone(mod_foo.get_defaults())

        defaults = mod_foo.get_defaults()
        self.assertIsNotNone(defaults)
        self.assertEqual(defaults.get_default_stream(), "x.y")

        stream = mod_foo.get_stream_by_NSVC(
            "latest", 20160927144203, "c0ffee43")
        self.assertIsNotNone(stream)
        self.assertEqual(stream.get_nsvc(),
                         "foo:latest:20160927144203:c0ffee43")
        self.assertEqual(
            stream.get_description(None),
            "A module for the demonstration of the metadata format. Also, the "
            "obligatory lorem ipsum dolor sit amet goes right here.")
        self.assertEqual(
            stream.get_description("C"),
            "A module for the demonstration of the metadata format. Also, the "
            "obligatory lorem ipsum dolor sit amet goes right here.")
        self.assertEqual(stream.get_description("en_GB"), "An example module.")


if __name__ == '__main__':
    unittest.main()
