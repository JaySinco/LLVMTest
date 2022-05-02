#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(git rev-parse --show-toplevel)"
DOCKER_IMAGE_TAG=build:v1
POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            echo "Usage: build.sh [options]"
            echo
            echo "Options:"
            echo "  -d, --docker          build docker image"
            echo "  -r, --run             run docker image"
            echo "  -t, --target [...]    target to be built"
            echo "  -h, --help            print command line options (currently set)"
            echo
            exit 0
            ;;
        -d|--docker)
            DOCKER_BUILD=YES
            shift
            ;;
        -r|--run)
            DOCKER_RUN=YES
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

if [ ! -z ${DOCKER_BUILD+x} ]; then
    docker build -f $PROJECT_ROOT/Dockerfile -t $DOCKER_IMAGE_TAG $PROJECT_ROOT
    exit 0
fi

if [ ! -z ${DOCKER_RUN+x} ]; then
    docker run -it --rm -v $PROJECT_ROOT:/code $DOCKER_IMAGE_TAG
    exit 0
fi
