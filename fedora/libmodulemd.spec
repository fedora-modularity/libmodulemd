%if  0%{?rhel} && 0%{?rhel} <= 7
  %global meson_python_flags -Dwith_py2=true
  %global build_python2 1
%else
  %global meson_python_flags -Dwith_py2=false
  %global build_python2 0
%endif

%global upstream_name libmodulemd

%if (0%{?rhel} && 0%{?rhel} <= 7)
  %global v2_suffix 2
%endif

# Rawhide builds currently fail when building with LTO
%define _lto_cflags %{nil}

Name:           %{upstream_name}%{?v2_suffix}
Version:        0.0.0
Release:        0
Summary:        Module metadata manipulation library

License:        MIT
URL:            https://github.com/fedora-modularity/libmodulemd
Source0:        %{url}/releases/download/%{upstream_name}-%{version}/modulemd-%{version}.tar.xz

BuildRequires:  meson >= 0.55
BuildRequires:  pkgconfig
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(gobject-2.0)
BuildRequires:  pkgconfig(gobject-introspection-1.0)
BuildRequires:  pkgconfig(yaml-0.1)
BuildRequires:  pkgconfig(gtk-doc)
BuildRequires:  glib2-doc
BuildRequires:  rpm-devel
%if %{build_python2}
BuildRequires:  python2-devel
BuildRequires:  python-gobject-base
%endif
BuildRequires:  python%{python3_pkgversion}-devel
BuildRequires:  python%{python3_pkgversion}-gobject-base


# Patches


%description
C Library for manipulating module metadata files.
See https://github.com/fedora-modularity/libmodulemd/blob/main/README.md for
more details.


%if %{build_python2}
%package -n python2-%{name}
Summary: Python 2 bindings for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: python-gobject-base
Requires: python-six

%description -n python2-%{name}
Python 2 bindings for %{name}
%endif


%package -n python%{python3_pkgversion}-%{name}
Summary: Python 3 bindings for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: python%{python3_pkgversion}-gobject-base

%if (0%{?rhel} && 0%{?rhel} <= 7)
# The py3_dist macro on EPEL 7 doesn't work right at the moment
Requires: python3.6dist(six)
%else
Requires: %{py3_dist six}
%endif

%description -n python%{python3_pkgversion}-%{name}
Python %{python3_pkgversion} bindings for %{name}


%package devel
Summary:        Development files for libmodulemd
Requires:       %{name}%{?_isa} = %{version}-%{release}
%if (0%{?rhel} && 0%{?rhel} <= 7)
Conflicts:      libmodulemd1-devel
Conflicts:      libmodulemd-devel
%endif


%description devel
Development files for libmodulemd.


%prep
%autosetup -p1 -n modulemd-%{version}


%build
%meson %{meson_python_flags}

%meson_build


%check

export LC_CTYPE=C.utf8

# Don't run tests on ARM for now. There are problems with
# performance on the builders and often these time out.
%ifnarch %{arm}
# The tests sometimes time out in CI, so give them a little extra time
%{__meson} test -C %{_vpath_builddir} %{?_smp_mesonflags} --print-errorlogs -t 5
%endif


%install
%meson_install

%if ( 0%{?rhel} && 0%{?rhel} <= 7)
# Don't conflict with modulemd-validator from 1.x included in the official
# RHEL 7 repos
mv %{buildroot}%{_bindir}/modulemd-validator \
   %{buildroot}%{_bindir}/modulemd-validator%{?v2_suffix}

mv %{buildroot}%{_mandir}/man1/modulemd-validator.1 \
   %{buildroot}%{_mandir}/man1/modulemd-validator%{?v2_suffix}.1
%endif


%ldconfig_scriptlets


%files
%license COPYING
%doc NEWS README.md
%{_bindir}/modulemd-validator%{?v2_suffix}
%{_mandir}/man1/modulemd-validator%{?v2_suffix}.1*
%{_libdir}/%{upstream_name}.so.2*
%dir %{_libdir}/girepository-1.0
%{_libdir}/girepository-1.0/Modulemd-2.0.typelib


%files devel
%{_libdir}/%{upstream_name}.so
%{_libdir}/pkgconfig/modulemd-2.0.pc
%{_includedir}/modulemd-2.0/
%dir %{_datadir}/gir-1.0
%{_datadir}/gir-1.0/Modulemd-2.0.gir
%dir %{_datadir}/gtk-doc
%dir %{_datadir}/gtk-doc/html
%{_datadir}/gtk-doc/html/modulemd-2.0/


%if %{build_python2}
%files -n python2-%{name}
%{python2_sitearch}/gi/overrides/
%endif


%files -n python%{python3_pkgversion}-%{name}
%{python3_sitearch}/gi/overrides/


%changelog
