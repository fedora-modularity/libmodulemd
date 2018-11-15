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
import unittest


class TestBase(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestBase, self).__init__(*args, **kwargs)
        self._caught_signal = False

    def _catch_signal(self, *sigargs):
        if self._caught_signal:
            raise AssertionError("Multiple signals were caught")
        self._caught_signal = True

    @contextmanager
    def expect_signal(self, expected_signal=signal.SIGTRAP):
        self._caught_signal = False

        saved_signal = signal.signal(expected_signal, self._catch_signal)
        yield None
        signal.signal(expected_signal, saved_signal)
        if not self._caught_signal:
            raise AssertionError("No signal got caught")
