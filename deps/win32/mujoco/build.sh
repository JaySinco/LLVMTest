#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=mujoco-2.1.5

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    unzip -o $SOURCE_DIR/$SOURCE_NAME-windows-x86_64.zip -d $SCRIPT_DIR/src/$SOURCE_NAME
fi

pushd $SCRIPT_DIR/src/$SOURCE_NAME \
&& mkdir -p -v ../../include \
&& cp -r -v ./include/* ../../include \
&& mkdir -p -v ../../lib \
&& cp -r -v ./lib/* ./bin/*.dll ../../lib \
&& popd
