#!/bin/bash

# ubuntu:20.04
exit 0

# vbox share folder
mkdir -p /media/jaysinco/share
grep -q 'init-vbox-share' /etc/fstab || printf '# init-vbox-share\nshare    /media/jaysinco/share    vboxsf    defaults,uid=1000,gid=1000    0    0\n' >> /etc/fstab

# basic
apt install -y curl binutils

# vscode
snap install --classic code
apt install -y fonts-firacode

# git
apt install -y git git-lfs
git config --global user.name jaysinco
git config --global user.email jaysinco@163.com
ssh-keygen -t rsa -b 4096
cat ~/.ssh/id_rsa.pub

# docker
apt install -y docker.io
groupadd docker
usermod -aG docker $USER
printf '{"registry-mirrors":["https://docker.mirrors.ustc.edu.cn/"]}' > /etc/docker/daemon.json
systemctl daemon-reload
systemctl restart docker

# nodejs
curl -sL https://deb.nodesource.com/setup_16.x | bash -
apt install -y nodejs
npm install -g yarn
