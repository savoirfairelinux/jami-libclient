#!/bin/bash

mkdir -p build
cd build
if [ -d scripts ]; then
  cd scripts
  svn up
  cd ..
else
  svn co svn://anonsvn.kde.org/home/kde/branches/stable/l10n-kde4/scripts
fi
cd ..
mkdir -p po
IGNORE="/build/" ./build/scripts/extract-messages.sh

