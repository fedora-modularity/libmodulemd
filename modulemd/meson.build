modulemd_srcs = [
    'modulemd.c',
    'modulemd.h',
    'modulemd-modulemetadata.c',
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
    sources : modulemd_srcs,
    nsversion : '0.1',
    namespace : 'Modulemd',
    symbol_prefix : 'modulemd_',
    identifier_prefix : 'Modulemd',
    includes : [
        'GObject-2.0',
    ],
    install : true,
)
