gobject_overrides_dir_py3 = get_option('gobject_overrides_dir_py3')
gobject_overrides_dir_py2 = get_option('gobject_overrides_dir_py2')

# Python 3
if with_py3
  if gobject_overrides_dir_py3 == ''
    ret = run_command([python3, '-c', 'import gi; print(gi._overridesdir)'],
      check: false)

    if ret.returncode() != 0
      error('Failed to determine Python 3 pygobject overridedir: @0@'.format(ret.stderr()))
    else
      gobject_overrides_dir_py3 = ret.stdout().strip()
    endif
  endif

  install_data('gi/overrides/Modulemd.py', install_dir: gobject_overrides_dir_py3)
endif

# Python 2
if with_py2
  if gobject_overrides_dir_py2 == ''
    ret2 = run_command([python2, '-c', 'import gi; print(gi._overridesdir)'],
      check: false)

    if ret2.returncode() != 0
      error('Failed to determine Python 2 pygobject overridedir: @0@'.format(ret2.stderr()))
    else
      gobject_overrides_dir_py2 = ret2.stdout().strip()
    endif
  endif

  install_data('gi/overrides/Modulemd.py', install_dir: gobject_overrides_dir_py2)
endif
