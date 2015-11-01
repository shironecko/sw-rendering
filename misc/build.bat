@echo off

if not exist ..\build mkdir ..\build
pushd ..\build
cl /nologo ..\source\main.cpp /W4 /wd4201 /MT /Od /Oi /Zi user32.lib gdi32.lib /link /subsystem:windows
popd
