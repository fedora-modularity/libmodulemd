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

modulemd_v1_lib = library(
    'modulemd',
    sources : modulemd_v1_srcs,
    include_directories : v1_include_dirs,
    dependencies : [
        gobject,
        yaml,
    ],
    install : true,
    soversion: libmodulemd_version_array[0],
    version: libmodulemd_version,
)

modulemd_v1_dep = declare_dependency(
    include_directories : v1_include_dirs,
    link_with : modulemd_v1_lib,
    dependencies : [
        gobject,
    ]
)


modulemd_validator = executable(
    'modulemd-validator',
    'modulemd-validator.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : true,
)

v1_header_path = 'modulemd'

install_headers(
  modulemd_v1_hdrs,
  subdir : v1_header_path,
)

# Test env with fatal warnings and criticals
test_env = environment()
test_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')
test_env.set('G_MESSAGES_DEBUG', 'all')
test_env.set ('MESON_SOURCE_ROOT', meson.source_root())
test_env.set ('GI_TYPELIB_PATH', meson.build_root() + '/modulemd/v1')
test_env.set ('MODULEMD_VERSION', libmodulemd_version)
test_env.set ('MODULEMD_NSVERSION', '.'.join([libmodulemd_version_array[0], '0']))
test_env.set ('LD_LIBRARY_PATH', meson.build_root() + '/modulemd/v1')

# Test env with release values
test_release_env = environment()
test_release_env.set('G_MESSAGES_DEBUG', 'all')
test_release_env.set ('MESON_SOURCE_ROOT', meson.source_root())
test_release_env.set ('GI_TYPELIB_PATH', meson.build_root() + '/modulemd/v1')
test_release_env.set ('MODULEMD_VERSION', libmodulemd_version)
test_release_env.set ('MODULEMD_NSVERSION', '.'.join([libmodulemd_version_array[0], '0']))
test_release_env.set ('LD_LIBRARY_PATH', meson.build_root() + '/modulemd/v1')

test_v1_modulemd_buildopts = executable(
    'test_v1_modulemd_buildopts',
    'tests/test-modulemd-buildopts.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_buildopts', test_v1_modulemd_buildopts,
     env : test_env)
test('test_v1_release_modulemd_buildopts', test_v1_modulemd_buildopts,
     env : test_release_env)

test_v1_modulemd_component = executable(
    'test_v1_modulemd_component',
    'tests/test-modulemd-component.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_component', test_v1_modulemd_component,
     env : test_env)
test('test_v1_release_modulemd_component', test_v1_modulemd_component,
     env : test_release_env)

test_v1_modulemd_defaults = executable(
    'test_v1_modulemd_defaults',
    'tests/test-modulemd-defaults.c',
    dependencies : [
        modulemd_v1_dep,
        yaml
    ],
    install : false,
)
test('test_v1_modulemd_defaults', test_v1_modulemd_defaults,
     env : test_env)
test('test_v1_release_modulemd_defaults', test_v1_modulemd_defaults,
     env : test_release_env)

test_v1_modulemd_dependencies = executable(
    'test_v1_modulemd_dependencies',
    'tests/test-modulemd-dependencies.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_dependencies', test_v1_modulemd_dependencies,
     env : test_env)
test('test_v1_release_modulemd_dependencies', test_v1_modulemd_dependencies,
     env : test_release_env)

test_v1_modulemd_intent = executable(
    'test_v1_modulemd_intent',
    'tests/test-modulemd-intent.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_intent', test_v1_modulemd_intent,
     env : test_env)
test('test_v1_release_modulemd_intent', test_v1_modulemd_intent,
     env : test_release_env)

test_v1_modulemd_module = executable(
    'test_v1_modulemd_module',
    'tests/test-modulemd-module.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_module', test_v1_modulemd_module,
     env : test_env)
test('test_v1_release_modulemd_module', test_v1_modulemd_module,
     env : test_release_env)

test_v1_modulemd_modulestream = executable(
    'test_v1_modulemd_modulestream',
    'tests/test-modulemd-modulestream.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_modulestream', test_v1_modulemd_modulestream,
     env : test_env)
test('test_v1_release_modulemd_modulestream', test_v1_modulemd_modulestream,
     env : test_release_env)

test_v1_modulemd_regressions = executable(
    'test_v1_modulemd_regressions',
    'tests/test-modulemd-regressions.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_regressions', test_v1_modulemd_regressions,
    env : test_env)
