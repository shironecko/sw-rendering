#!/bin/bash
set +v
#if !exist ../build mkdir ../build

#clang -o build ../source/linux32_platform.cpp $* -Wall -O0 -ggdb3 -std=c++11
clang -o build/build source/linux32_platform.cpp -DGAME_PROJECT -Wall -Wno-missing-braces -Wno-char-subscripts -O0 -ggdb3 -std=c++11 -lX11 -lm
