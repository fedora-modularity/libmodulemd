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
from ModulemdTranslationHelpers import Utils
from babel.messages import pofile
from six import text_type

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
        self.assertIsNotNone(index)
        ret, failures = index.update_from_file(
            "%s/modulemd/v2/tests/test_data/f29.yaml" %
            (os.getenv('MESON_SOURCE_ROOT')), True)
        self.assertTrue(ret)
        self.assertTrue(len(failures) == 0)

        # Create a babel catalog for the Modulemd.ModuleIndex object
        catalog = Utils.get_translation_catalog_from_index(
            index, "fedora-modularity-translations")
        self.assertTrue(catalog)
        # Check if the catalog is internally consistent
        self.assertTrue(catalog.check())

        # Check if each translatable string is mapped to its correct location
        for msg in catalog:
            for location in msg.locations:
                (module_name, stream_name, string_type,
                 profile_name) = Utils.split_location(location)

                module = index.get_module(module_name)
                self.assertIsNotNone(module)

                stream = module.search_streams(stream_name, 0)[0]
                self.assertIsNotNone(stream)

                if string_type == "summary":
                    self.assertEqual(stream.get_summary("C"), msg.id)
                elif string_type == "description":
                    self.assertEqual(stream.get_description("C"), msg.id)
                else:
                    self.assertEqual(string_type, "profile")
                    profile = stream.get_profile(profile_name)
                    self.assertIsNotNone(profile)
                    self.assertEqual(profile.get_description("C"), msg.id)

        # There are 75 unique summaries and descriptions in f29.yaml
        self.assertEqual(len(catalog), 75)

    def test_translations_from_catalog(self):
        translation_files = [
            "%s/modulemd/v2/tests/test_data/nl.po" %
            (os.getenv('MESON_SOURCE_ROOT'))]

        catalogs = list()
        for f in translation_files:
            with open(f, 'r') as infile:
                catalog = pofile.read_po(infile)
                self.assertIsNotNone(catalog)

                # Check if the catalog is internally consistent
                self.assertTrue(catalog.check())
                catalogs.append(catalog)

        index = Modulemd.ModuleIndex.new()
        self.assertIsNotNone(index)
        ret, failures = index.update_from_file(
            "%s/modulemd/v2/tests/test_data/f29.yaml" %
            (os.getenv('MESON_SOURCE_ROOT')), True)
        self.assertTrue(ret)
        self.assertTrue(len(failures) == 0)

        Utils.get_modulemd_translations_from_catalog(catalogs, index)

        for catalog in catalogs:
            for msg in catalog:
                # Handle UTF-8 differences between python 2 and 3
                if isinstance(msg.string, str):
                    msg_string = msg.string
                elif isinstance(msg.string, text_type):
                    msg_string = msg.string.encode('utf-8')
                else:
                    raise IOError("Message was not a str or text_type")

                for location, _ in msg.locations:
                    (module_name, stream_name, string_type,
                     profile_name) = Utils.split_location(location)

                    module = index.get_module(module_name)
                    self.assertIsNotNone(module)

                    streams = module.search_streams(stream_name, 0)
                    self.assertIsNotNone(streams)

                    for stream in streams:
                        if string_type == "summary":
                            self.assertEqual(
                                stream.get_summary(
                                    str(catalog.locale)), msg_string)
                        elif string_type == "description":
                            self.assertEqual(
                                stream.get_description(
                                    str(catalog.locale)), msg_string)
                        else:
                            self.assertEqual(string_type, "profile")
                            profile = stream.get_profile(profile_name)
                            self.assertIsNotNone(profile)
                            self.assertEqual(
                                profile.get_description(
                                    str(catalog.locale)), msg_string)


if __name__ == '__main__':
    unittest.main()
