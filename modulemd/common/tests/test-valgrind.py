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
import subprocess
import tempfile
import xml.etree.ElementTree as ET

if os.getenv('MMD_SKIP_VALGRIND'):
    sys.exit(77)

failed = False

with tempfile.TemporaryDirectory(prefix="libmodulemd_valgrind_") as tmpdirname:
    for test in sys.argv[1:]:
        valgrind_command = "/usr/bin/valgrind " \
                           "--leak-check=full " \
                           "--xml=yes " \
                           "--xml-file=%s/%s.xml" % (tmpdirname, test)
        proc_result = subprocess.run(
            [
                'meson',
                'test',
                '-t', '10',
                '--logbase=%s' % test,
                '--wrap=%s' % valgrind_command,
                test])

        if proc_result.returncode != 0:
            print("Valgrind exited with an error on %s" % test,
                  file=sys.stderr)
            failed = True
            continue

        # Process the XML for leaks
        tree = ET.parse('%s/%s.xml' % (tmpdirname, test))
        root = tree.getroot()

        for root_child in root:
            if (root_child.tag == "error"):
                for error_child in root_child:
                    if error_child.tag == 'kind':
                        if error_child.text == 'Leak_DefinitelyLost':
                            print("Memory leak detected in %s" % test,
                                  file=sys.stderr)
                            failed = True

                        elif error_child.text == 'InvalidFree':
                            print("Invalid free() detetected in %s" % test,
                                  file=sys.stderr)
                            failed = True

                        elif error_child.text == 'InvalidRead':
                            print("Invalid read detected in %s" % test,
                                  file=sys.stderr)
                            failed = True

                        elif error_child.text == 'UninitCondition':
                            print("Uninitialized usage detected in %s" % test,
                                  file=sys.stderr)
                            failed = True
        if failed:
            with open('%s/%s.xml' % (tmpdirname, test), 'r') as xml:
                print(xml.read())


if failed:
    sys.exit(1)
