#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=cpython-3.8.10

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    tar --force-local -zxf $SOURCE_DIR/$SOURCE_NAME.tar.gz -C $SCRIPT_DIR/src/
    unzip -q -o $SOURCE_DIR/$SOURCE_NAME-externals-win-x64.zip -d $SCRIPT_DIR/src/$SOURCE_NAME/
fi

pushd $SCRIPT_DIR/src/$SOURCE_NAME/PCbuild \
&& ./build.bat -p x64 \
&& popd
