modulemd_srcs = [
    'modulemd-common.c',
    'modulemd-component.c',
    'modulemd-component-module.c',
    'modulemd-component-rpm.c',
    'modulemd-defaults.c',
    'modulemd-dependencies.c',
    'modulemd-module.c',
    'modulemd-prioritizer.c',
    'modulemd-profile.c',
    'modulemd-simpleset.c',
    'modulemd-servicelevel.c',
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
    'modulemd-module.h',
    'modulemd-prioritizer.h',
    'modulemd-profile.h',
    'modulemd-simpleset.h',
    'modulemd-servicelevel.h',
]

modulemd_priv_hdrs = [
    'modulemd-util.h',
    'modulemd-yaml.h',
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

modulemd_validator = executable(
    'modulemd-validator',
    'modulemd-validator.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : true,
)

header_path = 'modulemd'

install_headers(
  modulemd_hdrs,
  subdir : header_path,
)

test_env = environment()
test_env.set('G_DEBUG', 'fatal-warnings,fatal-criticals')
test_env.set('G_MESSAGES_DEBUG', 'all')
test_env.set ('MESON_SOURCE_ROOT', meson.source_root())
test_env.set ('GI_TYPELIB_PATH', meson.build_root() + '/modulemd')
test_env.set ('MODULEMD_NSVERSION', '.'.join([libmodulemd_version_array[0], '0']))
test_env.set ('LD_LIBRARY_PATH', meson.build_root() + '/modulemd')

test_modulemd_component = executable(
    'test_modulemd_component',
    'test-modulemd-component.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_component', test_modulemd_component,
     env : test_env)

test_modulemd_defaults = executable(
    'test_modulemd_defaults',
    'test-modulemd-defaults.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
        yaml
    ],
    install : false,
)
test('test_modulemd_defaults', test_modulemd_defaults,
     env : test_env)

test_modulemd_dependencies = executable(
    'test_modulemd_dependencies',
    'test-modulemd-dependencies.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_dependencies', test_modulemd_dependencies,
     env : test_env)

test_modulemd_module = executable(
    'test_modulemd_module',
    'test-modulemd-module.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_module', test_modulemd_module,
     env : test_env)

test_modulemd_regressions = executable(
    'test_modulemd_regressions',
    'test-modulemd-regressions.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_regressions', test_modulemd_regressions,
    env : test_env)

test_modulemd_servicelevel = executable(
    'test_modulemd_servicelevel',
    'test-modulemd-servicelevel.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_servicelevel', test_modulemd_servicelevel)

test_modulemd_simpleset = executable(
    'test_modulemd_simpleset',
    'test-modulemd-simpleset.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_simpleset', test_modulemd_simpleset,
     env : test_env)

test_modulemd_yaml = executable(
    'test_modulemd_yaml',
    'test-modulemd-yaml.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
        yaml
    ],
    install : false,
)
test('test_modulemd_yaml', test_modulemd_yaml,
     env : test_env)

test_modulemd_python = find_program('test-modulemd-python.py')
test ('test_modulemd_python', test_modulemd_python, env : test_env)

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

