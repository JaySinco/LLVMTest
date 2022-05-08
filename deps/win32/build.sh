#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"

$SCRIPT_DIR/dataset/build.sh && \
$SCRIPT_DIR/gflags/build.sh && \
$SCRIPT_DIR/glog/build.sh && \
$SCRIPT_DIR/fmt/build.sh && \
$SCRIPT_DIR/expected-lite/build.sh && \
$SCRIPT_DIR/zlib/build.sh && \
$SCRIPT_DIR/python3/build.sh && \
$SCRIPT_DIR/pybind11/build.sh && \
$SCRIPT_DIR/qt5/build.sh && \
$SCRIPT_DIR/glfw/build.sh && \
$SCRIPT_DIR/mujoco/build.sh && \
$SCRIPT_DIR/torch/build.sh && \
$SCRIPT_DIR/imgui/build.sh && \
$SCRIPT_DIR/implot/build.sh && \
$SCRIPT_DIR/boost/build.sh && \
echo done!
