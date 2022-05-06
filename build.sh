#!/bin/bash

set -e

case "$OSTYPE" in
    linux*)   PLATFORM=linux ;;
    msys*)    PLATFORM=win32 ;;
esac
echo "platform: $PLATFORM"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
DOCKER_IMAGE_TAG=build:v1
DOCKER_PROJECT_DIR=/workspace
BUILD_TARGETS=()

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
            echo "      --edit-ui            edit qt5 ui files"
            echo "  -c, --clean              clean targets built"
            echo "  -h, --help               print command line options"
            echo
            exit 0
            ;;
        -b|--build)
            docker build --build-arg PROJECT_DIR=$DOCKER_PROJECT_DIR \
                -f $PROJECT_ROOT/Dockerfile \
                -t $DOCKER_IMAGE_TAG \
                $PROJECT_ROOT/.vscode
            exit 0
            ;;
        -r|--run)
            xhost +local:docker > /dev/null
            docker run -it --rm \
                -e DISPLAY \
                -e XMODIFIERS="@im=fcitx" \
                -e QT_IM_MODULE="fcitx" \
                -e GTK_IM_MODULE="fcitx" \
                -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
                -v /usr/share/icons:/usr/share/icons:ro \
                -v /home/$USER/.ssh:/root/.ssh:ro \
                -v $PROJECT_ROOT:$DOCKER_PROJECT_DIR:rw \
                --device=/dev/dri:/dev/dri \
                $DOCKER_IMAGE_TAG
            xhost -local:docker > /dev/null
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
        --edit-ui)
            $PROJECT_ROOT/deps/linux/qt5/bin/designer \
                $PROJECT_ROOT/src/qt5/go-to-cell-dialog.ui
            exit 0
            ;;
        -c|--clean)
            pushd $PROJECT_ROOT/out && make clean && popd
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

if [ $PLATFORM = "linux" ]; then
    BUILD_GENERATOR="Unix Makefiles"
    BUILD_PROGRAM="make"
elif [ $PLATFORM = "win32" ]; then
    BUILD_GENERATOR="Ninja"
    BUILD_PROGRAM="ninja"
    source $PROJECT_ROOT/vcvars64.sh
fi

pushd $PROJECT_ROOT \
&& find src -iname *.h -or -iname *.cpp | xargs clang-format -i \
&& mkdir -p out \
&& pushd out \
&& cmake -G "$BUILD_GENERATOR" .. \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$PROJECT_ROOT/bin \
    -DCMAKE_BUILD_TYPE=debug \
    -DTARGET_OS=$PLATFORM \
&& $BUILD_PROGRAM -j`nproc` ${BUILD_TARGETS[*]} \
&& popd \
&& popd
