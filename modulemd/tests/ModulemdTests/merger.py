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
import logging

try:
    import unittest
    import gi

    gi.require_version("Modulemd", "2.0")
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

        res, failures = idx1.update_from_file(
            path.join(self.test_data_path, "long-valid.yaml"), True
        )
        self.assertTrue(res)
        self.assertEqual(len(failures), 0)

        res, failures = idx2.update_from_file(
            path.join(self.test_data_path, "long-valid.yaml"), True
        )
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
        self.assertEqual(baseline, final)

    def test_merger(self):
        # Get a set of objects in a ModuleIndex
        base_index = Modulemd.ModuleIndex()
        base_index.update_from_file(
            path.join(self.test_data_path, "merging-base.yaml"), True
        )

        # Baseline
        httpd_defaults = base_index.get_module("httpd").get_defaults()
        self.assertIsNotNone(httpd_defaults)
        self.assertEqual(httpd_defaults.get_default_stream(), "2.2")
        httpd_profile_streams = (
            httpd_defaults.get_streams_with_default_profiles()
        )
        self.assertEqual(len(httpd_profile_streams), 2)
        self.assertTrue("2.2" in httpd_profile_streams)
        self.assertTrue("2.8" in httpd_profile_streams)
        self.assertEqual(
            len(httpd_defaults.get_default_profiles_for_stream("2.2")), 2
        )
        self.assertTrue(
            "client" in httpd_defaults.get_default_profiles_for_stream("2.2")
        )
        self.assertTrue(
            "server" in httpd_defaults.get_default_profiles_for_stream("2.2")
        )
        self.assertTrue(
            "notreal" in httpd_defaults.get_default_profiles_for_stream("2.8")
        )

        self.assertEqual(
            httpd_defaults.get_default_stream("workstation"), "2.4"
        )
        httpd_profile_streams = httpd_defaults.get_streams_with_default_profiles(
            "workstation"
        )
        self.assertEqual(len(httpd_profile_streams), 2)
        self.assertTrue("2.4" in httpd_profile_streams)
        self.assertTrue("2.6" in httpd_profile_streams)

        self.assertEqual(
            len(
                httpd_defaults.get_default_profiles_for_stream(
                    "2.4", "workstation"
                )
            ),
            1,
        )
        self.assertEqual(
            len(
                httpd_defaults.get_default_profiles_for_stream(
                    "2.6", "workstation"
                )
            ),
            3,
        )

        # Get another set of objects that will override the default stream for
        # nodejs
        override_nodejs_index = Modulemd.ModuleIndex()
        override_nodejs_index.update_from_file(
            path.join(self.test_data_path, "overriding-nodejs.yaml"), True
        )

        # Test that adding both of these at the same priority level results in
        # the no default stream
        merger = Modulemd.ModuleIndexMerger()
        merger.associate_index(base_index, 0)
        merger.associate_index(override_nodejs_index, 0)

        merged_index = merger.resolve()
        self.assertIsNotNone(merged_index)

        nodejs = merged_index.get_module("nodejs")
        self.assertIsNotNone(nodejs)

        nodejs_defaults = nodejs.get_defaults()
        self.assertIsNotNone(nodejs_defaults)
        self.assertIsNone(nodejs_defaults.get_default_stream())

        # Get another set of objects that will override the above
        override_index = Modulemd.ModuleIndex()
        override_index.update_from_file(
            path.join(self.test_data_path, "overriding.yaml"), True
        )

        # Test that override_index at a higher priority level succeeds
        # Test that adding both of these at the same priority level fails
        # with a merge conflict.
        # Use randomly-selected high and low values to make sure we don't have
        # sorting issues.
        merger = Modulemd.ModuleIndexMerger()
        random_low = random.randint(1, 100)
        random_high = random.randint(101, 999)
        print(
            "Low priority: %d, High priority: %d" % (random_low, random_high)
        )
        merger.associate_index(base_index, random_low)
        merger.associate_index(override_index, random_high)

        merged_index = merger.resolve()
        self.assertIsNotNone(merged_index)

        # Validate merged results

        # HTTPD
        httpd_defaults = merged_index.get_module("httpd").get_defaults()
        self.assertIsNotNone(httpd_defaults)
        self.assertEqual(httpd_defaults.get_default_stream(), "2.4")
        httpd_profile_streams = (
            httpd_defaults.get_streams_with_default_profiles()
        )
        self.assertEqual(len(httpd_profile_streams), 2)
        self.assertTrue("2.2" in httpd_profile_streams)
        self.assertTrue("2.4" in httpd_profile_streams)
        self.assertEqual(
            len(httpd_defaults.get_default_profiles_for_stream("2.2")), 2
        )
        self.assertTrue(
            "client" in httpd_defaults.get_default_profiles_for_stream("2.2")
        )
        self.assertTrue(
            "server" in httpd_defaults.get_default_profiles_for_stream("2.2")
        )
        self.assertTrue(
            "client" in httpd_defaults.get_default_profiles_for_stream("2.4")
        )
        self.assertTrue(
            "server" in httpd_defaults.get_default_profiles_for_stream("2.4")
        )

        self.assertEqual(
            httpd_defaults.get_default_stream("workstation"), "2.8"
        )
        httpd_profile_streams = httpd_defaults.get_streams_with_default_profiles(
            "workstation"
        )
        self.assertEqual(len(httpd_profile_streams), 3)
        self.assertTrue("2.4" in httpd_profile_streams)
        self.assertTrue("2.6" in httpd_profile_streams)
        self.assertTrue("2.8" in httpd_profile_streams)
        self.assertEqual(
            len(
                httpd_defaults.get_default_profiles_for_stream(
                    "2.4", "workstation"
                )
            ),
            1,
        )
        self.assertEqual(
            len(
                httpd_defaults.get_default_profiles_for_stream(
                    "2.6", "workstation"
                )
            ),
            3,
        )
        self.assertEqual(
            len(
                httpd_defaults.get_default_profiles_for_stream(
                    "2.8", "workstation"
                )
            ),
            4,
        )

    def test_merger_with_real_world_data(self):
        fedora_index = Modulemd.ModuleIndex()
        fedora_index.update_from_file(
            path.join(self.test_data_path, "f29.yaml"), True
        )

        updates_index = Modulemd.ModuleIndex()
        updates_index.update_from_file(
            path.join(self.test_data_path, "f29-updates.yaml"), True
        )

        merger = Modulemd.ModuleIndexMerger()
        merger.associate_index(fedora_index, 0)
        merger.associate_index(updates_index, 0)
        merged_index = merger.resolve()

        self.assertIsNotNone(merged_index)

    def test_merger_with_modified(self):
        pass

    def test_strict_default_streams(self):
        merger = Modulemd.ModuleIndexMerger.new()

        for stream in ("27", "38"):
            default = """
---
document: modulemd-defaults
version: 1
data:
    module: python
    stream: %s
...
""" % (
                stream
            )

            index = Modulemd.ModuleIndex()
            index.update_from_string(default, strict=True)
            merger.associate_index(index, 0)

        with self.assertRaisesRegex(
            gi.repository.GLib.GError,
            "Default stream mismatch in module python",
        ):
            merger.resolve_ext(True)

    def test_merge_add_only(self):
        base_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            base_idx.update_from_file(
                path.join(self.test_data_path, "merger", "base.yaml"), True
            )
        )

        add_only_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            add_only_idx.update_from_file(
                path.join(self.test_data_path, "merger", "add_only.yaml"), True
            )
        )

        merger = Modulemd.ModuleIndexMerger()
        merger.associate_index(base_idx, 0)
        merger.associate_index(add_only_idx, 0)

        merged_idx = merger.resolve()
        self.assertIsNotNone(merged_idx)

        httpd = merged_idx.get_module("httpd")
        self.assertIsNotNone(httpd)
        httpd_defs = httpd.get_defaults()

        self.assertEqual(httpd_defs.get_default_stream(), "2.8")
        expected_profile_defs = {
            "2.2": set(["client", "server"]),
            "2.8": set(["notreal"]),
            "2.10": set(["notreal"]),
        }

        for stream in expected_profile_defs.keys():
            self.assertEqual(
                set(httpd_defs.get_default_profiles_for_stream(stream)),
                expected_profile_defs[stream],
            )

        self.assertEqual(httpd_defs.get_default_stream("workstation"), "2.4")

    def test_merge_add_conflicting_stream(self):
        base_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            base_idx.update_from_file(
                path.join(self.test_data_path, "merger", "base.yaml"), True
            )
        )

        add_only_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            add_only_idx.update_from_file(
                path.join(
                    self.test_data_path,
                    "merger",
                    "add_conflicting_stream.yaml",
                ),
                True,
            )
        )

        merger = Modulemd.ModuleIndexMerger()
        merger.associate_index(base_idx, 0)
        merger.associate_index(add_only_idx, 0)

        merged_idx = merger.resolve()
        self.assertIsNotNone(merged_idx)

        psql = merged_idx.get_module("postgresql")
        self.assertIsNotNone(psql)

        psql_defs = psql.get_defaults()
        self.assertIsNotNone(psql_defs)

        self.assertIsNone(psql_defs.get_default_stream())

        expected_profile_defs = {
            "8.1": set(["client", "server", "foo"]),
            "8.2": set(["client", "server", "foo"]),
        }

        for stream in expected_profile_defs.keys():
            self.assertEqual(
                set(psql_defs.get_default_profiles_for_stream(stream)),
                expected_profile_defs[stream],
            )

    def test_merge_add_conflicting_stream_and_profile_modified(self):
        base_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            base_idx.update_from_file(
                path.join(self.test_data_path, "merger", "base.yaml"), True
            )
        )

        add_conflicting_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            add_conflicting_idx.update_from_file(
                path.join(
                    self.test_data_path,
                    "merger",
                    "add_conflicting_stream_and_profile_modified.yaml",
                ),
                True,
            )
        )

        merger = Modulemd.ModuleIndexMerger()
        merger.associate_index(base_idx, 0)
        merger.associate_index(add_conflicting_idx, 0)

        merged_idx = merger.resolve()
        self.assertIsNotNone(merged_idx)

        psql = merged_idx.get_module("postgresql")
        self.assertIsNotNone(psql)

        psql_defs = psql.get_defaults()
        self.assertIsNotNone(psql_defs)

        self.assertEqual(psql_defs.get_default_stream(), "8.2")

        expected_profile_defs = {
            "8.1": set(["client", "server"]),
            "8.2": set(["client", "server", "foo"]),
            "8.3": set(["client", "server"]),
        }

        for stream in expected_profile_defs:
            self.assertEqual(
                set(psql_defs.get_default_profiles_for_stream(stream)),
                expected_profile_defs[stream],
            )

    def test_merge_incompatible_streams(self):
        # This test verifies that if we encounter two streams with the same
        # NSVCA, but different content, we only retain one of them.
        # Note: the specification of the merger states that the behavior is
        # undefined, so we will only validate that the merge completes and
        # it only contains a single stream
        merger = Modulemd.ModuleIndexMerger.new()

        base_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            base_idx.update_from_file(
                path.join(self.test_data_path, "merger", "conflict_base.yaml"),
                True,
            )
        )
        merger.associate_index(base_idx, 0)

        bad_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            bad_idx.update_from_file(
                path.join(self.test_data_path, "merger", "conflict_base.yaml"),
                True,
            )
        )
        # Modify the stream to have a different summary
        stream = bad_idx.search_streams_by_nsvca_glob()[0]
        stream.set_summary("Invalid summary, should not merge successfully")
        merger.associate_index(bad_idx, 0)

        merged_idx = merger.resolve()
        all_streams = merged_idx.search_streams_by_nsvca_glob()
        self.assertEqual(len(all_streams), 1)
        self.assertEqual(all_streams[0].props.module_name, "nodejs")

    def test_merge_order_independent_obsoletes(self):
        # This test verifies that if we encounter two obsoletes with the same
        # module, stream and context we associate the newer one with exisitng
        # stream
        idx = Modulemd.ModuleIndex.new()
        stream = Modulemd.ModuleStream.new(2, "nodejs", "8.0")
        stream.props.context = "42"
        res = idx.add_module_stream(stream)

        newer_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            newer_idx.update_from_file(
                path.join(
                    self.test_data_path,
                    "merger",
                    "newer_obsoletes_resseted.yaml",
                ),
                True,
            )
        )

        base_idx = Modulemd.ModuleIndex()
        self.assertTrue(
            base_idx.update_from_file(
                path.join(
                    self.test_data_path, "merger", "base_obsoletes.yaml"
                ),
                True,
            )
        )

        merger = Modulemd.ModuleIndexMerger.new()
        merger.associate_index(idx, 0)
        merger.associate_index(newer_idx, 0)
        merger.associate_index(base_idx, 0)
        merged_idx = merger.resolve()

        module = merged_idx.get_module("nodejs")
        self.assertEqual(len(module.get_obsoletes()), 2)

        all_streams = merged_idx.search_streams_by_nsvca_glob()
        self.assertEqual(len(all_streams), 1)
        self.assertEqual(all_streams[0].props.module_name, "nodejs")

        obsoletes = all_streams[0].get_obsoletes_resolved()
        # obsoletes is None because it has reset: true
        self.assertEqual(obsoletes, None)

        # Merge the indexes in reverse order
        merger = Modulemd.ModuleIndexMerger.new()
        merger.associate_index(idx, 0)
        merger.associate_index(base_idx, 0)
        merger.associate_index(newer_idx, 0)
        merged_idx = merger.resolve()

        module = merged_idx.get_module("nodejs")
        self.assertEqual(len(module.get_obsoletes()), 2)

        all_streams = merged_idx.search_streams_by_nsvca_glob()
        self.assertEqual(len(all_streams), 1)
        self.assertEqual(all_streams[0].props.module_name, "nodejs")

        obsoletes = all_streams[0].get_obsoletes_resolved()
        # obsoletes is None because it has reset: true
        self.assertEqual(obsoletes, None)


if __name__ == "__main__":
    unittest.main()
