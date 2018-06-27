#!/usr/bin/python3

# test-modulemd-python.py
#
# Copyright (C) 2018 Stephen Gallagher
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
    sys.exit(77)


class TestStandard(unittest.TestCase):

    def test_version(self):
        # Make sure that we are linking against the correct version
        assert os.getenv('MODULEMD_VERSION') == Modulemd.get_version()

    def test_failures(self):
        (objects, failures) = Modulemd.objects_from_file_ext(
            "%s/test_data/mixed-v2.yaml" % os.getenv('MESON_SOURCE_ROOT'))

        # There should be two valid documents
        assert len(objects) == 2

        # And seven failed documents
        assert len(failures) == 7

        # Validate that all failures are due to failed document type detection
        for failure in failures:
            assert failure.props.gerror.code == 3

        assert failures[
            0].props.gerror.message == 'Document type is not recognized'
        assert failures[
            1].props.gerror.message == 'Document type was specified more than once'
        assert failures[
            2].props.gerror.message == 'Document version was specified more than once'
        assert failures[
            3].props.gerror.message == 'Document type was not a scalar value'
        assert failures[
            4].props.gerror.message == 'Document type is not recognized'
        assert failures[
            5].props.gerror.message == 'Document type is not recognized'
        assert failures[
            6].props.gerror.message == 'Unknown modulemd defaults version'

        # Read in a file that's definitely not YAML
        try:
            (objects, failures) = Modulemd.objects_from_file_ext(
                "%s/COPYING" % os.getenv('MESON_SOURCE_ROOT'))
        except GLib.GError as e:
            # Verify that it's a parser error
            assert e.message == "Parser error"

        # Read in a file that fails modulemd validation
        (objects, failures) = Modulemd.objects_from_file_ext(
            "%s/test_data/issue14-mismatch.yaml"
            % os.getenv('MESON_SOURCE_ROOT'))

        assert len(objects) == 0
        assert len(failures) == 1

        assert failures[
            0].props.gerror.message == 'Received scalar where sequence expected'


class TestDefaults(unittest.TestCase):

    def test_spec(self):
        defaults = Modulemd.Defaults.new_from_file(
            '%s/mod-defaults/spec.v1.yaml' % os.getenv('MESON_SOURCE_ROOT'))
        assert defaults

        assert defaults.props.profile_defaults['bar'].contains('baz')
        assert defaults.props.profile_defaults['bar'].contains('snafu')

        assert defaults.props.intents
        assert 'desktop' in defaults.props.intents
        assert defaults.props.intents['desktop'].props.default_stream == 'y.z'
        assert 'y.z' in defaults.props.intents[
            'desktop'].props.profile_defaults
        assert 'blah' in defaults.props.intents[
            'desktop'].props.profile_defaults['y.z'].dup()
        assert 'x.y' in defaults.props.intents[
            'desktop'].props.profile_defaults
        assert 'other' in defaults.props.intents[
            'desktop'].props.profile_defaults['x.y'].dup()

        assert 'server' in defaults.props.intents
        assert defaults.props.intents['server'].props.default_stream == 'x.y'
        assert 'x.y' in defaults.props.intents['server'].props.profile_defaults
        assert not defaults.props.intents[
            'server'].props.profile_defaults['x.y'].dup()

        nointents = Modulemd.Defaults.new_from_file(
            '%s/mod-defaults/ex2.yaml' % os.getenv('MESON_SOURCE_ROOT'))
        assert nointents

        assert not nointents.props.intents

    def test_construction(self):
        defaults = Modulemd.Defaults()
        defaults.set_version(1)
        defaults.set_module_name("foo")
        defaults.set_default_stream("stable")
        defaults.set_profiles_for_stream("stable", ['default'])

        intent = Modulemd.Intent.new('server')
        intent.set_default_stream('stable')
        intent.set_profiles_for_stream('stable', ['server', 'microservice'])
        intent.set_profiles_for_stream('PoC', ['cloud', 'microservice'])
        defaults.add_intent(intent)

        assert 'server' in defaults.props.intents
        assert 'server' in defaults.props.intents[
            'server'].props.profile_defaults['stable'].dup()
        assert 'microservice' in defaults.props.intents[
            'server'].props.profile_defaults['stable'].dup()
        assert 'cloud' in defaults.props.intents[
            'server'].props.profile_defaults['PoC'].dup()
        assert 'microservice' in defaults.props.intents[
            'server'].props.profile_defaults['PoC'].dup()


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
        print("YAML:\n%s" % dumped, file=sys.stderr)
        mmd2 = Modulemd.Module.new_from_string(dumped)
        assert mmd2.peek_rpm_buildopts() != {}
        assert mmd2.peek_rpm_buildopts()['macros'] == '%my_macro 1'

    def test_issue33(self):
        # We had a bug where this was returning an array as (transfer full)
        # instead of (transfer container) which resulted in the GI python
        # double-freeing memory.
        defs = Modulemd.Module.new_all_from_file_ext(
            "%s/mod-defaults/spec.v1.yaml" % os.getenv('MESON_SOURCE_ROOT'))
        print(defs)


class TestIntent(unittest.TestCase):

    def test_basic(self):
        intent = Modulemd.Intent.new("intent_name")
        intent.set_default_stream("default stream")

        assert intent.props.intent_name == 'intent_name'
        assert intent.props.default_stream == 'default stream'

        intent.set_profiles_for_stream("default stream", ['default', 'server'])

        contents = intent.props.profile_defaults['default stream']
        assert contents.contains('default')
        assert contents.contains('server')

        assert intent.props.profile_defaults[
            'default stream'].contains('default')
        assert intent.props.profile_defaults[
            'default stream'].contains('server')

        intent_copy = intent.copy()
        assert intent_copy.props.intent_name == 'intent_name'
        assert intent_copy.props.default_stream == 'default stream'
        assert intent_copy.props.profile_defaults[
            'default stream'].contains('default')
        assert intent_copy.props.profile_defaults[
            'default stream'].contains('server')


class TestIndexParser (unittest.TestCase):

    def test_file_parser(self):
        (module_index, failures) = Modulemd.index_from_file(
            "%s/test_data/long-valid.yaml" % os.getenv('MESON_SOURCE_ROOT'))

        assert len(failures) == 0

        assert 'django' in module_index

        assert module_index['django'].props.name == 'django'

        streams = module_index['django'].get_streams()

        assert '1.6' in streams


if __name__ == '__main__':
    unittest.main()
