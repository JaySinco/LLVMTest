#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=qt-5.15.3

if [ -d $SCRIPT_DIR/include ]; then
    echo "-- skip build $SOURCE_NAME" && exit 0
fi

mkdir -p $SCRIPT_DIR/src

# qtbase
# ----------------------
QT_BASE_NAME=qtbase-everywhere-src-5.15.3

if [ ! -d $SCRIPT_DIR/src/$QT_BASE_NAME ]; then
    tar -xf $SOURCE_DIR/qtbase-everywhere-opensource-src-5.15.3.tar.xz -C $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src/$QT_BASE_NAME \
&& ./configure --prefix=$SCRIPT_DIR --extprefix=$SCRIPT_DIR --hostprefix=$SCRIPT_DIR \
    --confirm-license -opensource --static --release --c++std=c++17 \
&& make -j`nproc` \
&& make install \
&& popd

# qttools
# ----------------------
QT_TOOLS_NAME=qttools-everywhere-src-5.15.3

if [ ! -d $SCRIPT_DIR/src/$QT_TOOLS_NAME ]; then
    tar -xf $SOURCE_DIR/qttools-everywhere-opensource-src-5.15.3.tar.xz -C $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src/$QT_TOOLS_NAME \
&& $SCRIPT_DIR/bin/qmake \
&& make -j`nproc` \
&& make install \
&& popd
