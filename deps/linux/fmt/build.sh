#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=fmt-8.1.1
CXX_FLAGS="-stdlib=libc++"
LINKER_FLAGS="-nostdlib++ -static-libgcc -fuse-ld=lld /usr/lib/llvm-13/lib/libc++.a /usr/lib/llvm-13/lib/libc++abi.a"

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    tar -zxf $SOURCE_DIR/$SOURCE_NAME.tar.gz -C $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src \
&& mkdir -p out \
&& pushd out \
&& cmake -G "Unix Makefiles" ../$SOURCE_NAME \
    -DCMAKE_INSTALL_PREFIX=$SCRIPT_DIR \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
    -DCMAKE_EXE_LINKER_FLAGS="$LINKER_FLAGS" \
    -DCMAKE_SHARED_LINKER_FLAGS="$LINKER_FLAGS" \
    -DBUILD_SHARED_LIBS=OFF \
    -DFMT_TEST=OFF \
&& make -j`nproc` \
&& make install \
&& popd \
&& popd
