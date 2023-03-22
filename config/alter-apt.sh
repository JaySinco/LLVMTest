#!/bin/bash

set -e

if ! grep -q "mirrors.tuna.tsinghua.edu.cn" /etc/apt/sources.list; then
    echo "alter apt sources"
    cp /etc/apt/sources.list /etc/apt/sources.list.bak

    cat > /etc/apt/sources.list <<- END
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse
deb https://launchpad.proxy.ustclug.org/ubuntu-toolchain-r/test/ubuntu focal main
deb https://mirrors.tuna.tsinghua.edu.cn/llvm-apt/focal/ llvm-toolchain-focal-15 main
deb https://mirrors.ustc.edu.cn/nodesource/deb/node_16.x focal main
END

    wget -q -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
    wget -q -O - https://deb.nodesource.com/gpgkey/nodesource.gpg.key | apt-key add -
    apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1e9377a2ba9ef27f
    apt-get update -y
fi
