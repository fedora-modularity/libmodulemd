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

import sys
try:
    import unittest
    import gi
    gi.require_version('Modulemd', '2.0')
    from gi.repository.Modulemd import ComponentRpm
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase


class TestComponentRpm(TestBase):

    def test_constructors(self):
        # Test that the new() function works
        rpm = ComponentRpm.new("testrpm")
        assert rpm
        assert rpm.props.buildorder == 0
        assert rpm.get_buildorder() == 0
        assert rpm.props.name == "testrpm"
        assert rpm.get_name() == "testrpm"
        assert rpm.props.rationale is None
        assert rpm.get_rationale() is None
        assert rpm.props.ref is None
        assert rpm.get_ref() is None
        assert rpm.props.repository is None
        assert rpm.get_repository() is None
        assert rpm.props.cache is None
        assert rpm.get_cache() is None
        assert rpm.get_arches_as_strv() == []
        assert rpm.get_multilib_arches_as_strv() == []

        # Test that object instantiation works
        rpm = ComponentRpm(
            buildorder=42,
            name='testrpm',
            rationale='Testing all the things',
            ref='someref',
            repository='somerepo',
            cache='somecache')
        assert rpm
        assert rpm.props.buildorder == 42
        assert rpm.get_buildorder() == 42
        assert rpm.props.name == 'testrpm'
        assert rpm.get_name() == 'testrpm'
        assert rpm.props.rationale == 'Testing all the things'
        assert rpm.get_rationale() == 'Testing all the things'
        assert rpm.props.ref == 'someref'
        assert rpm.get_ref() == 'someref'
        assert rpm.props.repository == 'somerepo'
        assert rpm.get_repository() == 'somerepo'
        assert rpm.props.cache == 'somecache'
        assert rpm.get_cache() == 'somecache'
        assert rpm.get_arches_as_strv() == []
        assert rpm.get_multilib_arches_as_strv() == []

    def test_copy(self):
        rpm_orig = ComponentRpm(
            buildorder=42,
            name='testrpm',
            rationale='Testing all the things',
            ref='someref',
            repository='somerepo',
            cache='somecache')
        rpm_orig.add_restricted_arch('x86_64')
        rpm_orig.add_restricted_arch('i386')
        rpm_orig.add_multilib_arch('ppc64le')
        rpm_orig.add_multilib_arch('s390x')

        rpm = rpm_orig.copy()
        assert rpm
        assert rpm.props.buildorder == 42
        assert rpm.get_buildorder() == 42
        assert rpm.props.name == 'testrpm'
        assert rpm.get_name() == 'testrpm'
        assert rpm.props.rationale == 'Testing all the things'
        assert rpm.get_rationale() == 'Testing all the things'
        assert rpm.props.ref == 'someref'
        assert rpm.get_ref() == 'someref'
        assert rpm.props.repository == 'somerepo'
        assert rpm.get_repository() == 'somerepo'
        assert rpm.props.cache == 'somecache'
        assert rpm.get_cache() == 'somecache'
        self.assertListEqual(rpm.get_arches_as_strv(), ['i386', 'x86_64'])
        self.assertListEqual(
            rpm.get_multilib_arches_as_strv(), [
                'ppc64le', 's390x'])

    def test_arches(self):
        rpm = ComponentRpm.new("testrpm")
        assert rpm

        rpm.add_restricted_arch('x86_64')
        rpm.add_restricted_arch('i386')
        self.assertListEqual(rpm.get_arches_as_strv(), ['i386', 'x86_64'])

        rpm.reset_arches()
        self.assertListEqual(rpm.get_arches_as_strv(), [])

    def test_multilib(self):
        rpm = ComponentRpm.new("testrpm")
        assert rpm

        rpm.add_multilib_arch('ppc64le')
        rpm.add_multilib_arch('s390x')
        self.assertListEqual(
            rpm.get_multilib_arches_as_strv(), [
                'ppc64le', 's390x'])

        rpm.reset_multilib_arches()
        self.assertListEqual(rpm.get_multilib_arches_as_strv(), [])


if __name__ == '__main__':
    unittest.main()
