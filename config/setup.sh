#!/bin/bash

# ssh key
# -----------------
# chmod 700 .ssh
# chmod 600 .ssh/id_rsa
# chmod 644 .ssh/id_rsa.pub
# ssh-add

set -e

git_root="$(git rev-parse --show-toplevel)"
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source_repo=/mnt/c/Users/jaysinco/OneDrive/src

if ! lsb_release -r | grep -q "20.04"; then
    echo "ubuntu:20.04 required!"
    exit 1
fi

sudo $script_dir/alter-apt.sh

git config --global i18n.filesEncoding utf-8
git config --global core.autocrlf input
git config --global core.safecrlf false
git config --global core.longpaths true
git config --global core.quotepath false
git config --global pull.rebase false
git config --global fetch.prune true

function clone_repo() {
    mkdir -p "$1"
    cd "$1"
    if [ ! -d "$1/.git" ]; then
        echo "git clone $2 -b $3" \
        && git init \
        && git remote add origin git@gitee.com:$2 \
        && git fetch \
        && git checkout origin/$3 -b $3
    fi
    if [ `git remote|wc --lines` -lt 2 ]; then
        echo "git remote add backup git@github.com:$2"
        git remote add backup git@github.com:$2
    fi
    git config user.name jaysinco
    git config user.email jaysinco@163.com
}

clone_repo $git_root jaysinco/Prototyping.git master
clone_repo $HOME/.config/nvim jaysinco/nvim.git master

if [ ! -f "/usr/bin/gcc-11" ]; then
    sudo apt-get install -y gcc-11 g++-11
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 60 \
        --slave /usr/bin/g++ g++ /usr/bin/g++-11
fi

if [ ! -f "/usr/bin/clangd-15" ]; then
    sudo apt-get install -y clangd-15 clang-format-15 clang-tidy-15 lldb-15
    sudo update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-15 60 \
        --slave /usr/bin/clang-format clang-format /usr/bin/clang-format-15 \
        --slave /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-15 \
        --slave /usr/bin/lldb lldb /usr/bin/lldb-15 \
        --slave /usr/bin/lldb-vscode lldb-vscode /usr/bin/lldb-vscode-15 \
        --slave /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-15
fi

if [ ! -f "/usr/bin/ninja" ]; then sudo apt-get install -y ninja-build; fi
if [ ! -f "/usr/bin/unzip" ]; then sudo apt-get install -y unzip; fi
if [ ! -f "/usr/bin/cmake" ]; then sudo apt-get install -y cmake; fi
if [ ! -f "/usr/bin/node" ]; then sudo apt-get install -y nodejs; fi
if [ ! -f "/usr/bin/pip3" ]; then sudo apt-get install -y python3-pip; fi
if [ ! -f "$HOME/.local/bin/conan" ]; then pip3 install conan==1.52 -i https://pypi.tuna.tsinghua.edu.cn/simple; fi
if [ ! -f "/usr/bin/pyright" ]; then sudo npm install -g pyright; fi

if [ ! -f "/usr/bin/nvim" ]; then
    echo "install nvim"
    sudo tar zxf $source_repo/nvim-v0.7.2-linux-x86_64.tar.gz --directory=/usr --strip-components=1
fi

nvim_data_dir=$HOME/.local/share/nvim
if [ ! -d "$nvim_data_dir/site" ]; then
    echo "copy nvim data"
    mkdir -p $nvim_data_dir
    unzip -q $source_repo/nvim-data-site-v0.7.2-linux-x86_64.zip -d $nvim_data_dir
fi
