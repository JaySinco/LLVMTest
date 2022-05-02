FROM ubuntu:20.04

RUN apt-get update -y \
    && apt-get install -y ca-certificates \
    && cp /etc/apt/sources.list /etc/apt/sources.list.bak \
    && printf 'deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse\ndeb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\n# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-security main restricted universe multiverse\n' > /etc/apt/sources.list \
    && apt-get update -y

RUN apt-get install -y clang-12 lldb-12 lld-12 clangd-12 \
        libc++-12-dev libc++abi-12-dev clang-format-12 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-12 10 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-12 10 \
    && update-alternatives --install /usr/bin/ld ld /usr/bin/ld.lld-12 10 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-12 10

# RUN cd /tmp \
#     && curl -L -o cmake-3.17.0-rc3.tar.gz https://github.com/Kitware/CMake/releases/download/v3.17.0-rc3/cmake-3.17.0-rc3.tar.gz \
#     && tar zxvf cmake-3.17.0-rc3.tar.gz \
#     && cd /tmp/cmake-3.17.0-rc3 \
#     && ./bootstrap --prefix=/usr/local -- -DCMAKE_USE_OPENSSL=OFF \
#     && make -j`nproc` \
#     && make install \
#     && rm -rf /tmp/cmake-3.17.0-rc3 \
#     && rm -f /tmp/cmake-3.17.0-rc3.tar.gz
