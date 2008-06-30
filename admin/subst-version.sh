#!/bin/sh

version=$($admindir/git-get-version)

files="basket.spec
Doxyfile
basket.kdevelop
configure.in.in
doc/en/index.docbook
kontact_plugin/basket.desktop
kontact_plugin/basket_v4.desktop
src/basket.lsm
src/basket_part.rc
src/basketui.rc"

sed -i 's/\$\$version_number\$\$/'$version'/g'  $files
