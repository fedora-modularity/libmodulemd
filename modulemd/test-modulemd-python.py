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

class TestStandard(unittest.TestCase):
    def test_failures (self):
        (objects, failures) = Modulemd.objects_from_file_ext (
            "%s/test_data/mixed-v2.yaml" % os.getenv('MESON_SOURCE_ROOT'))

        # There should be two valid documents
        assert len(objects) == 2

        # And seven failed documents
        assert len(failures) == 7

        # Validate that all failures are due to failed document type detection
        for failure in failures:
            assert failure.props.gerror.code == 3

        assert failures[0].props.gerror.message == 'Document type is not recognized'
        assert failures[1].props.gerror.message == 'Document type was specified more than once'
        assert failures[2].props.gerror.message == 'Document version was specified more than once'
        assert failures[3].props.gerror.message == 'Document type was not a scalar value'
        assert failures[4].props.gerror.message == 'Document type is not recognized'
        assert failures[5].props.gerror.message == 'Document type is not recognized'
        assert failures[6].props.gerror.message == 'Unknown modulemd defaults version'

        # Read in a file that's definitely not YAML
        try:
            (objects, failures) = Modulemd.objects_from_file_ext (
                "%s/COPYING" % os.getenv('MESON_SOURCE_ROOT'))
        except GLib.GError as e:
            # Verify that it's a parser error
            assert e.message == "Parser error"


        # Read in a file that fails modulemd validation
        (objects, failures) = Modulemd.objects_from_file_ext (
                "%s/test_data/issue14-mismatch.yaml"
                % os.getenv('MESON_SOURCE_ROOT'))

        assert len(objects) == 0
        assert len(failures) == 1

        assert failures[0].props.gerror.message == 'Received scalar where sequence expected'




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

    def test_issue33(self):
        # We had a bug where this was returning an array as (transfer full)
        # instead of (transfer container) which resulted in the GI python
        # double-freeing memory.
        defs = Modulemd.Module.new_all_from_file_ext(
            "%s/mod-defaults/spec.v1.yaml" % os.getenv('MESON_SOURCE_ROOT'))
        print (defs)

if __name__ == '__main__':
    unittest.main()
