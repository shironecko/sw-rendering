#!/bin/bash
set +v

#support running from project root and it's immediate children
if [ ! -d ./misc ]; then cd ../; fi

if [ -d ./data/cooked ]; then rm -rf ./data/cooked; fi

mkdir ./data/cooked ./data/cooked/{textures,meshes}

./misc/build.sh -o ./build/resource_converter -DRESOURCE_CONVERTER_PROJECT
./build/resource_converter
./misc/build.sh -o ./build/game -DGAME_PROJECT
./build/game

