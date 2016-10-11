@echo off

if not exist .\source cd ..\
if not exist .\source (
	echo [ERROR] Run this script from prof root or a forder one level deep!
	exit /b 1
)

if not "%MSVC_CONFIGURED%" == "TRUE" (
    call .\source\build_scripts\configure_msvc.bat
)

set CommonLinkOptions=/incremental:no .\build\SDL2.lib 
set CommonClOptions=/nologo /W4 /wd4204 /wd4100 /wd4152 /wd4201 ^
    /Od /Oi /Zi /MTd /Fe.\build\ /Fd.\build\ /Fo.\build\ /DPT_DEV_BUILD /DGL_RIGOROUS_CHECKS ^
    /I %CD%\source /I %CD%\source\3rdparty /I %CD%\source\3rdparty\SDL2\include
REM /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.10240.0\ucrt"

if not exist .\build\SDL2.dll (
    echo [INFO] Building SDL2...
    xcopy .\source\3rdparty\SDL2 .\build\SDL2 /h /y /s /i /q

    set SDL_MsbuildParams=/nologo /verbosity:quiet ^
        /p:PlatformToolset=%PlatformToolset%;Configuration=Debug;Platform=x64;useenv=true;OutDir=%cd%\build\ ^
		"/p:AdditionalIncludePaths=$(UniversalCRT_IncludePath)"

    msbuild .\build\SDL2\VisualC\SDL\SDL.vcxproj %SDL_MsbuildParams%
    if ERRORLEVEL 1 (
        echo [ERROR] SDL build failed!
        exit /b 1
    )
    msbuild .\build\SDL2\VisualC\SDLmain\SDLmain.vcxproj %SDL_MsbuildParams%
    if ERRORLEVEL 1 (
        echo [ERROR] SDL Main build failed!
        exit /b 1
    )
)

echo [INFO] Building platform layer...
cl %CommonClOptions% ^
  .\source\platform.c ^
  /link %CommonLinkOptions% /subsystem:windows .\build\SDL2main.lib

if ERRORLEVEL 1 (
    echo [WARNING] Platform layer build failed [ignore if hot-reloading]
)

echo [INFO] Building game...
echo wait a minute, you bastard! > .\build\game.lock
cl %CommonClOptions% ^
  .\source\game.c /LDd ^
  /link %CommonLinkOptions% /DLL /export:game_update /pdb:.\build\game_%random%.pdb OpenGL32.lib
del .\build\game.lock

if ERRORLEVEL 1 (
    echo [ERROR] Game build failed!
    exit /b 1
)

REM create symlink of assets folder
if not exist .\build\assets (
    mklink /D .\build\assets ..\assets
    if ERRORLEVEL 1 (
        echo [WARNING] Assets folder symlink creation failed, probably not running this as admin...
    )
)
