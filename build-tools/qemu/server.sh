#!/bin/bash
cd "$(dirname "$0")/../.."
qemu-system-x86_64 \
    -fda ./build/boot.img \
    -m 2048 \
    -s -S
