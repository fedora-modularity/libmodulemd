# This file is part of libmodulemd
# Copyright (C) 2017-2020 Stephen Gallagher
#
# Fedora-License-Identifier: MIT
# SPDX-2.0-License-Identifier: MIT
# SPDX-3.0-License-Identifier: MIT
#
# This program is free software.
# For more information on the license, see COPYING.
# For more information on free software, see <https://www.gnu.org/philosophy/free-sw.en.html>.

# -- Configuration Options -- #

test_installed_lib = get_option('test_installed_lib')
skip_introspection = get_option('skip_introspection')
verbose_tests = get_option('verbose_tests')
accept_overflowed_buildorder = get_option('accept_overflowed_buildorder')
clang_simple_version_script = find_program ('clang_simple_version.sh')


# -- Library -- #

if test_installed_lib
    # If we're just running the tests on a library that's already installed,
    # don't bother building it again.
    build_lib = disabler()
else
    build_lib = declare_dependency()
endif


modulemd_srcs = files(
    'modulemd.c',
    'modulemd-build-config.c',
    'modulemd-buildopts.c',
    'modulemd-component.c',
    'modulemd-component-module.c',
    'modulemd-component-rpm.c',
    'modulemd-compression.c',
    'modulemd-defaults.c',
    'modulemd-defaults-v1.c',
    'modulemd-dependencies.c',
    'modulemd-module.c',
    'modulemd-module-index.c',
    'modulemd-module-index-merger.c',
    'modulemd-module-stream.c',
    'modulemd-module-stream-v1.c',
    'modulemd-module-stream-v2.c',
    'modulemd-packager-v3.c',
    'modulemd-profile.c',
    'modulemd-rpm-map-entry.c',
    'modulemd-service-level.c',
    'modulemd-subdocument-info.c',
    'modulemd-translation.c',
    'modulemd-translation-entry.c',
    'modulemd-util.c',
    'modulemd-yaml-util.c',
    'modulemd-obsoletes.c',
)

modulemd_validator_srcs = files (
    'modulemd-validator.c',
)

modulemd_hdrs = files(
    'include/modulemd-2.0/modulemd.h',
    'include/modulemd-2.0/modulemd-build-config.h',
    'include/modulemd-2.0/modulemd-buildopts.h',
    'include/modulemd-2.0/modulemd-component.h',
    'include/modulemd-2.0/modulemd-component-module.h',
    'include/modulemd-2.0/modulemd-component-rpm.h',
    'include/modulemd-2.0/modulemd-compression.h',
    'include/modulemd-2.0/modulemd-defaults.h',
    'include/modulemd-2.0/modulemd-defaults-v1.h',
    'include/modulemd-2.0/modulemd-dependencies.h',
    'include/modulemd-2.0/modulemd-deprecated.h',
    'include/modulemd-2.0/modulemd-errors.h',
    'include/modulemd-2.0/modulemd-module.h',
    'include/modulemd-2.0/modulemd-module-index.h',
    'include/modulemd-2.0/modulemd-module-index-merger.h',
    'include/modulemd-2.0/modulemd-module-stream.h',
    'include/modulemd-2.0/modulemd-module-stream-v1.h',
    'include/modulemd-2.0/modulemd-module-stream-v2.h',
    'include/modulemd-2.0/modulemd-packager-v3.h',
    'include/modulemd-2.0/modulemd-profile.h',
    'include/modulemd-2.0/modulemd-rpm-map-entry.h',
    'include/modulemd-2.0/modulemd-service-level.h',
    'include/modulemd-2.0/modulemd-subdocument-info.h',
    'include/modulemd-2.0/modulemd-translation.h',
    'include/modulemd-2.0/modulemd-translation-entry.h',
    'include/modulemd-2.0/modulemd-obsoletes.h',
)

