import os
import sys
try:
    import unittest
    import gi

    gi.require_version('Modulemd', '2.0')
    from gi.repository import GLib
    from gi.repository import Modulemd
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase

modulestream_versions = [
    Modulemd.ModuleStreamVersionEnum.ONE,
    Modulemd.ModuleStreamVersionEnum.TWO]


class TestModuleStream(TestBase):

    def test_constructors(self):
        for version in modulestream_versions:

            # Test that the new() function works
            stream = Modulemd.ModuleStream.new(version, 'foo', 'latest')
            assert stream
            assert isinstance(stream, Modulemd.ModuleStream)

            assert stream.props.mdversion == version
            assert stream.get_mdversion() == version
            assert stream.props.module_name == 'foo'
            assert stream.get_module_name() == 'foo'
            assert stream.props.stream_name == 'latest'
            assert stream.get_stream_name() == 'latest'

            # Test that the new() function works without a stream name
            stream = Modulemd.ModuleStream.new(version, 'foo')
            assert stream
            assert isinstance(stream, Modulemd.ModuleStream)

            assert stream.props.mdversion == version
            assert stream.get_mdversion() == version
            assert stream.props.module_name == 'foo'
            assert stream.get_module_name() == 'foo'
            assert stream.props.stream_name is None
            assert stream.get_stream_name() is None

            # Test that the new() function works with no module name
            stream = Modulemd.ModuleStream.new(version, None, 'latest')
            assert stream
            assert isinstance(stream, Modulemd.ModuleStream)

            assert stream.props.mdversion == version
            assert stream.get_mdversion() == version
            assert stream.props.module_name is None
            assert stream.get_module_name() is None
            assert stream.props.stream_name == 'latest'
            assert stream.get_stream_name() == 'latest'

            # Test that the new() function works with no module or stream
            stream = Modulemd.ModuleStream.new(version)
            assert stream
            assert isinstance(stream, Modulemd.ModuleStream)

            assert stream.props.mdversion == version
            assert stream.get_mdversion() == version
            assert stream.props.module_name is None
            assert stream.get_module_name() is None
            assert stream.props.stream_name is None
            assert stream.get_stream_name() is None

        # Test that we cannot instantiate directly
        with self.assertRaisesRegex(TypeError, 'cannot create instance of abstract'):
            Modulemd.ModuleStream()

        # Test with a zero mdversion
        with self.assertRaisesRegex(TypeError, 'constructor returned NULL'):
            with self.expect_signal():
                defs = Modulemd.ModuleStream.new(0)

        # Test with an unknown mdversion
        with self.assertRaisesRegex(TypeError, 'constructor returned NULL'):
            with self.expect_signal():
                defs = Modulemd.ModuleStream.new(
                    Modulemd.ModuleStreamVersionEnum.LATEST + 1)

    def test_copy(self):
        for version in modulestream_versions:

            # Test that copying a stream with a stream name works
            stream = Modulemd.ModuleStream.new(version, 'foo', 'stable')
            copied_stream = stream.copy()

            assert copied_stream.props.module_name == stream.props.module_name
            assert copied_stream.get_module_name() == stream.get_module_name()
            assert copied_stream.props.stream_name == stream.props.stream_name
            assert copied_stream.get_stream_name() == stream.get_stream_name()

            # Test that copying a stream without a stream name works
            stream = Modulemd.ModuleStream.new(version, 'foo')
            copied_stream = stream.copy()

            assert copied_stream.props.module_name == stream.props.module_name
            assert copied_stream.get_module_name() == stream.get_module_name()
            assert copied_stream.props.stream_name == stream.props.stream_name
            assert copied_stream.get_stream_name() == stream.get_stream_name()

            # Test that copying a stream and changing the stream works
            stream = Modulemd.ModuleStream.new(version, 'foo', 'stable')
            copied_stream = stream.copy(module_stream='latest')

            assert copied_stream.props.module_name == stream.props.module_name
            assert copied_stream.get_module_name() == stream.get_module_name()
            assert copied_stream.props.stream_name != stream.props.stream_name
            assert copied_stream.get_stream_name() != stream.get_stream_name()
            assert copied_stream.props.stream_name == 'latest'
            assert copied_stream.get_stream_name() == 'latest'

            # Test that copying a stream without a module name works
            stream = Modulemd.ModuleStream.new(version, None, 'stable')
            copied_stream = stream.copy()

            assert copied_stream.props.module_name == stream.props.module_name
            assert copied_stream.get_module_name() == stream.get_module_name()
            assert copied_stream.props.stream_name == stream.props.stream_name
            assert copied_stream.get_stream_name() == stream.get_stream_name()

            # Test that copying a stream and changing the name works
            stream = Modulemd.ModuleStream.new(version, 'foo', 'stable')
            copied_stream = stream.copy(module_name='bar')

            assert copied_stream.props.module_name == 'bar'
            assert copied_stream.get_module_name() == 'bar'
            assert copied_stream.props.stream_name == stream.props.stream_name
            assert copied_stream.get_stream_name() == stream.get_stream_name()

            # Test the version and context
            assert copied_stream.props.version == 0
            assert copied_stream.get_version() == 0
            assert copied_stream.props.context is None
            assert copied_stream.get_context() is None

            # Set a version and check the copy
            stream.props.version = 42
            copied_stream = stream.copy()
            assert copied_stream.props.version == 42
            assert copied_stream.get_version() == 42

            # Set a context and check the copy
            stream.props.context = "c0ffee42"
            copied_stream = stream.copy()
            assert copied_stream.props.context == "c0ffee42"
            assert copied_stream.get_context() == "c0ffee42"

    def test_nsvc(self):
        for version in modulestream_versions:
            # First test that NSVC is None for a module with no name
            stream = Modulemd.ModuleStream.new(version)
            assert stream.get_nsvc_as_string() is None

            # Next, test for no stream name
            stream = Modulemd.ModuleStream.new(version, 'modulename')
            assert stream.get_nsvc_as_string() is None

            # Now with valid module and stream names
            stream = Modulemd.ModuleStream.new(
                version, 'modulename', 'streamname')
            assert stream.get_nsvc_as_string() == 'modulename:streamname:0'

            # Add a version number
            stream.props.version = 42
            assert stream.get_nsvc_as_string() == 'modulename:streamname:42'

            # Add a context
            stream.props.context = 'deadbeef'
            assert stream.get_nsvc_as_string() == 'modulename:streamname:42:deadbeef'

    def test_arch(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.arch is None
            assert stream.get_arch() is None

            # Test property setting
            stream.props.arch = 'x86_64'
            assert stream.props.arch == 'x86_64'
            assert stream.get_arch() == 'x86_64'

            # Test set_arch()
            stream.set_arch('ppc64le')
            assert stream.props.arch == 'ppc64le'
            assert stream.get_arch() == 'ppc64le'

            # Test setting it to None
            stream.props.arch = None
            assert stream.props.arch is None
            assert stream.get_arch() is None

    def test_buildopts(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.buildopts is None
            assert stream.get_buildopts() is None

            buildopts = Modulemd.Buildopts()
            buildopts.props.rpm_macros = '%demomacro 1'
            stream.set_buildopts(buildopts)

            assert stream.props.buildopts is not None
            assert stream.props.buildopts is not None
            assert stream.props.buildopts.props.rpm_macros == '%demomacro 1'

    def test_community(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.community is None
            assert stream.get_community() is None

            # Test property setting
            stream.props.community = 'http://example.com'
            assert stream.props.community == 'http://example.com'
            assert stream.get_community() == 'http://example.com'

            # Test set_community()
            stream.set_community('http://redhat.com')
            assert stream.props.community == 'http://redhat.com'
            assert stream.get_community() == 'http://redhat.com'

            # Test setting it to None
            stream.props.community = None
            assert stream.props.community is None
            assert stream.get_community() is None

    def test_description(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.description is None
            assert stream.get_description() is None

            # Test property setting
            stream.props.description = 'A description'
            assert stream.props.description == 'A description'
            assert stream.get_description() == 'A description'

            # Test set_description()
            stream.set_description('A different description')
            assert stream.props.description == 'A different description'
            assert stream.get_description() == 'A different description'

            # Test setting it to None
            stream.props.description = None
            assert stream.props.description is None
            assert stream.get_description() is None

    def test_documentation(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.documentation is None
            assert stream.get_documentation() is None

            # Test property setting
            stream.props.documentation = 'http://example.com'
            assert stream.props.documentation == 'http://example.com'
            assert stream.get_documentation() == 'http://example.com'

            # Test set_documentation()
            stream.set_documentation('http://redhat.com')
            assert stream.props.documentation == 'http://redhat.com'
            assert stream.get_documentation() == 'http://redhat.com'

            # Test setting it to None
            stream.props.documentation = None
            assert stream.props.documentation is None
            assert stream.get_documentation() is None

    def test_summary(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.summary is None
            assert stream.get_summary() is None

            # Test property setting
            stream.props.summary = 'A summary'
            assert stream.props.summary == 'A summary'
            assert stream.get_summary() == 'A summary'

            # Test set_summary()
            stream.set_summary('A different summary')
            assert stream.props.summary == 'A different summary'
            assert stream.get_summary() == 'A different summary'

            # Test setting it to None
            stream.props.summary = None
            assert stream.props.summary is None
            assert stream.get_summary() is None

    def test_tracker(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.tracker is None
            assert stream.get_tracker() is None

            # Test property setting
            stream.props.tracker = 'http://example.com'
            assert stream.props.tracker == 'http://example.com'
            assert stream.get_tracker() == 'http://example.com'

            # Test set_tracker()
            stream.set_tracker('http://redhat.com')
            assert stream.props.tracker == 'http://redhat.com'
            assert stream.get_tracker() == 'http://redhat.com'

            # Test setting it to None
            stream.props.tracker = None
            assert stream.props.tracker is None
            assert stream.get_tracker() is None

    def test_components(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Add an RPM component to a stream
            rpm_comp = Modulemd.ComponentRpm(name='rpmcomponent')
            stream.add_component(rpm_comp)
            assert 'rpmcomponent' in stream.get_rpm_component_names_as_strv()
            retrieved_comp = stream.get_rpm_component('rpmcomponent')
            assert retrieved_comp
            assert retrieved_comp.props.name == 'rpmcomponent'

            # Add a Module component to a stream
            mod_comp = Modulemd.ComponentModule(name='modulecomponent')
            stream.add_component(mod_comp)
            assert 'modulecomponent' in stream.get_module_component_names_as_strv()
            retrieved_comp = stream.get_module_component('modulecomponent')
            assert retrieved_comp
            assert retrieved_comp.props.name == 'modulecomponent'

            # Remove an RPM component from a stream
            stream.remove_rpm_component('rpmcomponent')

            # Remove a Module component from a stream
            stream.remove_module_component('modulecomponent')

    def test_licenses(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            stream.add_content_license('GPLv2+')
            assert 'GPLv2+' in stream.get_content_licenses_as_strv()

            stream.add_module_license('MIT')
            assert 'MIT' in stream.get_module_licenses_as_strv()

            stream.remove_content_license('GPLv2+')
            stream.remove_module_license('MIT')

    def test_profiles(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version, 'sssd')

            profile = Modulemd.Profile(name='client')
            profile.add_rpm('sssd-client')

            stream.add_profile(profile)
            assert len(stream.get_profile_names_as_strv()) == 1
            assert 'client' in stream.get_profile_names_as_strv()
            assert 'sssd-client' in stream.get_profile(
                'client').get_rpms_as_strv()

            stream.clear_profiles()
            assert len(stream.get_profile_names_as_strv()) == 0

    def test_rpm_api(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version, 'sssd')

            stream.add_rpm_api('sssd-common')
            assert 'sssd-common' in stream.get_rpm_api_as_strv()

            stream.remove_rpm_api('sssd-common')
            assert len(stream.get_rpm_api_as_strv()) == 0

    def test_rpm_artifacts(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            stream.add_rpm_artifact('bar-0:1.23-1.module_deadbeef.x86_64')
            assert 'bar-0:1.23-1.module_deadbeef.x86_64' in stream.get_rpm_artifacts_as_strv()

            stream.remove_rpm_artifact('bar-0:1.23-1.module_deadbeef.x86_64')
            assert len(stream.get_rpm_artifacts_as_strv()) == 0

    def test_rpm_filters(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            stream.add_rpm_filter('bar')
            assert 'bar' in stream.get_rpm_filters_as_strv()

            stream.remove_rpm_filter('bar')
            assert len(stream.get_rpm_filters_as_strv()) == 0

    def test_servicelevels(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)
            sl = Modulemd.ServiceLevel.new('rawhide')
            sl.set_eol_ymd(1980, 3, 2)

            stream.add_servicelevel(sl)

            assert 'rawhide' in stream.get_servicelevel_names_as_strv()

            retrieved_sl = stream.get_servicelevel('rawhide')
            assert retrieved_sl.props.name == 'rawhide'
            assert retrieved_sl.get_eol_as_string() == '1980-03-02'

    def test_v1_eol(self):
        stream = Modulemd.ModuleStreamV1.new()
        eol = GLib.Date.new_dmy(3, 2, 1998)

        stream.set_eol(eol)
        retrieved_eol = stream.get_eol()

        assert retrieved_eol.get_day() == 3
        assert retrieved_eol.get_month() == 2
        assert retrieved_eol.get_year() == 1998

        sl = stream.get_servicelevel('rawhide')
        assert sl.get_eol_as_string() == '1998-02-03'

    def test_v1_dependencies(self):
        stream = Modulemd.ModuleStreamV1.new()
        stream.add_buildtime_requirement('testmodule', 'stable')

        assert len(stream.get_buildtime_modules_as_strv()) == 1
        assert 'testmodule' in stream.get_buildtime_modules_as_strv()

        assert stream.get_buildtime_requirement_stream('testmodule') == \
            'stable'

        stream.add_runtime_requirement('testmodule', 'latest')
        assert len(stream.get_runtime_modules_as_strv()) == 1
        assert 'testmodule' in stream.get_runtime_modules_as_strv()
        assert stream.get_runtime_requirement_stream('testmodule') == 'latest'

    def test_v2_dependencies(self):
        stream = Modulemd.ModuleStreamV2.new()
        deps = Modulemd.Dependencies()

        deps.add_buildtime_stream('foo', 'stable')
        deps.set_empty_runtime_dependencies_for_module('bar')
        stream.add_dependencies(deps)

        assert len(stream.get_dependencies()) == 1
        assert len(stream.get_dependencies()) == 1

        assert 'foo' in stream.get_dependencies(
        )[0].get_buildtime_modules_as_strv()

        assert 'stable' in stream.get_dependencies(
        )[0].get_buildtime_streams_as_strv('foo')

        assert 'bar' in stream.get_dependencies(
        )[0].get_runtime_modules_as_strv()

    def test_xmd(self):
        if os.getenv('MMD_TEST_INSTALLED_LIB'):
            # The XMD python tests can only be run against the installed lib
            # because the overrides that translate between python and GVariant
            # must be installed in /usr/lib/python*/site-packages/gi/overrides
            # or they are not included when importing Modulemd
            stream = Modulemd.ModuleStreamV2.new()

            xmd = {'outer_key': ['scalar', {'inner_key': 'another_scalar'}]}

            stream.set_xmd(xmd)

            xmd_copy = stream.get_xmd()
            assert xmd_copy
            assert 'outer_key' in xmd
            assert 'scalar' in xmd['outer_key']
            assert 'inner_key' in xmd['outer_key'][1]
            assert xmd['outer_key'][1]['inner_key'] == 'another_scalar'

    def test_yaml(self):
        for version in modulestream_versions:
            yaml = "---\ndocument: modulemd\nversion: %d\ndata: {}\n...\n" % version
            stream = Modulemd.ModuleStream.read_string(yaml)

            # TODO: once the child classes are implemented, this should be
            # valid.
            assert stream is None


if __name__ == '__main__':
    unittest.main()
