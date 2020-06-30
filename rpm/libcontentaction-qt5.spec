Name:       libcontentaction-qt5
Summary:    Library for associating content with actions
Version:    0.3.5
Release:    1
License:    LGPLv2
URL:        https://git.sailfishos.org/mer-core/libcontentaction
Source0:    %{name}-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Test)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5SystemInfo)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  python3-base
BuildRequires:  qt5-qttools-linguist

%description
libcontentaction is a library for associating content with actions.


%package devel
Summary:    Development files for libcontentaction
Requires:   %{name} = %{version}-%{release}

%description devel
This package contains development files for building applications using
libcontentaction library.


%package tests
Summary:    Tests for libcontentaction
Requires:   %{name} = %{version}-%{release}
Requires:   dbus-python3
Requires:   python3-gobject
Requires:   python3-base
Requires:   tracker-utils
Requires:   qt5-qttools-qdbus

%description tests
This package contains the tests for libcontentaction library.


%package -n nemo-qml-plugin-contentaction
Summary:  Content Action QML plugin
Requires: %{name} = %{version}-%{release}

%description -n nemo-qml-plugin-contentaction
This package contains the Content Action QML plugin.


%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 "VERSION=%{version}"
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_bindir}/lca-tool
%dir %{_datadir}/contentaction
%{_datadir}/contentaction/highlight1.xml
%{_datadir}/contentaction/tracker1.xml
%{_libdir}/libcontentaction5.so.*
%{_sysconfdir}/dconf/db/vendor.d/locks/application_desktop_paths.txt
%license COPYING

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/contentaction5
%{_includedir}/contentaction5/contentaction.h
%{_includedir}/contentaction5/contentinfo.h
%{_libdir}/libcontentaction5.so
%{_libdir}/pkgconfig/contentaction5.pc

%files tests
%defattr(-,root,root,-)
%attr(0755, root, root) /opt/tests/libcontentaction5/bin/lca-cita-test
/opt/tests/libcontentaction5/*

%files -n nemo-qml-plugin-contentaction
%defattr(-,root,root,-)
%{_libdir}/qt5/qml/org/nemomobile/contentaction