test('test_v1_release_modulemd_regressions', test_v1_modulemd_regressions,
    env : test_release_env)

test_v1_modulemd_servicelevel = executable(
    'test_v1_modulemd_servicelevel',
    'tests/test-modulemd-servicelevel.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_servicelevel', test_v1_modulemd_servicelevel,
     env : test_env)
test('test_v1_release_modulemd_servicelevel', test_v1_modulemd_servicelevel,
     env : test_release_env)

test_v1_modulemd_simpleset = executable(
    'test_v1_modulemd_simpleset',
    'tests/test-modulemd-simpleset.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_simpleset', test_v1_modulemd_simpleset,
     env : test_env)
test('test_v1_release_modulemd_simpleset', test_v1_modulemd_simpleset,
     env : test_release_env)

test_v1_modulemd_subdocument = executable(
    'test_v1_modulemd_subdocument',
    'tests/test-modulemd-subdocument.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_subdocument', test_v1_modulemd_subdocument,
     env : test_env)
test('test_v1_release_modulemd_subdocument', test_v1_modulemd_subdocument,
     env : test_release_env)


test_v1_modulemd_translation = executable(
    'test_v1_modulemd_translation',
    'tests/test-modulemd-translation.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_translation',
     test_v1_modulemd_translation,
     env : test_env)
test('test_v1_release_modulemd_translation',
     test_v1_modulemd_translation,
     env : test_release_env)


test_v1_modulemd_translation_entry = executable(
    'test_v1_modulemd_translation_entry',
    'tests/test-modulemd-translation-entry.c',
    dependencies : [
        modulemd_v1_dep,
    ],
    install : false,
)
test('test_v1_modulemd_translation_entry',
     test_v1_modulemd_translation_entry,
     env : test_env)
test('test_v1_release_modulemd_translation_entry',
     test_v1_modulemd_translation_entry,
     env : test_release_env)


test_v1_modulemd_yaml = executable(
    'test_v1_modulemd_yaml',
    'tests/test-modulemd-yaml.c',
    dependencies : [
        modulemd_v1_dep,
        yaml
    ],
    install : false,
)
test('test_v1_modulemd_yaml', test_v1_modulemd_yaml,
     env : test_env)
test('test_v1_release_modulemd_yaml', test_v1_modulemd_yaml,
     env : test_release_env)

modulemd_python_scripts = files('tests/test-modulemd-python.py')
test ('test_v1_modulemd_python', python3,
      env : test_env,
      args : modulemd_python_scripts)
test ('test_v1_release_modulemd_python', python3,
      env : test_release_env,
      args : modulemd_python_scripts)

import_header_script = find_program('tests/test-import-headers.sh')
test ('test_v1_import_headers', import_header_script,
      env : test_env,
      args : modulemd_v1_hdrs)

if valgrind.found()
    modulemd_valgrind_scripts = files('tests/test-valgrind.py')
    test ('test_v1_valgrind', python3,
          env : test_env,
          args : modulemd_valgrind_scripts,
          timeout : 300)
endif

gnome.generate_gir(
    modulemd_v1_lib,
    sources : modulemd_v1_srcs + modulemd_v1_hdrs,
    nsversion : '.'.join([libmodulemd_version_array[0], '0']),
    namespace : 'Modulemd',
    symbol_prefix : 'modulemd_',
    identifier_prefix : 'Modulemd',
    includes : [
        'GObject-2.0',
    ],
    install : true,
    )

xcdata = configuration_data()
xcdata.set('VERSION', libmodulemd_version)
configure_file(
  input : 'version.xml.in',
  output : 'version.xml',
  configuration : xcdata
)

cdata = configuration_data()
cdata.set_quoted('LIBMODULEMD_VERSION', libmodulemd_version)
configure_file(
  output : 'config.h',
  configuration : cdata
)

gnome.gtkdoc(
    'modulemd-1.0',
    install_dir: 'modulemd-1.0',
    src_dir : './',
    main_xml : 'modulemd-docs.xml',
    install : true)

pkg.generate(
    libraries : modulemd_v1_lib,
    subdirs : v1_header_path,
    version : libmodulemd_version,
    name : 'modulemd',
    filebase : 'modulemd',
    description : 'Module metadata manipulation library',
    requires: [ 'glib-2.0', 'gobject-2.0' ],
)
