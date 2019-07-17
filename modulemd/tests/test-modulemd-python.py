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

import os
import sys

try:
    import unittest
    import gi
    gi.require_version('Modulemd', '1.0')
    from gi.repository import Modulemd
    from gi.repository import GLib
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)


class TestStandard(unittest.TestCase):

    def test_version(self):
        # Make sure that we are linking against the correct version
        expected_version = os.getenv('MODULEMD_VERSION')
        assert expected_version is None \
            or expected_version == Modulemd.get_version()

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

        assert failures[0].props.gerror.message.startswith(
            'Document type is not recognized [')

        assert failures[1].props.gerror.message == \
            'Document type was specified more than once'

        assert failures[2].props.gerror.message == \
            'Document version was specified more than once'

        assert failures[3].props.gerror.message == \
            'Document type was not a scalar value'

        assert failures[4].props.gerror.message.startswith(
            'Document type is not recognized [')

        assert failures[5].props.gerror.message.startswith(
            'Document type is not recognized [')

        assert failures[6].props.gerror.message == \
            'Unknown modulemd defaults version'

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
            0].props.gerror.message.startswith(
                'Received scalar where sequence expected [')


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
        mmd2 = Modulemd.Module.new_from_string(dumped)
        assert mmd2.peek_rpm_buildopts() != {}
        assert mmd2.peek_rpm_buildopts()['macros'] == '%my_macro 1'

    def test_issue33(self):
        # We had a bug where this was returning an array as (transfer full)
        # instead of (transfer container) which resulted in the GI python
        # double-freeing memory.
        defs = Modulemd.Module.new_all_from_file_ext(
            "%s/mod-defaults/spec.v1.yaml" % os.getenv('MESON_SOURCE_ROOT'))
        assert defs

    def test_issue77(self):
        # This would crash on a type constraint accepting a signed value
        component = Modulemd.ComponentRpm(name="pkg1",
                                          rationale="Just because",
                                          buildorder=-1)
        assert component.props.buildorder == -1

        component.props.buildorder = -2
        assert component.get_buildorder() == -2

        component.set_buildorder(5)
        assert component.props.buildorder == 5

    def test_issue80(self):
        mmd_translation = Modulemd.Translation(module_name="modulename",
                                               module_stream="modulestream",
                                               mdversion=1,
                                               modified=201809041500)
        entry = Modulemd.TranslationEntry(locale='en_US')
        mmd_translation.add_entry(entry)

        # Would crash attempting to dump to YAML
        try:
            yaml_output = mmd_translation.dumps()
        except GLib.GError as err:
            # A proper exception is expected here
            pass

    def test_issue85(self):
        """
        Component module refs are lost when dumping to YAML
        """
        mmd = Modulemd.Module().new_from_string("""
document: modulemd
version: 1
data:
    summary: A test module in all its beautiful beauty.
    description: >-
        This module demonstrates how to write simple modulemd files And
        can be used for testing the build and release pipeline.
    license:
        module: [ MIT ]
    dependencies:
        buildrequires:
            platform: el8
        requires:
            platform: el8
    references:
        community: https://fedoraproject.org/wiki/Modularity
        documentation: https://fedoraproject.org/wiki/Fedora_Packaging_Guidelines_for_Modules
        tracker: https://taiga.fedorainfracloud.org/project/modularity
    profiles:
        default:
            rpms:
                - acl
    api:
        rpms:
            - acl
    components:
        rpms:
            acl:
                rationale: needed
                ref: rhel-8.0
        modules:
            testmodule:
                ref: private-x
                rationale: Testing module inclusion.
                buildorder: 10
""")
        assert mmd.get_module_components(
        )['testmodule'].peek_ref() == 'private-x'

        mmd2 = Modulemd.Module.copy(mmd)
        assert mmd2.get_module_components(
        )['testmodule'].peek_ref() == 'private-x'

        mmd3 = Modulemd.Module.new_from_string(mmd.dumps())
        assert mmd3.get_module_components(
        )['testmodule'].peek_ref() == 'private-x'

    def test_issue88(self):
        # Here, load the same file twice.
        # All entries in these two lists should be duplicates of each other.
        objects_from_repo_a = Modulemd.objects_from_file(
            '%s/test_data/issue88.yaml' % os.getenv('MESON_SOURCE_ROOT'))
        objects_from_repo_b = Modulemd.objects_from_file(
            '%s/test_data/issue88.yaml' % os.getenv('MESON_SOURCE_ROOT'))

        # Test at the same priority level
        prioritizer = Modulemd.Prioritizer()
        prioritizer.add(objects_from_repo_a, 0)
        prioritizer.add(objects_from_repo_b, 0)
        supposedly_merged_objects = prioritizer.resolve()

        # Since they are all duplicates, they should be the same size.
        assert len(objects_from_repo_a) == len(objects_from_repo_b)
        assert len(objects_from_repo_a) == len(supposedly_merged_objects)

        # Test at different priorities
        prioritizer = Modulemd.Prioritizer()
        prioritizer.add(objects_from_repo_a, 0)
        prioritizer.add(objects_from_repo_b, 1)
        supposedly_merged_objects = prioritizer.resolve()

        # Since they are all duplicates, they should be the same size.
        assert len(objects_from_repo_a) == len(objects_from_repo_b)
        assert len(objects_from_repo_a) == len(supposedly_merged_objects)

    def test_issue94(self):
        # Verify that we can read in a module stream with a zero module
        # version.
        stream = Modulemd.ModuleStream.new()
        stream.import_from_string("""
document: modulemd
version: 1
data:
    name: foo
    stream: bar
    version: 0
    summary: A test module in all its beautiful beauty.
    description: This module demonstrates how to write simple modulemd files And can be used for testing the build and release pipeline.
    license:
        module: [ MIT ]
    dependencies:
        buildrequires:
            platform: el8
        requires:
            platform: el8
    references:
        community: https://fedoraproject.org/wiki/Modularity
        documentation: https://fedoraproject.org/wiki/Fedora_Packaging_Guidelines_for_Modules
        tracker: https://taiga.fedorainfracloud.org/project/modularity
    profiles:
        default:
            rpms:
                - acl
    api:
        rpms:
            - acl
    components:
        rpms:
            acl:
                rationale: needed
                ref: rhel-8.0
        modules:
            testmodule:
                ref: private-x
                rationale: Testing module inclusion.
                buildorder: 10
""")
        assert stream is not None
        assert stream.props.version == 0

        # Verify that get_nsvc() works with a zeroed module version
        assert stream.get_nsvc() == 'foo:bar:0'


