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

# -- Configuration Options -- #

test_dirty_git = get_option('test_dirty_git')
test_installed_lib = get_option('test_installed_lib')
skip_formatters = get_option('skip_formatters')
skip_introspection = get_option('skip_introspection')
developer_build = get_option('developer_build')

if not developer_build
    skip_formatters = true
endif

clang_simple_version_script = find_program ('clang_simple_version.sh')

if skip_formatters
    clang_format = disabler()
    python_formatter = disabler()
    pycodestyle = disabler()
else
    clang_format = find_program('clang-format', required: developer_build)
    if not clang_format.found()
        clang_format = disabler()
    endif

    python_formatter = find_program('black', required : developer_build)
    if not python_formatter.found()
        python_formatter = disabler()
    endif

    pycodestyle = find_program('pycodestyle-3', 'pycodestyle',
                               required : developer_build)
    if not pycodestyle.found()
        pycodestyle = disabler()
    endif
endif


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
cdata.set('HAVE_LIBMAGIC', magic.found())
cdata.set('HAVE_GDATE_AUTOPTR', has_gdate_autoptr)
cdata.set('HAVE_EXTEND_AND_STEAL', has_extend_and_steal)
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
        magic,
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

modulemd_validator = executable(
    'modulemd-validator',
    sources : modulemd_validator_srcs,
    include_directories : include_dirs,
    dependencies : [
        gobject,
        magic,
        rpm,
        yaml,
        modulemd_dep
    ],
    install : true
)

header_path = 'modulemd-2.0'

install_headers(
  modulemd_hdrs,
  subdir : header_path,
)


# --- TESTS --- #

# Test env with release values
test_release_env = environment()
test_release_env.set('LC_ALL', 'C')
test_release_env.set ('MESON_SOURCE_ROOT', meson.source_root())
test_release_env.set ('MESON_BUILD_ROOT', meson.build_root())
test_release_env.set ('TEST_DATA_PATH', meson.source_root() + '/modulemd/tests/test_data')

# Test env with fatal warnings and criticals
test_env = test_release_env
test_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')
test_env.set('G_MESSAGES_DEBUG', 'all')

# Python test env with fatal warnings and criticals
py_test_env = test_env

if not test_installed_lib
    # If we're testing an installed version, we want to use the default
    # locations for these paths.
    py_test_env.set ('GI_TYPELIB_PATH', meson.build_root() + '/modulemd')
    py_test_env.set ('LD_LIBRARY_PATH', meson.build_root() + '/modulemd')
    py_test_env.set ('PYTHONPATH', meson.source_root())

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


# Make sure we remembered to bump the version number since the last release
version_test = find_program('tests/test-version.sh')
if developer_build
    test('version',
         version_test,
         env : test_env,
         suite : ['smoketest', 'ci'])
endif

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
'buildopts'           : [ 'tests/test-modulemd-buildopts.c' ],
'common'              : [ 'tests/test-modulemd-common.c' ],
'component_module'    : [ 'tests/test-modulemd-component-module.c' ],
'component_rpm'       : [ 'tests/test-modulemd-component-rpm.c' ],
'compression'         : [ 'tests/test-modulemd-compression.c' ],
'defaults'            : [ 'tests/test-modulemd-defaults.c' ],
'defaultsv1'          : [ 'tests/test-modulemd-defaults-v1.c' ],
'dependencies'        : [ 'tests/test-modulemd-dependencies.c' ],
'module'              : [ 'tests/test-modulemd-module.c' ],
'module_index'        : [ 'tests/test-modulemd-moduleindex.c' ],
'module_index_merger' : [ 'tests/test-modulemd-merger.c' ],
'modulestream'        : [ 'tests/test-modulemd-modulestream.c' ],
'profile'             : [ 'tests/test-modulemd-profile.c' ],
'rpm_map'             : [ 'tests/test-modulemd-rpmmap.c' ],
'service_level'       : [ 'tests/test-modulemd-service-level.c' ],
'translation'         : [ 'tests/test-modulemd-translation.c' ],
'translation_entry'   : [ 'tests/test-modulemd-translation-entry.c' ],
'obsoletes'           : [ 'tests/test-modulemd-obsoletes.c' ],
}

foreach name, sources : c_tests
    test_srcs += files(sources)
    exe = executable(
        name,
        sources,
        dependencies : [
            modulemd_dep,
        ],
        link_with : [
            test_utils_lib,
        ],
        install : false,
    )
    test(name + '_debug', exe,
         env : test_env,
         timeout : 60,
         suite : ['ci', 'ci_valgrind', 'c', 'c_debug'])
    test(name + '_release', exe,
         env : test_release_env,
         timeout : 60,
         suite : ['ci', 'c', 'c_release'])
