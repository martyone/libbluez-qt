# 
# Do NOT Edit the Auto-generated Part!
# Generated by: spectacle version 0.27
# 

Name:       libbluez-qt5

# >> macros
# << macros

Summary:    Library for accessing bluetooth functionality in Qt
Version:    0.1.15
Release:    1
Group:      System/GUI/Other
License:    Apache License 2.0
URL:        https://github.com/nemomobile/libbluez-qt
Source0:    libbluez-qt5-%{version}.tar.bz2
Source100:  libbluez-qt5.yaml
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Test)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  doxygen

%description
This is a library for accessing bluetooth through bluez functionality in Qt.


%package devel
Summary:    Development files for bluez-qt
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
This package contains the files necessary to develop
applications using qt to access bluetooth devices using bluez


%package tests
Summary:    Test suite for libbluez-qt5
Group:      System/GUI/Other
Requires:   %{name} = %{version}-%{release}

%description tests
%{summary}.

%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
# << build pre

%qmake5 

make %{?jobs:-j%jobs}

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%qmake5_install

# >> install post
# << install post

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/libbluez-qt5.so.*
%{_libdir}/qt5/qml/Bluetooth/*
%{_sysconfdir}/dbus-1/system.d/bluez-qt.conf
# >> files
# << files

%files devel
%defattr(-,root,root,-)
%{_includedir}/bluez-qt5/*.h
%{_libdir}/libbluez-qt5.so
%{_libdir}/*.prl
%{_libdir}/pkgconfig/*.pc
%{_libdir}/*.pc
# >> files devel
# << files devel

%files tests
%defattr(-,root,root,-)
/opt/tests/libbluez-qt5/*
# >> files tests
# << files tests