class TestPrioritizer(unittest.TestCase):

    def test_latest_version(self):
        # Load YAML with two versions of the same (name, stream, context)
        objects = Modulemd.objects_from_file(
            '%s/test_data/latest_version.yaml' %
            os.getenv('MESON_SOURCE_ROOT'))

        prioritizer = Modulemd.Prioritizer()
        prioritizer.add(objects, 0)

        supposedly_merged_objects = prioritizer.resolve()

        # There should only be the latest one in the list
        print(supposedly_merged_objects)

        assert len(supposedly_merged_objects) == 1
        assert supposedly_merged_objects[0].props.name == 'foo'
        assert supposedly_merged_objects[0].props.stream == 'stream-name'
        assert supposedly_merged_objects[0].props.context == 'c0ffee43'
        assert supposedly_merged_objects[0].props.version == 20180928144203


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

        assert 'django:1.6:20180307130104:c2c572ec' in streams


class TestImprovedModule (unittest.TestCase):

    def test_dump(self):
        (module_index, failures) = Modulemd.index_from_file(
            "%s/test_data/long-valid.yaml" % os.getenv('MESON_SOURCE_ROOT'))
        assert len(failures) == 0

        assert 'nodejs' in module_index

        yaml_out = module_index['nodejs'].dumps()

        assert 'scalable network applications' in yaml_out
        assert '20180308155546' in yaml_out
        assert 'document: modulemd-defaults' in yaml_out
        assert 'module: nodejs' in yaml_out

    def test_custom_repo(self):
        (module_index, failures) = Modulemd.index_from_file(
            "%s/spec.v2.yaml" % os.getenv('MESON_SOURCE_ROOT'))
        assert len(failures) == 0

        foo_module = module_index['foo']
        assert foo_module

        foo_stream = foo_module.get_stream_by_nsvc(
            'foo:latest:20160927144203:c0ffee43')
        assert foo_stream

        rpm_components = foo_stream.get_rpm_components()
        assert rpm_components

        bar_component = rpm_components['bar']
        assert bar_component

        repo = bar_component.dup_repository()
        assert repo

        assert repo == "https://pagure.io/bar.git"


if __name__ == '__main__':
    unittest.main()
