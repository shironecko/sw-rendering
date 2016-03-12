@echo off
if not exist .\misc\build.bat cd ..\
call .\misc\build.bat -Fo.\build\ -Fd.\build\resource_converter.pdb -Fe.\build\resource_converter.exe -DRESOURCE_CONVERTER_PROJECT
