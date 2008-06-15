%define name basket
%define version 1.0.3
%define release mantic0

Summary: Taking care of your ideas.
Name: %{name}
Version: %{version}
Release: %{release}
License: GPL
Vendor: slaout@linux62.org
Url: http://basket.kde.org/
Packager: packages@mantic.org
Group: Utils
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}

%description
Taking care of your ideas.
A note-taking application that makes it easy to write down ideas as you think, and quickly find them back later. Organizing your notes has never been so easy.

%prep
%setup
if [ "`kde-config --prefix`" ]; then
	CONFIGURE_PREFIX="`kde-config --prefix`"
elif [ "${KDEDIR}" ]; then
	CONFIGURE_PREFIX="${KDEDIR}"
elif [ -d "/opt/kde" ]; then
	CONFIGURE_PREFIX="/opt/kde"
elif [ -d "/opt/kde3" ]; then
	CONFIGURE_PREFIX="/opt/kde3"
else
	CONFIGURE_PREFIX="/usr"
fi
./configure --prefix="${CONFIGURE_PREFIX}" --enable-final

%build
%configure
#%make

%install
make install DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > /var/tmp/file.list.%{name}
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >>  /var/tmp/file.list.%{name}
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >>  /var/tmp/file.list.%{name}

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/%{name}-%{version}
rm -rf /var/tmp/file.list.%{name}

%files -f /var/tmp/file.list.%{name}
