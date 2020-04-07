# Python 3
if gobject_overrides_dir_py3 == ''
  ret = run_command([python3, '-c', 'import gi; print(gi._overridesdir)'])

if ret.returncode() != 0
  error('Failed to determine Python 3 pygobject overridedir')
else
  pygobject_override_dir = ret.stdout().strip()
endif

install_data('gi/overrides/Modulemd.py', install_dir: pygobject_override_dir)

# Python 2
if with_py2
  ret2 = run_command([python2, '-c', 'import gi; print(gi._overridesdir)'])

  if ret2.returncode() != 0
    error('Failed to determine Python 2 pygobject overridedir')
  else
    pygobject2_override_dir = ret2.stdout().strip()
  endif

  install_data('gi/overrides/Modulemd.py', install_dir: pygobject2_override_dir)
endif
