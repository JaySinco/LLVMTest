#!/bin/bash

set -e

build_release=0

do_clean=0
do_run_docker=0
do_rm_cmake_cache=0

while [[ $# -gt 0 ]]; do
    case $1 in
        -h)
            echo
            echo "Usage: build.sh [options]"
            echo
            echo "Options:"
            echo "  -c   clean output"
            echo "  -r   run docker"
            echo "  -l   build release version"
            echo "  -f   remove cmake cache before build"
            echo "  -h   print command line options"
            echo
            exit 0
            ;;
        -c) do_clean=1 && shift ;;
        -r) do_run_docker=1 && shift ;;
        -l) build_release=1 && shift ;;
        -f) do_rm_cmake_cache=1 && shift ;;
        -*) echo "Unknown option: $1" && exit 1 ;;
    esac
done

case "$(uname -m)" in
    x86_64)   arch=x64 ;;
esac

case "$OSTYPE" in
    linux*)   os=linux ;;
    msys*)    os=windows ;;
esac

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
git_root="$(git rev-parse --show-toplevel)"
docker_image_tag=build:v1

build_type=Debug
if [ $build_release -eq 1 ]; then
    build_type=Release
fi

conan_profile=$git_root/profiles/$arch-$os.profile
build_folder=$git_root/out/$build_type

if [ $do_clean -eq 1 ]; then
    cd $build_folder && ninja clean
    exit 0
fi

if [ $do_run_docker -eq 1 ]; then
    mkdir -p \
        $HOME/.ssh \
        $HOME/.config/nvim \
        $HOME/.local/share/nvim \
        $HOME/.conan
    docker run -it --rm \
        -e DISPLAY \
        -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
        -v $HOME/.ssh:/home/jaysinco/.ssh:ro \
        -v $HOME/.config/nvim:/home/jaysinco/.config/nvim:rw \
        -v $HOME/.local/share/nvim:/home/jaysinco/.local/share/nvim:rw \
        -v $HOME/.conan:/home/jaysinco/.conan:rw \
        -v $git_root:/home/jaysinco/workspace:rw \
        $docker_image_tag
    exit 0
fi

if [ $do_rm_cmake_cache -eq 1 ]; then
    rm -f $build_folder/CMakeCache.txt
fi

pushd $git_root \
&& find src -iname *.h -or -iname *.cpp | xargs clang-format -i \
&& conan install . \
    --install-folder=$build_folder \
    --profile=$conan_profile \
    --profile:build=$conan_profile \
    --settings=build_type=$build_type \
    --build=never \
&& conan build --install-folder=$build_folder . \
&& cp $build_folder/compile_commands.json $git_root \
&& echo done!
