#!/bin/sh

BUILD_NUM=`grep '^#define BUILD_NUM' < VersionStr.h | sed 's/^#define BUILD_NUM //'`
BUILD_NUM=`expr $BUILD_NUM + 1`

sed 's/^#define BUILD_NUM .*$/#define BUILD_NUM '${BUILD_NUM}'/' < VersionStr.h > /tmp/build.$$
mv /tmp/build.$$ VersionStr.h

exit 0