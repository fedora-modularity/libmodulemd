import json
import logging
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

from copy import deepcopy
from base import TestBase

logging.basicConfig(level=logging.DEBUG)


class TestModulemdPackager(TestBase):
    def test_sanity(self):
        # Validate that the specification parses correctly
        stream = Modulemd.ModuleStream.read_file(
            os.path.join(
                self.source_root, "yaml_specs/modulemd_packager_v2.yaml"
            ),
            True,
        )
        self.assertIsNotNone(stream)

        # Once read in, a modulemd-packager document is a ModuleStream of the
        # same version.
        self.assertIs(type(stream), Modulemd.ModuleStreamV2)

    def test_read_minimal(self):
        minimal_valid = """---
document: modulemd-packager
version: 2
data:
  summary: A minimal valid module
  description: A minimalistic module description
  license:
    module:
      - DUMMY
...
"""
        stream = Modulemd.ModuleStream.read_string(minimal_valid, True)
        self.assertIsNotNone(stream)

        # Once read in, a modulemd-packager document is a ModuleStream of the
        # same version.
        self.assertIs(type(stream), Modulemd.ModuleStreamV2)

    def test_read_extra_values(self):
        minimal_valid = {
            "document": "modulemd-packager",
            "version": 2,
            "data": {
                "summary": "A minimal valid module",
                "description": "A minimalistic module description",
                "license": {"module": ["DUMMY1", "DUMMY2"]},
            },
        }

        extra_attrs = [
            "name",
            "stream",
            "context",
            "arch",
            "servicelevels",
            "xmd",
            "buildopts",
            "artifacts",
        ]

        for attr in extra_attrs:
            minimal_plus_extra = deepcopy(minimal_valid)

            # We can just pretend they're all scalars, because the parser will
            # halt before it tries to validate the value.
            minimal_plus_extra["data"][attr] = "invalid key"

            logging.debug(
                "YAML: {}".format(json.dumps(minimal_plus_extra, indent=2))
            )

            # This code takes advantage of the fact that JSON is a proper
            # subset of YAML, so we can avoid requiring pyYAML for the
            # tests.
            with self.assertRaises(gi.repository.GLib.GError) as cm:
                stream = Modulemd.ModuleStream.read_string(
                    json.dumps(minimal_plus_extra), True
                )

            gerror = cm.exception
            self.assertTrue(
                gerror.matches(
                    domain=Modulemd.yaml_error_quark(),
                    code=Modulemd.YamlError.UNKNOWN_ATTR,
                )
            )

        # Handle license.content as a special case since it's not directly
        # under `data`.
        minimal_plus_extra = deepcopy(minimal_valid)
        minimal_plus_extra["data"]["license"]["content"] = "invalid key"

        logging.debug(
            "YAML: {}".format(json.dumps(minimal_plus_extra, indent=2))
        )

        # This code takes advantage of the fact that JSON is a proper
        # subset of YAML, so we can avoid requiring pyYAML for the tests.
        with self.assertRaises(gi.repository.GLib.GError) as cm:
            stream = Modulemd.ModuleStream.read_string(
                json.dumps(minimal_plus_extra), True
            )

        gerror = cm.exception
        self.assertTrue(
            gerror.matches(
                domain=Modulemd.yaml_error_quark(),
                code=Modulemd.YamlError.UNKNOWN_ATTR,
            )
        )

    def test_fail_v1(self):
        minimal_v1 = {
            "document": "modulemd-packager",
            "version": 1,
            "data": {
                "summary": "A minimal valid module",
                "description": "A minimalistic module description",
                "license": {"module": ["DUMMY1", "DUMMY2"]},
            },
        }

        with self.assertRaises(gi.repository.GLib.Error) as cm:
            stream = Modulemd.ModuleStream.read_string(
                json.dumps(minimal_v1), True
            )
        gerror = cm.exception
        self.assertTrue(
            gerror.matches(
                domain=Modulemd.yaml_error_quark(),
                code=Modulemd.YamlError.INCONSISTENT,
            )
        )

    def test_v3_xmd(self):

        # We have a chicken-egg problem with overrides, since they can only
        # be tested if they are already installed. This means they need to
        # be run in the CI. In order to avoid changes to these tests or the
        # overrides breaking things, we'll skip them if the appropriate
        # override is not installed.
        if "_overrides_module" in dir(Modulemd) and hasattr(
            gi.overrides.Modulemd, "PackagerV3"
        ):

            # The XMD python tests can only be run against the installed lib
            # because the overrides that translate between python and GVariant
            # must be installed in /usr/lib/python*/site-packages/gi/overrides
            # or they are not included when importing Modulemd
            packager = Modulemd.PackagerV3.new()
            # An empty dictionary should be returned if no xmd value is set
            self.assertEqual(packager.get_xmd(), dict())

            xmd = {"outer_key": {"inner_key": ["scalar", "another_scalar"]}}

            packager.set_xmd(xmd)

            xmd_copy = packager.get_xmd()
            assert xmd_copy
            assert "outer_key" in xmd_copy
            assert "inner_key" in xmd_copy["outer_key"]
            assert "scalar" in xmd_copy["outer_key"]["inner_key"]
            assert "another_scalar" in xmd_copy["outer_key"]["inner_key"]

            # Verify that we can add content and save it back
            xmd["something"] = ["foo", "bar"]
            packager.set_xmd(xmd)

            xmd_copy = packager.get_xmd()
            assert xmd_copy
            assert "outer_key" in xmd_copy
            assert "inner_key" in xmd_copy["outer_key"]
            assert "scalar" in xmd_copy["outer_key"]["inner_key"]
            assert "another_scalar" in xmd_copy["outer_key"]["inner_key"]
            assert "something" in xmd_copy
            assert xmd_copy["something"] == ["foo", "bar"]


if __name__ == "__main__":
    unittest.main()
