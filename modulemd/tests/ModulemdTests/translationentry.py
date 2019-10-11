#!/usr/bin/python3
# -*- coding: utf-8 -*-

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

    gi.require_version("Modulemd", "2.0")
    from gi.repository import Modulemd
except ImportError:
    # Return error 77 to skip this test on platforms without the necessary
    # python modules
    sys.exit(77)

from base import TestBase


class TestTranslationEntry(TestBase):
    def test_constructors(self):
        # Test that the new() function works
        te = Modulemd.TranslationEntry.new("en_US")
        assert te
        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.props.summary is None
        assert te.get_summary() is None
        assert te.props.description is None
        assert te.get_description() is None
        assert te.get_profiles() == []
        assert te.get_profile_description("test") is None

        # Test that keyword arg locale is accepted
        te = Modulemd.TranslationEntry(locale="en_US")
        assert te
        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.props.summary is None
        assert te.get_summary() is None
        assert te.props.description is None
        assert te.get_description() is None
        assert te.get_profiles() == []
        assert te.get_profile_description("test") is None

        # Test that init works with locale and summary
        te = Modulemd.TranslationEntry(locale="en_US", summary="foobar")
        assert te
        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.props.summary == "foobar"
        assert te.get_summary() == "foobar"
        assert te.props.description is None
        assert te.get_description() is None
        assert te.get_profiles() == []
        assert te.get_profile_description("test") is None

        # Test that init works with locale and description
        te = Modulemd.TranslationEntry(locale="en_US", description="barfoo")
        assert te
        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.props.summary is None
        assert te.get_summary() is None
        assert te.props.description == "barfoo"
        assert te.get_description() == "barfoo"
        assert te.get_profiles() == []
        assert te.get_profile_description("test") is None

        # Test that init works with locale, summary and description
        te = Modulemd.TranslationEntry(
            locale="en_US", summary="foobar", description="barfoo"
        )
        assert te
        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.props.summary == "foobar"
        assert te.get_summary() == "foobar"
        assert te.props.description == "barfoo"
        assert te.get_description() == "barfoo"
        assert te.get_profiles() == []
        assert te.get_profile_description("test") is None

        # Test that init works with locale, unicode summary and unicode
        # description
        te = Modulemd.TranslationEntry(
            locale="ro_TA",  # robots_Tables
            summary="(┛ಠ_ಠ)┛彡┻━┻",
            description="(┛ಠ_ಠ)┛彡",
        )
        assert te
        assert te.props.locale == "ro_TA"
        assert te.get_locale() == "ro_TA"
        assert te.props.summary == "(┛ಠ_ಠ)┛彡┻━┻"
        assert te.get_summary() == "(┛ಠ_ಠ)┛彡┻━┻"
        assert te.props.description == "(┛ಠ_ಠ)┛彡"
        assert te.get_description() == "(┛ಠ_ಠ)┛彡"
        assert te.get_profiles() == []
        assert te.get_profile_description("test") is None

        # Test that we fail if we call new() with a None locale
        try:
            te = Modulemd.TranslationEntry.new(None)
            assert False
        except TypeError as e:
            assert "does not allow None as a value" in e.__str__()

        # Test that we fail if object is instantiated without a locale
        with self.expect_signal():
            Modulemd.TranslationEntry()

        # Test that we fail if object is instantiated with a None locale
        with self.expect_signal():
            Modulemd.TranslationEntry(locale=None)

    def test_copy(self):
        te_orig = Modulemd.TranslationEntry(locale="en_US")
        te = te_orig.copy()
        assert te
        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.props.summary is None
        assert te.get_summary() is None
        assert te.props.description is None
        assert te.get_description() is None
        assert te.get_profiles() == []
        assert te.get_profile_description("test") is None

        te_orig.set_summary("foobar")
        te_orig.set_description("barfoo")
        te_orig.set_profile_description("test1", "brown fox")
        te_orig.set_profile_description("test2", "jumped")

        te = te_orig.copy()
        assert te
        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.props.summary == "foobar"
        assert te.get_summary() == "foobar"
        assert te.props.description == "barfoo"
        assert te.get_description() == "barfoo"
        assert te.get_profiles() == ["test1", "test2"]
        assert te.get_profile_description("test") is None
        assert te.get_profile_description("test1") == "brown fox"
        assert te.get_profile_description("test2") == "jumped"

    def test_get_locale(self):
        te = Modulemd.TranslationEntry(locale="en_US")

        assert te.get_locale() == "en_US"
        assert te.props.locale == "en_US"

        with self.expect_signal():
            te.props.locale = "en_GB"

    def test_get_set_summary(self):
        te = Modulemd.TranslationEntry(locale="en_US")

        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.props.summary is None
        assert te.get_summary() is None

        te.set_summary("foobar")
        assert te.props.summary == "foobar"
        assert te.get_summary() == "foobar"

        te.props.summary = "barfoo"
        assert te.props.summary == "barfoo"
        assert te.get_summary() == "barfoo"

        te.props.summary = None
        assert te.props.summary is None
        assert te.get_summary() is None

    def test_get_set_description(self):
        te = Modulemd.TranslationEntry(locale="en_US")

        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.props.description is None
        assert te.get_description() is None

        te.set_description("foobar")
        assert te.props.description == "foobar"
        assert te.get_description() == "foobar"

        te.props.description = "barfoo"
        assert te.props.description == "barfoo"
        assert te.get_description() == "barfoo"

        te.props.description = None
        assert te.props.description is None
        assert te.get_description() is None

    def test_profile_descriptions(self):
        te = Modulemd.TranslationEntry(locale="en_US")

        assert te.props.locale == "en_US"
        assert te.get_locale() == "en_US"
        assert te.get_profiles() == []
        assert te.get_profile_description("test1") is None
        assert te.get_profile_description("test2") is None
        assert te.get_profile_description("test3") is None

        # Add a profile
        te.set_profile_description("test1", "foobar")
        assert te.get_profiles() == ["test1"]
        assert te.get_profile_description("test1") == "foobar"
        assert te.get_profile_description("test2") is None
        assert te.get_profile_description("test3") is None

        # Add a second profile
        te.set_profile_description("test2", "barfoo")
        assert te.get_profiles() == ["test1", "test2"]
        assert te.get_profile_description("test1") == "foobar"
        assert te.get_profile_description("test2") == "barfoo"
        assert te.get_profile_description("test3") is None

        # Add a third one that is supposed to go before the others
        te.set_profile_description("test3", "foobarfoo")
        assert te.get_profiles() == ["test1", "test2", "test3"]
        assert te.get_profile_description("test1") == "foobar"
        assert te.get_profile_description("test2") == "barfoo"
        assert te.get_profile_description("test3") == "foobarfoo"


if __name__ == "__main__":
    unittest.main()
