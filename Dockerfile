FROM ubuntu:20.04

# ###################
# docker
# ###################
# apt install -y docker.io
# usermod -aG docker $USER
# printf '{"registry-mirrors":["https://docker.mirrors.ustc.edu.cn/"]}' > /etc/docker/daemon.json
# systemctl daemon-reload && systemctl restart docker

# ###################
# vscode
# ###################
# snap install --classic code
# apt install -y fonts-firacode

RUN apt-get update -y \
    && apt-get install -y ca-certificates \
    && cp /etc/apt/sources.list /etc/apt/sources.list.bak \
    && printf 'deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\n' > /etc/apt/sources.list \
    && apt-get update -y

RUN ln -snf /usr/share/zoneinfo/$CONTAINER_TIMEZONE /etc/localtime && echo $CONTAINER_TIMEZONE > /etc/timezone \
    && apt-get install -y build-essential git cmake clang-format tcl zip tk-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && git config --global user.name jaysinco \
    && git config --global user.email jaysinco@163.com \
    && git config --global --add safe.directory /workspaces/Prototyping
