#!/usr/bin/python3

import os
import sys

try:
    import unittest
    import gi
    gi.require_version('Modulemd', os.getenv('MODULEMD_NSVERSION'))
    from gi.repository import Modulemd
    from gi.repository import GLib
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit (77)

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

    def test_issue25(self):
        mmd = Modulemd.Module.new_from_file("%s/test_data/issue25.yaml" %
                                            os.getenv('MESON_SOURCE_ROOT'))
        assert mmd.peek_rpm_buildopts() != {}
        assert mmd.peek_rpm_buildopts()['macros'] == '%my_macro 1'
        mmd.upgrade()
        assert mmd.peek_rpm_buildopts() != {}
        assert mmd.peek_rpm_buildopts()['macros'] == '%my_macro 1'
        mmd.set_rpm_buildopts({'macros': '%my_macro 1'})
        assert mmd.peek_rpm_buildopts() != {}
        assert mmd.peek_rpm_buildopts()['macros'] == '%my_macro 1'
        dumped = mmd.dumps()
        print ("YAML:\n%s" % dumped, file=sys.stderr)
        mmd2 = Modulemd.Module.new_from_string(dumped)
        assert mmd2.peek_rpm_buildopts() != {}
        assert mmd2.peek_rpm_buildopts()['macros'] == '%my_macro 1'

if __name__ == '__main__':
    unittest.main()
