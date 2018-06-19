project('modulemd', 'c',
        version : '1.5.0',
        default_options : [
          'buildtype=debugoptimized',
          'c_std=c11',
          'warning_level=1',
          'b_asneeded=true',
        ],
        license : 'MIT',
        meson_version : '>=0.44.0')

cc = meson.get_compiler('c')
test_cflags = [
  '-Wpointer-arith',
  '-Wmissing-declarations',
  '-Wmissing-prototypes',
  '-Wstrict-prototypes',
  '-Wuninitialized',
  '-Werror=implicit',
  '-Werror=init-self',
  '-Werror=main',
  '-Werror=missing-braces',
  '-Werror=return-type',
  '-Werror=array-bounds',
  '-Werror=write-strings',
  '-DG_LOG_USE_STRUCTURED',
  '-DG_LOG_DOMAIN="libmodulemd"',
]
foreach cflag: test_cflags
  if cc.has_argument(cflag)
    add_project_arguments(cflag, language : 'c')
  endif
endforeach

pymod = import('python3')
gnome = import('gnome')
pkg = import('pkgconfig')
gobject = dependency('gobject-2.0')
yaml = dependency('yaml-0.1')
gtkdoc = dependency('gtk-doc')

subdir('modulemd')
