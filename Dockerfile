FROM ubuntu:20.04

# docker
# -----------------
# apt install -y docker.io
# usermod -aG docker $USER
# printf '{"registry-mirrors":["https://docker.mirrors.ustc.edu.cn/"]}' > /etc/docker/daemon.json
# systemctl daemon-reload && systemctl restart docker

# google-pinyin
# -----------------
# apt install fictx fcitx-googlepinyin
# im-config
# fcitx-config-gtk3

# apt
# -----------------
RUN apt-get update -y \
    && apt-get install -y ca-certificates \
    && cp /etc/apt/sources.list /etc/apt/sources.list.bak \
    && printf 'deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\n' > /etc/apt/sources.list \
    && apt-get update -y

# locale
# -----------------
RUN ln -snf /usr/share/zoneinfo/$CONTAINER_TIMEZONE /etc/localtime \
    && echo $CONTAINER_TIMEZONE > /etc/timezone \
    && apt-get install -y language-pack-zh-hans wget
ENV LANG zh_CN.UTF-8
ENV LANGUAGE zh_CN:zh
ENV LC_ALL zh_CN.UTF-8

# vscode
# -----------------
RUN wget -q -O code_amd64.deb 'https://vscode.cdn.azure.cn/stable/dfd34e8260c270da74b5c2d86d61aee4b6d56977/code_1.66.2-1649664567_amd64.deb' \
    && apt-get install -y libnss3 gnupg libxkbfile1 libsecret-1-0 libgtk-3-0 libxss1 libgbm1 libasound2 \
        fonts-firacode ttf-wqy-microhei tk-dev \
    && dpkg -i code_amd64.deb \
    && rm -f code_amd64.deb

# entry
# -----------------
CMD code --no-sandbox --user-data-dir /root/.config/vscode --wait /workspace

# RUN apt-get install -y build-essential git cmake clang-format zip tcl \
#     && apt-get clean \
#     && rm -rf /var/lib/apt/lists/*

# RUN git config --global user.name jaysinco \
#     && git config --global user.email jaysinco@163.com \
#     && git config --global --add safe.directory /code
