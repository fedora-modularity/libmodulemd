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

test_installed_lib = get_option('test_installed_lib')
skip_introspection = get_option('skip_introspection')
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
valgrind_tests = []

if test_installed_lib
    # If we're just running the tests on a library that's already installed,
    # don't bother building it again.
    build_lib = disabler()
else
    build_lib = declare_dependency()
endif


modulemd_srcs = files(
    'modulemd-buildopts.c',
    'modulemd-common.c',
    'modulemd-component.c',
    'modulemd-component-module.c',
    'modulemd-component-rpm.c',
    'modulemd-defaults.c',
    'modulemd-dependencies.c',
    'modulemd-improvedmodule.c',
    'modulemd-intent.c',
    'modulemd-module.c',
    'modulemd-modulestream.c',
    'modulemd-prioritizer.c',
    'modulemd-profile.c',
    'modulemd-simpleset.c',
    'modulemd-servicelevel.c',
    'modulemd-subdocument.c',
    'modulemd-translation.c',
    'modulemd-translation-entry.c',
    'modulemd-util.c',
    'modulemd-yaml-emitter.c',
    'modulemd-yaml-emitter-defaults.c',
    'modulemd-yaml-emitter-modulemd.c',
    'modulemd-yaml-emitter-translation.c',
    'modulemd-yaml-parser.c',
    'modulemd-yaml-parser-defaults.c',
    'modulemd-yaml-parser-modulemd.c',
    'modulemd-yaml-parser-translation.c',
    'modulemd-yaml-utils.c'
)

modulemd_hdrs = files(
    'include/modulemd-1.0/modulemd.h',
    'include/modulemd-1.0/modulemd-buildopts.h',
    'include/modulemd-1.0/modulemd-component.h',
    'include/modulemd-1.0/modulemd-component-module.h',
    'include/modulemd-1.0/modulemd-component-rpm.h',
    'include/modulemd-1.0/modulemd-defaults.h',
    'include/modulemd-1.0/modulemd-dependencies.h',
    'include/modulemd-1.0/modulemd-deprecated.h',
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
    'include/private/modulemd-improvedmodule-private.h',
    'include/private/modulemd-private.h',
    'include/private/modulemd-profile-private.h',
    'include/private/modulemd-subdocument-private.h',
    'include/private/modulemd-util.h',
    'include/private/modulemd-yaml.h',
)

include_dirs = include_directories (
    'include/modulemd-1.0',
    'include'
)

test_srcs = files(
    'tests/test-modulemd-buildopts.c',
    'tests/test-modulemd-component.c',
    'tests/test-modulemd-defaults.c',
    'tests/test-modulemd-dependencies.c',
    'tests/test-modulemd-intent.c',
    'tests/test-modulemd-module.c',
    'tests/test-modulemd-modulestream.c',
    'tests/test-modulemd-regressions.c',
    'tests/test-modulemd-servicelevel.c',
    'tests/test-modulemd-simpleset.c',
    'tests/test-modulemd-subdocument.c',
    'tests/test-modulemd-translation.c',
    'tests/test-modulemd-translation-entry.c',
    'tests/test-modulemd-yaml.c',
)


modulemd_lib = library(
    'modulemd',
    sources : modulemd_srcs,
    include_directories : include_dirs,
    dependencies : [
        gobject,
        yaml,
        build_lib,
    ],
    install : true,
    version: libmodulemd_version,
)

if test_installed_lib
    # Run tests against an installed library instead of in-tree
    modulemd_dep = declare_dependency(
        include_directories : include_dirs,
        dependencies : [
            gobject,
            dependency(
                'modulemd',
                version: ['>= 1.0.0', '< 2.0.0']
            ),
        ]
    )
else
    modulemd_dep = declare_dependency(
        include_directories : include_dirs,
        link_with : modulemd_lib,
        dependencies : [
            gobject,
        ]
    )
endif


modulemd_validator_v1 = executable(
    'modulemd-validator-v1',
    'modulemd-validator.c',
    dependencies : [
        modulemd_dep,
    ],
    install : true,
)

header_path = 'modulemd'

install_headers(
  modulemd_hdrs,
  subdir : header_path,
)


