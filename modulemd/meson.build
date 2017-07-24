modulemd_srcs = [
    'modulemd-module.c',
]

modulemd_hdrs = [
    'modulemd.h',
    'modulemd-module.h',
]

modulemd_lib = library(
    'modulemd',
    sources : modulemd_srcs,
    dependencies : [
        gobject,
    ],
    install : true,
    soversion: '0',
    version: '0.1.0',

)

install_headers(
  modulemd_hdrs,
  subdir : 'modulemd',
)

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


