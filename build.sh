#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
DOCKER_IMAGE_TAG=build:v1
DOCKER_PROJECT_DIR=/workspace
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
            echo "  -c, --clean              clean target"
            echo "      --ui                 edit qt5 ui files"
            echo "  -t, --target [...]       target to be built"
            echo "  -h, --help               print command line options (currently set)"
            echo
            exit 0
            ;;
        -b|--docker-build)
            docker build --build-arg PROJECT_DIR=$DOCKER_PROJECT_DIR \
                -f $PROJECT_ROOT/Dockerfile \
                -t $DOCKER_IMAGE_TAG \
                $PROJECT_ROOT/.vscode
            exit 0
            ;;
        -r|--docker-run)
            xhost +local:docker > /dev/null
            docker run -it --rm \
                -e DISPLAY \
                -e XMODIFIERS="@im=fcitx" \
                -e QT_IM_MODULE="fcitx" \
                -e GTK_IM_MODULE="fcitx" \
                -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
                -v /usr/share/icons:/usr/share/icons:ro \
                -v /home/$USER/.ssh:/root/.ssh:ro \
                -v $PROJECT_ROOT:$DOCKER_PROJECT_DIR \
                --device=/dev/dri:/dev/dri \
                $DOCKER_IMAGE_TAG
            xhost -local:docker > /dev/null
            exit 0
            ;;
        --vbox-mount)
            mkdir -p $PROJECT_ROOT/deps/src
            sudo mount -t vboxsf -o defaults,uid=$(id -u),gid=$(id -g) \
                share $PROJECT_ROOT/deps/src
            exit 0
            ;;
        --vbox-umount)
            sudo umount -a -t vboxsf
            exit 0
            ;;
        -c|--clean)
            pushd $PROJECT_ROOT/out && make clean && popd
            exit 0
            ;;
        --ui)
            $PROJECT_ROOT/deps/linux/qt5/bin/designer \
                $PROJECT_ROOT/src/qt5/go-to-cell-dialog.ui
            exit 0
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
