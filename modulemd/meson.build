modulemd_srcs = [
    'modulemd-component.c',
    'modulemd-component-module.c',
    'modulemd-component-rpm.c',
    'modulemd-module.c',
    'modulemd-profile.c',
    'modulemd-simpleset.c',
    'modulemd-servicelevel.c',
    'modulemd-util.c',
    'modulemd-yaml-emitter.c',
    'modulemd-yaml-parser.c',
    'modulemd-yaml-utils.c'
]

modulemd_hdrs = [
    'modulemd.h',
    'modulemd-component.h',
    'modulemd-component-module.h',
    'modulemd-component-rpm.h',
    'modulemd-module.h',
    'modulemd-profile.h',
    'modulemd-simpleset.h',
    'modulemd-servicelevel.h',
]

modulemd_priv_hdrs = [
    'modulemd-util.h',
    'modulemd-yaml.h',
]

modulemd_lib = library(
    'modulemd',
    sources : modulemd_srcs,
    dependencies : [
        gobject,
        yaml,
    ],
    install : true,
    soversion: '0',
    version: '0.1.1',
)

header_path = 'modulemd'

install_headers(
  modulemd_hdrs,
  subdir : header_path,
)

test_modulemd_component = executable(
    'test_modulemd_component',
    'test-modulemd-component.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_component', test_modulemd_component)

test_modulemd_module = executable(
    'test_modulemd_module',
    'test-modulemd-module.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_module', test_modulemd_module)

test_modulemd_simpleset = executable(
    'test_modulemd_simpleset',
    'test-modulemd-simpleset.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_simpleset', test_modulemd_simpleset)

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
yaml_test_env = environment()
yaml_test_env.set('G_MESSAGES_DEBUG', 'all')
yaml_test_env.set ('MESON_SOURCE_ROOT', meson.source_root())
test('test_modulemd_yaml', test_modulemd_yaml,
     env : yaml_test_env)

gnome.generate_gir(
    modulemd_lib,
    sources : modulemd_srcs + modulemd_hdrs,
    nsversion : '0.1',
    namespace : 'Modulemd',
    symbol_prefix : 'modulemd_',
    identifier_prefix : 'Modulemd',
    includes : [
        'GObject-2.0',
    ],
    install : true,
)

cdata = configuration_data()
cdata.set('VERSION', '0.1.0')
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
    version : '0.1',
    name : 'modulemd',
    filebase : 'modulemd',
    description : 'Module metadata manipulation library',
    requires: [ 'glib-2.0', 'gobject-2.0' ],
)

