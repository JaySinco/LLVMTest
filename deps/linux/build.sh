#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"

$SCRIPT_DIR/gflags/build.sh && \
$SCRIPT_DIR/glog/build.sh && \
$SCRIPT_DIR/fmt/build.sh && \
$SCRIPT_DIR/expected-lite/build.sh && \
$SCRIPT_DIR/openssl/build.sh && \
$SCRIPT_DIR/zlib/build.sh && \
echo done!
