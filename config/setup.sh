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
os_arch=`arch`

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

if [ ! -f "/usr/bin/gcc" ]; then sudo apt-get install -y gcc; fi
if [ ! -f "/usr/bin/g++" ]; then sudo apt-get install -y g++; fi
if [ ! -f "/usr/bin/gdb" ]; then sudo apt-get install -y gdb; fi
if [ ! -f "/usr/bin/clangd-15" ]; then sudo apt-get install -y clangd-15; fi
if [ ! -f "/usr/bin/clang-format-15" ]; then sudo apt-get install -y clang-format-15; fi
if [ ! -f "/usr/bin/clang-tidy-15" ]; then sudo apt-get install -y clang-tidy-15; fi
if [ ! -f "/usr/bin/clangd" ]; then sudo ln -s /usr/bin/clangd-15 /usr/bin/clangd; fi
if [ ! -f "/usr/bin/clang-format" ]; then sudo ln -s /usr/bin/clang-format-15 /usr/bin/clang-format; fi
if [ ! -f "/usr/bin/clang-tidy" ]; then sudo ln -s /usr/bin/clang-tidy-15 /usr/bin/clang-tidy; fi
if [ ! -f "/usr/bin/ninja" ]; then sudo apt-get install -y ninja-build; fi
if [ ! -f "/usr/bin/xclip" ]; then sudo apt-get install -y xclip; fi
if [ ! -f "/usr/bin/zip" ]; then sudo apt-get install -y zip; fi
if [ ! -f "/usr/bin/unzip" ]; then sudo apt-get install -y unzip; fi
if [ ! -f "/usr/bin/cmake" ]; then sudo apt-get install -y cmake; fi
if [ ! -f "/usr/bin/node" ]; then sudo apt-get install -y nodejs; fi
if [ ! -f "/usr/bin/pip3" ]; then sudo apt-get install -y python3-pip; fi
if [ ! -f "$HOME/.local/bin/conan" ]; then pip3 install conan==1.52 -i https://pypi.tuna.tsinghua.edu.cn/simple; fi
if [ ! -f "/usr/bin/pyright" ]; then sudo npm install -g pyright; fi
if [ ! -f "/usr/bin/rg" ]; then sudo apt-get install -y ripgrep; fi

MY_SOURCE_REPO=/mnt/c/Users/jaysinco/OneDrive/src 
if ! grep -q "MY_SOURCE_REPO=" ~/.bashrc; then
    echo "set source repo"
    printf "\nexport MY_SOURCE_REPO=$MY_SOURCE_REPO\n" >> ~/.bashrc
fi

if [ ! -f "/usr/bin/nvim" ]; then
    echo "install nvim"
    sudo tar zxf $MY_SOURCE_REPO/nvim-v0.7.2-linux-$os_arch.tar.gz --directory=/usr --strip-components=1
fi

nvim_data_dir=$HOME/.local/share/nvim
if [ ! -d "$nvim_data_dir/site" ]; then
    echo "copy nvim data"
    mkdir -p $nvim_data_dir
    unzip -q $MY_SOURCE_REPO/nvim-data-site-v0.7.2-linux-$os_arch.zip -d $nvim_data_dir
fi

