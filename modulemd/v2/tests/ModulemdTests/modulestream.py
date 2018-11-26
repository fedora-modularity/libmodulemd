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


class TestModuleStream(TestBase):

    def test_constructors(self):
        for version in [
                Modulemd.ModuleStreamVersionEnum.ONE,
                Modulemd.ModuleStreamVersionEnum.TWO]:

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
        for version in [
                Modulemd.ModuleStreamVersionEnum.ONE,
                Modulemd.ModuleStreamVersionEnum.TWO]:

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

    def test_yaml(self):
        for version in [
                Modulemd.ModuleStreamVersionEnum.ONE,
                Modulemd.ModuleStreamVersionEnum.TWO]:
            yaml = "---\ndocument: modulemd\nversion: %d\ndata: {}\n...\n" % version
            stream = Modulemd.ModuleStream.read_string(yaml)

            # TODO: once the child classes are implemented, this should be
            # valid.
            assert stream is None


if __name__ == '__main__':
    unittest.main()
