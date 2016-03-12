@echo off

if not exist .\misc\build.bat cd ..\

if exist ..\data\cooked rmdir /Q /S ..\data\cooked
mkdir ..\data\cooked\textures ..\data\cooked\meshes

call .\misc\build.bat -Fo.\build\ -Fd.\build\resource_converter.pdb -Fe.\build\resource_converter.exe -DRESOURCE_CONVERTER_PROJECT
call start .\build\resource_converter.exe -wo ..\
call .\misc\build.bat -Fo.\build\ -Fd.\build\game.pdb -Fe.\build\game.exe -DGAME_PROJECT
call start .\build\game.exe -wo ..\