modulemd_priv_hdrs = files(
    'include/private/glib-extensions.h',
    'include/private/modulemd-build-config-private.h',
    'include/private/modulemd-buildopts-private.h',
    'include/private/modulemd-component-private.h',
    'include/private/modulemd-component-module-private.h',
    'include/private/modulemd-component-rpm-private.h',
    'include/private/modulemd-compression-private.h',
    'include/private/modulemd-dependencies-private.h',
    'include/private/modulemd-profile-private.h',
    'include/private/modulemd-defaults-private.h',
    'include/private/modulemd-defaults-v1-private.h',
    'include/private/modulemd-module-private.h',
    'include/private/modulemd-module-index-private.h',
    'include/private/modulemd-module-stream-private.h',
    'include/private/modulemd-module-stream-v1-private.h',
    'include/private/modulemd-module-stream-v2-private.h',
    'include/private/modulemd-packager-v3-private.h',
    'include/private/modulemd-service-level-private.h',
    'include/private/modulemd-subdocument-info-private.h',
    'include/private/modulemd-translation-private.h',
    'include/private/modulemd-translation-entry-private.h',
    'include/private/modulemd-util.h',
    'include/private/modulemd-yaml.h',
    'include/private/gi-binding-renames.h',
    'include/private/modulemd-obsoletes-private.h',
)

test_srcs = files(
    'tests/test-utils.c',
)

test_priv_hdrs = files(
    'include/private/test-utils.h',
)

test_python_scripts = files(
    'tests/ModulemdTests/base.py',
)

include_dirs = include_directories (
    'include/modulemd-2.0',
    'include'
)


enums = gnome.mkenums_simple ('modulemd-enums', sources : modulemd_hdrs)

cdata = configuration_data()
cdata.set_quoted('LIBMODULEMD_VERSION', libmodulemd_version)
cdata.set('HAVE_RPMIO', rpm.found())
cdata.set('HAVE_GDATE_AUTOPTR', has_gdate_autoptr)
cdata.set('HAVE_EXTEND_AND_STEAL', has_extend_and_steal)
cdata.set('HAVE_G_SPAWN_CHECK_WAIT_STATUS', has_g_spawn_check_wait_status)
cdata.set('HAVE_OVERFLOWED_BUILDORDER', accept_overflowed_buildorder)
configure_file(
  output : 'config.h',
  configuration : cdata
)

modulemd_lib = library(
    'modulemd',
    sources : modulemd_srcs + enums,
    include_directories : include_dirs,
    dependencies : [
        gobject,
        rpm,
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
            yaml,
            dependency(
                'modulemd-2.0',
            ),
        ]
    )
else
    modulemd_dep = declare_dependency(
        include_directories : include_dirs,
        link_with : modulemd_lib,
        dependencies : [
            gobject,
            yaml,
        ]
    )
endif

if test_installed_lib
    # Run tests against an installed tool of in-tree
    modulemd_validator = find_program('modulemd-validator').full_path()
else
    modulemd_validator = executable(
        'modulemd-validator',
        sources : modulemd_validator_srcs,
        include_directories : include_dirs,
        dependencies : [
            gobject,
            rpm,
            yaml,
            modulemd_dep
        ],
        install : true
    )
endif

header_path = 'modulemd-2.0'

install_headers(
  modulemd_hdrs,
  subdir : header_path,
)


# --- GOBject Introspection -- #

if skip_introspection
else
girs = gnome.generate_gir(
        modulemd_lib,
        sources : modulemd_srcs + modulemd_hdrs + ['include/private/gi-binding-renames.h'],
        nsversion : '2.0',
        namespace : 'Modulemd',
        symbol_prefix : 'modulemd_',
        identifier_prefix : 'Modulemd',
        includes : [
            'GObject-2.0',
        ],
        extra_args : [ '--accept-unprefixed' ],
        install : true,
        fatal_warnings : true,
    )
endif

