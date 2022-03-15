#!/usr/bin/python3
# -*- coding: utf-8 -*-

# This file is part of libmodulemd
# Copyright (C) 2018-2020 Red Hat, Inc.
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

    if os.getenv("MMD_TEST_INSTALLED_LIBS") != "TRUE":
        gi._overridesdir = os.path.join(
            os.getenv("MESON_SOURCE_ROOT"),
            "bindings",
            "python",
            "gi",
            "overrides",
        )

    gi.require_version("Modulemd", "2.0")
    from gi.repository import GLib
    from gi.repository import Modulemd
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase

modulestream_versions = [
    Modulemd.ModuleStreamVersionEnum.ONE,
    Modulemd.ModuleStreamVersionEnum.TWO,
]


class TestModuleStream(TestBase):
    def test_constructors(self):
        for version in modulestream_versions:

            # Test that the new() function works
            stream = Modulemd.ModuleStream.new(version, "foo", "latest")
            assert stream
            assert isinstance(stream, Modulemd.ModuleStream)

            assert stream.props.mdversion == version
            assert stream.get_mdversion() == version
            assert stream.props.module_name == "foo"
            assert stream.get_module_name() == "foo"
            assert stream.props.stream_name == "latest"
            assert stream.get_stream_name() == "latest"

            # Test that the new() function works without a stream name
            stream = Modulemd.ModuleStream.new(version, "foo")
            assert stream
            assert isinstance(stream, Modulemd.ModuleStream)

            assert stream.props.mdversion == version
            assert stream.get_mdversion() == version
            assert stream.props.module_name == "foo"
            assert stream.get_module_name() == "foo"
            assert stream.props.stream_name is None
            assert stream.get_stream_name() is None

            # Test that the new() function works with no module name
            stream = Modulemd.ModuleStream.new(version, None, "latest")
            assert stream
            assert isinstance(stream, Modulemd.ModuleStream)

            assert stream.props.mdversion == version
            assert stream.get_mdversion() == version
            assert stream.props.module_name is None
            assert stream.get_module_name() is None
            assert stream.props.stream_name == "latest"
            assert stream.get_stream_name() == "latest"

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
        with self.assertRaisesRegex(
            TypeError, "cannot create instance of abstract"
        ):
            Modulemd.ModuleStream()

        # Test with a zero mdversion
        with self.assertRaisesRegex(TypeError, "constructor returned NULL"):
            with self.expect_signal():
                defs = Modulemd.ModuleStream.new(0)

        # Test with an unknown mdversion
        with self.assertRaisesRegex(TypeError, "constructor returned NULL"):
            with self.expect_signal():
                defs = Modulemd.ModuleStream.new(
                    Modulemd.ModuleStreamVersionEnum.LATEST + 1
                )

    def test_copy(self):
        for version in modulestream_versions:

            # Test that copying a stream with a stream name works
            stream = Modulemd.ModuleStream.new(version, "foo", "stable")
            copied_stream = stream.copy()

            assert copied_stream.props.module_name == stream.props.module_name
            assert copied_stream.get_module_name() == stream.get_module_name()
            assert copied_stream.props.stream_name == stream.props.stream_name
            assert copied_stream.get_stream_name() == stream.get_stream_name()

            # Test that copying a stream without a stream name works
            stream = Modulemd.ModuleStream.new(version, "foo")
            copied_stream = stream.copy()

            assert copied_stream.props.module_name == stream.props.module_name
            assert copied_stream.get_module_name() == stream.get_module_name()
            assert copied_stream.props.stream_name == stream.props.stream_name
            assert copied_stream.get_stream_name() == stream.get_stream_name()

            # Test that copying a stream and changing the stream works
            stream = Modulemd.ModuleStream.new(version, "foo", "stable")
            copied_stream = stream.copy(module_stream="latest")

            assert copied_stream.props.module_name == stream.props.module_name
            assert copied_stream.get_module_name() == stream.get_module_name()
            assert copied_stream.props.stream_name != stream.props.stream_name
            assert copied_stream.get_stream_name() != stream.get_stream_name()
            assert copied_stream.props.stream_name == "latest"
            assert copied_stream.get_stream_name() == "latest"

            # Test that copying a stream without a module name works
            stream = Modulemd.ModuleStream.new(version, None, "stable")
            copied_stream = stream.copy()

            assert copied_stream.props.module_name == stream.props.module_name
            assert copied_stream.get_module_name() == stream.get_module_name()
            assert copied_stream.props.stream_name == stream.props.stream_name
            assert copied_stream.get_stream_name() == stream.get_stream_name()

            # Test that copying a stream and changing the name works
            stream = Modulemd.ModuleStream.new(version, "foo", "stable")
            copied_stream = stream.copy(module_name="bar")

            assert copied_stream.props.module_name == "bar"
            assert copied_stream.get_module_name() == "bar"
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
            assert stream.get_nsvc() is None

            # Next, test for no stream name
            stream = Modulemd.ModuleStream.new(version, "modulename")
            assert stream.get_nsvc() is None

            # Now with valid module and stream names
            stream = Modulemd.ModuleStream.new(
                version, "modulename", "streamname"
            )
            assert stream.get_nsvc() == "modulename:streamname:0"

            # Add a version number
            stream.props.version = 42
            assert stream.get_nsvc() == "modulename:streamname:42"

            # Add a context
            stream.props.context = "deadbeef"
            assert stream.get_nsvc() == "modulename:streamname:42:deadbeef"

    def test_nsvca(self):
        for version in modulestream_versions:
            # First test that NSVCA is None for a module with no name
            stream = Modulemd.ModuleStream.new(version)
            self.assertIsNone(stream.get_NSVCA())

            # Next, test for no stream name
            stream = Modulemd.ModuleStream.new(version, "modulename")
            self.assertEqual(stream.get_NSVCA(), "modulename")

            # Now with valid module and stream names
            stream = Modulemd.ModuleStream.new(
                version, "modulename", "streamname"
            )
            self.assertEqual(stream.get_NSVCA(), "modulename:streamname")

            # Add a version number
            stream.props.version = 42
            self.assertEqual(stream.get_NSVCA(), "modulename:streamname:42")

            # Add a context
            stream.props.context = "deadbeef"
            self.assertEqual(
                stream.get_NSVCA(), "modulename:streamname:42:deadbeef"
            )

            # Add an architecture
            stream.props.arch = "x86_64"
            self.assertEqual(
                stream.get_NSVCA(), "modulename:streamname:42:deadbeef:x86_64"
            )

            # Now try removing some of the bits in the middle
            stream.props.context = None
            self.assertEqual(
                stream.get_NSVCA(), "modulename:streamname:42::x86_64"
            )

            stream = Modulemd.ModuleStream.new(version, "modulename")
            stream.props.arch = "x86_64"
            self.assertEqual(stream.get_NSVCA(), "modulename::::x86_64")

            stream.props.version = 2019
            self.assertEqual(stream.get_NSVCA(), "modulename::2019::x86_64")
            # Add a context
            stream.props.context = "feedfeed"
            self.assertEqual(
                stream.get_NSVCA(), "modulename::2019:feedfeed:x86_64"
            )

    def test_arch(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.arch is None
            assert stream.get_arch() is None

            # Test property setting
            stream.props.arch = "x86_64"
            assert stream.props.arch == "x86_64"
            assert stream.get_arch() == "x86_64"

            # Test set_arch()
            stream.set_arch("ppc64le")
            assert stream.props.arch == "ppc64le"
            assert stream.get_arch() == "ppc64le"

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
            buildopts.props.rpm_macros = "%demomacro 1"
            stream.set_buildopts(buildopts)

            assert stream.props.buildopts is not None
            assert stream.props.buildopts is not None
            assert stream.props.buildopts.props.rpm_macros == "%demomacro 1"

    def test_community(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.community is None
            assert stream.get_community() is None

            # Test property setting
            stream.props.community = "http://example.com"
            assert stream.props.community == "http://example.com"
            assert stream.get_community() == "http://example.com"

            # Test set_community()
            stream.set_community("http://redhat.com")
            assert stream.props.community == "http://redhat.com"
            assert stream.get_community() == "http://redhat.com"

            # Test setting it to None
            stream.props.community = None
            assert stream.props.community is None
            assert stream.get_community() is None

    def test_description(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.get_description(locale="C") is None

            # Test set_description()
            stream.set_description("A different description")
            assert (
                stream.get_description(locale="C") == "A different description"
            )

            # Test setting it to None
            stream.set_description(None)
            assert stream.get_description(locale="C") is None

    def test_documentation(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.documentation is None
            assert stream.get_documentation() is None

            # Test property setting
            stream.props.documentation = "http://example.com"
            assert stream.props.documentation == "http://example.com"
            assert stream.get_documentation() == "http://example.com"

            # Test set_documentation()
            stream.set_documentation("http://redhat.com")
            assert stream.props.documentation == "http://redhat.com"
            assert stream.get_documentation() == "http://redhat.com"

            # Test setting it to None
            stream.props.documentation = None
            assert stream.props.documentation is None
            assert stream.get_documentation() is None

            # Test unicode characters
            unicode_test_str = (
                "À϶￥🌭∮⇒⇔¬β∀₂⌀ıəˈ⍳⍴V)═€ίζησθლბშიнстемองจึองታሽ።ደለᚢᛞᚦᚹ⠳⠞⠊⠎▉▒▒▓😃"
            )

            stream.props.documentation = unicode_test_str
            assert stream.props.documentation == unicode_test_str
            assert stream.get_documentation() == unicode_test_str

    def test_summary(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.get_summary(locale="C") is None

            # Test set_summary()
            stream.set_summary("A different summary")
            assert stream.get_summary(locale="C") == "A different summary"

            # Test setting it to None
            stream.set_summary(None)
            assert stream.get_summary(locale="C") is None

    def test_tracker(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Check the defaults
            assert stream.props.tracker is None
            assert stream.get_tracker() is None

            # Test property setting
            stream.props.tracker = "http://example.com"
            assert stream.props.tracker == "http://example.com"
            assert stream.get_tracker() == "http://example.com"

            # Test set_tracker()
            stream.set_tracker("http://redhat.com")
            assert stream.props.tracker == "http://redhat.com"
            assert stream.get_tracker() == "http://redhat.com"

            # Test setting it to None
            stream.props.tracker = None
            assert stream.props.tracker is None
            assert stream.get_tracker() is None

    def test_components(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            # Add an RPM component to a stream
            rpm_comp = Modulemd.ComponentRpm(name="rpmcomponent")
            stream.add_component(rpm_comp)
            assert "rpmcomponent" in stream.get_rpm_component_names()
            retrieved_comp = stream.get_rpm_component("rpmcomponent")
            assert retrieved_comp
            assert retrieved_comp.props.name == "rpmcomponent"

            # Add a Module component to a stream
            mod_comp = Modulemd.ComponentModule(name="modulecomponent")
            stream.add_component(mod_comp)
            assert "modulecomponent" in stream.get_module_component_names()
            retrieved_comp = stream.get_module_component("modulecomponent")
            assert retrieved_comp
            assert retrieved_comp.props.name == "modulecomponent"

            # Remove an RPM component from a stream
            stream.remove_rpm_component("rpmcomponent")

            # Remove a Module component from a stream
            stream.remove_module_component("modulecomponent")

    def test_licenses(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            stream.add_content_license("DUMMY1")
            assert "DUMMY1" in stream.get_content_licenses()

            stream.add_module_license("DUMMY2")
            assert "DUMMY2" in stream.get_module_licenses()

            stream.remove_content_license("DUMMY1")
            stream.remove_module_license("DUMMY2")

    def test_profiles(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version, "sssd")

            profile = Modulemd.Profile(name="client")
            profile.add_rpm("sssd-client")

            stream.add_profile(profile)
            assert len(stream.get_profile_names()) == 1
            assert "client" in stream.get_profile_names()
            assert "sssd-client" in stream.get_profile("client").get_rpms()

            stream.clear_profiles()
            assert len(stream.get_profile_names()) == 0

    def test_rpm_api(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version, "sssd")

            stream.add_rpm_api("sssd-common")
            assert "sssd-common" in stream.get_rpm_api()

            stream.remove_rpm_api("sssd-common")
            assert len(stream.get_rpm_api()) == 0

    def test_rpm_artifacts(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            stream.add_rpm_artifact("bar-0:1.23-1.module_deadbeef.x86_64")
            assert (
                "bar-0:1.23-1.module_deadbeef.x86_64"
                in stream.get_rpm_artifacts()
            )

            stream.remove_rpm_artifact("bar-0:1.23-1.module_deadbeef.x86_64")
            assert len(stream.get_rpm_artifacts()) == 0

    def test_rpm_filters(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)

            stream.add_rpm_filter("foo")
            stream.add_rpm_filter("bar")
            assert "foo" in stream.get_rpm_filters()
            assert "bar" in stream.get_rpm_filters()
            assert len(stream.get_rpm_filters()) == 2

            stream.remove_rpm_filter("bar")
            assert "foo" in stream.get_rpm_filters()
            assert len(stream.get_rpm_filters()) == 1

            stream.clear_rpm_filters()
            assert len(stream.get_rpm_filters()) == 0

    def test_demodularized_rpms(self):
        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.new(version)
            # demodularized_rpms are supported since v2
            if version < Modulemd.ModuleStreamVersionEnum.TWO:
                continue
            stream.add_demodularized_rpm("foo")
            stream.add_demodularized_rpm("bar")
            assert "foo" in stream.get_demodularized_rpms()
            assert "bar" in stream.get_demodularized_rpms()
            assert len(stream.get_demodularized_rpms()) == 2

            stream.remove_demodularized_rpm("bar")
            assert "foo" in stream.get_demodularized_rpms()
            assert len(stream.get_demodularized_rpms()) == 1

            stream.clear_demodularized_rpms()
            assert len(stream.get_demodularized_rpms()) == 0

    def test_servicelevels(self):
        for version in modulestream_versions:
            # servicelevels are not supported after v2
            if version > Modulemd.ModuleStreamVersionEnum.TWO:
                continue
            stream = Modulemd.ModuleStream.new(version)
            sl = Modulemd.ServiceLevel.new("rawhide")
            sl.set_eol_ymd(1980, 3, 2)

            stream.add_servicelevel(sl)

            assert "rawhide" in stream.get_servicelevel_names()

            retrieved_sl = stream.get_servicelevel("rawhide")
            assert retrieved_sl.props.name == "rawhide"
            assert retrieved_sl.get_eol_as_string() == "1980-03-02"

    def test_v1_eol(self):
        stream = Modulemd.ModuleStreamV1.new()
        eol = GLib.Date.new_dmy(3, 2, 1998)

        stream.set_eol(eol)
        retrieved_eol = stream.get_eol()

        assert retrieved_eol.get_day() == 3
        assert retrieved_eol.get_month() == 2
        assert retrieved_eol.get_year() == 1998

        sl = stream.get_servicelevel("rawhide")
        assert sl.get_eol_as_string() == "1998-02-03"

    def test_v1_dependencies(self):
        stream = Modulemd.ModuleStreamV1.new()
        stream.add_buildtime_requirement("testmodule", "stable")

        assert len(stream.get_buildtime_modules()) == 1
        assert "testmodule" in stream.get_buildtime_modules()

        assert (
            stream.get_buildtime_requirement_stream("testmodule") == "stable"
        )

        stream.add_runtime_requirement("testmodule", "latest")
        assert len(stream.get_runtime_modules()) == 1
        assert "testmodule" in stream.get_runtime_modules()
        assert stream.get_runtime_requirement_stream("testmodule") == "latest"

    def test_v2_dependencies(self):
        stream = Modulemd.ModuleStreamV2.new()
        deps = Modulemd.Dependencies()

        deps.add_buildtime_stream("foo", "stable")
        deps.set_empty_runtime_dependencies_for_module("bar")
        stream.add_dependencies(deps)

        assert len(stream.get_dependencies()) == 1
        assert len(stream.get_dependencies()) == 1

        assert "foo" in stream.get_dependencies()[0].get_buildtime_modules()

        assert "stable" in stream.get_dependencies()[0].get_buildtime_streams(
            "foo"
        )

        assert "bar" in stream.get_dependencies()[0].get_runtime_modules()

        retrieved_deps = stream.get_dependencies()
        stream.clear_dependencies()
        self.assertEqual(len(retrieved_deps), 1)
        self.assertEqual(len(stream.get_dependencies()), 0)

        stream.add_dependencies(deps)
        self.assertEqual(len(stream.get_dependencies()), 1)

        stream.remove_dependencies(deps)
        self.assertEqual(len(stream.get_dependencies()), 0)

    def test_xmd(self):

        for version in modulestream_versions:
            # We have a chicken-egg problem with overrides, since they can only
            # be tested if they are already installed. This means they need to
            # be run in the CI. In order to avoid changes to these tests or the
            # overrides breaking things, we'll skip them if the appropriate
            # override is not installed.
            if "_overrides_module" in dir(Modulemd) and hasattr(
                gi.overrides.Modulemd,
                type(Modulemd.ModuleStream.new(version)).__name__,
            ):

                # The XMD python tests can only be run against the installed lib
                # because the overrides that translate between python and GVariant
                # must be installed in /usr/lib/python*/site-packages/gi/overrides
                # or they are not included when importing Modulemd
                stream = Modulemd.ModuleStream.new(version, "foo", "bar")
                # An empty dictionary should be returned if no xmd value is set
                self.assertEqual(stream.get_xmd(), dict())

                xmd = {
                    "outer_key": {"inner_key": ["scalar", "another_scalar"]}
                }

                stream.set_xmd(xmd)

                xmd_copy = stream.get_xmd()
                assert xmd_copy
                assert "outer_key" in xmd_copy
                assert "inner_key" in xmd_copy["outer_key"]
                assert "scalar" in xmd_copy["outer_key"]["inner_key"]
                assert "another_scalar" in xmd_copy["outer_key"]["inner_key"]

                # Verify that we can add content and save it back
                xmd["something"] = ["foo", "bar"]
                stream.set_xmd(xmd)

                stream.set_summary("foo")
                stream.set_description("bar")
                stream.add_module_license("DUMMY")
                if hasattr(stream, "set_platform"):
                    stream.set_platform("f33")

                index = Modulemd.ModuleIndex()
                index.add_module_stream(stream)

                out_yaml = index.dump_to_string()

                self.assertIsNotNone(out_yaml)

    def test_upgrade_v1_to_v2(self):
        v1_stream = Modulemd.ModuleStreamV1.new("SuperModule", "latest")
        v1_stream.set_context("ctx")
        v1_stream.set_summary("Summary")
        v1_stream.set_description("Description")
        v1_stream.add_module_license("DUMMY")

        v1_stream.add_buildtime_requirement("ModuleA", "streamZ")
        v1_stream.add_buildtime_requirement("ModuleB", "streamY")
        v1_stream.add_runtime_requirement("ModuleA", "streamZ")
        v1_stream.add_runtime_requirement("ModuleB", "streamY")
        v1_stream.add_runtime_requirement("platform", "f33")

        v2_stream = v1_stream.upgrade(Modulemd.ModuleStreamVersionEnum.TWO)
        self.assertIsNotNone(v2_stream)

        idx = Modulemd.ModuleIndex.new()
        idx.add_module_stream(v2_stream)

        self.assertEqual(
            idx.dump_to_string(),
            """---
document: modulemd
version: 2
data:
  name: SuperModule
  stream: \"latest\"
  context: ctx
  summary: Summary
  description: >-
    Description
  license:
    module:
    - "DUMMY"
  dependencies:
  - buildrequires:
      ModuleA: ["streamZ"]
      ModuleB: ["streamY"]
    requires:
      ModuleA: ["streamZ"]
      ModuleB: ["streamY"]
      platform: ["f33"]
...
""",
        )

    def test_v2_yaml(self):
        yaml = """
---
document: modulemd
version: 2
data:
  name: modulename
  stream: \"streamname\"
  version: 1
  context: c0ffe3
  arch: x86_64
  summary: Module Summary
  description: >-
    Module Description
  api:
    rpms:
      - rpm_a
      - rpm_b
  filter:
    rpms: rpm_c

  demodularized:
    rpms:
      - rpm_d

  artifacts:
    rpms:
      - bar-0:1.23-1.module_deadbeef.x86_64

  servicelevels:
    rawhide: {}
    production:
      eol: 2099-12-31

  license:
    content:
      - DUMMY1
      - DUMMY2
    module: DUMMY3

  dependencies:
    - buildrequires:
          platform: [-f27, -f28, -epel7]
      requires:
          platform: [-f27, -f28, -epel7]
    - buildrequires:
          platform: [f27]
          buildtools: [v1, v2]
          compatible: [v3]
      requires:
          platform: [f27]
          compatible: [v3, v4]
    - buildrequires:
          platform: [f28]
      requires:
          platform: [f28]
          runtime: [a, b]
    - buildrequires:
          platform: [epel7]
          extras: []
          moreextras: [foo, bar]
      requires:
          platform: [epel7]
          extras: []
          moreextras: [foo, bar]
  references:
        community: http://www.example.com/
        documentation: http://www.example.com/
        tracker: http://www.example.com/
  profiles:
        default:
            rpms:
                - bar
                - bar-extras
                - baz
        container:
            rpms:
                - bar
                - bar-devel
        minimal:
            description: Minimal profile installing only the bar package.
            rpms:
                - bar
        buildroot:
            rpms:
                - bar-devel
        srpm-buildroot:
            rpms:
                - bar-extras
  buildopts:
        rpms:
            macros: |
                %demomacro 1
                %demomacro2 %{demomacro}23
            whitelist:
                - fooscl-1-bar
                - fooscl-1-baz
                - xxx
                - xyz
        arches: [i686, x86_64]
  components:
        rpms:
            bar:
                rationale: We need this to demonstrate stuff.
                repository: https://pagure.io/bar.git
                cache: https://example.com/cache
                ref: 26ca0c0
            baz:
                rationale: This one is here to demonstrate other stuff.
            xxx:
                rationale: xxx demonstrates arches and multilib.
                arches: [i686, x86_64]
                multilib: [x86_64]
            xyz:
                rationale: xyz is a bundled dependency of xxx.
                buildorder: 10
        modules:
            includedmodule:
                rationale: Included in the stack, just because.
                repository: https://pagure.io/includedmodule.git
                ref: somecoolbranchname
                buildorder: 100
  xmd:
        some_key: some_data
        some_list:
            - a
            - b
        some_dict:
            a: alpha
            b: beta
            some_other_list:
                - c
                - d
            some_other_dict:
                another_key: more_data
                yet_another_key:
                    - this
                    - is
                    - getting
                    - silly
        can_bool: TRUE
...
"""
        stream = Modulemd.ModuleStream.read_string(yaml, True)

        assert stream is not None
        assert stream.props.module_name == "modulename"
        assert stream.props.stream_name == "streamname"
        assert stream.props.version == 1
        assert stream.props.context == "c0ffe3"
        assert stream.props.arch == "x86_64"
        assert stream.get_summary(locale="C") == "Module Summary"
        assert stream.get_description(locale="C") == "Module Description"

        assert "rpm_a" in stream.get_rpm_api()
        assert "rpm_b" in stream.get_rpm_api()

        assert "rpm_c" in stream.get_rpm_filters()

        assert "rpm_d" in stream.get_demodularized_rpms()

        assert (
            "bar-0:1.23-1.module_deadbeef.x86_64" in stream.get_rpm_artifacts()
        )

        assert "rawhide" in stream.get_servicelevel_names()
        assert "production" in stream.get_servicelevel_names()

        sl = stream.get_servicelevel("rawhide")
        assert sl is not None
        assert sl.props.name == "rawhide"
        assert sl.get_eol() is None

        sl = stream.get_servicelevel("production")
        assert sl is not None
        assert sl.props.name == "production"
        assert sl.get_eol() is not None
        assert sl.get_eol_as_string() == "2099-12-31"

        assert "DUMMY1" in stream.get_content_licenses()
        assert "DUMMY2" in stream.get_content_licenses()

        assert len(stream.get_dependencies()) == 4

        assert stream.props.community == "http://www.example.com/"
        assert stream.props.documentation == "http://www.example.com/"
        assert stream.props.tracker == "http://www.example.com/"

        assert len(stream.get_profile_names()) == 5

        buildopts = stream.get_buildopts()
        assert buildopts is not None

        assert (
            "%demomacro 1\n%demomacro2 %{demomacro}23\n"
            == buildopts.props.rpm_macros
        )
        assert "fooscl-1-bar" in buildopts.get_rpm_whitelist()
        assert "fooscl-1-baz" in buildopts.get_rpm_whitelist()
        assert "xxx" in buildopts.get_rpm_whitelist()
        assert "xyz" in buildopts.get_rpm_whitelist()
        assert "i686" in buildopts.get_arches()
        assert "x86_64" in buildopts.get_arches()

        if os.getenv("MMD_TEST_INSTALLED_LIB"):
            # The XMD python tests can only be run against the installed
            # lib because the overrides that translate between python and
            # GVariant must be installed in
            # /usr/lib/python*/site-packages/gi/overrides
            # or they are not included when importing Modulemd
            xmd = stream.get_xmd()
            assert xmd is not None

            assert "some_key" in xmd
            assert xmd["some_key"] == "some_data"

            assert "some_list" in xmd

            assert "a" in xmd["some_list"]
            assert "b" in xmd["some_list"]

            assert "some_dict" in xmd
            assert "a" in xmd["some_dict"]
            assert xmd["some_dict"]["a"] == "alpha"

            assert "some_other_dict" in xmd["some_dict"]
            assert "yet_another_key" in xmd["some_dict"]["some_other_dict"]
            assert (
                "silly"
                in xmd["some_dict"]["some_other_dict"]["yet_another_key"]
            )

            assert "can_bool" in xmd
            assert xmd["can_bool"] is True

        # Validate a trivial modulemd
        trivial_yaml = """
---
document: modulemd
version: 2
data:
    summary: Trivial Summary
    description: >-
      Trivial Description
    license:
        module: DUMMY
...
"""

        stream = Modulemd.ModuleStream.read_string(trivial_yaml, True)
        assert stream

        # Sanity check of spec.v2.yaml
        stream = Modulemd.ModuleStream.read_file(
            os.path.join(
                self.source_root, "yaml_specs/modulemd_stream_v2.yaml"
            ),
            True,
        )
        assert stream

    def test_v1_yaml(self):
        for version in modulestream_versions:
            yaml = """
---
document: modulemd
version: 1
data:
  name: modulename
  stream: streamname
  version: 1
  context: c0ffe3
  arch: x86_64
  summary: Module Summary
  description: >-
    Module Description
  api:
    rpms:
      - rpm_a
      - rpm_b
  filter:
    rpms: rpm_c

  artifacts:
    rpms:
      - bar-0:1.23-1.module_deadbeef.x86_64

  eol: 2033-08-04
  servicelevels:
    foo: {}
    production:
      eol: 2099-12-31

  license:
    content:
      - DUMMY1
      - DUMMY2
    module: DUMMY3

  dependencies:
        buildrequires:
            platform: and-its-stream-name
            extra-build-env: and-its-stream-name-too
        requires:
            runtimeplatform: and-its-stream-name-2

  references:
        community: http://www.example.com/
        documentation: http://www.example.com/
        tracker: http://www.example.com/
  profiles:
        default:
            rpms:
                - bar
                - bar-extras
                - baz
        container:
            rpms:
                - bar
                - bar-devel
        minimal:
            description: Minimal profile installing only the bar package.
            rpms:
                - bar
        buildroot:
            rpms:
                - bar-devel
        srpm-buildroot:
            rpms:
                - bar-extras
  buildopts:
        rpms:
            macros: |
                %demomacro 1
                %demomacro2 %{demomacro}23
            whitelist:
                - fooscl-1-bar
                - fooscl-1-baz
                - xxx
                - xyz
  components:
        rpms:
            bar:
                rationale: We need this to demonstrate stuff.
                repository: https://pagure.io/bar.git
                cache: https://example.com/cache
                ref: 26ca0c0
            baz:
                rationale: This one is here to demonstrate other stuff.
            xxx:
                rationale: xxx demonstrates arches and multilib.
                arches: [i686, x86_64]
                multilib: [x86_64]
            xyz:
                rationale: xyz is a bundled dependency of xxx.
                buildorder: 10
        modules:
            includedmodule:
                rationale: Included in the stack, just because.
                repository: https://pagure.io/includedmodule.git
                ref: somecoolbranchname
                buildorder: 100
  xmd:
        some_key: some_data
        some_list:
            - a
            - b
        some_dict:
            a: alpha
            b: beta
            some_other_list:
                - c
                - d
            some_other_dict:
                another_key: more_data
                yet_another_key:
                    - this
                    - is
                    - getting
                    - silly
        can_bool: TRUE
...
"""
            stream = Modulemd.ModuleStream.read_string(yaml, True)

            assert stream is not None
            assert stream.props.module_name == "modulename"
            assert stream.props.stream_name == "streamname"
            assert stream.props.version == 1
            assert stream.props.context == "c0ffe3"
            assert stream.props.arch == "x86_64"
            assert stream.get_summary(locale="C") == "Module Summary"
            assert stream.get_description(locale="C") == "Module Description"

            assert "rpm_a" in stream.get_rpm_api()
            assert "rpm_b" in stream.get_rpm_api()

            assert "rpm_c" in stream.get_rpm_filters()

            assert (
                "bar-0:1.23-1.module_deadbeef.x86_64"
                in stream.get_rpm_artifacts()
            )

            assert "rawhide" in stream.get_servicelevel_names()
            assert "production" in stream.get_servicelevel_names()

            sl = stream.get_servicelevel("rawhide")
            assert sl is not None
            assert sl.props.name == "rawhide"
            assert sl.get_eol_as_string() == "2033-08-04"

            sl = stream.get_servicelevel("foo")
            assert sl is not None
            assert sl.props.name == "foo"
            assert sl.get_eol() is None

            sl = stream.get_servicelevel("production")
            assert sl is not None
            assert sl.props.name == "production"
            assert sl.get_eol() is not None
            assert sl.get_eol_as_string() == "2099-12-31"

            assert "DUMMY1" in stream.get_content_licenses()
            assert "DUMMY2" in stream.get_content_licenses()

            buildrequires = stream.get_buildtime_modules()
            assert len(buildrequires) == 2
            assert "platform" in buildrequires
            assert (
                stream.get_buildtime_requirement_stream("platform")
                == "and-its-stream-name"
            )
            assert "extra-build-env" in buildrequires
            assert (
                stream.get_buildtime_requirement_stream("extra-build-env")
                == "and-its-stream-name-too"
            )

            requires = stream.get_runtime_modules()
            assert len(requires) == 1
            assert "runtimeplatform" in requires
            assert (
                stream.get_runtime_requirement_stream("runtimeplatform")
                == "and-its-stream-name-2"
            )

            assert stream.props.community == "http://www.example.com/"
            assert stream.props.documentation == "http://www.example.com/"
            assert stream.props.tracker == "http://www.example.com/"

            assert len(stream.get_profile_names()) == 5

            buildopts = stream.get_buildopts()
            assert buildopts is not None

            assert (
                "%demomacro 1\n%demomacro2 %{demomacro}23\n"
                == buildopts.props.rpm_macros
            )
            assert "fooscl-1-bar" in buildopts.get_rpm_whitelist()
            assert "fooscl-1-baz" in buildopts.get_rpm_whitelist()
            assert "xxx" in buildopts.get_rpm_whitelist()
            assert "xyz" in buildopts.get_rpm_whitelist()

            if os.getenv("MMD_TEST_INSTALLED_LIB"):
                # The XMD python tests can only be run against the installed
                # lib because the overrides that translate between python and
                # GVariant must be installed in
                # /usr/lib/python*/site-packages/gi/overrides
                # or they are not included when importing Modulemd
                xmd = stream.get_xmd()
                assert xmd is not None

                assert "some_key" in xmd
                assert xmd["some_key"] == "some_data"

                assert "some_list" in xmd

                assert "a" in xmd["some_list"]
                assert "b" in xmd["some_list"]

                assert "some_dict" in xmd
                assert "a" in xmd["some_dict"]
                assert xmd["some_dict"]["a"] == "alpha"

                assert "some_other_dict" in xmd["some_dict"]
                assert "yet_another_key" in xmd["some_dict"]["some_other_dict"]
                assert (
                    "silly"
                    in xmd["some_dict"]["some_other_dict"]["yet_another_key"]
                )

                assert "can_bool" in xmd
                assert xmd["can_bool"] is True

            # Validate a trivial modulemd
            trivial_yaml = """
---
document: modulemd
version: 1
data:
  summary: Trivial Summary
  description: >-
    Trivial Description
  license:
    module: DUMMY
...
"""

            stream = Modulemd.ModuleStream.read_string(trivial_yaml, True)
            assert stream

            # Sanity check of spec.v1.yaml
            stream = Modulemd.ModuleStream.read_file(
                "%s/yaml_specs/modulemd_stream_v1.yaml"
                % os.getenv("MESON_SOURCE_ROOT"),
                True,
            )
            assert stream

    def test_packager_sanity(self):
        stream = Modulemd.ModuleStream.read_file(
            "%s/yaml_specs/modulemd_packager_v2.yaml"
            % os.getenv("MESON_SOURCE_ROOT"),
            True,
        )
        assert stream

    def test_depends_on_stream(self):

        for version in modulestream_versions:
            stream = Modulemd.ModuleStream.read_file(
                "%s/dependson_v%d.yaml"
                % (os.getenv("TEST_DATA_PATH"), version),
                True,
            )
            self.assertIsNotNone(stream)

            self.assertEqual(stream.depends_on_stream("platform", "f30"), True)
            self.assertEqual(
                stream.build_depends_on_stream("platform", "f30"), True
            )

            self.assertEqual(
                stream.depends_on_stream("platform", "f28"), False
            )
            self.assertEqual(
                stream.build_depends_on_stream("platform", "f28"), False
            )

            self.assertEqual(stream.depends_on_stream("base", "f30"), False)
            self.assertEqual(
                stream.build_depends_on_stream("base", "f30"), False
            )

            if version >= Modulemd.ModuleStreamVersionEnum.TWO:
                self.assertEqual(
                    stream.depends_on_stream("streamname", "f30"), True
                )
                self.assertEqual(
                    stream.build_depends_on_stream("streamname", "f30"), True
                )

    def test_validate_buildafter(self):
        for version in modulestream_versions:
            # buildafter is supported starting with v2
            if version < Modulemd.ModuleStreamVersionEnum.TWO:
                continue
            # Test a valid module stream with buildafter set
            stream = Modulemd.ModuleStream.read_file(
                "%s/buildafter/good_buildafter_v%d.yaml"
                % (self.test_data_path, version),
                True,
            )
            self.assertIsNotNone(stream)
            self.assertTrue(stream.validate())

            # Should fail validation if both buildorder and buildafter are set for
            # the same component.
            with self.assertRaisesRegex(
                gi.repository.GLib.GError,
                "Cannot mix buildorder and buildafter",
            ):
                stream = Modulemd.ModuleStream.read_file(
                    "%s/buildafter/both_same_component_v%d.yaml"
                    % (self.test_data_path, version),
                    True,
                )

            # Should fail validation if both buildorder and buildafter are set in
            # different components of the same stream.
            with self.assertRaisesRegex(
                gi.repository.GLib.GError,
                "Cannot mix buildorder and buildafter",
            ):
                stream = Modulemd.ModuleStream.read_file(
                    "%s/buildafter/mixed_buildorder_v%d.yaml"
                    % (self.test_data_path, version),
                    True,
                )

            # Should fail if a key specified in a buildafter set does not exist
            # for this module stream.
            with self.assertRaisesRegex(
                gi.repository.GLib.GError, "not found in components list"
            ):
                stream = Modulemd.ModuleStream.read_file(
                    "%s/buildafter/invalid_key_v%d.yaml"
                    % (self.test_data_path, version),
                    True,
                )

    def test_unicode_desc(self):
        for version in modulestream_versions:
            # Test a valid module stream with unicode in the description
            stream = Modulemd.ModuleStream.read_file(
                "%s/stream_unicode_v%d.yaml"
                % (os.getenv("TEST_DATA_PATH"), version),
                True,
                "",
                "",
            )

            self.assertIsNotNone(stream)
            self.assertTrue(stream.validate())

    def test_xmd_issue_274(self):
        # Test a valid module stream with unicode in the description
        stream = Modulemd.ModuleStream.read_file(
            "%s/stream_unicode_v2.yaml" % (os.getenv("TEST_DATA_PATH")),
            True,
            "",
            "",
        )

        # In this bug, we were getting a traceback attemping to call
        # get_xmd() more than once on the same stream. There were subtle
        # memory issues at play here.
        if "_overrides_module" in dir(Modulemd):
            # The XMD python tests can only be run against the installed lib
            # because the overrides that translate between python and GVariant
            # must be installed in /usr/lib/python*/site-packages/gi/overrides
            # or they are not included when importing Modulemd

            xmd = stream.get_xmd()
            mbs_xmd = stream.get_xmd()["mbs"]
            mbs_xmd2 = stream.get_xmd()["mbs"]

        else:
            stream.get_xmd()
            stream.get_xmd()

    def test_xmd_issue_290(self):
        if "_overrides_module" in dir(Modulemd):
            stream = Modulemd.ModuleStream.read_file(
                "%s/290.yaml" % (os.getenv("TEST_DATA_PATH")), True, "", ""
            )

            self.assertIsNotNone(stream)

            xmd = stream.get_xmd()

            xmd["something"] = ["foo", "bar"]
            stream.set_xmd(xmd)

            index = Modulemd.ModuleIndex()
            index.add_module_stream(stream)
            self.maxDiff = None
            output_yaml = index.dump_to_string()
            self.assertIsNotNone(output_yaml)
        pass

    def test_search_profiles(self):
        stream = Modulemd.ModuleStreamV2.new("themodule", "thestream")

        # First with no profiles added. Make sure we get back a zero-length
        # array

        profiles = stream.search_profiles(None)
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 0)

        profiles = stream.search_profiles("*")
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 0)

        profiles = stream.search_profiles("thefirstprofile")
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 0)

        # Now add three profiles, and confirm that searches are
        # returned in alphabetical order
        stream.add_profile(Modulemd.Profile.new("thesecondprofile"))
        stream.add_profile(Modulemd.Profile.new("thefirstprofile"))
        stream.add_profile(Modulemd.Profile.new("thethirdprofile"))

        profiles = stream.search_profiles(None)
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 3)
        self.assertIsInstance(profiles[0], Modulemd.Profile)
        self.assertEqual(profiles[0].get_name(), "thefirstprofile")
        self.assertIsInstance(profiles[1], Modulemd.Profile)
        self.assertEqual(profiles[1].get_name(), "thesecondprofile")
        self.assertIsInstance(profiles[2], Modulemd.Profile)
        self.assertEqual(profiles[2].get_name(), "thethirdprofile")

        profiles = stream.search_profiles("*")
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 3)
        self.assertIsInstance(profiles[0], Modulemd.Profile)
        self.assertEqual(profiles[0].get_name(), "thefirstprofile")
        self.assertIsInstance(profiles[1], Modulemd.Profile)
        self.assertEqual(profiles[1].get_name(), "thesecondprofile")
        self.assertIsInstance(profiles[2], Modulemd.Profile)
        self.assertEqual(profiles[2].get_name(), "thethirdprofile")

        profiles = stream.search_profiles("thefirstprofile")
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 1)
        self.assertIsInstance(profiles[0], Modulemd.Profile)
        self.assertEqual(profiles[0].get_name(), "thefirstprofile")

        profiles = stream.search_profiles("*profile")
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 3)
        self.assertIsInstance(profiles[0], Modulemd.Profile)
        self.assertEqual(profiles[0].get_name(), "thefirstprofile")
        self.assertIsInstance(profiles[1], Modulemd.Profile)
        self.assertEqual(profiles[1].get_name(), "thesecondprofile")
        self.assertIsInstance(profiles[2], Modulemd.Profile)
        self.assertEqual(profiles[2].get_name(), "thethirdprofile")

        profiles = stream.search_profiles("*dprofile*")
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 2)
        self.assertIsInstance(profiles[0], Modulemd.Profile)
        self.assertEqual(profiles[0].get_name(), "thesecondprofile")
        self.assertIsInstance(profiles[1], Modulemd.Profile)
        self.assertEqual(profiles[1].get_name(), "thethirdprofile")

        profiles = stream.search_profiles("the*profile")
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 3)
        self.assertIsInstance(profiles[0], Modulemd.Profile)
        self.assertEqual(profiles[0].get_name(), "thefirstprofile")
        self.assertIsInstance(profiles[1], Modulemd.Profile)
        self.assertEqual(profiles[1].get_name(), "thesecondprofile")
        self.assertIsInstance(profiles[2], Modulemd.Profile)
        self.assertEqual(profiles[2].get_name(), "thethirdprofile")

        profiles = stream.search_profiles("the*")
        self.assertIsNotNone(profiles)
        self.assertEqual(len(profiles), 3)
        self.assertIsInstance(profiles[0], Modulemd.Profile)
        self.assertEqual(profiles[0].get_name(), "thefirstprofile")
        self.assertIsInstance(profiles[1], Modulemd.Profile)
        self.assertEqual(profiles[1].get_name(), "thesecondprofile")
        self.assertIsInstance(profiles[2], Modulemd.Profile)
        self.assertEqual(profiles[2].get_name(), "thethirdprofile")

    def test_static_context(self):
        stream = Modulemd.ModuleStreamV2.new("themodule", "thestream")

        # Verify that it defaults to FALSE
        self.assertFalse(stream.props.static_context)
        self.assertFalse(stream.is_static_context())

        # Test setting and unsetting it via methods
        stream.set_static_context()
        self.assertTrue(stream.props.static_context)
        self.assertTrue(stream.is_static_context())

        stream.unset_static_context()
        self.assertFalse(stream.props.static_context)
        self.assertFalse(stream.is_static_context())

        # Test setting and unsetting it via properties
        stream.props.static_context = True
        self.assertTrue(stream.props.static_context)
        self.assertTrue(stream.is_static_context())

        stream.props.static_context = False
        self.assertFalse(stream.props.static_context)
        self.assertFalse(stream.is_static_context())

        # Read in a stream with a static context
        idx = Modulemd.load_file(
            "%s/static_context.yaml" % (os.getenv("TEST_DATA_PATH"))
        )
        streams = idx.search_streams()
        self.assertEqual(1, len(streams))
        stream = streams[0]
        self.assertTrue(stream.props.static_context)
        self.assertTrue(stream.is_static_context())

        expected = """---
document: modulemd
version: 2
data:
  name: nodejs
  stream: "8"
  version: 20180816123422
  context: RealCTX
  static_context: true
  arch: x86_64
  summary: Javascript runtime
  description: >-
    Node.js is a platform built on Chrome''s JavaScript runtime for easily building
    fast, scalable network applications. Node.js uses an event-driven, non-blocking
    I/O model that makes it lightweight and efficient, perfect for data-intensive
    real-time applications that run across distributed devices.
  license:
    module:
    - "MIT"
    content:
    - "DUMMY"
  dependencies:
  - buildrequires:
      platform: ["f29"]
    requires:
      platform: ["f29"]
  references:
    community: http://nodejs.org
    documentation: http://nodejs.org/en/docs
    tracker: https://github.com/nodejs/node/issues
  profiles:
    default:
      rpms:
      - nodejs
      - npm
    development:
      rpms:
      - nodejs
      - nodejs-devel
      - npm
    minimal:
      rpms:
      - nodejs
  api:
    rpms:
    - "nodejs"
    - "nodejs-devel"
    - "npm"
  components:
    rpms:
      nodejs:
        rationale: Javascript runtime and npm package manager.
        repository: git://pkgs.fedoraproject.org/rpms/nodejs
        cache: http://pkgs.fedoraproject.org/repo/pkgs/nodejs
        ref: 8
        buildorder: 10
  artifacts:
    rpms:
    - "nodejs-1:8.11.4-1.module_2030+42747d40.x86_64"
    - "nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64"
    - "nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch"
    - "npm-1:5.6.0-1.8.11.4.1.module_2030+42747d40.x86_64"
...
"""
        self.maxDiff = None
        self.assertEqual(expected, idx.dump_to_string())


if __name__ == "__main__":
    unittest.main()
