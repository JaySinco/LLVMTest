#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=boost_1_79_0

if [ -d $SCRIPT_DIR/include ]; then
    echo "-- skip build $SOURCE_NAME" && exit 0
fi

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    tar -zxf $SOURCE_DIR/$SOURCE_NAME.tar.gz -C $SCRIPT_DIR/src/
fi

pushd $SCRIPT_DIR/src/$SOURCE_NAME \
&& ./bootstrap.sh --without-libraries=python --with-toolset=gcc \
&& ./b2 --prefix=$SCRIPT_DIR toolset=gcc \
    variant=release link=static runtime-link=shared \
    architecture=x86 address-model=64 threading=multi \
    install \
&& popd
