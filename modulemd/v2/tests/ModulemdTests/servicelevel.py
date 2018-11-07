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

import signal


class TestServiceLevel(unittest.TestCase):

    def test_constructors(self):
        # Test that the new() function works
        sl = Modulemd.ServiceLevel.new('foo')
        assert sl
        assert sl.props.name == 'foo'
        assert sl.get_name() == 'foo'
        assert sl.props.eol is None
        assert sl.get_eol() is None
        assert sl.get_eol_string() is None

        # Test that standard object instatiation works with a name
        sl = Modulemd.ServiceLevel(name='foo')
        assert sl
        assert sl.props.name == 'foo'
        assert sl.get_name() == 'foo'
        assert sl.props.eol is None
        assert sl.get_eol() is None
        assert sl.get_eol_string() is None

        # Test that standard object instantiation works with a name and EOL
        sl = Modulemd.ServiceLevel(
            name='foo',
            eol=GLib.Date.new_dmy(7, 11, 2018))
        assert sl
        assert sl.props.name == 'foo'
        assert sl.get_name() == 'foo'
        assert sl.props.eol is not None
        assert sl.props.eol.get_day() == 7
        assert sl.props.eol.get_month() == 11
        assert sl.props.eol.get_year() == 2018
        assert sl.get_eol() is not None
        assert sl.get_eol().get_day() == 7
        assert sl.get_eol().get_month() == 11
        assert sl.get_eol().get_year() == 2018
        assert sl.get_eol_string() is not None
        assert sl.get_eol_string() == '2018-11-07'

        # Test that we fail if we call new() with a None name
        try:
            sl = Modulemd.ServiceLevel.new(None)
        except TypeError as e:
            assert 'does not allow None as a value' in e.__str__()

        # Test that we fail if object is instantiated without a name
        saved_signal = signal.signal(signal.SIGTRAP, lambda sig, frame: None)
        sl = Modulemd.ServiceLevel()
        signal.signal(signal.SIGTRAP, saved_signal)

        # Test that we fail if object is instantiated with a None name
        saved_signal = signal.signal(signal.SIGTRAP, lambda sig, frame: None)
        sl = Modulemd.ServiceLevel(name=None)
        signal.signal(signal.SIGTRAP, saved_signal)

    def test_get_name(self):
        sl = Modulemd.ServiceLevel.new('foo')

        assert sl.get_name() == 'foo'
        assert sl.props.name == 'foo'

        # This property is not writable, make sure it fails to attempt it
        saved_signal = signal.signal(signal.SIGTRAP, lambda sig, frame: None)
        sl.props.name = 'bar'
        signal.signal(signal.SIGTRAP, saved_signal)

    def test_get_set_eol(self):
        sl = Modulemd.ServiceLevel.new('foo')

        # Test that EOL is initialized to None
        assert sl.get_eol() is None
        assert sl.props.eol is None

        # Test the set_eol() method
        eol = GLib.Date.new_dmy(7, 11, 2018)
        sl.set_eol(eol)

        for returned_eol in [sl.get_eol(), sl.props.eol]:
            assert returned_eol is not None
            assert returned_eol.get_day() == eol.get_day()
            assert returned_eol.get_month() == eol.get_month()
            assert returned_eol.get_year() == eol.get_year()
        assert sl.get_eol_string() == '2018-11-07'

        # Test the set_eol_ymd() method
        sl.set_eol_ymd(2019, 12, 3)

        for returned_eol in [sl.get_eol(), sl.props.eol]:
            assert returned_eol is not None
            assert returned_eol.get_day() == 3
            assert returned_eol.get_month() == 12
            assert returned_eol.get_year() == 2019
        assert sl.get_eol_string() == '2019-12-03'

        # Try setting some invalid dates
        # An initialized but unset date
        eol = GLib.Date.new()
        sl.set_eol(eol)
        assert sl.get_eol() is None
        assert sl.props.eol is None

        # There is no February 31
        sl.set_eol_ymd(2011, 2, 31)
        assert sl.get_eol() is None
        assert sl.props.eol is None

if __name__ == '__main__':
    unittest.main()
