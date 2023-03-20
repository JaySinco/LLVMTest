#!/bin/bash

# apt mirror
# --------------
# cp /etc/apt/sources.list /etc/apt/sources.list.bak
# printf 'deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\n' > /etc/apt/sources.list
# apt-get update -y

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

if [ ! -f "/usr/bin/ninja" ]; then sudo apt-get install ninja-build; fi
if [ ! -f "/usr/bin/unzip" ]; then sudo apt-get install unzip; fi

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