pkg.generate(
    libraries : modulemd_lib,
    subdirs : header_path,
    version : libmodulemd_version,
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


# --- Documenatation --- #

if with_docs
    fixxref_args = []
    foreach path : gtk_doc_referred_paths
      fixxref_args += [ '--extra-dir=@0@'.format(path) ]
    endforeach
    gnome.gtkdoc(
        'modulemd-2.0',
        install_dir: 'modulemd-2.0',
        src_dir : './modulemd',
        main_xml : 'modulemd-docs.xml',
        gobject_typesfile : join_paths(meson.current_build_dir(), 'modulemd-2.0.types'),
        dependencies : [
            modulemd_dep,
        ],
        fixxref_args: [ fixxref_args ],
        install : true,
    )
endif

if with_manpages
    install_man('modulemd-validator.1')
endif


# --- TESTS --- #

# Test env with release values
test_release_env = environment()
test_release_env.set('LC_ALL', 'C')

if meson.version().version_compare('>=0.56.0')
    project_source_root = meson.project_source_root()
    project_build_root = meson.project_build_root()
else
    project_source_root = meson.source_root()
    project_build_root = meson.build_root()
endif

test_release_env.set ('MESON_SOURCE_ROOT', project_source_root)
test_release_env.set ('MESON_BUILD_ROOT', project_build_root)
test_release_env.set ('TEST_DATA_PATH', project_source_root + '/modulemd/tests/test_data')

# Test env with fatal warnings and criticals
test_env = test_release_env
test_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')

if verbose_tests
    test_env.set('G_MESSAGES_DEBUG', 'all')
endif

# Python test env with fatal warnings and criticals
py_test_env = test_env

if not test_installed_lib
    # If we're testing an installed version, we want to use the default
    # locations for these paths.
    py_test_env.set ('GI_TYPELIB_PATH', project_build_root + '/modulemd')
    py_test_env.set ('LD_LIBRARY_PATH', project_build_root + '/modulemd')
    py_test_env.set ('PYTHONPATH', project_source_root)

    # This test is just to catch whether we are accidentally not testing
    # the built version.
    py_test_env.set ('MODULEMD_VERSION', libmodulemd_version)
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
    include_directories : include_dirs,
    dependencies : [
        gobject,
        yaml,
    ],
    install : false,
)

c_tests = {
'buildconfig'         : [ 'tests/test-modulemd-build-config.c' ],
'buildopts'           : [ 'tests/test-modulemd-buildopts.c' ],
'common'              : [ 'tests/test-modulemd-common.c' ],
'component_module'    : [ 'tests/test-modulemd-component-module.c' ],
'component_rpm'       : [ 'tests/test-modulemd-component-rpm.c' ],
'compression'         : [ 'tests/test-modulemd-compression.c' ],
'context'             : [ 'tests/test-modulemd-context.c' ],
'defaults'            : [ 'tests/test-modulemd-defaults.c' ],
'defaultsv1'          : [ 'tests/test-modulemd-defaults-v1.c' ],
'dependencies'        : [ 'tests/test-modulemd-dependencies.c' ],
'module'              : [ 'tests/test-modulemd-module.c' ],
'module_index'        : [ 'tests/test-modulemd-moduleindex.c' ],
'module_index_merger' : [ 'tests/test-modulemd-merger.c' ],
'modulestream'        : [ 'tests/test-modulemd-modulestream.c' ],
'packagerv3'          : [ 'tests/test-modulemd-packager-v3.c' ],
'parse_int64'         : [ 'tests/test-modulemd-parse_int64.c' ],
'profile'             : [ 'tests/test-modulemd-profile.c' ],
'rpm_map'             : [ 'tests/test-modulemd-rpmmap.c' ],
'service_level'       : [ 'tests/test-modulemd-service-level.c' ],
'translation'         : [ 'tests/test-modulemd-translation.c' ],
'translation_entry'   : [ 'tests/test-modulemd-translation-entry.c' ],
'variant_deep_copy'   : [ 'tests/test-modulemd-variant_deep_copy.c' ],
'obsoletes'           : [ 'tests/test-modulemd-obsoletes.c' ],
'quoting'             : [ 'tests/test-modulemd-quoting.c' ],
}

foreach name, sources : c_tests
    test_srcs += files(sources)
    exe = executable(
        name,
        sources,
        dependencies : [
            modulemd_dep,
            rpm,
        ],
        link_with : [
            test_utils_lib,
        ],
        install : false,
    )
    test(name + '_debug', exe,
         env : test_env,
         timeout : 120,
         suite : ['ci', 'ci_valgrind', 'c', 'c_debug'])
    test(name + '_release', exe,
         env : test_release_env,
         timeout : 60,
         suite : ['ci', 'c', 'c_release'])
endforeach


