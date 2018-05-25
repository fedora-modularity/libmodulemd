modulemd_srcs = [
    'modulemd-common.c',
    'modulemd-component.c',
    'modulemd-component-module.c',
    'modulemd-component-rpm.c',
    'modulemd-defaults.c',
    'modulemd-dependencies.c',
    'modulemd-intent.c',
    'modulemd-module.c',
    'modulemd-prioritizer.c',
    'modulemd-profile.c',
    'modulemd-simpleset.c',
    'modulemd-servicelevel.c',
    'modulemd-subdocument.c',
    'modulemd-util.c',
    'modulemd-yaml-emitter.c',
    'modulemd-yaml-emitter-defaults.c',
    'modulemd-yaml-emitter-modulemd.c',
    'modulemd-yaml-parser.c',
    'modulemd-yaml-parser-defaults.c',
    'modulemd-yaml-parser-modulemd.c',
    'modulemd-yaml-utils.c'
]

modulemd_hdrs = [
    'modulemd.h',
    'modulemd-component.h',
    'modulemd-component-module.h',
    'modulemd-component-rpm.h',
    'modulemd-defaults.h',
    'modulemd-dependencies.h',
    'modulemd-intent.h',
    'modulemd-module.h',
    'modulemd-prioritizer.h',
    'modulemd-profile.h',
    'modulemd-simpleset.h',
    'modulemd-servicelevel.h',
    'modulemd-subdocument.h',
]

modulemd_priv_hdrs = [
    'modulemd-util.h',
    'modulemd-yaml.h',
    'modulemd-subdocument-private.h',
]

test_srcs = [
    'test-modulemd-component.c',
    'test-modulemd-defaults.c',
    'test-modulemd-dependencies.c',
    'test-modulemd-intent.c',
    'test-modulemd-module.c',
    'test-modulemd-regressions.c',
    'test-modulemd-servicelevel.c',
    'test-modulemd-simpleset.c',
    'test-modulemd-subdocument.c',
    'test-modulemd-yaml.c',
]

libmodulemd_version = meson.project_version()
libmodulemd_version_array = libmodulemd_version.split('.')

modulemd_lib = library(
    'modulemd',
    sources : modulemd_srcs,
    dependencies : [
        gobject,
        yaml,
    ],
    install : true,
    soversion: libmodulemd_version_array[0],
    version: libmodulemd_version,
)

modulemd_dep = declare_dependency(
    include_directories : include_directories('.'),
    link_with : modulemd_lib,
    dependencies : [
        gobject,
    ]
)

