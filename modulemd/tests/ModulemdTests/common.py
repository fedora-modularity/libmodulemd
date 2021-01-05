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

from base import TestBase

logging.basicConfig(level=logging.DEBUG)


class TestCommon(TestBase):
    def test_packager_read_file(self):
        # We have a chicken-egg problem with overrides, since they can only
        # be tested if they are already installed. This means they need to
        # be run in the CI. In order to avoid changes to these tests or the
        # overrides breaking things, we'll skip them if the appropriate
        # override is not installed.
        if not (
            "_overrides_module" in dir(Modulemd)
            and hasattr(gi.overrides.Modulemd, "read_packager_file")
        ):
            logging.debug(
                "read_packager_file() override not installed, skipping tests"
            )
            return

        # Validate that the PackagerV2 specification parses correctly
        doc = Modulemd.read_packager_file(
            os.path.join(
                self.source_root, "yaml_specs/modulemd_packager_v2.yaml"
            ),
        )
        self.assertIsNotNone(doc)
        # Once read in, a modulemd-packager v2 document is a ModuleStream of the
        # same version.
        self.assertIs(type(doc), Modulemd.ModuleStreamV2)
        # Confirm module and stream names are undefined
        self.assertIsNone(doc.get_module_name())
        self.assertIsNone(doc.get_stream_name())

        # Read the PackagerV2 specification with module/stream name overrides
        doc = Modulemd.read_packager_file(
            os.path.join(
                self.source_root, "yaml_specs/modulemd_packager_v2.yaml"
            ),
            "modulename-override",
            "streamname-override",
        )
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.ModuleStreamV2)
        # Confirm module and stream names were set
        self.assertEqual(doc.get_module_name(), "modulename-override")
        self.assertEqual(doc.get_stream_name(), "streamname-override")

        # Valid PackagerV3 file
        doc = Modulemd.read_packager_file(
            os.path.join(
                self.source_root, "yaml_specs/modulemd_packager_v3.yaml"
            ),
        )
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.PackagerV3)

        # Read PackagerV3 with module/stream name overrides
        doc = Modulemd.read_packager_file(
            os.path.join(
                self.source_root, "yaml_specs/modulemd_packager_v3.yaml"
            ),
            "modulename-override",
            "streamname-override",
        )
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.PackagerV3)
        # Confirm module and stream names were set
        self.assertEqual(doc.get_module_name(), "modulename-override")
        self.assertEqual(doc.get_stream_name(), "streamname-override")

        # Validate that the ModuleStreamV2 specification parses correctly
        doc = Modulemd.read_packager_file(
            os.path.join(
                self.source_root, "yaml_specs/modulemd_stream_v2.yaml"
            ),
        )
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.ModuleStreamV2)
        # Confirm module and stream names are correct
        self.assertEqual(doc.get_module_name(), "foo")
        self.assertEqual(doc.get_stream_name(), "latest")

        # Read the ModuleStreamV2 specification with module/stream name overrides
        doc = Modulemd.read_packager_file(
            os.path.join(
                self.source_root, "yaml_specs/modulemd_stream_v2.yaml"
            ),
            "modulename-override",
            "streamname-override",
        )
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.ModuleStreamV2)
        # Confirm module and stream names were set
        self.assertEqual(doc.get_module_name(), "modulename-override")
        self.assertEqual(doc.get_stream_name(), "streamname-override")

    def test_packager_read_string(self):
        # We have a chicken-egg problem with overrides, since they can only
        # be tested if they are already installed. This means they need to
        # be run in the CI. In order to avoid changes to these tests or the
        # overrides breaking things, we'll skip them if the appropriate
        # override is not installed.
        if not (
            "_overrides_module" in dir(Modulemd)
            and hasattr(gi.overrides.Modulemd, "read_packager_string")
        ):
            logging.debug(
                "read_packager_string() override not installed, skipping tests"
            )
            return

        # PackagerV2
        minimal_valid = """---
document: modulemd-packager
version: 2
data:
  summary: A minimal valid module
  description: A minimalistic module description
  license:
    module:
      - MIT
...
"""
        doc = Modulemd.read_packager_string(minimal_valid)
        self.assertIsNotNone(doc)
        # Once read in, a modulemd-packager document is a ModuleStream of the
        # same version.
        self.assertIs(type(doc), Modulemd.ModuleStreamV2)
        # Confirm module and stream names are undefined
        self.assertIsNone(doc.get_module_name())
        self.assertIsNone(doc.get_stream_name())

        # Read again with module/stream name overrides
        doc = Modulemd.read_packager_string(
            minimal_valid, "modulename-override", "streamname-override",
        )
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.ModuleStreamV2)
        # Confirm module and stream names were set
        self.assertEqual(doc.get_module_name(), "modulename-override")
        self.assertEqual(doc.get_stream_name(), "streamname-override")

        # PackagerV3
        minimal_valid = """---
document: modulemd-packager
version: 3
data:
  summary: A minimal valid module
  description: A minimalistic module description
  license:
    - MIT
...
"""
        doc = Modulemd.read_packager_string(minimal_valid)
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.PackagerV3)
        # Confirm module and stream names are undefined
        self.assertIsNone(doc.get_module_name())
        self.assertIsNone(doc.get_stream_name())

        # Read again with module/stream name overrides
        doc = Modulemd.read_packager_string(
            minimal_valid, "modulename-override", "streamname-override",
        )
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.PackagerV3)
        # Confirm module and stream names were set
        self.assertEqual(doc.get_module_name(), "modulename-override")
        self.assertEqual(doc.get_stream_name(), "streamname-override")

        # ModuleStreamV2
        minimal_valid = """---
document: modulemd-stream
version: 2
data:
  summary: A minimal valid module
  description: A minimalistic module description
  license:
    module:
      - MIT
...
"""
        doc = Modulemd.read_packager_string(minimal_valid)
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.ModuleStreamV2)
        # Confirm module and stream names are undefined
        self.assertIsNone(doc.get_module_name())
        self.assertIsNone(doc.get_stream_name())

        # Read again with module/stream name overrides
        doc = Modulemd.read_packager_string(
            minimal_valid, "modulename-override", "streamname-override",
        )
        self.assertIsNotNone(doc)
        self.assertIs(type(doc), Modulemd.ModuleStreamV2)
        # Confirm module and stream names were set
        self.assertEqual(doc.get_module_name(), "modulename-override")
        self.assertEqual(doc.get_stream_name(), "streamname-override")


if __name__ == "__main__":
    unittest.main()
