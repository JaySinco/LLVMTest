#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=cpython-3.8.10

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    tar -zxf $SOURCE_DIR/$SOURCE_NAME.tar.gz -C $SCRIPT_DIR/src/
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SCRIPT_DIR/../libffi/lib
pushd $SCRIPT_DIR/src/$SOURCE_NAME \
&& ./configure --prefix=$SCRIPT_DIR --enable-optimizations \
    --with-openssl=$SCRIPT_DIR/../openssl \
    CPPFLAGS="-I$SCRIPT_DIR/../zlib/include -I$SCRIPT_DIR/../sqlite3/include -I$SCRIPT_DIR/../libffi/include" \
    LDFLAGS="-L$SCRIPT_DIR/../zlib/lib -L$SCRIPT_DIR/../sqlite3/lib -L$SCRIPT_DIR/../libffi/lib" \
&& make -j`nproc` \
&& make install \
&& popd
