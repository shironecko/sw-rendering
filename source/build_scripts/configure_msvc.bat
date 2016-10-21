@echo off

if defined VS140COMNTOOLS         ( 
    echo [INFO] Configuring MSVC 2015...
    set VSTOOLS=VS140COMNTOOLS
    set PlatformToolset=v140
) else if defined VS120COMNTOOLS  ( 
    echo [INFO] Configuring MSVC 2013...
    set VSTOOLS=VS120COMNTOOLS
    set PlatformToolset=v120
) else (
    echo [ERROR] Visual Studio 2015 or 2013 installation is not found!
    exit /b 1
)

call "%%%VSTOOLS%%%..\..\VC\vcvarsall.bat" amd64

set MSVC_CONFIGURED=TRUE

set > .\env.txt
