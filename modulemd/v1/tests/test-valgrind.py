#!/usr/bin/python3

import os
import sys
import subprocess
import tempfile
import xml.etree.ElementTree as ET

if os.getenv('MMD_SKIP_VALGRIND'):
    sys.exit(77)

tests = [
    'test_v1_modulemd_buildopts',
    'test_v1_modulemd_component',
    'test_v1_modulemd_defaults',
    'test_v1_modulemd_dependencies',
    'test_v1_modulemd_intent',
    'test_v1_modulemd_module',
    'test_v1_modulemd_modulestream',
    'test_v1_modulemd_regressions',
    'test_v1_modulemd_servicelevel',
    'test_v1_modulemd_simpleset',
    'test_v1_modulemd_subdocument',
    'test_v1_modulemd_yaml',
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
                        elif error_child.text == 'InvalidRead':
                            print("Invalid read detected in %s" % test,
                                  file=sys.stderr)
                            failed = True
                        elif error_child.text == 'UninitCondition':
                            print("Invalid read detected in %s" % test,
                                  file=sys.stderr)


if failed:
    sys.exit(1)
