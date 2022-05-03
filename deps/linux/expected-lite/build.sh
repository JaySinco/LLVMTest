#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=expected-lite-0.5.0

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    tar -zxf $SOURCE_DIR/$SOURCE_NAME.tar.gz -C $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src/$SOURCE_NAME \
&& mkdir -p -v ../../include \
&& cp -r -v ./include/* ../../include \
&& popd
