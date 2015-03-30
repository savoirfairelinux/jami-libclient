#! /bin/sh

$EXTRACT_TR_STRINGS `find . -path ./build -prune -o -name '*.cpp'` -o $podir/libringclient_qt.pot
