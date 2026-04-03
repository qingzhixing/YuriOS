#!/bin/bash
cd "$(dirname "$0")/../.."
gdb -tui -x ./build-tools/qemu/client.gdb
