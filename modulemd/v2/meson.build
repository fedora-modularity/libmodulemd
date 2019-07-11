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

# -- Library -- #

test_installed_lib = get_option('test_installed_lib')
skip_introspection = get_option('skip_introspection')

if test_installed_lib
    # If we're just running the tests on a library that's already installed,
    # don't bother building it again.
    build_lib = disabler()
else
    build_lib = declare_dependency()
endif

enums = gnome.mkenums_simple ('modulemd-enums', sources : modulemd_v2_hdrs)

cdata = configuration_data()
cdata.set_quoted('LIBMODULEMD_VERSION', libmodulemd_v2_version)
configure_file(
  output : 'config.h',
  configuration : cdata
)

modulemd_v2_lib = library(
    'modulemd',
    sources : modulemd_v2_srcs + enums,
    include_directories : v2_include_dirs,
    dependencies : [
        gobject,
        yaml,
        build_lib,
    ],
    install : true,
    version: libmodulemd_v2_version,
)

if test_installed_lib
    # Run tests against an installed library instead of in-tree
    modulemd_v2_dep = declare_dependency(
        include_directories : v2_include_dirs,
        dependencies : [
            gobject,
            yaml,
            dependency(
                'modulemd-2.0',
            ),
        ]
    )
else
    modulemd_v2_dep = declare_dependency(
        include_directories : v2_include_dirs,
        link_with : modulemd_v2_lib,
        dependencies : [
            gobject,
            yaml,
        ]
    )
endif

modulemd_v2_validator = executable(
    'modulemd-validator',
    sources : modulemd_v2_validator_srcs,
    include_directories : v2_include_dirs,
    dependencies : [
        gobject,
        yaml,
        modulemd_v2_dep
    ],
    install : true
)

v2_header_path = 'modulemd-2.0'

install_headers(
  modulemd_v2_hdrs,
  subdir : v2_header_path,
)


# --- TESTS --- #

# Test env with release values
test_release_env = environment()
test_release_env.set('LC_ALL', 'C')
test_release_env.set ('MESON_SOURCE_ROOT', meson.source_root())

# Test env with fatal warnings and criticals
test_env = test_release_env
test_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')
test_env.set('G_MESSAGES_DEBUG', 'all')

# Python test env with fatal warnings and criticals
py_test_env = test_env

if not test_installed_lib
    # If we're testing an installed version, we want to use the default
    # locations for these paths.
    py_test_env.set ('GI_TYPELIB_PATH', meson.build_root() + '/modulemd/v2')
    py_test_env.set ('LD_LIBRARY_PATH', meson.build_root() + '/modulemd/v2')
    py_test_env.set ('PYTHONPATH', meson.source_root())

    # This test is just to catch whether we are accidentally not testing
    # the built version.
    py_test_env.set ('MODULEMD_VERSION', libmodulemd_v2_version)
else
    # Add an env var to exercise the Python overrides, which can only be done
    # against installed libs.
    py_test_env.set ('MMD_TEST_INSTALLED_LIBS', 'TRUE')
endif

# Python test env with release values
py_test_release_env = py_test_env
py_test_release_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')


# --- Test utility library --- #
test_utils_lib = library(
    'modulemd_tests',
    sources : [
        'tests/test-utils.c',
    ],
    include_directories : v2_include_dirs,
    dependencies : [
        gobject,
        yaml,
    ],
    install : false,
)

v2_c_tests = {
'buildopts_v2'           : [ 'tests/test-modulemd-buildopts.c' ],
'component_module_v2'    : [ 'tests/test-modulemd-component-module.c' ],
'component_rpm_v2'       : [ 'tests/test-modulemd-component-rpm.c' ],
'defaults_v2'            : [ 'tests/test-modulemd-defaults.c' ],
'defaultsv1_v2'          : [ 'tests/test-modulemd-defaults-v1.c' ],
'dependencies_v2'        : [ 'tests/test-modulemd-dependencies.c' ],
'module_v2'              : [ 'tests/test-modulemd-module.c' ],
'module_index_v2'        : [ 'tests/test-modulemd-moduleindex.c' ],
'module_index_merger_v2' : [ 'tests/test-modulemd-merger.c' ],
'modulestream_v2'        : [ 'tests/test-modulemd-modulestream.c' ],
'profile_v2'             : [ 'tests/test-modulemd-profile.c' ],
'rpm_map'                : [ 'tests/test-modulemd-rpmmap.c' ],
'service_level_v2'       : [ 'tests/test-modulemd-service-level.c' ],
'translation_v2'         : [ 'tests/test-modulemd-translation.c' ],
'translation_entry_v2'   : [ 'tests/test-modulemd-translation-entry.c' ],
}

