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

clang_format = find_program('clang-format', required: developer_build)
if not clang_format.found()
    clang_format = disabler()
endif

pycodestyle = find_program('pycodestyle-3', 'pycodestyle',
                           required : developer_build)
if not pycodestyle.found()
    pycodestyle = disabler()
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
build_api_v2 = get_option('build_api_v2')

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
    'v1/include/modulemd-1.0/modulemd.h',
    'v1/include/modulemd-1.0/modulemd-buildopts.h',
    'v1/include/modulemd-1.0/modulemd-component.h',
    'v1/include/modulemd-1.0/modulemd-component-module.h',
    'v1/include/modulemd-1.0/modulemd-component-rpm.h',
    'v1/include/modulemd-1.0/modulemd-defaults.h',
    'v1/include/modulemd-1.0/modulemd-dependencies.h',
    'v1/include/modulemd-1.0/modulemd-deprecated.h',
    'v1/include/modulemd-1.0/modulemd-improvedmodule.h',
    'v1/include/modulemd-1.0/modulemd-intent.h',
    'v1/include/modulemd-1.0/modulemd-module.h',
    'v1/include/modulemd-1.0/modulemd-modulestream.h',
    'v1/include/modulemd-1.0/modulemd-prioritizer.h',
    'v1/include/modulemd-1.0/modulemd-profile.h',
    'v1/include/modulemd-1.0/modulemd-simpleset.h',
    'v1/include/modulemd-1.0/modulemd-servicelevel.h',
    'v1/include/modulemd-1.0/modulemd-subdocument.h',
    'v1/include/modulemd-1.0/modulemd-translation.h',
    'v1/include/modulemd-1.0/modulemd-translation-entry.h',
)

modulemd_v1_priv_hdrs = files(
    'v1/include/private/modulemd-improvedmodule-private.h',
    'v1/include/private/modulemd-private.h',
    'v1/include/private/modulemd-profile-private.h',
    'v1/include/private/modulemd-subdocument-private.h',
    'v1/include/private/modulemd-util.h',
    'v1/include/private/modulemd-yaml.h',
)

v1_include_dirs = include_directories (
    'v1/include/modulemd-1.0',
    'v1/include'
)

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


modulemd_v2_srcs = files(
    'v2/modulemd.c',
    'v2/modulemd-buildopts.c',
    'v2/modulemd-component.c',
    'v2/modulemd-component-module.c',
    'v2/modulemd-component-rpm.c',
    'v2/modulemd-defaults.c',
    'v2/modulemd-defaults-v1.c',
    'v2/modulemd-dependencies.c',
    'v2/modulemd-module.c',
    'v2/modulemd-module-index.c',
    'v2/modulemd-module-index-merger.c',
    'v2/modulemd-module-stream.c',
    'v2/modulemd-module-stream-v1.c',
    'v2/modulemd-module-stream-v2.c',
    'v2/modulemd-profile.c',
    'v2/modulemd-rpm-map-entry.c',
    'v2/modulemd-service-level.c',
    'v2/modulemd-subdocument-info.c',
    'v2/modulemd-translation.c',
    'v2/modulemd-translation-entry.c',
    'v2/modulemd-util.c',
    'v2/modulemd-yaml-util.c',
)

modulemd_v2_validator_srcs = files (
    'v2/modulemd-validator.c',
)

modulemd_v2_hdrs = files(
    'v2/include/modulemd-2.0/modulemd.h',
    'v2/include/modulemd-2.0/modulemd-buildopts.h',
    'v2/include/modulemd-2.0/modulemd-component.h',
    'v2/include/modulemd-2.0/modulemd-component-module.h',
    'v2/include/modulemd-2.0/modulemd-component-rpm.h',
    'v2/include/modulemd-2.0/modulemd-defaults.h',
    'v2/include/modulemd-2.0/modulemd-defaults-v1.h',
    'v2/include/modulemd-2.0/modulemd-dependencies.h',
    'v2/include/modulemd-2.0/modulemd-deprecated.h',
    'v2/include/modulemd-2.0/modulemd-module.h',
    'v2/include/modulemd-2.0/modulemd-module-index.h',
    'v2/include/modulemd-2.0/modulemd-module-index-merger.h',
    'v2/include/modulemd-2.0/modulemd-module-stream.h',
    'v2/include/modulemd-2.0/modulemd-module-stream-v1.h',
    'v2/include/modulemd-2.0/modulemd-module-stream-v2.h',
    'v2/include/modulemd-2.0/modulemd-profile.h',
    'v2/include/modulemd-2.0/modulemd-rpm-map-entry.h',
    'v2/include/modulemd-2.0/modulemd-service-level.h',
    'v2/include/modulemd-2.0/modulemd-subdocument-info.h',
    'v2/include/modulemd-2.0/modulemd-translation.h',
    'v2/include/modulemd-2.0/modulemd-translation-entry.h',
)

