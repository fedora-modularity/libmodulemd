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

developer_build = get_option('developer_build')
test_dirty_git = get_option('test_dirty_git')
test_installed_lib = get_option('test_installed_lib')
skip_clang_tidy = get_option('skip_clang_tidy')
skip_introspection = get_option('skip_introspection')


clang_simple_version_script = find_program ('clang_simple_version.sh')

if skip_clang_tidy
    clang_tidy = disabler()
else
    clang_tidy = find_program('clang-tidy', required: false)
endif

if clang_tidy.found()
    ret = run_command (clang_simple_version_script, clang_tidy.path())
    if ret.returncode() != 0
        warning('Unable to determine clang-tidy version.')
        clang_tidy = disabler()
    else
        clang_tidy_version = ret.stdout().strip()

        # Older versions of clang_tidy have issues with producing changes that
        # do not compile. Only use this tool if we have a sufficiently new
        # version.
        if not clang_tidy_version.version_compare('>=9.0.0')
            warning('clang-tidy version too old: ' +
                    clang_tidy_version +
                    ' < 9.0.0')
            clang_tidy = disabler()
        endif
    endif
else
    clang_tidy = disabler()
endif

if clang_tidy.found()
    clang_format = disabler()
else
    clang_format = find_program('clang-format', required: developer_build)
    if not clang_format.found()
        clang_format = disabler()
    endif
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

valgrind = find_program('valgrind', required: developer_build)

if not valgrind.found()
    valgrind = disabler()
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
)

test_srcs = files(
    'tests/test-modulemd-buildopts.c',
    'tests/test-modulemd-component-module.c',
    'tests/test-modulemd-component-rpm.c',
    'tests/test-modulemd-compression.c',
    'tests/test-modulemd-defaults.c',
    'tests/test-modulemd-defaults-v1.c',
    'tests/test-modulemd-dependencies.c',
    'tests/test-modulemd-merger.c',
    'tests/test-modulemd-module.c',
    'tests/test-modulemd-moduleindex.c',
    'tests/test-modulemd-modulestream.c',
    'tests/test-modulemd-profile.c',
    'tests/test-modulemd-rpmmap.c',
    'tests/test-modulemd-service-level.c',
    'tests/test-modulemd-translation.c',
    'tests/test-modulemd-translation-entry.c',
    'tests/test-utils.c',
)

test_priv_hdrs = files(
    'include/private/test-utils.h',
)

test_python_scripts = files(
    'tests/ModulemdTests/base.py',
    'tests/ModulemdTests/buildopts.py',
    'tests/ModulemdTests/componentrpm.py',
    'tests/ModulemdTests/defaults.py',
    'tests/ModulemdTests/defaultsv1.py',
    'tests/ModulemdTests/dependencies.py',
    'tests/ModulemdTests/merger.py',
    'tests/ModulemdTests/module.py',
    'tests/ModulemdTests/moduleindex.py',
    'tests/ModulemdTests/modulestream.py',
    'tests/ModulemdTests/profile.py',
    'tests/ModulemdTests/rpmmap.py',
    'tests/ModulemdTests/servicelevel.py',
    'tests/ModulemdTests/translation.py',
    'tests/ModulemdTests/translationentry.py',
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

valgrind_tests = []


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
}

foreach name, sources : c_tests
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
         timeout : 60)
    test(name + '_release', exe,
         env : test_release_env)

    valgrind_tests += name + '_debug'
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
'modulestream'     : 'tests/ModulemdTests/modulestream.py',
'profile'          : 'tests/ModulemdTests/profile.py',
'rpmmap'           : 'tests/ModulemdTests/rpmmap.py',
'servicelevel'     : 'tests/ModulemdTests/servicelevel.py',
'translation'      : 'tests/ModulemdTests/translation.py',
'translationentry' : 'tests/ModulemdTests/translationentry.py',
}

foreach name, script : python_tests
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


# --- Formatting Helpers -- #
# Run these after the functional tests so we get those results more quickly


# Fake test to ensure that all sources and headers are formatted properly
clang_files = modulemd_srcs + modulemd_hdrs + modulemd_priv_hdrs + modulemd_validator_srcs + test_srcs + test_priv_hdrs

clang_args = [ '-i' ]
test('clang_format', clang_format,
     args : clang_args + clang_files)

clang_tidy_args = [ '-p', meson.build_root(),
                    '--checks=*,-llvm-header-guard,-bugprone-macro-parentheses',
                    '--format-style=file',
                    '--fix',
                    '--fix-errors' ]
test('clang_tidy', clang_tidy,
     args: clang_tidy_args + clang_files,
     timeout: 300)


# Fake test to ensure that the python tests are formatted according to PEP8
python_formatter_args = [ '--verbose', '--line-length', '79' ]
test('python_formatter', python_formatter,
     args: python_formatter_args + [ meson.source_root() ] )


# Test all python files for compliance with pycodestyle
# Ignore line-length as there are a number of places where this is unavoidable
python_scripts = [ files('tests/test-dirty.py',
                         'tests/test-valgrind.py',
                         '../bindings/python/gi/overrides/Modulemd.py') +
                   test_python_scripts ]

pycodestyle_args = [
    '--show-pep8',
    '--ignore=E121,E123,E126,E226,E24,E501,E704,W503,W504'
]
test('pycodestyle', pycodestyle,
     args: pycodestyle_args + python_scripts)


# Fake test to ensure that the autoformatters didn't end up making changes
# when run in CI (such as during a pull-request). This test is only run if
# meson was configured with `-Dtest_dirty_git=true`

if test_dirty_git
    dirty_repo_scripts = [ files('tests/test-dirty.py') ]
    test('test_dirty_repo', python3,
         args: dirty_repo_scripts)
endif


# -- C/C++ Header test -- #
# Ensures that all public headers can be imported by consumers
# This test takes a while, so run it near the end so that the functional test
# results are visible more quickly.
import_header_script = find_program('tests/test-import-headers.sh')
test ('test_import_headers', import_header_script,
      env : test_env,
      args : modulemd_hdrs,
      timeout : 300)


# -- Run Valgrind against all tests that support it -- #
if valgrind.found()
    modulemd_valgrind_scripts = files('tests/test-valgrind.py')
    test ('valgrind', python3,
          env : test_env,
          args : modulemd_valgrind_scripts + valgrind_tests,
          timeout : 1200)
endif


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
        install : true,
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