python_tests = {
'buildopts'        : 'tests/ModulemdTests/buildopts.py',
'common'           : 'tests/ModulemdTests/common.py',
'componentrpm'     : 'tests/ModulemdTests/componentrpm.py',
'defaults'         : 'tests/ModulemdTests/defaults.py',
'defaultsv1'       : 'tests/ModulemdTests/defaultsv1.py',
'dependencies'     : 'tests/ModulemdTests/dependencies.py',
'merger'           : 'tests/ModulemdTests/merger.py',
'module'           : 'tests/ModulemdTests/module.py',
'moduleindex'      : 'tests/ModulemdTests/moduleindex.py',
'modulepackager'   : 'tests/ModulemdTests/modulepackager.py',
'modulestream'     : 'tests/ModulemdTests/modulestream.py',
'obsoletes'        : 'tests/ModulemdTests/obsoletes.py',
'profile'          : 'tests/ModulemdTests/profile.py',
'rpmmap'           : 'tests/ModulemdTests/rpmmap.py',
'servicelevel'     : 'tests/ModulemdTests/servicelevel.py',
'translation'      : 'tests/ModulemdTests/translation.py',
'translationentry' : 'tests/ModulemdTests/translationentry.py',
}

foreach name, script : python_tests
    if with_py3
      test_python_scripts += files(script)
      test (name + '_python3_debug', python3,
            env : py_test_env,
            timeout : 60,
            args : files(script),
            depends : girs,
            suite : ['ci', 'py3', 'py3_debug'])
      test (name + '_python3_release', python3,
            env : py_test_release_env,
            timeout : 60,
            args : files(script),
            depends : girs,
            suite : ['ci', 'py3', 'py3_release'])
    endif

    if with_py2
        test (name + '_python2_debug', python2,
              env : py_test_env,
              timeout : 60,
              args : files(script),
              depends : girs,
              suite : ['ci', 'py2', 'py2_debug'])
        test (name + '_python2_release', python2,
              env : py_test_release_env,
              timeout : 60,
              args : files(script),
              depends : girs,
              suite : ['ci', 'py2', 'py2_release'])
    endif #with_py2
endforeach


