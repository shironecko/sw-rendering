#!/bin/bash
set +v

#support running from project root and it's immediate children
if [ ! -d ./build ]; then cd ../; fi

if [ "$CC" == "" ]; then CC=gcc; fi

$CC source/linux32_platform.cpp $* \
  -Wall -Wno-missing-braces -Wno-char-subscripts -O0 -ggdb3 -std=c++11 \
  -lX11 -lm -lGL -lGLU
