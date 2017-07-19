project('modulemd', 'c',
        default_options : [
          'buildtype=debugoptimized',
          'c_std=c11',
          'warning_level=1',
          'b_asneeded=true',
        ],
        license : 'MIT',
        meson_version : '>=0.36.0')

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
]
foreach cflag: test_cflags
  if cc.has_argument(cflag)
    add_project_arguments(cflag, language : 'c')
  endif
endforeach

gnome = import('gnome')
gobject = dependency('gobject-2.0')

subdir('modulemd')
