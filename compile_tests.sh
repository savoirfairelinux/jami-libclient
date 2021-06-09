#!/bin/bash
# Build lrc, client-qt and pass tests

# Get number of CPU available
cpuCount=$(nproc || echo -n 4)

# Project directories
topDir=$(pwd)/..
echo "Project root dir: "${topDir}

installDir=$topDir/install
daemonDir=$topDir/daemon
lrcDir=$topDir/lrc
clientDir=$topDir/client-qt

# Build lrc
cd ${lrcDir}
mkdir -p build
cd build
echo "Building lrc in "$PWD
cmake .. -DCMAKE_INSTALL_PREFIX=$installDir/lrc \
      -DRING_INCLUDE_DIR=$daemonDir/src/jami \
      -DRING_XML_INTERFACES_DIR=$daemonDir/bin/dbus
make -j${cpuCount}
make install

# Build client and tests
cd $clientDir
mkdir -p build
cd build
echo "Building client in "$PWD
cmake ..
make -j${cpuCount}

# Pass Tests
cd tests
./unittests
./qml_tests -input $clientDir/tests/qml
