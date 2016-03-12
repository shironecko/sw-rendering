@echo off
if not exist .\misc\build.bat cd ..\
call .\misc\build.bat -Fo.\build\ -Fd.\build\game.pdb -Fe.\build\game.exe -DGAME_PROJECT
