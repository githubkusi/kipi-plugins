#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp
$XGETTEXT *.cpp -o $podir/kipiplugin_picasawebexport.pot
rm -f rc.cpp
