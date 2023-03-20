#!/bin/bash

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

if [ ! -f ~/.ssh/id_rsa ]; then
    echo "copy ssh key"
    mkdir -p ~/.ssh
    cp $source_repo/res/id_rsa ~/.ssh
    cp $source_repo/res/id_rsa.pub ~/.ssh
    chmod 700 ~/.ssh
    chmod 600 ~/.ssh/id_rsa
    chmod 644 ~/.ssh/id_rsa.pub
fi

if [ ! -f ~/.local/share/fonts/'Hack Regular Nerd Font Complete.ttf' ]; then
    echo "copy fonts"
    mkdir -p ~/.local/share/fonts
    unzip $source_repo/font-hack.zip -d ~/.local/share/fonts/
fi

if [ ! -f "/etc/profile.d/git-prompt.sh" ]; then
    echo "copy git-prompt.sh"
    cp $script_dir/git-prompt.sh /etc/profile.d/
fi

if ! grep -q "mirrors.tuna.tsinghua.edu.cn" /etc/apt/sources.list; then
    echo "change apt sources list"
    sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
    sudo -- printf 'deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\n' > /etc/apt/sources.list
    sudo apt-get update -y
fi

if [ ! -f "/usr/bin/ninja" ]; then sudo apt-get install ninja-build; fi