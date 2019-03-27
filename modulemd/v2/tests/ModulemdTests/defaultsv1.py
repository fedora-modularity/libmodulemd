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


class TestDefaults(TestBase):

    def test_constructors(self):
        # Test that the new() function works
        defs = Modulemd.DefaultsV1.new('foo')
        assert defs
        assert defs.props.mdversion == Modulemd.DefaultsVersionEnum.ONE
        assert defs.get_mdversion() == Modulemd.DefaultsVersionEnum.ONE
        assert defs.props.module_name == 'foo'
        assert defs.get_module_name() == 'foo'

        # Test gobject instantiation
        defs = Modulemd.DefaultsV1(module_name='foo')
        assert defs
        assert defs.props.mdversion == Modulemd.DefaultsVersionEnum.ONE
        assert defs.get_mdversion() == Modulemd.DefaultsVersionEnum.ONE
        assert defs.props.module_name == 'foo'
        assert defs.get_module_name() == 'foo'

        # Test with no name
        with self.assertRaisesRegexp(TypeError, 'does not allow None as a value'):
            defs = Modulemd.DefaultsV1.new(None)

    def test_copy(self):
        defs = Modulemd.DefaultsV1.new('foo')
        assert defs

        copied_defs = defs.copy()
        assert copied_defs
        assert copied_defs.props.mdversion == defs.props.mdversion
        assert copied_defs.props.module_name == defs.props.module_name

        # Add some actual intents and streams and compare the copy
        defs.set_default_stream("latest")
        defs.set_default_stream("latest", "server_intent")

        copied_defs = defs.copy()
        assert copied_defs
        assert copied_defs.props.mdversion == defs.props.mdversion
        assert copied_defs.props.module_name == defs.props.module_name
        assert copied_defs.get_default_stream() == defs.get_default_stream()
        assert copied_defs.get_default_stream(
            "server_intent") == defs.get_default_stream("server_intent")

    def test_get_set_default_stream(self):
        defs = Modulemd.DefaultsV1.new('foo')
        assert defs
        defs.set_default_stream("latest")
        assert defs.get_default_stream() == "latest"

        defs.set_default_stream("experimental", "workstation_intent")
        assert defs.get_default_stream("workstation_intent") == "experimental"

        defs.set_default_stream(None)
        assert defs.get_default_stream() is None

        defs.set_default_stream(None, "minimal_intent")
        assert defs.get_default_stream("minimal_intent") is None

    def test_get_set_profiles(self):
        defs = Modulemd.DefaultsV1.new('foo')

        defs.add_default_profile_for_stream("latest", "client")
        defs.add_default_profile_for_stream("latest", "server")
        defs.add_default_profile_for_stream(
            "latest", "server", "server_intent")

        assert "client" in defs.get_default_profiles_for_stream(
            "latest")
        assert "client" in defs.get_default_profiles_for_stream(
            "latest")

    def test_validate(self):
        defs = Modulemd.Defaults.new(
            Modulemd.DefaultsVersionEnum.LATEST, 'foo')
        assert defs

        assert defs.validate()


if __name__ == '__main__':
    unittest.main()
