#!/usr/bin/python3

import os
import sys
import subprocess
import tempfile
import xml.etree.ElementTree as ET

tests = [
    'test_modulemd_buildopts',
    'test_modulemd_component',
    'test_modulemd_defaults',
    'test_modulemd_dependencies',
    'test_modulemd_intent',
    'test_modulemd_module',
    'test_modulemd_regressions',
    'test_modulemd_servicelevel',
    'test_modulemd_simpleset',
    'test_modulemd_subdocument',
    'test_modulemd_yaml',
]

failed = False

with tempfile.TemporaryDirectory(prefix="libmodulemd_valgrind_") as tmpdirname:
    for test in tests:
        valgrind_command = "/usr/bin/valgrind " \
                           "--leak-check=full " \
                           "--xml=yes " \
                           "--xml-file=%s/%s.xml" % (tmpdirname, test)
        proc_result = subprocess.run(
            [
                'meson',
                'test',
                '--logbase=%s' % test,
                '--wrap=%s' % valgrind_command,
                test])

        if proc_result.returncode != 0:
            print("Valgrind exited with an error on %s" % test,
                  file=sys.stderr)
            failed = True

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


if failed:
    os.exit(1)
