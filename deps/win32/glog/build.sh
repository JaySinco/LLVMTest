#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=glog-0.6.0

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    tar --force-local -zxf $SOURCE_DIR/$SOURCE_NAME.tar.gz -C $SCRIPT_DIR/src/
fi

source $PROJECT_ROOT/vcvars64.sh

pushd $SCRIPT_DIR/src \
&& mkdir -p out \
&& pushd out \
&& cmake -G "Ninja" ../$SOURCE_NAME \
    -DCMAKE_INSTALL_PREFIX=$SCRIPT_DIR \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL" \
    -DBUILD_SHARED_LIBS=OFF \
    -DWITH_THREADS=ON \
    -Dgflags_DIR=$SCRIPT_DIR/../gflags/lib/cmake/gflags \
&& ninja -j`nproc` \
&& ninja install \
&& popd \
&& popd
