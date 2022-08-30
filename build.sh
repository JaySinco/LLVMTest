#!/bin/bash

set -e

case "$OSTYPE" in
    linux*)   PLATFORM=linux ;;
    msys*)    PLATFORM=win32 ;;
esac

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
DOCKER_IMAGE_TAG=build:v1
DOCKER_PROJECT_DIR=/workspace
BUILD_TYPE=debug
BUILD_TARGETS=()

if [ $PLATFORM = "linux" ]; then
    BUILD_GENERATOR="Unix Makefiles"
    BUILD_PROGRAM="make"
    BUILD_C_COMPILER="clang"
    BUILD_CXX_COMPILER="clang++"
elif [ $PLATFORM = "win32" ]; then
    BUILD_GENERATOR="Ninja"
    BUILD_PROGRAM="ninja"
    BUILD_C_COMPILER="clang-cl"
    BUILD_CXX_COMPILER="clang-cl"
    BUILD_LINKER="lld-link"
    source $PROJECT_ROOT/vcvars64.sh
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            echo
            echo "Usage: build.sh [options] [targets]"
            echo
            echo "Options:"
            echo "  -b, --build              build image from dockerfile"
            echo "  -r, --run                run dev container"
            echo "      --mount              mount vbox share"
            echo "      --umount             unmount vbox share"
            echo "  -u, --edit-ui            edit qt ui files"
            echo "  -q, --qt-doc             open qt assistant"
            echo "  -c, --clean              clean targets built"
            echo "      --release            build release version"
            echo "  -f, --force-config       remove cmake cache before build"
            echo "  -h, --help               print command line options"
            echo
            exit 0
            ;;
        -b|--build)
            docker build --build-arg PROJECT_DIR=$DOCKER_PROJECT_DIR \
                -f $PROJECT_ROOT/Dockerfile \
                -t $DOCKER_IMAGE_TAG \
                $PROJECT_ROOT/deps
            exit 0
            ;;
        -r|--run)
            docker run -it --rm \
                -v /home/$USER/.ssh:/root/.ssh:ro \
                -v /home/$USER/.config/nvim:/root/.config/nvim:rw \
                -v /home/$USER/.local/share/nvim:/root/.local/share/nvim:rw \
                -v $PROJECT_ROOT:$DOCKER_PROJECT_DIR:rw \
                $DOCKER_IMAGE_TAG
            exit 0
            ;;
        --mount)
            mkdir -p $PROJECT_ROOT/deps/src
            sudo mount -t vboxsf -o ro,uid=$(id -u),gid=$(id -g) \
                share $PROJECT_ROOT/deps/src
            exit 0
            ;;
        --umount)
            sudo umount -a -t vboxsf
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
&& ls -AU1 deps/src > deps/.src.lst \
&& find src -iname *.h -or -iname *.cpp | xargs clang-format -i \
&& mkdir -p out \
&& pushd out \
&& cmake -G "$BUILD_GENERATOR" .. \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$PROJECT_ROOT/bin \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$BUILD_C_COMPILER \
    -DCMAKE_CXX_COMPILER=$BUILD_CXX_COMPILER \
    -DCMAKE_LINKER=$BUILD_LINKER \
    -DTARGET_OS=$PLATFORM \
&& cp compile_commands.json $PROJECT_ROOT \
&& $BUILD_PROGRAM -j`nproc` ${BUILD_TARGETS[*]} \
&& echo done!
