#!/bin/bash

set -e

build_release=0

do_clean=0
do_rm_cmake_cache=0

while [[ $# -gt 0 ]]; do
    case $1 in
        -h)
            echo
            echo "Usage: build.sh [options]"
            echo
            echo "Options:"
            echo "  -c   clean output"
            echo "  -l   build release version"
            echo "  -f   remove cmake cache before build"
            echo "  -h   print command line options"
            echo
            exit 0
            ;;
        -c) do_clean=1 && shift ;;
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

if [ $os = "linux" ]; then
    source_repo=$git_root/../dev-setup/linux/src
elif [ $os = "windows" ]; then
    source_repo=$USERPROFILE/OneDrive/src
fi

export JAYSINCO_SOURCE_REPO=$source_repo

build_type=Debug
if [ $build_release -eq 1 ]; then
    build_type=Release
fi

conan_profile=$git_root/../dev-setup/$os/$arch/conan.profile
build_folder=$git_root/out/$build_type

if [ $do_clean -eq 1 ]; then
    rm -rf $git_root/out $git_root/bin
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
&& echo done!
