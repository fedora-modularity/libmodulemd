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

if test_installed_lib
    # If we're just running the tests on a library that's already installed,
    # don't bother building it again.
    build_lib = disabler()
else
    build_lib = declare_dependency()
endif

modulemd_v2_lib = library(
    'modulemd',
    sources : modulemd_v2_srcs,
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

v2_header_path = 'modulemd-2.0'

install_headers(
  modulemd_v2_hdrs,
  subdir : v2_header_path,
)


# --- TESTS --- #

# Test env with release values
test_release_env = environment()
test_release_env.set('G_MESSAGES_DEBUG', 'all')
test_release_env.set ('MESON_SOURCE_ROOT', meson.source_root())

# Test env with fatal warnings and criticals
test_env = test_release_env
test_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')

# Python test env with fatal warnings and criticals
py_test_env = test_env

if not test_installed_lib
    # If we're testing an installed version, we want to use the default
    # locations for these paths.
    py_test_env.set ('GI_TYPELIB_PATH', meson.build_root() + '/modulemd/v2')
    py_test_env.set ('LD_LIBRARY_PATH', meson.build_root() + '/modulemd/v2')

    # This test is just to catch whether we are accidentally not testing
    # the built version.
    py_test_env.set ('MODULEMD_VERSION', libmodulemd_v2_version)
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

# -- Test Modulemd.TranslationEntry (C) -- #
test_v2_profile = executable(
    'profile_v2',
    'tests/test-modulemd-profile.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('profile_v2_debug', test_v2_profile,
     env : test_env)
test('profile_v2_release', test_v2_profile,
     env : test_release_env)

# -- Test Modulemd.ServiceLevel (C) -- #
test_v2_servicelevel = executable(
    'service_level_v2',
    'tests/test-modulemd-service-level.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('service_level_v2_debug', test_v2_servicelevel,
     env : test_env)
test('service_level_v2_release', test_v2_servicelevel,
     env : test_release_env)

# -- Test Modulemd.TranslationEntry (C) -- #
test_v2_translationentry = executable(
    'translation_entry_v2',
    'tests/test-modulemd-translation-entry.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('translation_entry_v2_debug', test_v2_translationentry,
     env : test_env)
test('translation_entry_v2_release', test_v2_translationentry,
     env : test_release_env)


# -- Test Modulemd.Profile (Python) -- #
profile_python_script = files('tests/ModulemdTests/profile.py')
test ('profile_python_debug', python3,
      env : py_test_env,
      args : profile_python_script)
test ('profile_python_release', python3,
      env : py_test_release_env,
      args : profile_python_script)


# -- Test Modulemd.ServiceLevel (Python) -- #
service_level_python_script = files('tests/ModulemdTests/servicelevel.py')
test ('service_level_python_debug', python3,
      env : py_test_env,
      args : service_level_python_script)
test ('service_level_python_release', python3,
      env : py_test_release_env,
      args : service_level_python_script)


# -- Test Modulemd.TranslationEntry (Python) -- #
translation_entry_python_script = files('tests/ModulemdTests/translationentry.py')
test ('translation_entry_python_debug', python3,
      env : py_test_env,
      args : translation_entry_python_script)
test ('translation_entry_python_release', python3,
      env : py_test_release_env,
      args : translation_entry_python_script)


# --- GOBject Introspection -- #

gnome.generate_gir(
    modulemd_v2_lib,
    sources : modulemd_v2_srcs + modulemd_v2_hdrs,
    nsversion : '2.0',
    namespace : 'Modulemd',
    symbol_prefix : 'modulemd_',
    identifier_prefix : 'Modulemd',
    includes : [
        'GObject-2.0',
    ],
    install : true,
)

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
    install : true,
)

