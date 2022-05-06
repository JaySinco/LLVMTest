#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=libtorch

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    unzip -o $SOURCE_DIR/$SOURCE_NAME-cxx11-abi-shared-with-deps-1.8.2+cpu.zip -d $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src/$SOURCE_NAME \
&& mkdir -p -v ../../include \
&& cp -r -v ./include/* ../../include \
&& mkdir -p -v ../../lib \
&& cp -r -v ./lib/* ../../lib \
&& popd
