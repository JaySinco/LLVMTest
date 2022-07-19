#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src

mkdir -p $SCRIPT_DIR/src
source $PROJECT_ROOT/vcvars64.sh

# qtbase
# ----------------------
QT_BASE_NAME=qtbase-everywhere-src-5.15.3

if [ ! -d $SCRIPT_DIR/src/$QT_BASE_NAME ]; then
    tar --force-local -xf $SOURCE_DIR/qtbase-everywhere-opensource-src-5.15.3.tar.xz -C $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src/$QT_BASE_NAME \
&& ./configure.bat --prefix=$SCRIPT_DIR --extprefix=$SCRIPT_DIR --hostprefix=$SCRIPT_DIR \
    --confirm-license -opensource --static --release --opengl=desktop --c++std=c++17 \
&& jom -j`nproc` \
&& jom install \
&& popd

# qttools
# ----------------------
QT_TOOLS_NAME=qttools-everywhere-src-5.15.3
export LLVM_INSTALL_DIR="C:\Program Files\LLVM"

if [ ! -d $SCRIPT_DIR/src/$QT_TOOLS_NAME ]; then
    tar --force-local -xf $SOURCE_DIR/qttools-everywhere-opensource-src-5.15.3.tar.xz -C $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src/$QT_TOOLS_NAME \
&& $SCRIPT_DIR/bin/qmake \
&& jom -j`nproc` \
&& jom install \
&& popd

# qch-docs
# ----------------------
pushd $SCRIPT_DIR/src/$QT_BASE_NAME \
&& jom docs \
&& jom install_qch_docs \
&& pushd $SCRIPT_DIR/src/$QT_TOOLS_NAME \
&& jom docs \
&& jom install_qch_docs
