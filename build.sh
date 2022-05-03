#!/bin/bash

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
            echo "  -m, --mount           mount vbox share folder"
            echo "  -u, --umount          umount vbox share folder"
            echo "  -t, --target [...]    target to be built"
            echo "  -h, --help            print command line options (currently set)"
            echo
            exit 0
            ;;
        -m|--mount)
            MOUNT_VBOX_SHARE=ON
            shift
            ;;
        -u|--umount)
            UMOUNT_VBOX_SHARE=ON
            shift
            ;;
        -t|--target)
            BUILD_TARGET="$2"
            shift
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

if [ ! -z ${MOUNT_VBOX_SHARE+x} ]; then
    mkdir -p $PROJECT_ROOT/deps/src
    sudo mount -t vboxsf -o defaults,uid=$(id -u),gid=$(id -g) share $PROJECT_ROOT/deps/src
    exit 0
fi

if [ ! -z ${UMOUNT_VBOX_SHARE+x} ]; then
    sudo umount -a -t vboxsf
    exit 0
fi

pushd $PROJECT_ROOT
find src -iname *.h -or -iname *.cpp | xargs clang-format -i
mkdir -p out
pushd out
cmake -G "Unix Makefiles" .. \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$PROJECT_ROOT/bin \
    -DCMAKE_BUILD_TYPE=debug \
&& make -j`nproc`
popd
popd
