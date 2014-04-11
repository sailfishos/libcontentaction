Name:       libcontentaction-qt5
Summary:    Library for associating content with actions
Version:    0.2.0
Release:    1
Group:      System/Desktop
License:    LGPLv2.1
URL:        https://github.com/nemomobile/libcontentaction
Source0:    %{name}-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Test)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5SystemInfo)
BuildRequires:  python
BuildRequires:  qt5-qttools-linguist

%description
libcontentaction is a library for associating content with actions.


%package devel
Summary:    Development files for libcontentaction
Group:      Development/System
Requires:   %{name} = %{version}-%{release}

%description devel
This package contains development files for building applications using
libcontentaction library.


%package tests
Summary:    Tests for libcontentaction
Group:      System/X11
Requires:   %{name} = %{version}-%{release}
Requires:   dbus-python
Requires:   pygobject2
Requires:   python
Requires:   tracker-utils
Requires:   qt5-default
Requires:   qt5-qttools-qdbus

%description tests
This package contains the tests for libcontentaction library.


%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_bindir}/lca-tool
%{_datadir}/contentaction/highlight1.xml
%{_datadir}/contentaction/tracker1.xml
%{_libdir}/libcontentaction5.so.*
%exclude %{_datadir}/applications/defaults.list

%files devel
%defattr(-,root,root,-)
%{_includedir}/contentaction5/contentaction.h
%{_includedir}/contentaction5/contentinfo.h
%{_libdir}/libcontentaction5.so
%{_libdir}/pkgconfig/contentaction5.pc

%files tests
%defattr(-,root,root,-)
%attr(0755, root, root) /opt/tests/libcontentaction5/bin/lca-cita-test
/opt/tests/libcontentaction5/*