# Tests for modulemd-validator tool
modulemd_validator_tests = {
'help'        : [['--code', '0'], ['--help']],
'version'     : [['--code', '0'], ['--version']],
'version_with_uknown_option':
                [['--code', '1', '--stderr', '--does-not-exist'],
                ['--version', '--does-not-exist']],
'valid'       : [['--code', '0'],
                [files('tests/test_data/static_context.yaml')]],
'invalid'     : [['--code', '1', '--stdout', 'No data section provided'],
                [files('tests/test_data/good_and_bad.yaml')]],
'stream_packagerv3_invalid_mix_as_index':
    [['--code', '1'],
    [files('tests/test_data/stream_packager_mix.yaml')]],
'valid_defaultsv1.yaml':
    [['--code', '0'],
    [files('../yaml_specs/modulemd_defaults_v1.yaml')]],
'invalid_defaultsv1.yaml':
    [['--code', '1'],
    [files('tests/test_data/invalid_defaults.yaml')]],
'valid_defaultsv1_as_defaultsv1':
    [['--code', '0'],
    ['--type', 'modulemd-defaults-v1', files('../yaml_specs/modulemd_defaults_v1.yaml')]],
'invalid_defaultsv1_as_defaultsv1':
    [['--code', '1'],
    ['--type', 'modulemd-defaults-v1', files('tests/test_data/invalid_defaults.yaml')]],
'valid_defaultsv1_with_garbage_as_defaultsv1':
    [['--code', '1'],
    ['--type', 'modulemd-defaults-v1', files('tests/test_data/valid_defaults_with_garbage.yaml')]],
'valid_defaultsv1_with_unexpected_document_as_defaultsv1':
    [['--code', '1'],
    ['--type', 'modulemd-defaults-v1', files('tests/test_data/valid_defaults_with_unexpected_document.yaml')]],
'valid_modulemdv1_as_modulemdv1':
    [['--code', '0'],
    ['--type', 'modulemd-v1', files('../yaml_specs/modulemd_stream_v1.yaml')]],
'invalid_modulemdv1_as_modulemdv1':
    [['--code', '1', '--stderr', 'not in valid N-E:V-R.A format'],
    ['--type', 'modulemd-v1', files('tests/test_data/bad_stream_v1.yaml')]],
'valid_modulemdv2_as_modulemdv1':
    [['--code', '1'],
    ['--type', 'modulemd-v1', files('../yaml_specs/modulemd_stream_v2.yaml')]],
'valid_modulemdv2_as_modulemdv2':
    [['--code', '0'],
    ['--type', 'modulemd-v2', files('../yaml_specs/modulemd_stream_v2.yaml')]],
'invalid_modulemdv2_as_modulemdv2':
    [['--code', '1', '--stderr', 'Stream context'],
    ['--type', 'modulemd-v2', files('tests/test_data/bad_stream_v2.yaml')]],
'valid_packagerv3_as_modulemdv2':
    [['--code', '1'],
    ['--type', 'modulemd-v2', files('../yaml_specs/modulemd_packager_v3.yaml')]],
'valid_obsoletesv1.yaml':
    [['--code', '0'],
    [files('../yaml_specs/modulemd_obsoletes_v1.yaml')]],
'invalid_obsoletesv1.yaml':
    [['--code', '1'],
    [files('tests/test_data/invalid_obsoletes_v1.yaml')]],
'valid_obsoletesv1_as_obsoletesv1.yaml':
    [['--code', '0'],
    ['--type', 'modulemd-obsoletes-v1', files('../yaml_specs/modulemd_obsoletes_v1.yaml')]],
'invalid_obsoletesv1_as_obseletesv1.yaml':
    [['--code', '1'],
    ['--type', 'modulemd-obsoletes-v1', files('tests/test_data/invalid_obsoletes_v1.yaml')]],
'modulemdv2_as_defaultsv1':
    [['--code', '1'],
    ['--type', 'defaults-v1', files('../yaml_specs/modulemd_stream_v2.yaml')]],
'valid_packagerv2_as_packagerv2':
    [['--code', '0'],
    ['--type', 'modulemd-packager-v2', files('../yaml_specs/modulemd_packager_v2.yaml')]],
'invalid_packagerv2_as_packagerv2':
    [['--code', '1', '--stderr', 'license is missing'],
    ['--type', 'modulemd-packager-v2', files('tests/test_data/bad_packager_v2.yaml')]],
'valid_modulemdv2_as_packagerv2':
    [['--code', '1'],
    ['--type', 'modulemd-packager-v2', files('../yaml_specs/modulemd_stream_v2.yaml')]],
'valid_packagerv3_as_packagerv3':
    [['--code', '0'],
    ['--type', 'modulemd-packager-v3', files('../yaml_specs/modulemd_packager_v3.yaml')]],
'invalid_packagerv3_as_packagerv3':
    [['--code', '1', '--stderr', 'context exceeds'],
    ['--type', 'modulemd-packager-v3', files('tests/test_data/bad_packager_v3.yaml')]],
'valid_modulemdv2_as_packagerv3':
    [['--code', '1'],
    ['--type', 'modulemd-packager-v3', files('../yaml_specs/modulemd_stream_v2.yaml')]],
'valid_translationsv1_as_translationsv1':
    [['--code', '0'],
    ['--type', 'modulemd-translations-v1', files('../yaml_specs/modulemd_translations_v1.yaml')]],
'invalid_translationsv1_as_translationsv1':
    [['--code', '1', '--stderr', 'not a valid integer'],
    ['--type', 'modulemd-translations-v1', files('tests/test_data/bad_translations_v1.yaml')]],
'valid_modulemdv2_as_translationsv1':
    [['--code', '1'],
    ['--type', 'modulemd-translations-v1', files('../yaml_specs/modulemd_stream_v2.yaml')]],
}
test_modulemd_validator = executable(
    'test-modulemd-validator',
    files('tests/test-modulemd-validator.c'),
    dependencies : [ glib ],
    install : false,
)

foreach name, arguments : modulemd_validator_tests
    test(name, test_modulemd_validator,
         args : [arguments[0], '--', modulemd_validator, arguments[1]],
         env : test_env,
         timeout : 60,
         suite : ['ci', 'modulemd_validator'])
endforeach


# -- C/C++ Header test -- #
# Ensures that all public headers can be imported by consumers
# This test takes a while, so run it near the end so that the functional test
# results are visible more quickly.
import_header_script = find_program('tests/test-import-headers.sh')
test('test_import_headers', import_header_script,
      env : test_env,
      args : modulemd_hdrs,
      timeout : 300,
      suite : ['smoketest', 'ci'])
