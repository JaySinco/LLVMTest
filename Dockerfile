FROM ubuntu:20.04

# docker
# -----------------
# apt-get install -y docker.io
# usermod -aG docker $USER
# printf '{"registry-mirrors":["https://docker.mirrors.ustc.edu.cn/"]}' > /etc/docker/daemon.json
# systemctl daemon-reload && systemctl restart docker

# google-pinyin
# -----------------
# apt-get install -y fictx fcitx-googlepinyin
# im-config
# fcitx-config-gtk3

ARG PROJECT_DIR

# locale
# -----------------
ENV TZ=Asia/Shanghai \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y \
    && apt-get install -y ca-certificates \
    && cp /etc/apt/sources.list /etc/apt/sources.list.bak \
    && printf 'deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\ndeb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\n' > /etc/apt/sources.list \
    && apt-get update -y \
    && apt-get install -y tzdata language-pack-zh-hans \
    && ln -fs /usr/share/zoneinfo/$TZ /etc/localtime \
    && echo $TZ > /etc/timezone \
    && dpkg-reconfigure --frontend noninteractive tzdata \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=zh_CN.UTF-8 \
    LANGUAGE=zh_CN:zh \
    LC_ALL=zh_CN.UTF-8

# vscode
# -----------------
ENV code='code --no-sandbox --user-data-dir /root/.config/vscode'

RUN apt-get update -y \
    && apt-get install -y wget \
    && wget -q -O code_amd64.deb 'https://vscode.cdn.azure.cn/stable/dfd34e8260c270da74b5c2d86d61aee4b6d56977/code_1.66.2-1649664567_amd64.deb' \
    && apt-get install -y libnss3 gnupg libxkbfile1 libsecret-1-0 libgtk-3-0 libxss1 libgbm1 libasound2 \
        fonts-firacode ttf-wqy-microhei tk-dev \
    && dpkg -i code_amd64.deb \
    && rm -f code_amd64.deb \
    && $code --install-extension MS-CEINTL.vscode-language-pack-zh-hans \
    && $code --install-extension twxs.cmake \
    && $code --install-extension ms-vscode.cpptools \
    && $code --install-extension vscode-icons-team.vscode-icons \
    && rm -rf /var/lib/apt/lists/*

# devtools
# -----------------
RUN apt-get update -y \
    && apt-get build-dep -y qt5-default \
    && apt-get install -y build-essential gdb git cmake clang-format zip \
        tcl libxcb-xinerama0-dev \
    && rm -rf /var/lib/apt/lists/*

# config
# -----------------
ENV XDG_RUNTIME_DIR=/tmp/xdg-runtime-root

RUN git config --global user.name jaysinco \
    && git config --global user.email jaysinco@163.com \
    && git config --global --add safe.directory $PROJECT_DIR \
    && mkdir -p $XDG_RUNTIME_DIR \
    && chmod 700 $XDG_RUNTIME_DIR

# entry
# -----------------
WORKDIR $PROJECT_DIR
ENTRYPOINT ["/bin/bash"]
