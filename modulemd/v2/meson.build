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

# -- Test Modulemd.Buildopts (C) -- #
test_v2_buildopts = executable(
    'buildopts_v2',
    'tests/test-modulemd-buildopts.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('buildopts_v2_debug', test_v2_buildopts,
     env : test_env)
test('buildopts_v2_release', test_v2_buildopts,
     env : test_release_env)

# -- Test Modulemd.ComponentModule (C) -- #
test_v2_component_module = executable(
    'component_module_v2',
    'tests/test-modulemd-component-module.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('component_module_v2_debug', test_v2_component_module,
     env : test_env)
test('component_module_v2_release', test_v2_component_module,
     env : test_release_env)

# -- Test Modulemd.ComponentRpm (Python) -- #
componentrpm_python_script = files('tests/ModulemdTests/componentrpm.py')
test ('componentrpm_python_debug', python3,
      env : py_test_env,
      args : componentrpm_python_script)
test ('componentrpm_python_release', python3,
      env : py_test_release_env,
      args : componentrpm_python_script)

# -- Test Modulemd.ComponentRpm (C) -- #
test_v2_component_rpm = executable(
    'component_rpm_v2',
    'tests/test-modulemd-component-rpm.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('component_rpm_v2_debug', test_v2_component_rpm,
     env : test_env)
test('component_rpm_v2_release', test_v2_component_rpm,
     env : test_release_env)

# -- Test Modulemd.Dependencies (C) -- #
test_v2_dependencies = executable(
    'dependencies_v2',
    'tests/test-modulemd-dependencies.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('dependencies_v2_debug', test_v2_dependencies,
     env : test_env)
test('dependencies_v2_release', test_v2_dependencies,
     env : test_release_env)

# -- Test Modulemd.Module (C) -- #
test_v2_module = executable(
    'module_v2',
    'tests/test-modulemd-module.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('module_v2_debug', test_v2_module,
     env : test_env)
test('module_v2_release', test_v2_module,
     env : test_release_env)

# -- Test Modulemd.ModuleIndex (C) -- #
test_v2_module_index = executable(
    'module_index_v2',
    'tests/test-modulemd-moduleindex.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('module_index_v2_debug', test_v2_module_index,
     env : test_env)
test('module_index_v2_release', test_v2_module_index,
     env : test_release_env)


# -- Test Modulemd.ModuleIndexMerger (C) -- #
test_v2_module_index_merger = executable(
    'module_index_merger_v2',
    'tests/test-modulemd-merger.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('module_index_merger_v2_debug', test_v2_module_index_merger,
     env : test_env)
test('module_index_merger_v2_release', test_v2_module_index_merger,
     env : test_release_env)


# -- Test Modulemd.Modulestream (C) -- #
test_v2_modulestream = executable(
    'modulestream_v2',
    'tests/test-modulemd-modulestream.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('modulestream_v2_debug', test_v2_modulestream,
     env : test_env)
test('modulestream_v2_release', test_v2_modulestream,
     env : test_release_env)

# -- Test Modulemd.Profile (C) -- #
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

# -- Test Modulemd.Translation (C) -- #
test_v2_translation = executable(
    'translation_v2',
    'tests/test-modulemd-translation.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('translation_v2_debug', test_v2_translation,
     env : test_env)
