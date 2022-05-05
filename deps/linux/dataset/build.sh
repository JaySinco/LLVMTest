#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
SOURCE_DIR=$PROJECT_ROOT/deps/src
SOURCE_NAME=zlib-1.2.12

mkdir -p $SCRIPT_DIR/lib \
&& cp $SOURCE_DIR/t10k-labels-idx1-ubyte.gz \
    $SOURCE_DIR/t10k-images-idx3-ubyte.gz \
    $SOURCE_DIR/train-labels-idx1-ubyte.gz \
    $SOURCE_DIR/train-images-idx3-ubyte.gz \
    $SCRIPT_DIR/lib \
&& gzip -d $SCRIPT_DIR/lib/*.gz
