@echo off
setlocal

if not exist .\build.bat cd misc
if not exist ..\build mkdir ..\build
cd ..\build

if defined VS140COMNTOOLS         ( 
  echo Found VS2015, compiling with it...
  set VSTOOLS=VS140COMNTOOLS
) else if defined VS120COMNTOOLS  ( 
  echo Found VS2013, compiling with it...
  set VSTOOLS=VS120COMNTOOLS
) else echo "Visual Studio 2015 or 2013 installation is not found!" && exit /b 1

call "%%%VSTOOLS%%%vsvars32.bat"
call "%%%VSTOOLS%%%..\..\VC\bin\cl" ^
  /nologo /Febuild.exe ..\source\win32_platform.cpp ^
  %* /W4 /wd4201 /MT /Od /Oi /Zi ^
  /link /subsystem:windows user32.lib gdi32.lib

endlocal
