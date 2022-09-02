#!/bin/bash

set -e

case "$(uname -m)" in
    x86_64)   ARCH=x64 ;;
esac

case "$OSTYPE" in
    linux*)   PLATFORM=linux ;;
    msys*)    PLATFORM=win32 ;;
esac

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
DOCKER_WORKSPACE_DIR=/workspace
DOCKER_IMAGE_TAG=build:v1
BUILD_TYPE=debug
BUILD_TARGETS=()
CONAN_PROFILE="$PROJECT_ROOT/config/$PLATFORM/$ARCH/conan.profile"

if [ $PLATFORM = "linux" ]; then
    BUILD_C_COMPILER="clang"
    BUILD_CXX_COMPILER="clang++"
elif [ $PLATFORM = "win32" ]; then
    BUILD_C_COMPILER="cl"
    BUILD_CXX_COMPILER="cl"
    source $PROJECT_ROOT/vcvars64.sh
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            echo
            echo "Usage: build.sh [options] [targets]"
            echo
            echo "Options:"
            echo "  -r, --docker-run      run dev container"
            echo "  -u, --edit-ui         edit qt ui files"
            echo "  -q, --qt-doc          open qt assistant"
            echo "  -c, --clean           clean targets built"
            echo "      --release         build release version"
            echo "  -f, --force-config    remove cmake cache before build"
            echo "  -h, --help            print command line options"
            echo
            exit 0
            ;;
        -r|--docker-run)
            docker run -it --rm \
                -v $HOME/.ssh:/root/.ssh:ro \
                -v $HOME/.config/nvim:/root/.config/nvim:rw \
                -v $HOME/.local/share/nvim:/root/.local/share/nvim:rw \
                -v $PROJECT_ROOT:$DOCKER_WORKSPACE_DIR:rw \
                $DOCKER_IMAGE_TAG
            exit 0
            ;;
        -u|--edit-ui)
            $PROJECT_ROOT/deps/$PLATFORM/qt5/bin/designer \
                $PROJECT_ROOT/src/qt5/*.ui &
            exit 0
            ;;
        -q|--qt-doc)
            $PROJECT_ROOT/deps/$PLATFORM/qt5/bin/assistant &
            exit 0
            ;;
        --release)
            BUILD_TYPE=release
            shift
            ;;
        -f|--force-config)
            rm -f $PROJECT_ROOT/out/CMakeCache.txt
            shift
            ;;
        -c|--clean)
            pushd $PROJECT_ROOT/out && $BUILD_PROGRAM clean && popd
            exit 0
            ;;
        -*|--*)
            echo "Unknown option: $1"
            exit 1
            ;;
        *)
            BUILD_TARGETS+=("$1")
            shift
            ;;
    esac
done

echo "[settings]"
echo "platform=$PLATFORM"
echo "build_type=$BUILD_TYPE"
echo "compiler=$BUILD_CXX_COMPILER"
echo

pushd $PROJECT_ROOT \
&& find src -iname *.h -or -iname *.cpp | xargs clang-format -i \
&& conan install \
    --profile=$CONAN_PROFILE\
    --install-folder=$PROJECT_ROOT/out \
    --build=never \
    . \
&& mkdir -p out \
&& pushd out \
&& cmake -G "Ninja" .. \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$PROJECT_ROOT/bin \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$BUILD_C_COMPILER \
    -DCMAKE_CXX_COMPILER=$BUILD_CXX_COMPILER \
    -DTARGET_OS=$PLATFORM \
&& cp compile_commands.json $PROJECT_ROOT \
&& ninja -j`nproc` ${BUILD_TARGETS[*]} \
&& echo done!
