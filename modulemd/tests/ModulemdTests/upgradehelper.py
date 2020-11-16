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


class TestUpgradeHelper(TestBase):
    def test_constructor(self):
        # Test that the new() function works
        helper = Modulemd.UpgradeHelper.new()
        self.assertIsNotNone(helper)

        modules = helper.get_known_modules()
        self.assertIsNotNone(modules)
        self.assertEqual(0, len(modules))

    def test_known_streams(self):
        helper = Modulemd.UpgradeHelper.new()
        self.assertIsNotNone(helper)

        if hasattr(helper, "set_known_streams"):
            helper.set_known_streams(
                {"platform": ["f33", "f34", "eln"], "django": ["3.0"]}
            )
        else:
            helper.add_known_stream("platform", "f33")
            helper.add_known_stream("platform", "f34")
            helper.add_known_stream("platform", "eln")
            helper.add_known_stream("django", "3.0")

        modules = helper.get_known_modules()
        self.assertEqual(2, len(modules))
        self.assertIn("platform", modules)
        self.assertIn("django", modules)

        self.assertIn("f33", helper.get_known_streams("platform"))
        self.assertIn("f34", helper.get_known_streams("platform"))
        self.assertIn("eln", helper.get_known_streams("platform"))

        self.assertIn("3.0", helper.get_known_streams("django"))

        # Add a duplicate to verify deduplication
        helper.add_known_stream("platform", "f33")

        modules = helper.get_known_modules()
        self.assertEqual(2, len(modules))
        self.assertIn("platform", modules)
        self.assertIn("django", modules)

        self.assertIn("f33", helper.get_known_streams("platform"))
        self.assertIn("f34", helper.get_known_streams("platform"))
        self.assertIn("eln", helper.get_known_streams("platform"))

        self.assertIn("3.0", helper.get_known_streams("django"))

        self.assertEqual(0, len(helper.get_known_streams("unknownmodule")))


if __name__ == "__main__":
    unittest.main()
