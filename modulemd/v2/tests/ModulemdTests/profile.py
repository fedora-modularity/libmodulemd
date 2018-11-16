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
    from gi.repository import Modulemd
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase


class TestProfile(TestBase):

    def test_constructor(self):
        # Test that the new() function works
        p = Modulemd.Profile.new('testprofile')
        assert p
        assert p.props.name == 'testprofile'
        assert p.get_name() == 'testprofile'
        assert p.props.description is None
        assert p.get_description() is None
        assert p.get_rpms_as_strv() == []

        # Test that keywork name is accepted
        p = Modulemd.Profile(name='testprofile')
        assert p
        assert p.props.name == 'testprofile'
        assert p.get_name() == 'testprofile'
        assert p.props.description is None
        assert p.get_description() is None
        assert p.get_rpms_as_strv() == []

        # Test that init works with name and description
        p = Modulemd.Profile(name='testprofile', description='A test profile')
        assert p
        assert p.props.name == 'testprofile'
        assert p.get_name() == 'testprofile'
        assert p.props.description == 'A test profile'
        assert p.get_description() == 'A test profile'
        assert p.get_rpms_as_strv() == []

        # Test that we fail without name
        with self.assertRaises(TypeError) as cm:
            Modulemd.Profile.new(None)
        assert 'does not allow None as a value' in cm.exception.__str__()

        with self.expect_signal():
            Modulemd.Profile()

        with self.expect_signal():
            Modulemd.Profile(name=None)

    def test_copy(self):
        p_orig = Modulemd.Profile(name='testprofile')
        p = p_orig.copy()
        assert p
        assert p.props.name == 'testprofile'
        assert p.get_name() == 'testprofile'
        assert p.props.description is None
        assert p.get_description() is None
        assert p.get_rpms_as_strv() == []

        p_orig.set_description('Test profile')
        p.add_rpm('test2')
        p.add_rpm('test3')
        p.add_rpm('test1')

        p = p_orig.copy()
        assert p
        assert p.props.name == 'testprofile'
        assert p.get_name() == 'testprofile'
        assert p.props.description == 'Test profile'
        assert p.get_description() == 'Test profile'
        assert p.get_rpms_as_strv() == ['test1', 'test2', 'test3']

    def test_get_name(self):
        p = Modulemd.Profile(name='testprofile')

        assert p.get_name() == 'testprofile'
        assert p.props.name == 'testprofile'

        with self.expect_signal():
            p.props.name = 'notadrill'

    def test_get_set_description(self):
        p = Modulemd.Profile(name='testprofile')

        assert p.props.description is None
        assert p.get_description() is None

        p.set_description('foobar')
        assert p.props.description == 'foobar'
        assert p.get_description() == 'foobar'

        p.props.description = 'barfoo'
        assert p.props.description == 'barfoo'
        assert p.get_description() == 'barfoo'

        p.props.description = None
        assert p.props.description is None
        assert p.get_description() is None

    def test_rpms(self):
        p = Modulemd.Profile(name='testprofile')

        assert p.get_rpms_as_strv() == []

        p.add_rpm('test2')
        assert p.get_rpms_as_strv() == ['test2']

        p.add_rpm('test3')
        p.add_rpm('test1')
        assert p.get_rpms_as_strv() == ['test1', 'test2', 'test3']

        p.add_rpm('test2')
        assert p.get_rpms_as_strv() == ['test1', 'test2', 'test3']

        p.remove_rpm('test1')
        assert p.get_rpms_as_strv() == ['test2', 'test3']


if __name__ == '__main__':
    unittest.main()
