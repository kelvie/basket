%define name basket
%define version 0.6.0Beta2.2
%define release mantic0

Summary: A set of baskets to keep a full range of data on hand.
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
A set of baskets to keep a full range of data on hand.
It offer virtual baskets where you can drop or paste every sort of content:
text, rich text, image, animation, sound, file, link, app launcher, color...
and you can edit, arrange or drag off those items.
It's useful to take notes (not only text ones), but also to collect a lot of
data (from internet, documents, conversations...) and its HTML exportation
feature allow you to present the result of your search to everybody else...

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