test('translation_v2_release', test_v2_translation,
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


# -- Test Modulemd.Buildopts (Python) -- #
buildopts_python_script = files('tests/ModulemdTests/buildopts.py')
test ('buildopts_python3_debug', python3,
      env : py_test_env,
      args : buildopts_python_script)
test ('buildopts_python3_release', python3,
      env : py_test_release_env,
      args : buildopts_python_script)

  test ('buildopts_python2_debug', python2,
      env : py_test_env,
      args : buildopts_python_script)
test ('buildopts_python2_release', python2,
      env : py_test_release_env,
      args : buildopts_python_script)


# -- Test Modulemd.Dependencies (Python) -- #
dependencies_python_script = files('tests/ModulemdTests/dependencies.py')
test ('dependencies_python3_debug', python3,
      env : py_test_env,
      args : dependencies_python_script)
test ('dependencies_python3_release', python3,
      env : py_test_release_env,
      args : dependencies_python_script)

test ('dependencies_python2_debug', python2,
      env : py_test_env,
      args : dependencies_python_script)
test ('dependencies_python2_release', python2,
      env : py_test_release_env,
      args : dependencies_python_script)

# -- Test Modulemd.ModuleIndex (Python) -- #
moduleindex_python_script = files('tests/ModulemdTests/moduleindex.py')
test ('moduleindex_python3_debug', python3,
      env : py_test_env,
      args : moduleindex_python_script)
test ('moduleindex_python3_release', python3,
      env : py_test_release_env,
      args : moduleindex_python_script)

moduleindex_python_script = files('tests/ModulemdTests/moduleindex.py')
test ('moduleindex_python2_debug', python2,
      env : py_test_env,
      args : moduleindex_python_script)
test ('moduleindex_python2_release', python2,
      env : py_test_release_env,
      args : moduleindex_python_script)

# -- Test Modulemd.ModuleStream (Python) -- #
modulestream_python_script = files('tests/ModulemdTests/modulestream.py')
test ('modulestream_python3_debug', python3,
      env : py_test_env,
      args : modulestream_python_script)
test ('modulestream_python3_release', python3,
      env : py_test_release_env,
      args : modulestream_python_script)

test ('modulestream_python2_debug', python2,
      env : py_test_env,
      args : modulestream_python_script)
test ('modulestream_python2_release', python2,
      env : py_test_release_env,
      args : modulestream_python_script)


# -- Test Modulemd.ModuleIndexMerger (Python) -- #
merger_python_script = files('tests/ModulemdTests/merger.py')
test ('merger_python3_debug', python3,
      env : py_test_env,
      args : merger_python_script)
test ('merger_python3_release', python3,
      env : py_test_release_env,
      args : merger_python_script)

test ('merger_python2_debug', python2,
      env : py_test_env,
      args : merger_python_script)
test ('merger_python2_release', python2,
      env : py_test_release_env,
      args : merger_python_script)


# -- Test Modulemd.Profile (Python) -- #
profile_python_script = files('tests/ModulemdTests/profile.py')
test ('profile_python3_debug', python3,
      env : py_test_env,
      args : profile_python_script)
test ('profile_python3_release', python3,
      env : py_test_release_env,
      args : profile_python_script)

test ('profile_python2_debug', python2,
      env : py_test_env,
      args : profile_python_script)
test ('profile_python2_release', python2,
      env : py_test_release_env,
      args : profile_python_script)

# -- Test Modulemd.ServiceLevel (Python) -- #
service_level_python_script = files('tests/ModulemdTests/servicelevel.py')
test ('service_level_python3_debug', python3,
      env : py_test_env,
      args : service_level_python_script)
test ('service_level_python3_release', python3,
      env : py_test_release_env,
      args : service_level_python_script)

test ('service_level_python2_debug', python2,
      env : py_test_env,
      args : service_level_python_script)
test ('service_level_python2_release', python2,
      env : py_test_release_env,
      args : service_level_python_script)


# -- Test Modulemd.Translation (Python) -- #
translation_python_script = files('tests/ModulemdTests/translation.py')
test ('translation_python3_debug', python3,
      env : py_test_env,
      args : translation_python_script)
test ('translation_python3_release', python3,
      env : py_test_release_env,
      args : translation_python_script)

test ('translation_python2_debug', python2,
      env : py_test_env,
      args : translation_python_script)
test ('translation_python2_release', python2,
      env : py_test_release_env,
      args : translation_python_script)

# -- Test Modulemd.TranslationEntry (Python) -- #
translation_entry_python_script = files('tests/ModulemdTests/translationentry.py')
test ('translation_entry_python3_debug', python3,
      env : py_test_env,
      args : translation_entry_python_script)
test ('translation_entry_python3_release', python3,
      env : py_test_release_env,
      args : translation_entry_python_script)

test ('translation_entry_python2_debug', python2,
      env : py_test_env,
      args : translation_entry_python_script)
test ('translation_entry_python2_release', python2,
      env : py_test_release_env,
      args : translation_entry_python_script)


# -- Test Modulemd.Defaults (C) -- #
test_v2_defaults = executable(
    'defaults_v2',
    'tests/test-modulemd-defaults.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('defaults_v2_debug', test_v2_defaults,
     env : test_env)
test('defaults_v2_release', test_v2_defaults,
     env : test_release_env)

# -- Test Modulemd.Defaults (Python) -- #
defaults_python_script = files('tests/ModulemdTests/defaults.py')
test ('defaults_python3_debug', python3,
      env : py_test_env,
      args : defaults_python_script)
test ('defaults_python3_release', python3,
      env : py_test_release_env,
      args : defaults_python_script)

defaults_python_script = files('tests/ModulemdTests/defaults.py')
test ('defaults_python2_debug', python2,
      env : py_test_env,
      args : defaults_python_script)
test ('defaults_python2_release', python2,
      env : py_test_release_env,
      args : defaults_python_script)


# -- Test Modulemd.DefaultsV1 (C) -- #
test_v2_defaultsv1 = executable(
    'defaultsv1_v2',
    'tests/test-modulemd-defaults-v1.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test('defaultsv1_v2_debug', test_v2_defaultsv1,
     env : test_env)
test('defaultsv1_v2_release', test_v2_defaultsv1,
     env : test_release_env)

# -- Test Modulemd.DefaultsV1 (Python) -- #
defaultsv1_python_script = files('tests/ModulemdTests/defaultsv1.py')
test ('defaultsv1_python3_debug', python3,
      env : py_test_env,
      args : defaultsv1_python_script)
test ('defaultsv1_python3_release', python3,
      env : py_test_release_env,
      args : defaultsv1_python_script)

test ('defaultsv1_python2_debug', python2,
      env : py_test_env,
      args : defaultsv1_python_script)
test ('defaultsv1_python2_release', python2,
      env : py_test_release_env,
      args : defaultsv1_python_script)


# -- Test Modulemd.RpmMapEntry (C) -- #
test_rpm_map = executable(
    'rpm_map',
    'tests/test-modulemd-rpmmap.c',
    dependencies : [
        modulemd_v2_dep,
    ],
    link_with : [
        test_utils_lib,
    ],
    install : false,
)
test ('rpm_map_debug', test_rpm_map,
      env : test_env)
test ('rpm_map_release', test_rpm_map,
      env : test_release_env)

# -- Test Modulemd.RpmMapEntry (Python) -- #
rpmmap_python_script = files('tests/ModulemdTests/rpmmap.py')
test ('rpmmap_python3_debug', python3,
      env : py_test_env,
      args : rpmmap_python_script)
test ('rpmmap_python3_release', python3,
      env : py_test_release_env,
      args : rpmmap_python_script)

test ('rpmmap_python2_debug', python2,
      env : py_test_env,
      args : rpmmap_python_script)
test ('rpmmap_python2_release', python2,
      env : py_test_release_env,
      args : rpmmap_python_script)


# --- GOBject Introspection -- #

if skip_introspection
else
    gnome.generate_gir(
        modulemd_v2_lib,
        sources : modulemd_v2_srcs + modulemd_v2_hdrs + ['include/modulemd-2.0/private/gi-binding-renames.h'],
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
    install : true,
)

