@echo off

if not exist .\misc\build.bat cd ..\
if not exist .\build mkdir .\build

if not "%MSVC_CONFIGURED%" == "TRUE" (
    call .\misc\configure-msvc.bat
)

cl /nologo .\source\win32_platform.cpp ^
    -Fo.\build\ -Fd.\build\game.pdb -Fe.\build\game.exe ^
    /W4 /wd4100 /wd4201 /wd4505 /MT /Od /Oi /Zi ^
    /link /subsystem:windows user32.lib gdi32.lib

REM create symlink of assets folder
if not exist .\build\assets (
    mklink /D .\build\assets ..\assets
    if ERRORLEVEL 1 (
        echo [WARNING] Assets folder symlink creation failed, probably not running this as admin...
    )
)
