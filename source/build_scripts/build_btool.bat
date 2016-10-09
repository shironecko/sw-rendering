@echo off

if not exist .\source cd ..\

if not "%MSVC_CONFIGURED%" == "TRUE" (
    call .\source\configure-msvc.bat
)

cl /nologo /Od /Zi /Fe.\build\ /Fd.\build\ /Fo.\build\ ^
    .\source\win-build-tool.c ^
    /link /incremental:no
