#!/bin/bash
set +v

#support running from project root and it's immediate children
if [ ! -d ./source ]; then cd ../; fi

if [ -z "$CC" ]; then CC=clang; fi

$CC source/linux32_platform.cpp -o ./build/game \
  -Wall -Wno-missing-braces -Wno-char-subscripts -O0 -ggdb3 -std=c++11 \
  -lX11 -lm -lGL -lGLU
