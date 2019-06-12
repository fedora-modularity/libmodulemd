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

import os
import sys

try:
    import unittest
    import gi
    gi.require_version('Modulemd', '2.0')
    from gi.repository import Modulemd
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase


class TestTranslationHelpers(TestBase):

    def test_translation_catalog_from_index(self):
        # Create a Modulemd.ModuleIndex object containing the YAML stream
        index = Modulemd.ModuleIndex.new()
        yaml_path = "../test_data/f29.yaml"
        ret, failures = index.update_from_file(yaml_path, True)
        self.assertIsNotNone(index)

        # Create a babel catalog for the Modulemd.ModuleIndex object
        catalog = get_translation_catalog_from_index(index)
        self.assertTrue(catalog)

        # Check if the catalog is internally consistent
        _, err = catalog.check()
        self.assertNone(err)

        # Check if each translatable string is mapped to its correct location
        for msg in catalog:
            for location in msg.locations:
                props = location.split(";")

                module_name = props[0].split(":")[0]
                module = index.get_module(module_name)

                stream_name = props[0].split(":")[1]
                stream = module.search_streams(stream_name, 0)[0]

                string_type = props[1]
                if string_type == "summary":
                    assert stream.get_summary("C") == msg.id
                elif string_type == "description":
                    assert stream.get_description("C") == msg.id
                else:
                    profile = stream.get_profile(stream_type.split(";")[1])
                    assert profile.get_description("C") == msg.id

        # There are 34 unique summaries and descriptions in f29.yaml
        self.assertEquals(len(catalog), 34)


if __name__ == '__main__':
    unittest.main()
