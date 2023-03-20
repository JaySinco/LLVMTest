#!/bin/bash

set -e

do_build_all=0
do_mount=0
do_unmount=0
do_vmware=0
do_build_docker=0
do_run_docker=0
do_save_docker=0
do_load_docker=0
do_list_ext=0
do_install_ext=0
do_env_setup=0
do_update_repo=0
do_status_repo=0
do_sync_repo=0

while [[ $# -gt 0 ]]; do
    case $1 in
        -h)
            echo
            echo "Usage: build.sh [options]"
            echo
            echo "Options:"
            echo "  -a    build all targets"
            echo "  -m    mount share [default: vbox]"
            echo "  -u    unmount share [default: vbox]"
            echo "  -vm   u/mount vmware"
            echo "  -k    build docker"
            echo "  -r    run docker"
            echo "  -ds   save docker"
            echo "  -dl   load docker"
            echo "  -vl   list vscode extensions"
            echo "  -vi   install vscode extensions"
            echo "  -n    env setup"
            echo "  -p    pull/push all repo"
            echo "  -s    list all repo status"
            echo "  -y    sync all repo"
            echo "  -h    print command line options"
            echo
            exit 0
            ;;
         -a) do_build_all=1 && shift ;;
         -m) do_mount=1 && shift ;;
         -u) do_unmount=1 && shift ;;
        -vm) do_vmware=1 && shift ;;
         -k) do_build_docker=1 && shift ;;
         -r) do_run_docker=1 && shift ;;
        -ds) do_save_docker=1 && shift ;;
        -dl) do_load_docker=1 && shift ;;
        -vl) do_list_ext=1 && shift ;;
        -vi) do_install_ext=1 && shift ;;
         -n) do_env_setup=1 && shift ;;
         -p) do_update_repo=1 && shift ;;
         -s) do_status_repo=1 && shift ;;
         -y) do_sync_repo=1 && shift ;;
          *) echo "Unknown option: $1" && exit 1 ;;
    esac
done

case "$OSTYPE" in
    linux*)   os=linux ;;
    msys*)    os=windows ;;
esac

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
git_root="$(git rev-parse --show-toplevel)"

if [ $os = "linux" ]; then
    res_dir=$git_root/../dev-setup/linux
    docker_image_tag=jaysinco-dev:v1
    docker_image_file=docker-dev-image-$os-x86_64
    docker_home=/home/jaysinco
    source_repo=$res_dir/src
    nvim_config_dir=$HOME/.config/nvim
    nvim_data_dir=$HOME/.local/share/nvim
    vscode_config_dir=$HOME/.config/Code
elif [ $os = "windows" ]; then
    res_dir=$git_root/../dev-setup/windows
    source_repo=$USERPROFILE/OneDrive/src
    nvim_config_dir=$LOCALAPPDATA/nvim
    nvim_data_dir=$LOCALAPPDATA/nvim-data
    vscode_config_dir=$APPDATA/Code
    wt_config_dir=$LOCALAPPDATA/Microsoft/Windows\ Terminal
fi

function package() {
    local build_debug=$1
    local name=$2
    $git_root/../dev-setup/recipes/build.sh $name -r && \
    if [ $build_debug -eq 1 ]; then
        $git_root/../dev-setup/recipes/build.sh $name -r -d
    fi
}

function package_linux() {
    if [ $os = "linux" ]; then
        package $*
    fi
}

function package_win() {
    if [ $os = "windows" ]; then
        package $*
    fi
}

if [ $do_build_all -eq 1 ]; then
    echo start! \
    && package_win 0 jom \
    && package_win 0 nasm \
    && package_win 0 strawberryperl \
    && package 1 gflags \
    && package 1 glog \
    && package 1 gtest \
    && package 1 fmt \
    && package 1 spdlog \
    && package 0 boost \
    && package 1 glfw \
    && package 1 imgui \
    && package 1 implot \
    && package 1 catch2 \
    && package 0 qt \
    && package 0 expected-lite \
    && package 1 qhull \
    && package 1 lodepng \
    && package 1 libccd \
    && package 1 tinyobjloader \
    && package 1 tinyxml2 \
    && package 1 mujoco \
    && package 0 torch \
    && package 1 double-conversion \
    && package 1 bzip2 \
    && package 1 zlib \
    && package 0 openssl \
    && package 1 libevent \
    && package 1 zstd \
    && package 1 lz4 \
    && package 1 snappy \
    && package 0 libxl \
    && package 0 argparse \
    && package 0 range-v3 \
    && package 1 libiconv \
    && package 1 raylib \
    && package 0 nlohmann-json \
    && package 1 sdl \
    && package 1 libuv \
    && package 1 usockets \
    && package 0 uwebsockets \
    && package 1 libcurl \
    && package 1 cpr \
    && package 1 sqlite3 \
    && package 1 mongoose \
    && package 0 concurrentqueue \
    && package 0 threadpool \
    && package 1 libusb \
    && package_linux 1 libpcap \
    && package_win 1 npcap \
    && echo done!
    exit 0
fi

if [ $do_mount -eq 1 ]; then
    mkdir -p $source_repo
    if [ $do_vmware -eq 0 ]; then
        sudo mount -t vboxsf -o rw,uid=$(id -u),gid=$(id -g) \
            share $source_repo
    else
        # apt install open-vm-tools open-vm-tools-desktop
        vmhgfs-fuse -o rw,uid=$(id -u),gid=$(id -g) \
            .host:/share $source_repo
    fi
    exit 0
