#!/bin/bash

autoreconf --force --verbose --install

# AX_SUBDIRS_CONFIGURE does not trigger subdir autoreconf, unlike 
# AC_CONFIG_SUBDIRS, so do it manually
pushd src/3rdparty/osu-micro-benchmarks
autoreconf --force --verbose --install
popd