modulemd_validator = executable(
    'modulemd-validator',
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

# Fake test to ensure that all sources and headers are formatted properly
test_clang_format = find_program('clang-format')
clang_args = [ '-i' ]
test('test_clang_format', test_clang_format,
     args : clang_args + files(
         modulemd_srcs +
         modulemd_hdrs +
         modulemd_priv_hdrs +
         test_srcs))

# Fake test to ensure that the python tests are formatted according to PEP8
test_autopep8 = find_program('python3-autopep8', 'autopep8')
autopep8_args = [ '--in-place', '-a', '-a' ]
autopep8_scripts = [ files('test-modulemd-python.py') ]
test('test_autopep8', test_autopep8,
     args: autopep8_args + autopep8_scripts)

# Test env with fatal warnings and criticals
test_env = environment()
test_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')
test_env.set('G_MESSAGES_DEBUG', 'all')
test_env.set ('MESON_SOURCE_ROOT', meson.source_root())
test_env.set ('GI_TYPELIB_PATH', meson.build_root() + '/modulemd')
test_env.set ('MODULEMD_NSVERSION', '.'.join([libmodulemd_version_array[0], '0']))
test_env.set ('LD_LIBRARY_PATH', meson.build_root() + '/modulemd')

# Test env with release values
test_release_env = environment()
test_release_env.set('G_MESSAGES_DEBUG', 'all')
test_release_env.set ('MESON_SOURCE_ROOT', meson.source_root())
test_release_env.set ('GI_TYPELIB_PATH', meson.build_root() + '/modulemd')
test_release_env.set ('MODULEMD_NSVERSION', '.'.join([libmodulemd_version_array[0], '0']))
test_release_env.set ('LD_LIBRARY_PATH', meson.build_root() + '/modulemd')

test_modulemd_component = executable(
    'test_modulemd_component',
    'test-modulemd-component.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_component', test_modulemd_component,
     env : test_env)
test('test_release_modulemd_component', test_modulemd_component,
     env : test_release_env)

test_modulemd_defaults = executable(
    'test_modulemd_defaults',
    'test-modulemd-defaults.c',
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

test_modulemd_dependencies = executable(
    'test_modulemd_dependencies',
    'test-modulemd-dependencies.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_dependencies', test_modulemd_dependencies,
     env : test_env)
test('test_release_modulemd_dependencies', test_modulemd_dependencies,
     env : test_release_env)

test_modulemd_intent = executable(
    'test_modulemd_intent',
    'test-modulemd-intent.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_intent', test_modulemd_intent,
     env : test_env)
test('test_release_modulemd_intent', test_modulemd_intent,
     env : test_release_env)

test_modulemd_module = executable(
    'test_modulemd_module',
    'test-modulemd-module.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_module', test_modulemd_module,
     env : test_env)
test('test_release_modulemd_module', test_modulemd_module,
     env : test_release_env)

test_modulemd_regressions = executable(
    'test_modulemd_regressions',
    'test-modulemd-regressions.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_regressions', test_modulemd_regressions,
    env : test_env)
test('test_release_modulemd_regressions', test_modulemd_regressions,
    env : test_release_env)

test_modulemd_servicelevel = executable(
    'test_modulemd_servicelevel',
    'test-modulemd-servicelevel.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_servicelevel', test_modulemd_servicelevel,
     env : test_env)
test('test_release_modulemd_servicelevel', test_modulemd_servicelevel,
     env : test_release_env)

test_modulemd_simpleset = executable(
    'test_modulemd_simpleset',
    'test-modulemd-simpleset.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_simpleset', test_modulemd_simpleset,
     env : test_env)
test('test_release_modulemd_simpleset', test_modulemd_simpleset,
     env : test_release_env)

test_modulemd_subdocument = executable(
    'test_modulemd_subdocument',
    'test-modulemd-subdocument.c',
    dependencies : [
        modulemd_dep,
    ],
    install : false,
)
test('test_modulemd_subdocument', test_modulemd_subdocument,
     env : test_env)
test('test_release_modulemd_subdocument', test_modulemd_subdocument,
     env : test_release_env)

test_modulemd_yaml = executable(
    'test_modulemd_yaml',
    'test-modulemd-yaml.c',
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

test_modulemd_python = find_program('test-modulemd-python.py')
test ('test_modulemd_python', test_modulemd_python, env : test_env)
test ('test_release_modulemd_python', test_modulemd_python,
      env : test_release_env)

gnome.generate_gir(
    modulemd_lib,
    sources : modulemd_srcs + modulemd_hdrs,
    nsversion : '.'.join([libmodulemd_version_array[0], '0']),
    namespace : 'Modulemd',
    symbol_prefix : 'modulemd_',
    identifier_prefix : 'Modulemd',
    includes : [
        'GObject-2.0',
    ],
    install : true,
)

cdata = configuration_data()
cdata.set('VERSION', libmodulemd_version)
configure_file(input : 'version.xml.in',
  output : 'version.xml',
  configuration : cdata)

gnome.gtkdoc(
    'modulemd',
    src_dir : './',
    main_xml : 'modulemd-docs.xml',
    install : true)

pkg.generate(
    libraries : modulemd_lib,
    subdirs : header_path,
    version : libmodulemd_version,
    name : 'modulemd',
    filebase : 'modulemd',
    description : 'Module metadata manipulation library',
    requires: [ 'glib-2.0', 'gobject-2.0' ],
)

