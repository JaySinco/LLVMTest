#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT=$SCRIPT_DIR
POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            echo
            echo "Usage: build.sh [options]"
            echo
            echo "Options:"
            echo "  -m, --mount           mount vbox share folder"
            echo "  -t, --target [...]    target to be built"
            echo "  -h, --help            print command line options (currently set)"
            echo
            exit 0
            ;;
        -t|--target)
            BUILD_TARGET="$2"
            shift
            shift
            ;;
        -*|--*)
            echo "Unknown option: $1"
            exit 1
            ;;
        *)
            POSITIONAL_ARGS+=("$1")
            shift
            ;;
    esac
done

# vbox share folder
mkdir -p /media/jaysinco/share
grep -q 'init-vbox-share' /etc/fstab || printf '# init-vbox-share\nshare    /media/jaysinco/share    vboxsf    defaults,uid=1000,gid=1000    0    0\n' >> /etc/fstab
