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

project('modulemd', 'c',
        version : '2.8.0',
        default_options : [
          'buildtype=debugoptimized',
          'c_std=c11',
          'warning_level=1',
          'b_asneeded=true',
        ],
        license : 'MIT',
        meson_version : '>=0.47.0')

libmodulemd_version = meson.project_version()

cc = meson.get_compiler('c')
test_cflags = [
  '-Wpointer-arith',
  '-Werror=missing-declarations',
  '-Wmissing-prototypes',
  '-Wstrict-prototypes',
  '-Wuninitialized',
  ['-Werror=format-security', '-Werror=format=2'], # Must be checked together
  '-Werror=implicit',
  '-Werror=init-self',
  '-Werror=main',
  '-Werror=missing-braces',
  '-Werror=return-type',
  '-Werror=array-bounds',
  '-Werror=write-strings',
  '-D_POSIX_SOURCE',
  '-DG_LOG_USE_STRUCTURED',
  '-DG_LOG_DOMAIN="libmodulemd"',
]
foreach cflag: test_cflags
  if cc.has_multi_arguments(cflag)
    add_project_arguments(cflag, language : 'c')
  endif
endforeach

pymod = import('python')
gnome = import('gnome')
pkg = import('pkgconfig')
gobject = dependency('gobject-2.0')
yaml = dependency('yaml-0.1')
gtkdoc = dependency('gtk-doc')

with_rpmio = get_option('rpmio')
with_libmagic = get_option('libmagic')

rpm = dependency('rpm', required : with_rpmio)
magic = cc.find_library('magic', required : with_libmagic)

glib_prefix = dependency('glib-2.0').get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')

sh = find_program('sh')
sed = find_program('sed')
test = find_program('test')

ret = run_command ([test, '-e', join_paths(glib_docpath, 'glib/index.html')])
if ret.returncode() != 0
  error('Missing documentation for GLib.')
endif

ret = run_command ([test, '-e', join_paths(glib_docpath, 'gobject/index.html')])
if ret.returncode() != 0
  error('Missing documentation for GObject.')
endif

python_name = get_option('python_name')

if python_name != ''
    # If we've been instructed to use a specific python version
    python3 = pymod.find_installation(python_name)
else
    # Use the python installation that is running meson
    python3 = pymod.find_installation()
endif

with_py2_overrides = get_option('with_py2_overrides')
if with_py2_overrides
    python2 = pymod.find_installation('python2')
else
    python2 = disabler()
endif

spec_tmpl = find_program('spec_tmpl.sh')

specfile_template = files('libmodulemd.spec.in')

mkdir = find_program('mkdir')

rpmsetup_target = custom_target(
    'rpmsetup',
    command: [
        mkdir, '-p',
        'rpmbuild/BUILD',
        'rpmbuild/RPMS',
        'rpmbuild/SPECS',
        'rpmbuild/SRPMS'],
    output: 'rpmbuild',
)

spec_target = custom_target(
    'specfile',
    capture: true,
    build_by_default: true,
    build_always_stale: true,
    command: [sh, spec_tmpl,
              meson.project_version(),
              '@INPUT@'],
    input: specfile_template,
    output: 'libmodulemd.spec',
    depends: rpmsetup_target,
)

rpm_cdata = configuration_data()
rpm_cdata.set('VERSION', meson.project_version())
rpm_cdata.set('BUILDFLAG', '-bb')

srpm_cdata = configuration_data()
srpm_cdata.set('VERSION', meson.project_version())
srpm_cdata.set('BUILDFLAG', '-bs')

configure_file(
    input: 'make_rpms.sh.in',
    output: 'make_srpm.sh',
    configuration: srpm_cdata,
)

configure_file(
    input: 'make_rpms.sh.in',
    output: 'make_rpms.sh',
    configuration: rpm_cdata,
)

subdir('modulemd')
subdir('bindings/python')
