#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=libtorch

if [ -d $SCRIPT_DIR/include ]; then
    echo "-- skip build $SOURCE_NAME" && exit 0
fi

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    unzip -q -o $SOURCE_DIR/$SOURCE_NAME-win-shared-with-deps-1.8.2+cu111.zip -d $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src/$SOURCE_NAME \
&& mv ./include/ ../../include \
&& mv ./lib/ ../../lib \
&& popd
