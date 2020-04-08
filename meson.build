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

project(
  'modulemd',
  'c',
version : '2.9.4',
default_options : ['buildtype=debugoptimized', 'c_std=c11', 'warning_level=1', 'b_asneeded=true'],
license : 'MIT',
meson_version : '>=0.47.0'
)

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
  '-D_GNU_SOURCE',
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

with_rpmio = get_option('rpmio')
with_libmagic = get_option('libmagic')

rpm = dependency('rpm', required : with_rpmio)
magic = cc.find_library('magic', required : with_libmagic)

glib = dependency('glib-2.0')
glib_prefix = glib.get_pkgconfig_variable('prefix')

bash = find_program('bash')
sed = find_program('sed')
test = find_program('test')

with_docs = get_option('with_docs')
if with_docs
  gtkdoc = dependency('gtk-doc')
  glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')

  ret = run_command ([test, '-e', join_paths(glib_docpath, 'glib/index.html')])
  if ret.returncode() != 0
    error('Missing documentation for GLib.')
  endif

  ret = run_command ([test, '-e', join_paths(glib_docpath, 'gobject/index.html')])
  if ret.returncode() != 0
    error('Missing documentation for GObject.')
  endif
endif

with_manpages = get_option('with_manpages')
help2man = find_program('help2man', required: with_manpages)
if not help2man.found()
    help2man = disabler()
endif

# Check whether this version of glib has the GDate autoptr defined
gdate_check = '''#include <glib.h>
int main (int argc, char **argv)
{
  g_autoptr(GDate) date = NULL;
  return 0;
}
'''
has_gdate_autoptr = cc.compiles(
    gdate_check,
    dependencies : [ glib ],
    name : 'g_autoptr(GDate)')

# Check whether glib2 has g_ptr_array_extend_and_steal or if we
# need to bundle it.
has_extend_and_steal = cc.has_function(
    'g_ptr_array_extend_and_steal',
    dependencies : [ glib ])

python_name = get_option('python_name')

if python_name != ''
    # If we've been instructed to use a specific python version
    python3 = pymod.find_installation(python_name)
else
    # Use the python installation that is running meson
    python3 = pymod.find_installation()
endif

with_py2 = get_option('with_py2')
if with_py2
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
    command: [bash, spec_tmpl,
              'version=' + meson.project_version(),
              'template=@INPUT@'],
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

if meson.version().version_compare('>=0.53')
    if magic.found()
        if with_libmagic.enabled()
            magic_status = 'Enabled'
        elif with_libmagic.auto()
            magic_status = 'Enabled (autodetected)'
        else
            error('libmagic state is unknown')
        endif
    else
        if with_libmagic.disabled()
            magic_status = 'Disabled'
        elif with_libmagic.auto()
            magic_status = 'Disabled (autodetection could not locate libmagic)'
        else
            error('libmagic state is unknown')
        endif
    endif

    if rpm.found()
        if with_rpmio.enabled()
            rpmio_status = 'Enabled'
        elif with_rpmio.auto()
            rpmio_status = 'Enabled (autodetected)'
        else
            error('rpmio state is unknown')
        endif
    else
        if with_rpmio.disabled()
            rpmio_status = 'Disabled'
        elif with_rpmio.auto()
            rpmio_status = 'Disabled (autodetection could not locate librpm)'
        else
            error('rpmio state is unknown')
        endif
    endif

    if help2man.found()
        if with_manpages.enabled()
            manpages_status = 'Enabled'
        elif with_manpages.auto()
            manpages_status = 'Enabled (autodetected)'
        else
            error('manpages state is unknown')
        endif
    else
        if with_manpages.disabled()
            manpages_status = 'Disabled'
        elif with_rpmio.auto()
            manpages_status = 'Disabled (autodetection could not locate help2man)'
        else
            error('manpages state is unknown')
        endif
    endif

    summary({'prefix': get_option('prefix'),
             'bindir': get_option('bindir'),
             'libdir': get_option('libdir'),
             'datadir': get_option('datadir'),
             'Python 2 GObject Overrides': gobject_overrides_dir_py2,
             'Python 3 GObject Overrides': gobject_overrides_dir_py3
            }, section: 'Directories')

    summary({'Developer Build': get_option('developer_build'),
             'libmagic Support': magic_status,
             'Custom Python': get_option('python_name'),
             'RPMIO Support': rpmio_status,
             'Generate Manpages': manpages_status,
             'Generate HTML Documentation': get_option('with_docs'),
             'Python 2 Support': get_option('with_py2'),
             'Skip Formatters': get_option('skip_formatters'),
             'Skip Introspection': get_option('skip_introspection'),
             'Test Dirty Git': get_option('test_dirty_git'),
             'Test Installed Library': get_option('test_installed_lib'),
            }, section: 'Build Configuration')
endif