fi

if [ $do_unmount -eq 1 ]; then
    if [ $do_vmware -eq 0 ]; then
        sudo umount -a -t vboxsf
    else
        sudo umount $source_repo
    fi
    exit 0
fi

if [ $do_build_docker -eq 1 ]; then
    docker build \
        -f $res_dir/Dockerfile \
        -t $docker_image_tag \
        $res_dir
    exit 0
fi

if [ $do_run_docker -eq 1 ]; then
    mkdir -p \
        $HOME/.ssh \
        $nvim_config_dir \
        $nvim_data_dir \
        $vscode_config_dir \
        $HOME/.vscode \
        $HOME/.conan
    docker run -it --rm \
        --security-opt seccomp=unconfined \
        --shm-size=1G \
        -e DISPLAY=$DISPLAY \
        -e LOCAL_UID=$(id -u) \
        -e LOCAL_GID=$(id -g) \
        -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
        -v $HOME/.ssh:$docker_home/.ssh:ro \
        -v $nvim_config_dir:$docker_home/.config/nvim:rw \
        -v $nvim_data_dir:$docker_home/.local/share/nvim:rw \
        -v $vscode_config_dir:$docker_home/.config/Code:rw \
        -v $HOME/.vscode:$docker_home/.vscode:rw \
        -v $HOME/.conan:$docker_home/.conan:rw \
        -v $git_root/../dev-setup:$docker_home/dev-setup:rw \
        -v $git_root/../Prototyping:$docker_home/Prototyping:rw \
        $docker_image_tag
    exit 0
fi

if [ $do_save_docker -eq 1 ]; then
    docker save $docker_image_tag | \
        gzip > $source_repo/$docker_image_file.tar.gz
    exit 0
fi

if [ $do_load_docker -eq 1 ]; then
    docker load < $source_repo/$docker_image_file.tar.gz
    exit 0
fi

if [ $do_list_ext -eq 1 ]; then
    code --list-extensions | jq -R -s '{recommendations:split("\n")[:-1]}' \
        --indent 4 > $git_root/../dev-setup/.vscode/extensions.json
    exit 0
fi

if [ $do_install_ext -eq 1 ]; then
    for ext in $(jq -r '.recommendations[]' < $git_root/../dev-setup/.vscode/extensions.json); do
        code --install-extension `echo $ext | sed 's/\r$//'`
    done
    exit 0
fi

function copy_nvim_data() {
    if [ ! -d $nvim_data_dir/site ]; then
        echo "copy nvim data"
        nvim_version="0.7.2"
        nvim_data_file=$source_repo/nvim-data-site-v$nvim_version-$os-x86_64.zip
        mkdir -p $nvim_data_dir
        unzip -q $nvim_data_file -d $nvim_data_dir
    fi
}

function clone_repo() {
    mkdir -p "$1" 
    cd "$1" 
    if [ ! -d "$1/.git" ]; then
        echo "** CLONE $2 -b $3" \
        && git init \
        && git remote add origin git@gitee.com:$2 \
        && git fetch \
        && git checkout origin/$3 -b $3 \
        && git remote add backup git@github.com:$2
    fi
    git config user.name jaysinco
    git config user.email jaysinco@163.com
}

if [ $do_env_setup -eq 1 ]; then
    if [ $os = "windows" ]; then
        clone_repo "$wt_config_dir" jaysinco/windows-terminal.git master
    fi \
    && clone_repo $vscode_config_dir/User jaysinco/vscode.git master \
    && clone_repo $nvim_config_dir jaysinco/nvim.git master \
    && clone_repo $git_root/../dev-setup jaysinco/dev-setup.git master \
    && clone_repo $git_root/../Prototyping jaysinco/Prototyping.git master \
    && $res_dir/set-env.sh \
    && copy_nvim_data \
    exit 0
fi

function update_repo() {
    cd "$1"
    echo "pull* " `realpath "$1"`
    git pull
    git merge-base --is-ancestor HEAD @{u}
    if [ $? -ne 0 ]; then
        echo "push*" `realpath "$1"`
        git push
    fi
}

if [ $do_update_repo -eq 1 ]; then
    if [ $os = "windows" ]; then
        update_repo "$wt_config_dir"
    fi \
    && update_repo $vscode_config_dir/User \
    && update_repo $nvim_config_dir \
    && update_repo $git_root/../dev-setup \
    && update_repo $git_root/../Prototyping \
    exit 0
fi

function status_repo() {
    cd "$1"
    if [ ! -z "$(git status --porcelain)" ]; then
        echo ================== \
        && echo `realpath "$1"` \
        && echo ================== \
        && git status --porcelain \
        && echo
    fi
}

if [ $do_status_repo -eq 1 ]; then
    if [ $os = "windows" ]; then \
        status_repo "$wt_config_dir"
    fi \
    && status_repo $vscode_config_dir/User \
    && status_repo $nvim_config_dir \
    && status_repo $git_root/../dev-setup \
    && status_repo $git_root/../Prototyping \
    exit 0
fi

function sync_repo() {
    cd "$1"
    echo "[sync] push*" `realpath "$1"`
    git push backup
}

if [ $do_sync_repo -eq 1 ]; then
    if [ $os = "windows" ]; then \
        sync_repo "$wt_config_dir"
    fi \
    && sync_repo $vscode_config_dir/User \
    && sync_repo $nvim_config_dir \
    && sync_repo $git_root/../dev-setup \
    && sync_repo $git_root/../Prototyping \
    exit 0
fi