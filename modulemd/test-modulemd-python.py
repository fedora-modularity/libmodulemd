#!/usr/bin/python3

import os
import unittest

import gi
gi.require_version('Modulemd', os.getenv('MODULEMD_NSVERSION'))
from gi.repository import Modulemd
from gi.repository import GLib

class TestIssues(unittest.TestCase):

    def test_issue24(self):
        # Verify that we can handle boolean variant types
        name = stream = context = "x"
        version = 10

        mmd = Modulemd.Module()
        mmd.set_mdversion(2)
        mmd.set_name(name)
        mmd.set_stream(stream)
        mmd.set_version(int(version))
        mmd.set_context(context)
        mmd.set_summary("foo")
        mmd.set_description("foo")
        licenses = Modulemd.SimpleSet()
        licenses.add("GPL")
        mmd.set_module_licenses(licenses)

        d = {"x": GLib.Variant('b', True)}
        mmd.set_xmd(d)
        mmd.dumps()

if __name__ == '__main__':
    unittest.main()
