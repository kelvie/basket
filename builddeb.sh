#!/bin/sh
ln -sf debian-upstream debian
cat debian/changelog.in | sed "s/##DATE##/`date +%Y%m%d`/g" | sed "s/##RDATE##/`date -R`/g" | sed "s/##DIST##/`lsb_release -cs`/g" > debian/changelog
debuild -b
fakeroot debian/rules clean
rm debian/changelog
rm debian
