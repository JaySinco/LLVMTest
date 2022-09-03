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
            echo "  -e   build release version"
            echo "  -f   remove cmake cache before build"
            echo "  -h   print command line options"
            echo
            exit 0
            ;;
        -c) do_clean=1 && shift ;;
        -r) do_run_docker=1 && shift ;;
        -e) build_release=1 && shift ;;
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

docker_workspace_dir=/workspace
docker_image_tag=build:v1

conan_build_type=Debug
if [ $build_release -eq 1 ]; then
    conan_build_type=Release
fi

conan_profile=$git_root/profiles/$arch-$os.profile
conan_build_folder=$git_root/out/$conan_build_type

if [ $do_clean -eq 1 ]; then
    cd $conan_build_folder && ninja clean
    exit 0
fi

if [ $do_run_docker -eq 1 ]; then
    docker run -it --rm \
        -v $HOME/.ssh:/root/.ssh:ro \
        -v $HOME/.config/nvim:/root/.config/nvim:rw \
        -v $HOME/.local/share/nvim:/root/.local/share/nvim:rw \
        -v $git_root:$docker_workspace_dir:rw \
        $docker_image_tag
    exit 0
fi

if [ $do_rm_cmake_cache -eq 1 ]; then
    rm -f $conan_build_folder/CMakeCache.txt
fi

pushd $git_root \
&& find src -iname *.h -or -iname *.cpp | xargs clang-format -i \
&& conan install \
    --install-folder=$conan_build_folder \
    --profile=$conan_profile \
    --profile:build=$conan_profile \
    --conf=tools.cmake.cmaketoolchain:generator=Ninja \
    --settings=build_type=$conan_build_type \
    --build=never \
    . \
&& conan build --install-folder=$conan_build_folder . \
&& cp $conan_build_folder/compile_commands.json $git_root \
&& echo done!
