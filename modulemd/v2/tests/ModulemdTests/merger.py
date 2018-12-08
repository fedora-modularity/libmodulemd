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
    from gi.repository import GLib
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase
import random


class TestModuleIndexMerger(TestBase):

    def test_constructors(self):
        merger = Modulemd.ModuleIndexMerger()
        self.assertIsNotNone(merger)

        merger = Modulemd.ModuleIndexMerger.new()
        self.assertIsNotNone(merger)

    def test_deduplicate(self):
        merger = Modulemd.ModuleIndexMerger()
        idx1 = Modulemd.ModuleIndex()
        idx2 = Modulemd.ModuleIndex()

        res, failures = idx1.update_from_file(path.join(
            self.source_root, "modulemd/v2/tests/test_data/long-valid.yaml"))
        self.assertTrue(res)
        self.assertEqual(len(failures), 0)

        res, failures = idx2.update_from_file(path.join(
            self.source_root, "modulemd/v2/tests/test_data/long-valid.yaml"))
        self.assertTrue(res)
        self.assertEqual(len(failures), 0)

        # Save the original document for comparing later
        baseline = idx1.dump_to_string()

        # Add the same index twice
        merger.associate_index(idx1, 0)
        merger.associate_index(idx2, 0)
        deduplicated_idx = merger.resolve()
        final = deduplicated_idx.dump_to_string()

        # Verify that it outputs the same content as the baseline
        print("Baseline:\n%s" % baseline, file=sys.stderr)
        print("Resolved:\n%s" % final, file=sys.stderr)

        self.assertEqual(baseline, deduplicated_idx.dump_to_string())

    def test_merger(self):
        # Get a set of objects in a ModuleIndex
        base_index = Modulemd.ModuleIndex()
        base_index.update_from_file(
            path.join(
                self.source_root,
                "modulemd/v2/tests/test_data/merging-base.yaml"))

        # Baseline
        httpd_defaults = base_index.get_module('httpd').get_defaults()
        self.assertIsNotNone(httpd_defaults)
        self.assertEqual(httpd_defaults.get_default_stream(), '2.2')
        httpd_profile_streams = httpd_defaults.get_streams_with_default_profiles()
        self.assertEqual(len(httpd_profile_streams), 2)
        self.assertTrue('2.2' in httpd_profile_streams)
        self.assertTrue('2.8' in httpd_profile_streams)
        self.assertEqual(
            len(httpd_defaults.get_default_profiles_for_stream('2.2')), 2)
        self.assertTrue(
            'client' in httpd_defaults.get_default_profiles_for_stream('2.2'))
        self.assertTrue(
            'server' in httpd_defaults.get_default_profiles_for_stream('2.2'))
        self.assertTrue(
            'notreal' in httpd_defaults.get_default_profiles_for_stream('2.8'))

        self.assertEqual(
            httpd_defaults.get_default_stream('workstation'), '2.4')
        httpd_profile_streams = httpd_defaults.get_streams_with_default_profiles(
            'workstation')
        self.assertEqual(len(httpd_profile_streams), 2)
        self.assertTrue('2.4' in httpd_profile_streams)
        self.assertTrue('2.6' in httpd_profile_streams)

        self.assertEqual(
            len(httpd_defaults.get_default_profiles_for_stream('2.4', 'workstation')), 1)
        self.assertEqual(
            len(httpd_defaults.get_default_profiles_for_stream('2.6', 'workstation')), 3)

        # Get another set of objects that will override the above
        override_index = Modulemd.ModuleIndex()
        override_index.update_from_file(
            path.join(
                self.source_root,
                "modulemd/v2/tests/test_data/overriding.yaml"))

        # Test that adding both of these at the same priority level fails
        # with a merge conflict.
        merger = Modulemd.ModuleIndexMerger()
        merger.associate_index(base_index, 0)
        merger.associate_index(override_index, 0)

        with self.assertRaisesRegex(GLib.GError, 'Module stream mismatch in merge'):
            merged_index = merger.resolve()

        # Test that override_index at a higher priority level succeeds
        # Test that adding both of these at the same priority level fails
        # with a merge conflict.
        # Use randomly-selected high and low values to make sure we don't have
        # sorting issues.
        merger = Modulemd.ModuleIndexMerger()
        random_low = random.randint(1, 100)
        random_high = random.randint(101, 999)
        print(
            "Low priority: %d, High priority: %d" %
            (random_low, random_high))
        merger.associate_index(base_index, random_low)
        merger.associate_index(override_index, random_high)

        merged_index = merger.resolve()
        self.assertIsNotNone(merged_index)

        # Validate merged results

        # HTTPD
        httpd_defaults = merged_index.get_module('httpd').get_defaults()
        self.assertIsNotNone(httpd_defaults)
        self.assertEqual(httpd_defaults.get_default_stream(), '2.4')
        httpd_profile_streams = httpd_defaults.get_streams_with_default_profiles()
        self.assertEqual(len(httpd_profile_streams), 2)
        self.assertTrue('2.2' in httpd_profile_streams)
        self.assertTrue('2.4' in httpd_profile_streams)
        self.assertEqual(
            len(httpd_defaults.get_default_profiles_for_stream('2.2')), 2)
        self.assertTrue(
            'client' in httpd_defaults.get_default_profiles_for_stream('2.2'))
        self.assertTrue(
            'server' in httpd_defaults.get_default_profiles_for_stream('2.2'))
        self.assertTrue(
            'client' in httpd_defaults.get_default_profiles_for_stream('2.4'))
        self.assertTrue(
            'server' in httpd_defaults.get_default_profiles_for_stream('2.4'))

        self.assertEqual(
            httpd_defaults.get_default_stream('workstation'), '2.8')
        httpd_profile_streams = httpd_defaults.get_streams_with_default_profiles(
            'workstation')
        self.assertEqual(len(httpd_profile_streams), 3)
        self.assertTrue('2.4' in httpd_profile_streams)
        self.assertTrue('2.6' in httpd_profile_streams)
        self.assertTrue('2.8' in httpd_profile_streams)
        self.assertEqual(
            len(httpd_defaults.get_default_profiles_for_stream('2.4', 'workstation')), 1)
        self.assertEqual(
            len(httpd_defaults.get_default_profiles_for_stream('2.6', 'workstation')), 3)
        self.assertEqual(
            len(httpd_defaults.get_default_profiles_for_stream('2.8', 'workstation')), 4)

    def test_merger_with_modified(self):
        pass

if __name__ == '__main__':
    unittest.main()
