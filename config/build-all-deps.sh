#!/bin/bash

set -e

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
git_root="$(git rev-parse --show-toplevel)"

function package() {
    local build_debug=$1
    local name=$2
    $git_root/recipes/build.sh $name -r && \
    if [ $build_debug -eq 1 ]; then
        $git_root/recipes/build.sh $name -r -d
    fi
}

echo start! \
&& package 1 gflags \
&& package 1 glog \
&& package 1 gtest \
&& package 1 fmt \
&& package 1 spdlog \
&& package 0 boost \
&& package 1 glfw \
&& package 1 imgui \
&& package 1 implot \
&& package 1 catch2 \
&& package 0 qt \
&& package 0 expected-lite \
&& package 1 qhull \
&& package 1 lodepng \
&& package 1 libccd \
&& package 1 tinyobjloader \
&& package 1 tinyxml2 \
&& package 1 mujoco \
&& package 0 torch \
&& package 1 double-conversion \
&& package 1 bzip2 \
&& package 1 zlib \
&& package 0 openssl \
&& package 1 libevent \
&& package 1 zstd \
&& package 1 lz4 \
&& package 1 snappy \
&& package 0 libxl \
&& package 0 argparse \
&& package 0 range-v3 \
&& package 1 libiconv \
&& package 1 raylib \
&& package 0 nlohmann-json \
&& package 1 sdl \
&& package 1 libuv \
&& package 1 usockets \
&& package 0 uwebsockets \
&& package 1 libcurl \
&& package 1 cpr \
&& package 1 sqlite3 \
&& package 1 mongoose \
&& package 0 concurrentqueue \
&& package 0 threadpool \
&& package 1 libusb \
&& package 1 libpcap \
&& echo done!
