#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
DOCKER_IMAGE_TAG=build:v1
DOCKER_PROJECT=/workspace
POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            echo
            echo "Usage: build.sh [options]"
            echo
            echo "Options:"
            echo "  -b, --docker-build       build docker"
            echo "  -r, --docker-run         run docker"
            echo "      --vbox-mount         mount vbox share"
            echo "      --vbox-umount        unmount vbox share"
            echo "  -t, --target [...]       target to be built"
            echo "  -c, --clean              clean target"
            echo "  -h, --help               print command line options (currently set)"
            echo
            exit 0
            ;;
        -b|--docker-build)
            DOCKER_BUILD=ON
            shift
            ;;
        -r|--docker-run)
            DOCKER_RUN=ON
            shift
            ;;
        --vbox-mount)
            VBOX_MOUNT=ON
            shift
            ;;
        --vbox-umount)
            VBOX_UNDO_MOUNT=ON
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

if [ ! -z $DOCKER_BUILD ]; then
    docker build --build-arg project=$DOCKER_PROJECT \
        -f $PROJECT_ROOT/Dockerfile \
        -t $DOCKER_IMAGE_TAG \
        $PROJECT_ROOT/.vscode
    exit 0
elif [ ! -z $DOCKER_RUN ]; then
    xhost +local:docker > /dev/null
    docker run -it --rm \
        -e DISPLAY \
        -e XMODIFIERS="@im=fcitx" \
        -e QT_IM_MODULE="fcitx" \
        -e GTK_IM_MODULE="fcitx" \
        -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
        -v /home/$USER/.ssh:/root/.ssh:ro \
        -v $PROJECT_ROOT:$DOCKER_PROJECT \
        $DOCKER_IMAGE_TAG
    xhost -local:docker > /dev/null
    exit 0
elif [ ! -z $VBOX_MOUNT ]; then
    mkdir -p $PROJECT_ROOT/deps/src
    sudo mount -t vboxsf -o defaults,uid=$(id -u),gid=$(id -g) \
        share $PROJECT_ROOT/deps/src
    exit 0
elif [ ! -z $VBOX_UNDO_MOUNT ]; then
    sudo umount -a -t vboxsf
    exit 0
elif [ ! -z $BUILD_CLEAN ]; then
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
