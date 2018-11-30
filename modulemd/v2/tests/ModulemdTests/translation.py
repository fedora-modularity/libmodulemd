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


class TestTranslation(TestBase):

    def test_constructors(self):
        # Test that the new() function works
        t = Modulemd.Translation.new(1, "testmodule", "teststream", 42)
        assert t

        # Test that keywords are accepted
        t = Modulemd.Translation(
            version=1,
            module_name='testmodule',
            module_stream='teststream',
            modified=42)
        assert t
        assert t.validate()
        assert t.get_locales() == []

    def test_copy(self):
        t_orig = Modulemd.Translation.new(1, "testmodule", "teststream", 42)
        t = t_orig.copy()
        assert t
        assert t.validate()
        assert t.get_locales() == []


if __name__ == '__main__':
    unittest.main()
