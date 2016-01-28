@echo off

if not exist .\build.bat cd misc

if exist ..\data\cooked rmdir /Q /S ..\data\cooked
mkdir ..\data\cooked\textures ..\data\cooked\meshes

call ".\build.bat" -DRESOURCE_CONVERTER_PROJECT
call "..\build\build.exe"
call ".\build.bat" -DGAME_PROJECT
call "..\build\build.exe"
