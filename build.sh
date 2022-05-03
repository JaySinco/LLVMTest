#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            echo
            echo "Usage: build.sh [options]"
            echo
            echo "Options:"
            echo "  -i, --init            init host env"
            echo "  -r, --restore         restore host env"
            echo "  -t, --target [...]    target to be built"
            echo "  -c, --clean           make clean"
            echo "  -h, --help            print command line options (currently set)"
            echo
            exit 0
            ;;
        -i|--init)
            BUILD_INIT=ON
            shift
            ;;
        -r|--restore)
            BUILD_RESTORE=ON
            shift
            ;;
        -t|--target)
            BUILD_TARGET="$2"
            shift
            shift
            ;;
        -c|--clean)
            BUILD_CLEAN=ON
            shift
            ;;
        -*|--*)
            echo "Unknown option: $1"
            exit 1
            ;;
        *)
            POSITIONAL_ARGS+=("$1")
            shift
            ;;
    esac
done

if [ ! -z ${BUILD_INIT+x} ]; then
    xhost +local:docker
    mkdir -p $PROJECT_ROOT/deps/src
    sudo mount -t vboxsf -o defaults,uid=$(id -u),gid=$(id -g) share $PROJECT_ROOT/deps/src
    exit 0
fi

if [ ! -z ${BUILD_RESTORE+x} ]; then
    xhost -local:docker
    sudo umount -a -t vboxsf
    exit 0
fi

if [ ! -z ${BUILD_CLEAN+x} ]; then
    pushd $PROJECT_ROOT/out && make clean && popd
    exit 0
fi

pushd $PROJECT_ROOT \
&& find src -iname *.h -or -iname *.cpp | xargs clang-format -i \
&& mkdir -p out \
&& pushd out \
&& cmake -G "Unix Makefiles" .. \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$PROJECT_ROOT/bin \
    -DCMAKE_BUILD_TYPE=debug \
&& make -j`nproc` $BUILD_TARGET \
&& popd \
&& popd
