#!/usr/bin/env sh

cc=clang++
cflags="-Wall -Wextra -fsanitize=address -O0 -g -fPIC -pipe"
# cflags="-Wall -Wextra -O2 -flto -fPIC -pipe -static"

set -xeu

$cc $cflags main.cpp -o main
