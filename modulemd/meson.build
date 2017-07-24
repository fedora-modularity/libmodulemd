modulemd_srcs = [
    'modulemd-modulemetadata.c',
]

modulemd_hdrs = [
    'modulemd.h',
    'modulemd-modulemetadata.h',
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

test_modulemd_modulemetadata = executable(
    'test_modulemd_modulemetadata',
    'test-modulemd-modulemetadata.c',
    link_with : modulemd_lib,
        dependencies : [
        gobject,
    ],
    install : false,
)
test('test_modulemd_modulemetadata', test_modulemd_modulemetadata)

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