modulemd_v2_priv_hdrs = files(
    'v2/include/private/glib-extensions.h',
    'v2/include/private/modulemd-buildopts-private.h',
    'v2/include/private/modulemd-component-private.h',
    'v2/include/private/modulemd-component-module-private.h',
    'v2/include/private/modulemd-component-rpm-private.h',
    'v2/include/private/modulemd-dependencies-private.h',
    'v2/include/private/modulemd-profile-private.h',
    'v2/include/private/modulemd-defaults-private.h',
    'v2/include/private/modulemd-defaults-v1-private.h',
    'v2/include/private/modulemd-module-private.h',
    'v2/include/private/modulemd-module-index-private.h',
    'v2/include/private/modulemd-module-stream-private.h',
    'v2/include/private/modulemd-module-stream-v1-private.h',
    'v2/include/private/modulemd-module-stream-v2-private.h',
    'v2/include/private/modulemd-service-level-private.h',
    'v2/include/private/modulemd-subdocument-info-private.h',
    'v2/include/private/modulemd-translation-private.h',
    'v2/include/private/modulemd-translation-entry-private.h',
    'v2/include/private/modulemd-util.h',
    'v2/include/private/modulemd-yaml.h',
    'v2/include/private/gi-binding-renames.h',
)

test_v2_srcs = files(
    'v2/tests/test-modulemd-buildopts.c',
    'v2/tests/test-modulemd-component-module.c',
    'v2/tests/test-modulemd-component-rpm.c',
    'v2/tests/test-modulemd-defaults.c',
    'v2/tests/test-modulemd-defaults-v1.c',
    'v2/tests/test-modulemd-dependencies.c',
    'v2/tests/test-modulemd-merger.c',
    'v2/tests/test-modulemd-module.c',
    'v2/tests/test-modulemd-moduleindex.c',
    'v2/tests/test-modulemd-modulestream.c',
    'v2/tests/test-modulemd-profile.c',
    'v2/tests/test-modulemd-rpmmap.c',
    'v2/tests/test-modulemd-service-level.c',
    'v2/tests/test-modulemd-translation.c',
    'v2/tests/test-modulemd-translation-entry.c',
    'v2/tests/test-utils.c',
)

test_v2_priv_hdrs = files(
    'v2/include/private/test-utils.h',
)

test_v2_python_scripts = files(
    'v2/tests/ModulemdTests/base.py',
    'v2/tests/ModulemdTests/buildopts.py',
    'v2/tests/ModulemdTests/componentrpm.py',
    'v2/tests/ModulemdTests/defaults.py',
    'v2/tests/ModulemdTests/defaultsv1.py',
    'v2/tests/ModulemdTests/dependencies.py',
    'v2/tests/ModulemdTests/merger.py',
    'v2/tests/ModulemdTests/module.py',
    'v2/tests/ModulemdTests/moduleindex.py',
    'v2/tests/ModulemdTests/modulestream.py',
    'v2/tests/ModulemdTests/profile.py',
    'v2/tests/ModulemdTests/rpmmap.py',
    'v2/tests/ModulemdTests/servicelevel.py',
    'v2/tests/ModulemdTests/translation.py',
    'v2/tests/ModulemdTests/translationentry.py',
)

v2_include_dirs = include_directories (
    'v2/include/modulemd-2.0',
    'v2/include'
)


# Fake test to ensure that all sources and headers are formatted properly
clang_args = [ '-i' ]
test('clang_format', clang_format,
     args : clang_args +
            modulemd_v1_srcs +
            modulemd_v1_hdrs +
            modulemd_v1_priv_hdrs +
            test_v1_srcs +
            modulemd_v2_srcs +
            modulemd_v2_hdrs +
            modulemd_v2_priv_hdrs +
            modulemd_v2_validator_srcs +
            test_v2_srcs +
            test_v2_priv_hdrs)


# Fake test to ensure that the python tests are formatted according to PEP8
autopep8_args = [ '--in-place', '-a', '-a', '--global-config', '../tox.ini' ]
autopep8_scripts = [ files('v1/tests/test-modulemd-python.py',
                           'common/tests/test-dirty.py',
                           'common/tests/test-valgrind.py',
                           '../bindings/python/gi/overrides/Modulemd.py',
                           '../ModulemdTranslationHelpers/Utils.py',
                           'v2/tests/ModulemdTests/translation_helpers.py') +
                     test_v2_python_scripts ]
test('autopep8', autopep8,
     args: autopep8_args + autopep8_scripts)


# Test all python files for compliance with pycodestyle
# Ignore line-length as there are a number of places where this is unavoidable
pycodestyle_args = [
    '--show-pep8',
    '--ignore=E121,E123,E126,E226,E24,E501,E704,W503,W504'
]
test('pycodestyle', pycodestyle,
     args: pycodestyle_args + autopep8_scripts)


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
    import_header_script = find_program('common/tests/test-import-headers.sh')
    test ('test_v1_import_headers', import_header_script,
          env : test_env,
          args : modulemd_v1_hdrs)
endif

if build_api_v2
    subdir('v2')
    import_header_script = find_program('common/tests/test-import-headers.sh')
    test ('test_v2_import_headers', import_header_script,
          env : test_env,
          args : modulemd_v2_hdrs)
endif

if valgrind.found()
    modulemd_valgrind_scripts = files('common/tests/test-valgrind.py')
    test ('valgrind', python3,
          env : test_env,
          args : modulemd_valgrind_scripts,
          timeout : 1200)
endif