foreach name, sources : v2_c_tests
    exe = executable(
        name,
        sources,
        dependencies : [
            modulemd_v2_dep,
        ],
        link_with : [
            test_utils_lib,
        ],
        install : false,
    )
    test(name + '_debug', exe,
         env : test_env)
    test(name + '_release', exe,
         env : test_release_env)

    valgrind_tests += name + '_debug'
endforeach


v2_python_tests = {
'buildopts'        : 'tests/ModulemdTests/buildopts.py',
'componentrpm'     : 'tests/ModulemdTests/componentrpm.py',
'defaults'         : 'tests/ModulemdTests/defaults.py',
'defaultsv1'       : 'tests/ModulemdTests/defaultsv1.py',
'dependencies'     : 'tests/ModulemdTests/dependencies.py',
'merger'           : 'tests/ModulemdTests/merger.py',
'module'           : 'tests/ModulemdTests/module.py',
'moduleindex'      : 'tests/ModulemdTests/moduleindex.py',
'modulestream'     : 'tests/ModulemdTests/modulestream.py',
'profile'          : 'tests/ModulemdTests/profile.py',
'rpmmap'           : 'tests/ModulemdTests/rpmmap.py',
'servicelevel'     : 'tests/ModulemdTests/servicelevel.py',
'translation'      : 'tests/ModulemdTests/translation.py',
'translationentry' : 'tests/ModulemdTests/translationentry.py',
}

foreach name, script : v2_python_tests
    test (name + '_python3_debug', python3,
          env : py_test_env,
          args : files(script))
    test (name + '_python3_release', python3,
          env : py_test_release_env,
          args : files(script))

    test (name + '_python2_debug', python2,
          env : py_test_env,
          args : files(script))
    test (name + '_python2_release', python2,
          env : py_test_release_env,
          args : files(script))
endforeach


# -- Test Modulemd.TranslationHelpers (Python) -- #

translation_helpers_python_script = files('tests/ModulemdTests/translation_helpers.py')
get_babel = '''
import babel
'''

# Python 2
if (get_option('with_py2_overrides') and not test_installed_lib)
  ret2 = run_command([python2, '-c', get_babel])

  if ret2.returncode() != 0
    warning('No Babel on Python 2: skipping TranslationHelper tests')
  else
    test ('translation_helpers_python2_debug', python2,
          env : py_test_env,
          args : translation_helpers_python_script)
    test ('translation_helpers_python2_release', python2,
          env : py_test_release_env,
          args : translation_helpers_python_script)
  endif
endif

# Python 3
if (get_option('with_py3_overrides') and not test_installed_lib)
  ret2 = run_command([python3, '-c', get_babel])

  if ret2.returncode() != 0
    warning('No Babel on Python 3: skipping TranslationHelper tests')
  else
    test ('translation_helpers_python3_debug', python3,
          env : py_test_env,
          args : translation_helpers_python_script)
    test ('translation_helpers_python3_release', python3,
          env : py_test_release_env,
          args : translation_helpers_python_script)
  endif
endif


# --- GOBject Introspection -- #

if skip_introspection
else
    gnome.generate_gir(
        modulemd_v2_lib,
        sources : modulemd_v2_srcs + modulemd_v2_hdrs + ['include/private/gi-binding-renames.h'],
        nsversion : '2.0',
        namespace : 'Modulemd',
        symbol_prefix : 'modulemd_',
        identifier_prefix : 'Modulemd',
        includes : [
            'GObject-2.0',
        ],
        install : true,
    )
endif

pkg.generate(
    libraries : modulemd_v2_lib,
    subdirs : v2_header_path,
    version : libmodulemd_v2_version,
    name : 'modulemd-2.0',
    filebase : 'modulemd-2.0',
    description : 'Module metadata manipulation library',
    requires: [ 'glib-2.0', 'gobject-2.0' ],
)

xcdata = configuration_data()
xcdata.set('VERSION', meson.project_version())
configure_file(
  input : 'version.xml.in',
  output : 'version.xml',
  configuration : xcdata
)

gnome.gtkdoc(
    'modulemd-2.0',
    install_dir: 'modulemd-2.0',
    src_dir : './modulemd/v2',
    main_xml : 'modulemd-v2-docs.xml',
    gobject_typesfile : join_paths(meson.current_build_dir(), 'modulemd-2.0.types'),
    dependencies : [
        modulemd_v2_dep,
    ],
    install : true,
)

