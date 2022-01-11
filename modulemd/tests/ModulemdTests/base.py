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

from contextlib import contextmanager
import signal
import os
import unittest


class TestBase(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestBase, self).__init__(*args, **kwargs)
        self._caught_signal = False

    @property
    def source_root(self):
        return os.getenv("MESON_SOURCE_ROOT")

    @property
    def test_data_path(self):
        return os.getenv("TEST_DATA_PATH")

    def _catch_signal(self, *sigargs):
        if self._caught_signal:
            raise AssertionError("Multiple signals were caught")
        self._caught_signal = True

    @contextmanager
    def expect_signal(
        self, expected_signal=signal.SIGTRAP, only_on_fatal_warnings=False
    ):
        expect_signal = (not only_on_fatal_warnings) or self.warnings_fatal

        self._caught_signal = False

        saved_signal = signal.signal(expected_signal, self._catch_signal)
        yield None
        signal.signal(expected_signal, saved_signal)
        if not self._caught_signal and expect_signal:
            raise AssertionError("No signal got caught")
        elif self._caught_signal and not expect_signal:
            raise AssertionError("Signal caught in non-warning state")

    @property
    def warnings_fatal(self):
        gdebug = os.getenv("G_DEBUG", "").split(",")
        return "fatal-warnings" in gdebug

    def assertRaisesRegex(
        self, expected_exception, expected_regex, *args, **kwargs
    ):
        """Asserts that the message in a raised exception matches a regex.

        Args:
            expected_exception: Exception class expected to be raised.
            expected_regex: Regex (re.Pattern object or string) expected
                    to be found in error message.
            args: Function to be called and extra positional args.
            kwargs: Extra kwargs.
            msg: Optional message used in case of failure. Can only be used
                    when assertRaisesRegex is used as a context manager.
        """
        try:
            return super().assertRaisesRegex(
                expected_exception, expected_regex, *args, **kwargs
            )
        except AttributeError:
            return super().assertRaisesRegexp(
                expected_exception, expected_regex, *args, **kwargs
            )
