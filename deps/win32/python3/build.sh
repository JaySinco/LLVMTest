#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=cpython-3.8.10

if [ -d $SCRIPT_DIR/include ]; then
    echo "-- skip build $SOURCE_NAME" && exit 0
fi

mkdir -p $SCRIPT_DIR/src
if [ ! -d $SCRIPT_DIR/src/$SOURCE_NAME ]; then
    tar --force-local -zxf $SOURCE_DIR/$SOURCE_NAME.tar.gz -C $SCRIPT_DIR/src/
    unzip -q -o $SOURCE_DIR/$SOURCE_NAME-externals-win-x64.zip -d $SCRIPT_DIR/src/$SOURCE_NAME/
fi

pushd $SCRIPT_DIR/src/$SOURCE_NAME/PCbuild \
&& ./build.bat -p x64 \
&& mkdir -p -v $SCRIPT_DIR/include/python3.8 \
&& cp -r -v ../Include/* ../PC/pyconfig.h $SCRIPT_DIR/include/python3.8 \
&& mkdir -p -v $SCRIPT_DIR/lib \
&& cp -r -v amd64/python38.lib $SCRIPT_DIR/lib \
&& mkdir -p -v $SCRIPT_DIR/bin \
&& cp -r -v amd64/python38.dll amd64/python.exe $SCRIPT_DIR/bin \
&& printf "./DLLs\n./Lib\n./Lib/site-packages\n.\n" > $SCRIPT_DIR/bin/python38._pth \
&& mkdir -p -v $SCRIPT_DIR/bin/DLLs \
&& cp -r -v amd64/*.pyd amd64/*.dll $SCRIPT_DIR/bin/DLLs \
&& pushd $SCRIPT_DIR/bin/DLLs \
&& rm -f vcruntime140.dll vcruntime140_1.dll python3.dll python38.dll \
&& popd \
&& mkdir -p -v $SCRIPT_DIR/bin/Lib \
&& cp -r ../Lib/* $SCRIPT_DIR/bin/Lib \
&& $SCRIPT_DIR/bin/python.exe $SCRIPT_DIR/get-pip.py \
&& popd
