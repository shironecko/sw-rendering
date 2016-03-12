@echo off

if not exist .\misc\build.bat cd ..\

if exist .\data\cooked rmdir /Q /S .\data\cooked
mkdir .\data\cooked\textures .\data\cooked\meshes

call .\misc\build_rc.bat
call start .\build\resource_converter.exe -wo ..\
call .\misc\build_game.bat
call start .\build\game.exe -wo ..\
