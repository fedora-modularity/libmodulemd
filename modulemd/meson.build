# This file is part of libmodulemd
# Copyright (C) 2017-2018 Stephen Gallagher
#
# Fedora-License-Identifier: MIT
# SPDX-2.0-License-Identifier: MIT
# SPDX-3.0-License-Identifier: MIT
#
# This program is free software.
# For more information on the license, see COPYING.
# For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.

developer_build = get_option('developer_build')
test_dirty_git = get_option('test_dirty_git')

python3 = pymod.find_python()

clang_format = find_program('clang-format', required: developer_build)
if not clang_format.found()
    clang_format = disabler()
endif

autopep8 = find_program('python3-autopep8', 'autopep8',
                        required : developer_build)
if not autopep8.found()
    autopep8 = disabler()
endif

valgrind = find_program('valgrind', required: developer_build)

if not valgrind.found()
    valgrind = disabler()
endif

build_api_v1 = get_option('build_api_v1')

modulemd_v1_srcs = files(
    'v1/modulemd-buildopts.c',
    'v1/modulemd-common.c',
    'v1/modulemd-component.c',
    'v1/modulemd-component-module.c',
    'v1/modulemd-component-rpm.c',
    'v1/modulemd-defaults.c',
    'v1/modulemd-dependencies.c',
    'v1/modulemd-improvedmodule.c',
    'v1/modulemd-intent.c',
    'v1/modulemd-module.c',
    'v1/modulemd-modulestream.c',
    'v1/modulemd-prioritizer.c',
    'v1/modulemd-profile.c',
    'v1/modulemd-simpleset.c',
    'v1/modulemd-servicelevel.c',
    'v1/modulemd-subdocument.c',
    'v1/modulemd-translation.c',
    'v1/modulemd-translation-entry.c',
    'v1/modulemd-util.c',
    'v1/modulemd-yaml-emitter.c',
    'v1/modulemd-yaml-emitter-defaults.c',
    'v1/modulemd-yaml-emitter-modulemd.c',
    'v1/modulemd-yaml-emitter-translation.c',
    'v1/modulemd-yaml-parser.c',
    'v1/modulemd-yaml-parser-defaults.c',
    'v1/modulemd-yaml-parser-modulemd.c',
    'v1/modulemd-yaml-parser-translation.c',
    'v1/modulemd-yaml-utils.c'
)

modulemd_v1_hdrs = files(
    'include/modulemd-1.0/modulemd.h',
    'include/modulemd-1.0/modulemd-buildopts.h',
    'include/modulemd-1.0/modulemd-component.h',
    'include/modulemd-1.0/modulemd-component-module.h',
    'include/modulemd-1.0/modulemd-component-rpm.h',
    'include/modulemd-1.0/modulemd-defaults.h',
    'include/modulemd-1.0/modulemd-dependencies.h',
    'include/modulemd-1.0/modulemd-improvedmodule.h',
    'include/modulemd-1.0/modulemd-intent.h',
    'include/modulemd-1.0/modulemd-module.h',
    'include/modulemd-1.0/modulemd-modulestream.h',
    'include/modulemd-1.0/modulemd-prioritizer.h',
    'include/modulemd-1.0/modulemd-profile.h',
    'include/modulemd-1.0/modulemd-simpleset.h',
    'include/modulemd-1.0/modulemd-servicelevel.h',
    'include/modulemd-1.0/modulemd-subdocument.h',
    'include/modulemd-1.0/modulemd-translation.h',
    'include/modulemd-1.0/modulemd-translation-entry.h',
)

modulemd_priv_hdrs = files(
    'include/modulemd-1.0/private/modulemd-improvedmodule-private.h',
    'include/modulemd-1.0/private/modulemd-private.h',
    'include/modulemd-1.0/private/modulemd-profile-private.h',
    'include/modulemd-1.0/private/modulemd-subdocument-private.h',
    'include/modulemd-1.0/private/modulemd-util.h',
    'include/modulemd-1.0/private/modulemd-yaml.h',
)

v1_include_dirs = include_directories ('include/modulemd-1.0')

test_v1_srcs = files(
    'v1/tests/test-modulemd-buildopts.c',
    'v1/tests/test-modulemd-component.c',
    'v1/tests/test-modulemd-defaults.c',
    'v1/tests/test-modulemd-dependencies.c',
    'v1/tests/test-modulemd-intent.c',
    'v1/tests/test-modulemd-module.c',
    'v1/tests/test-modulemd-modulestream.c',
    'v1/tests/test-modulemd-regressions.c',
    'v1/tests/test-modulemd-servicelevel.c',
    'v1/tests/test-modulemd-simpleset.c',
    'v1/tests/test-modulemd-subdocument.c',
    'v1/tests/test-modulemd-translation.c',
    'v1/tests/test-modulemd-translation-entry.c',
    'v1/tests/test-modulemd-yaml.c',
)

libmodulemd_version = meson.project_version()
libmodulemd_version_array = libmodulemd_version.split('.')

# Fake test to ensure that all sources and headers are formatted properly
clang_args = [ '-i' ]
test('clang_format', clang_format,
     args : clang_args +
            modulemd_v1_srcs +
            modulemd_v1_hdrs +
            modulemd_priv_hdrs +
            test_v1_srcs)

# Fake test to ensure that the python tests are formatted according to PEP8
autopep8_args = [ '--in-place', '-a', '-a' ]
autopep8_scripts = [ files('v1/tests/test-modulemd-python.py',
                           'common/tests/test-dirty.py',
                           'v1/tests/test-valgrind.py') ]
test('autopep8', autopep8,
     args: autopep8_args + autopep8_scripts)


# Fake test to ensure that the autoformatters didn't end up making changes
# when run in CI (such as during a pull-request). This test is only run if
# meson was configured with `-Dtest_dirty_git=true`

if test_dirty_git
    dirty_repo_scripts = [ files('common/tests/test-dirty.py') ]
    test('test_dirty_repo', python3,
         args: dirty_repo_scripts)
endif

if build_api_v1
    subdir('v1')
endif
