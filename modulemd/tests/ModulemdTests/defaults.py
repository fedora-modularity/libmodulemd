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

import os
import sys

try:
    import unittest
    import gi

    gi.require_version("Modulemd", "2.0")
    from gi.repository import Modulemd
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase


def _zero_mdversion():
    defs = Modulemd.Defaults.new(0, "foo")


def _unknown_mdversion():
    defs = Modulemd.Defaults.new(
        Modulemd.DefaultsVersionEnum.LATEST + 1, "foo"
    )


def _set_module_name_to_none(defs):
    defs.props.module_name = None


class TestDefaults(TestBase):
    def test_constructors(self):
        # Test that the new() function works
        defs = Modulemd.Defaults.new(Modulemd.DefaultsVersionEnum.ONE, "foo")
        assert defs

        assert defs.props.mdversion == Modulemd.DefaultsVersionEnum.ONE
        assert defs.get_mdversion() == Modulemd.DefaultsVersionEnum.ONE

        assert defs.props.module_name == "foo"
        assert defs.get_module_name() == "foo"

        # Test that we cannot instantiate directly
        with self.assertRaisesRegex(
            TypeError, "cannot create instance of abstract"
        ):
            Modulemd.Defaults()

        # Test with a zero mdversion
        self.assertRaisesRegexOrDies(
            _zero_mdversion, TypeError, "constructor returned NULL"
        )

        # Test with an unknown mdversion
        self.assertRaisesRegexOrDies(
            _unknown_mdversion, TypeError, "constructor returned NULL"
        )

        # Test with no name
        with self.assertRaisesRegex(
            TypeError, "does not allow None as a value"
        ):
            defs = Modulemd.Defaults.new(
                Modulemd.DefaultsVersionEnum.ONE, None
            )

    def test_copy(self):
        defs = Modulemd.Defaults.new(Modulemd.DefaultsVersionEnum.ONE, "foo")
        assert defs

        copied_defs = defs.copy()
        assert copied_defs
        assert copied_defs.props.mdversion == defs.props.mdversion
        assert copied_defs.props.module_name == defs.props.module_name

    def test_mdversion(self):
        defs = Modulemd.Defaults.new(
            Modulemd.DefaultsVersionEnum.LATEST, "foo"
        )
        assert defs

        assert defs.props.mdversion == Modulemd.DefaultsVersionEnum.ONE
        assert defs.get_mdversion() == Modulemd.DefaultsVersionEnum.ONE

        # Ensure we cannot set the mdversion
        with self.assertRaisesRegex(TypeError, "is not writable"):
            defs.props.mdversion = 0

    def test_module_name(self):
        defs = Modulemd.Defaults.new(
            Modulemd.DefaultsVersionEnum.LATEST, "foo"
        )
        assert defs

        assert defs.props.module_name == "foo"
        assert defs.get_module_name() == "foo"

        # Ensure we cannot set the module_name
        self.assertProcessFailure(_set_module_name_to_none, defs)

    def test_modified(self):
        defs = Modulemd.Defaults.new(
            Modulemd.DefaultsVersionEnum.LATEST, "foo"
        )
        self.assertIsNotNone(defs)

        self.assertEqual(defs.get_modified(), 0)

        defs.set_modified(201901110830)

        self.assertEqual(defs.get_modified(), 201901110830)

        # Load a defaults object into an Index
        index = Modulemd.ModuleIndex.new()
        index.update_from_file(
            "%s/yaml_specs/modulemd_defaults_v1.yaml"
            % (os.getenv("MESON_SOURCE_ROOT")),
            True,
        )
        module_names = index.get_module_names()
        self.assertEqual(len(module_names), 1)

        defs = index.get_module(index.get_module_names()[0]).get_defaults()
        self.assertIsNotNone(defs)

        self.assertEqual(defs.get_modified(), 201812071200)

    def test_validate(self):
        defs = Modulemd.Defaults.new(
            Modulemd.DefaultsVersionEnum.LATEST, "foo"
        )
        assert defs

        assert defs.validate()

    def test_upgrade(self):
        defs = Modulemd.Defaults.new(Modulemd.DefaultsVersionEnum.ONE, "foo")
        assert defs

        # test upgrading to the same version
        upgraded_defs = defs.upgrade(Modulemd.DefaultsVersionEnum.ONE)
        assert upgraded_defs
        assert defs.props.mdversion == Modulemd.DefaultsVersionEnum.ONE
        assert defs.props.module_name == "foo"

        # test upgrading to the latest version
        upgraded_defs = defs.upgrade(Modulemd.DefaultsVersionEnum.LATEST)
        assert upgraded_defs
        assert defs.props.mdversion == Modulemd.DefaultsVersionEnum.LATEST
        assert defs.props.module_name == "foo"


if __name__ == "__main__":
    unittest.main()