endforeach


python_tests = {
'buildopts'        : 'tests/ModulemdTests/buildopts.py',
'componentrpm'     : 'tests/ModulemdTests/componentrpm.py',
'defaults'         : 'tests/ModulemdTests/defaults.py',
'defaultsv1'       : 'tests/ModulemdTests/defaultsv1.py',
'dependencies'     : 'tests/ModulemdTests/dependencies.py',
'merger'           : 'tests/ModulemdTests/merger.py',
'module'           : 'tests/ModulemdTests/module.py',
'moduleindex'      : 'tests/ModulemdTests/moduleindex.py',
'modulepackager'   : 'tests/ModulemdTests/modulepackager.py',
'modulestream'     : 'tests/ModulemdTests/modulestream.py',
'profile'          : 'tests/ModulemdTests/profile.py',
'rpmmap'           : 'tests/ModulemdTests/rpmmap.py',
'servicelevel'     : 'tests/ModulemdTests/servicelevel.py',
'translation'      : 'tests/ModulemdTests/translation.py',
'translationentry' : 'tests/ModulemdTests/translationentry.py',
}

foreach name, script : python_tests
    test_python_scripts += files(script)
    test (name + '_python3_debug', python3,
          env : py_test_env,
          timeout : 60,
          args : files(script),
          suite : ['ci', 'py3', 'py3_debug'])
    test (name + '_python3_release', python3,
          env : py_test_release_env,
          timeout : 60,
          args : files(script),
          suite : ['ci', 'py3', 'py3_release'])

    if with_py2
        test (name + '_python2_debug', python2,
              env : py_test_env,
              timeout : 60,
              args : files(script),
              suite : ['ci', 'py2', 'py2_debug'])
        test (name + '_python2_release', python2,
              env : py_test_release_env,
              timeout : 60,
              args : files(script),
              suite : ['ci', 'py2', 'py2_release'])
    endif #with_py2
endforeach


# --- Formatting Helpers -- #
# Run these after the functional tests so we get those results more quickly


# Fake test to ensure that all sources and headers are formatted properly
clang_files = modulemd_srcs + modulemd_hdrs + modulemd_priv_hdrs + modulemd_validator_srcs + test_srcs + test_priv_hdrs

clang_args = [ '-i' ]
test('clang_format', clang_format,
     args : clang_args + clang_files,
     suite : 'formatters')


# Fake test to ensure that the python tests are formatted according to PEP8
python_formatter_args = [ '--verbose', '--line-length', '79' ]
test('python_formatter', python_formatter,
     args : python_formatter_args + [ meson.source_root() ],
     suite : 'formatters')


# Test all python files for compliance with pycodestyle
# Ignore line-length as there are a number of places where this is unavoidable
python_scripts = [ files('tests/test-dirty.py',
                         '../bindings/python/gi/overrides/Modulemd.py') +
                   test_python_scripts ]

pycodestyle_args = [
    '--show-pep8',
    '--ignore=E121,E123,E126,E226,E24,E501,E704,W503,W504'
]
test('pycodestyle', pycodestyle,
     args : pycodestyle_args + python_scripts,
     suite : 'formatters')


# Fake test to ensure that the autoformatters didn't end up making changes
# when run in CI (such as during a pull-request). This test is only run if
# meson was configured with `-Dtest_dirty_git=true`

if test_dirty_git
    dirty_repo_scripts = [ files('tests/test-dirty.py') ]
    test('test_dirty_repo', python3,
         args : dirty_repo_scripts,
         suite : ['ci', 'dirty'])
endif


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


# --- GOBject Introspection -- #

if skip_introspection
else
    gnome.generate_gir(
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

if with_docs
    gnome.gtkdoc(
        'modulemd-2.0',
        install_dir: 'modulemd-2.0',
        src_dir : './modulemd',
        main_xml : 'modulemd-docs.xml',
        gobject_typesfile : join_paths(meson.current_build_dir(), 'modulemd-2.0.types'),
        dependencies : [
            modulemd_dep,
        ],
        fixxref_args: [
                     '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
                     '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
                   ],
        install : true,
    )
endif

help2man_opts = [
  '--no-info',
  '--section=1',
]
custom_target(
    'modulemd-validator.1',
    output: 'modulemd-validator.1',
    command: [
      help2man, help2man_opts, '--output=@OUTPUT@', modulemd_validator,
    ],
    install: true,
    install_dir: join_paths(get_option('mandir'), 'man1'))
