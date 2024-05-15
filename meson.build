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
version : '2.15.0',
default_options : ['buildtype=debugoptimized', 'c_std=c11', 'warning_level=1', 'b_asneeded=true'],
license : 'MIT',
meson_version : '>=0.55.0'
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

with_libmagic = get_option('libmagic')
if with_libmagic.enabled() or with_libmagic.disabled()
    warning('libmagic option is obsolete. libmodulemd can detect compression formats without a magic library now. Please stop using this option. It will be removed in the future and will cause a meson failure.')
endif

with_rpmio = get_option('rpmio')
rpm = dependency('rpm', required : with_rpmio)

glib = dependency('glib-2.0')
glib_prefix = glib.get_variable(pkgconfig: 'prefix')

bash = find_program('bash')
sed = find_program('sed')
test = find_program('test')

with_docs = get_option('with_docs')
gtk_doc_referred_paths = []
if with_docs
  gtkdoc = dependency('gtk-doc')

  if glib.version().version_compare('<2.79.0')
    glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')
    modules = ['glib', 'gobject' ]
  elif glib.version().version_compare('<2.80.1')
    glib_docpath = join_paths(glib_prefix, 'share', 'doc', 'glib-2.0')
    modules = ['glib', 'gobject' ]
    warning('glib >= 2.79.0 documention might not be properly referred from libmodulemd documentation.')
  else
    glib_docpath = join_paths(glib_prefix, 'share', 'doc')
    modules = ['glib-2.0', 'gobject-2.0' ]
    warning('glib >= 2.79.0 documention might not be properly referred from libmodulemd documentation.')
  endif

  foreach referred_module : modules
    doc_module_path = join_paths(glib_docpath, referred_module)
    doc_index_file = join_paths(doc_module_path, 'index.html')
    ret = run_command ([test, '-e', doc_index_file],
      check: false)
    if ret.returncode() != 0
      error('Missing GTK documentation for @0@: @1@'.format(referred_module, doc_index_file))
    endif
    gtk_doc_referred_paths += [ doc_module_path ]
  endforeach
endif

# Keep with_manpages option a tristate feature for backward compatibility.
if get_option('with_manpages').disabled()
  with_manpages = false
else
  with_manpages = true
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

# Check whether we can use a new g_spawn_check_wait_status() from glib2.
has_g_spawn_check_wait_status = cc.has_function(
    'g_spawn_check_wait_status',
    dependencies : [ glib ])

with_py3 = get_option('with_py3')
if with_py3
    if get_option('skip_introspection')
        error('A Python3 binding requires a GObject introspection.')
    endif

    python_name = get_option('python_name')
    if python_name != ''
        # If we've been instructed to use a specific python version
        python3 = pymod.find_installation(python_name)
    else
        # Use the python installation that is running meson
        python3 = pymod.find_installation()
    endif

    # Verify Python version <https://github.com/mesonbuild/meson/issues/11057>.
    python3_version = python3.language_version()
    if not python3_version.startswith('3.')
      error('Python3 interpreter has an unexpected version: @0@'.format(python3_version))
    endif
else
    python3 = disabler()
endif

with_py2 = get_option('with_py2')
if with_py2
    if get_option('skip_introspection')
        error('A Python2 binding requires a GObject introspection.')
    endif
    python2 = pymod.find_installation('python2')

    # Verify Python version <https://github.com/mesonbuild/meson/issues/11057>.
    python2_version = python2.language_version()
    if not python2_version.startswith('2.')
      error('Python2 interpreter has an unexpected version:', python2_version)
    endif
else
    python2 = disabler()
endif

rpm_cdata = configuration_data()
rpm_cdata.set('VERSION', meson.project_version())
rpm_cdata.set('BUILDFLAG', '-bb')

srpm_cdata = configuration_data()
srpm_cdata.set('VERSION', meson.project_version())
srpm_cdata.set('BUILDFLAG', '-bs')

subdir('modulemd')
subdir('bindings/python')

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

if with_manpages
    manpages_status = 'Enabled'
else
    manpages_status = 'Disabled'
endif

summary({'prefix': get_option('prefix'),
         'bindir': get_option('bindir'),
         'libdir': get_option('libdir'),
         'datadir': get_option('datadir'),
         'Python 2 GObject Overrides': gobject_overrides_dir_py2,
         'Python 3 GObject Overrides': gobject_overrides_dir_py3,
         'GTK-Doc Referred Paths': gtk_doc_referred_paths,
        }, section: 'Directories')

summary({'Custom Python': get_option('python_name'),
         'RPMIO Support': rpmio_status,
         'Generate Manual Pages': manpages_status,
         'Generate HTML Documentation': get_option('with_docs'),
         'Python 2 Support': get_option('with_py2'),
         'Python 3 Support': get_option('with_py3'),
         'Skip Introspection': get_option('skip_introspection'),
         'Accept overflowed buildorder': get_option('accept_overflowed_buildorder'),
         'Test Installed Library': get_option('test_installed_lib'),
        }, section: 'Build Configuration')
