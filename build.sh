#!/bin/bash

set -e

rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -v
cd tests
ctest
cd ../checks
# make sure the linking is okay (no rpath to private libs etc.):
readelf -d check_zpool
readelf -d check_zpool_free
readelf -d check_smart