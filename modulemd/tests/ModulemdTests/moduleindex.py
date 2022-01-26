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


def debug_dump_failures(failures):
    if failures is None or len(failures) == 0:
        return
    print("{} YAML subdocuments were invalid".format(len(failures)))
    for f in failures:
        print(
            "Failed subdocument ({}):\n{}\n".format(
                str(f.get_gerror()), f.get_yaml()
            )
        )


class TestModuleIndex(TestBase):
    def test_constructors(self):
        # Test that the new() function works
        idx = ModuleIndex.new()
        self.assertIsNotNone(idx)
        self.assertListEqual(idx.get_module_names(), [])
        self.assertIsNone(idx.get_module("foo"))

    def test_read(self):
        idx = ModuleIndex.new()

        with open(
            path.join(self.source_root, "yaml_specs/modulemd_stream_v1.yaml"),
            "r",
        ) as v1:
            res, failures = idx.update_from_string(v1.read(), True)
            debug_dump_failures(failures)
            self.assertListEqual(failures, [])
            self.assertTrue(res)

        for fname in [
            "yaml_specs/modulemd_stream_v2.yaml",
            "yaml_specs/modulemd_translations_v1.yaml",
            "yaml_specs/modulemd_defaults_v1.yaml",
        ]:
            res, failures = idx.update_from_file(
                path.join(self.source_root, fname), True
            )
            debug_dump_failures(failures)
            self.assertListEqual(failures, [])
            self.assertTrue(res)

        res, failures = idx.update_from_file(
            path.join(self.test_data_path, "te.yaml"), True
        )
        self.assertFalse(res)
        self.assertEqual(len(failures), 1)
        self.assertIn(
            "No document type specified", str(failures[0].get_gerror())
        )
        self.assertMultiLineEqual(
            failures[0].get_yaml(),
            """---
summary: An example module
description: An example module.
profiles:
  profile_a: An example profile
...
""",
        )

        self.assertListEqual(idx.get_module_names(), ["foo"])

        mod_foo = idx.get_module("foo")
        self.assertTrue(mod_foo.validate())
        self.assertEqual(mod_foo.get_module_name(), "foo")
        with self.assertRaisesRegex(
            gi.repository.GLib.GError, "No streams matched"
        ):
            mod_foo.get_stream_by_NSVCA("a", 5, "c")
        self.assertEqual(len(mod_foo.get_all_streams()), 2)
        self.assertIsNotNone(mod_foo.get_defaults())

        defaults = mod_foo.get_defaults()
        self.assertIsNotNone(defaults)
        self.assertEqual(defaults.get_default_stream(), "x.y")

        stream = mod_foo.get_stream_by_NSVCA(
            "latest", 20160927144203, "c0ffee43"
        )
        self.assertIsNotNone(stream)
        self.assertEqual(
            stream.get_nsvc(), "foo:latest:20160927144203:c0ffee43"
        )
        self.assertEqual(
            stream.get_description(None),
            "A module for the demonstration of the metadata format. Also, the "
            "obligatory lorem ipsum dolor sit amet goes right here.",
        )
        self.assertEqual(
            stream.get_description("C"),
            "A module for the demonstration of the metadata format. Also, the "
            "obligatory lorem ipsum dolor sit amet goes right here.",
        )
        self.assertEqual(stream.get_description("en_GB"), "An example module.")

    def test_get_default_streams(self):
        idx = Modulemd.ModuleIndex.new()
        idx.update_from_file(path.join(self.test_data_path, "f29.yaml"), True)

        default_streams = idx.get_default_streams()
        self.assertIsNotNone(default_streams)

        self.assertIn("dwm", default_streams.keys())
        self.assertEqual("6.1", default_streams["dwm"])

        self.assertIn("stratis", default_streams.keys())
        self.assertEqual("1", default_streams["stratis"])

        self.assertNotIn("nodejs", default_streams.keys())

    def test_dump_empty_index(self):
        idx = Modulemd.ModuleIndex.new()

        with self.assertRaisesRegex(
            gi.repository.GLib.GError, "Index contains no modules."
        ):
            yaml = idx.dump_to_string()
            self.assertIsNone(yaml)

    def test_update_from_defaults_directory(self):
        idx = Modulemd.ModuleIndex.new()
        self.assertIsNotNone(idx)

        # First, verify that it works without overrides
        ret = idx.update_from_defaults_directory(
            path=path.join(self.test_data_path, "defaults"), strict=True
        )
        self.assertTrue(ret)

        # There should be three modules here: meson, ninja and nodejs
        self.assertEqual(len(idx.get_module_names()), 3)
        self.assertIn("meson", idx.get_module_names())
        self.assertIn("ninja", idx.get_module_names())
        self.assertIn("nodejs", idx.get_module_names())

        # Check default streams
        defs = idx.get_default_streams()
        self.assertIn("meson", defs)
        self.assertEqual("latest", defs["meson"])
        self.assertIn("ninja", defs)
        self.assertEqual("latest", defs["ninja"])
        self.assertNotIn("nodejs", defs)

        # Now add overrides too
        # First, verify that it works without overrides
        ret = idx.update_from_defaults_directory(
            path=path.join(self.test_data_path, "defaults"),
            overrides_path=path.join(
                self.test_data_path, "defaults", "overrides"
            ),
            strict=True,
        )
        self.assertTrue(ret)

        # There should be four modules here: meson, ninja, nodejs and
        # testmodule
        self.assertEqual(len(idx.get_module_names()), 4)
        self.assertIn("meson", idx.get_module_names())
        self.assertIn("ninja", idx.get_module_names())
        self.assertIn("nodejs", idx.get_module_names())
        self.assertIn("testmodule", idx.get_module_names())

        # Check default streams
        defs = idx.get_default_streams()
        self.assertIn("meson", defs)
        self.assertEqual("latest", defs["meson"])
        self.assertIn("ninja", defs)
        self.assertEqual("latest", defs["ninja"])
        self.assertIn("nodejs", defs)
        self.assertEqual("12", defs["nodejs"])
        self.assertIn("testmodule", defs)
        self.assertIn("teststream", defs["testmodule"])

        # Nonexistent defaults dir
        with self.assertRaisesRegex(GLib.Error, "No such file or directory"):
            ret = idx.update_from_defaults_directory(
                path=path.join(self.test_data_path, "defaults_nonexistent"),
                strict=True,
            )
            self.assertFalse(ret)

        # Nonexistent override dir
        with self.assertRaisesRegex(GLib.Error, "No such file or directory"):
            ret = idx.update_from_defaults_directory(
                path=path.join(self.test_data_path, "defaults"),
                overrides_path="overrides_nonexistent",
                strict=True,
            )
            self.assertFalse(ret)

    def test_clear_xmds(self):
        if "_overrides_module" in dir(Modulemd) and hasattr(
            gi.overrides.Modulemd, "ModuleStreamV2"
        ):
            idx = Modulemd.ModuleIndex.new()
            self.assertIsNotNone(idx)

            ret, failures = idx.update_from_string(
                """
---
document: modulemd
version: 2
data:
    name: foo
    stream: latest
    version: 1
    static_context: true
    context: c0ffee43
    arch: s390x
    summary: An example module
    description: A longer description
    license:
        module: MIT
    xmd:
        a_key: a_value
        another_key: another_value
        an_array:
          - a
          - b
...
---
document: modulemd
version: 2
data:
    name: foo
    stream: latest
    version: 2
    static_context: true
    context: c0ffee43
    arch: s390x
    summary: An example module
    description: A longer description
    license:
        module: MIT
    xmd:
        a_key: a_value
        another_key: another_value
        an_array:
          - a
          - b
...
""",
                strict=True,
            )
            self.assertEqual(len(failures), 0)

            module_names = idx.get_module_names()
            self.assertEqual(len(module_names), 1)
            self.assertEqual(module_names[0], "foo")

            module = idx.get_module(module_names[0])
            self.assertEqual(len(module.get_all_streams()), 2)

            ref_xmd = {
                "a_key": "a_value",
                "another_key": "another_value",
                "an_array": ["a", "b"],
            }

            for stream in module.get_all_streams():
                # Verify that the XMD data is present.
                xmd = stream.get_xmd()
                self.assertEqual(xmd, ref_xmd)

            idx.clear_xmds()

            for stream in module.get_all_streams():
                # Verify that the XMD data is gone now.
                self.assertEqual(stream.get_xmd(), {})


if __name__ == "__main__":
    unittest.main()