# Test env with release values
test_release_env = environment()
test_release_env.set('G_MESSAGES_DEBUG', 'all')
test_release_env.set ('MESON_SOURCE_ROOT', meson.source_root())


# Test env with fatal warnings and criticals
test_env = test_release_env
test_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')

test_modulemd_buildopts = executable(
    'test_modulemd_buildopts',
    'tests/test-modulemd-buildopts.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)

test('test_modulemd_buildopts', test_modulemd_buildopts,
     env : test_env)
test('test_release_modulemd_buildopts', test_modulemd_buildopts,
     env : test_release_env)
valgrind_tests += 'test_modulemd_buildopts'

test_modulemd_component = executable(
    'test_modulemd_component',
    'tests/test-modulemd-component.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_component', test_modulemd_component,
     env : test_env)
test('test_release_modulemd_component', test_modulemd_component,
     env : test_release_env)
valgrind_tests += 'test_modulemd_component'

test_modulemd_defaults = executable(
    'test_modulemd_defaults',
    'tests/test-modulemd-defaults.c',
    dependencies : [
        modulemd_dep,
        yaml
    ],
    install : false,
)
test('test_modulemd_defaults', test_modulemd_defaults,
     env : test_env)
test('test_release_modulemd_defaults', test_modulemd_defaults,
     env : test_release_env)
valgrind_tests += 'test_modulemd_defaults'

test_modulemd_dependencies = executable(
    'test_modulemd_dependencies',
    'tests/test-modulemd-dependencies.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_dependencies', test_modulemd_dependencies,
     env : test_env)
test('test_release_modulemd_dependencies', test_modulemd_dependencies,
     env : test_release_env)
valgrind_tests += 'test_modulemd_dependencies'

test_modulemd_intent = executable(
    'test_modulemd_intent',
    'tests/test-modulemd-intent.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_intent', test_modulemd_intent,
     env : test_env)
test('test_release_modulemd_intent', test_modulemd_intent,
     env : test_release_env)
valgrind_tests += 'test_modulemd_intent'

test_modulemd_module = executable(
    'test_modulemd_module',
    'tests/test-modulemd-module.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_module', test_modulemd_module,
     env : test_env)
test('test_release_modulemd_module', test_modulemd_module,
     env : test_release_env)
valgrind_tests += 'test_modulemd_module'

test_modulemd_modulestream = executable(
    'test_modulemd_modulestream',
    'tests/test-modulemd-modulestream.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_modulestream', test_modulemd_modulestream,
     env : test_env)
test('test_release_modulemd_modulestream', test_modulemd_modulestream,
     env : test_release_env)
valgrind_tests += 'test_modulemd_modulestream'

test_modulemd_regressions = executable(
    'test_modulemd_regressions',
    'tests/test-modulemd-regressions.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_regressions', test_modulemd_regressions,
    env : test_env)
test('test_release_modulemd_regressions', test_modulemd_regressions,
    env : test_release_env)
valgrind_tests += 'test_modulemd_regressions'

test_modulemd_servicelevel = executable(
    'test_modulemd_servicelevel',
    'tests/test-modulemd-servicelevel.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_servicelevel', test_modulemd_servicelevel,
     env : test_env)
test('test_release_modulemd_servicelevel', test_modulemd_servicelevel,
     env : test_release_env)
valgrind_tests += 'test_modulemd_servicelevel'

test_modulemd_simpleset = executable(
    'test_modulemd_simpleset',
    'tests/test-modulemd-simpleset.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_simpleset', test_modulemd_simpleset,
     env : test_env)
test('test_release_modulemd_simpleset', test_modulemd_simpleset,
     env : test_release_env)
valgrind_tests += 'test_modulemd_simpleset'

test_modulemd_subdocument = executable(
    'test_modulemd_subdocument',
    'tests/test-modulemd-subdocument.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_subdocument', test_modulemd_subdocument,
     env : test_env)
test('test_release_modulemd_subdocument', test_modulemd_subdocument,
     env : test_release_env)
valgrind_tests += 'test_modulemd_subdocument'


test_modulemd_translation = executable(
    'test_modulemd_translation',
    'tests/test-modulemd-translation.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_translation',
     test_modulemd_translation,
     env : test_env)
test('test_release_modulemd_translation',
     test_modulemd_translation,
     env : test_release_env)
valgrind_tests += 'test_modulemd_translation'


test_modulemd_translation_entry = executable(
    'test_modulemd_translation_entry',
    'tests/test-modulemd-translation-entry.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_translation_entry',
     test_modulemd_translation_entry,
     env : test_env)
test('test_release_modulemd_translation_entry',
     test_modulemd_translation_entry,
     env : test_release_env)
valgrind_tests += 'test_modulemd_translation_entry'


test_modulemd_yaml = executable(
    'test_modulemd_yaml',
    'tests/test-modulemd-yaml.c',
    dependencies : [
        modulemd_dep,
        yaml
    ],
    install : false,
)
test('test_modulemd_yaml', test_modulemd_yaml,
     env : test_env)
test('test_release_modulemd_yaml', test_modulemd_yaml,
     env : test_release_env)
valgrind_tests += 'test_modulemd_yaml'

# Test env with fatal warnings and criticals
py_test_env = test_env

if not test_installed_lib
    # If we're testing an installed version, we want to use the default
    # locations for these paths.
    py_test_env.set ('GI_TYPELIB_PATH', meson.build_root() + '/modulemd')
    py_test_env.set ('LD_LIBRARY_PATH', meson.build_root() + '/modulemd')

    # This test is just to catch whether we are accidentally not testing
    # the built version.
    py_test_env.set ('MODULEMD_VERSION', libmodulemd_version)
endif

# Test env with release values
py_test_release_env = py_test_env
py_test_release_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')

modulemd_python_scripts = files('tests/test-modulemd-python.py')
test ('test_modulemd_python', python3,
      env : py_test_env,
      args : modulemd_python_scripts)
test ('test_release_modulemd_python', python3,
      env : py_test_release_env,
      args : modulemd_python_scripts)


# Fake test to ensure that all sources and headers are formatted properly
clang_args = [ '-i' ]
test('clang_format', clang_format,
     args : clang_args +
            modulemd_srcs +
            modulemd_hdrs +
            modulemd_priv_hdrs +
            test_srcs)


# Fake test to ensure that the python tests are formatted according to PEP8
autopep8_args = [ '--in-place', '-a', '-a', '--global-config', '../tox.ini' ]
autopep8_scripts = [ files('tests/test-modulemd-python.py',
                           'tests/test-dirty.py',
                           'tests/test-valgrind.py',) ]
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
    dirty_repo_scripts = [ files('tests/test-dirty.py') ]
    test('test_dirty_repo', python3,
         args: dirty_repo_scripts)
endif


import_header_script = find_program('tests/test-import-headers.sh')
test ('test_import_headers', import_header_script,
      env : test_env,
      args : modulemd_hdrs)


if valgrind.found()
    modulemd_valgrind_scripts = files('tests/test-valgrind.py')
    test ('valgrind', python3,
          env : test_env,
          args : modulemd_valgrind_scripts + valgrind_tests,
          timeout : 1200)
endif


if skip_introspection
else
    gnome.generate_gir(
        modulemd_lib,
        sources : modulemd_srcs + modulemd_hdrs,
        nsversion : '1.0',
        namespace : 'Modulemd',
        symbol_prefix : 'modulemd_',
        identifier_prefix : 'Modulemd',
        includes : [
            'GObject-2.0',
        ],
        install : true,
        )
endif

cdata = configuration_data()
cdata.set_quoted('LIBMODULEMD_VERSION', libmodulemd_version)
configure_file(
  output : 'config.h',
  configuration : cdata
)

pkg.generate(
    libraries : modulemd_lib,
    subdirs : header_path,
    version : libmodulemd_version,
    name : 'modulemd',
    filebase : 'modulemd',
    description : 'Module metadata manipulation library',
    requires: [ 'glib-2.0', 'gobject-2.0' ],
)
xcdata = configuration_data()
xcdata.set('VERSION', libmodulemd_version)
configure_file(
  input : 'version.xml.in',
  output : 'version.xml',
  configuration : xcdata
)

gnome.gtkdoc(
    'modulemd-1.0',
    install_dir: 'modulemd-1.0',
    src_dir : './modulemd',
    main_xml : 'modulemd-docs.xml',
    install : true,
)

