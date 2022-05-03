#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=sqlite-version-3.35.5

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    tar -zxf $SOURCE_DIR/$SOURCE_NAME.tar.gz -C $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src/$SOURCE_NAME \
&& ./configure --prefix=$SCRIPT_DIR \
&& make -j`nproc` \
&& make install \
&& cp sqlite3.c ../../lib \
&& popd
