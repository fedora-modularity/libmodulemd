get_overridedir = '''
import os
import sysconfig

libdir = sysconfig.get_config_var('SCRIPTDIR')

if not libdir:
  libdir = '/usr/lib'

try:
  import gi
  overridedir = gi._overridesdir
except ImportError:
  purelibdir = sysconfig.get_path('purelib')
  overridedir = os.path.join(purelibdir, 'gi', 'overrides')

if overridedir.startswith(libdir): # Should always be True..
  overridedir = overridedir[len(libdir) + 1:]

print(overridedir)
'''

# Python 3
ret = run_command([python3, '-c', get_overridedir])

if ret.returncode() != 0
  error('Failed to determine Python 3 pygobject overridedir')
else
  pygobject_override_dir = join_paths(get_option('libdir'), ret.stdout().strip())
endif

install_data('gi/overrides/Modulemd.py', install_dir: pygobject_override_dir)

# Python 2
if with_py2
  ret2 = run_command([python2, '-c', get_overridedir])

  if ret2.returncode() != 0
    error('Failed to determine Python 2 pygobject overridedir')
  else
    pygobject2_override_dir = join_paths(get_option('libdir'), ret2.stdout().strip())
  endif

  install_data('gi/overrides/Modulemd.py', install_dir: pygobject2_override_dir)
endif
