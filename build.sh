#!/usr/bin/env sh

cc=clang++
cflags="-Wall -Wextra -O0 -g -fPIC -pipe"
[ "$1" = "release" ] && cflags="-Wall -Wextra -O2 -flto -fPIC -pipe"

set -xeu

$cc $cflags main.cpp -o main
