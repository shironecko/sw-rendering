@echo off

if not exist .\source cd ..\

if not "%MSVC_CONFIGURED%" == "TRUE" (
    call .\source\build_scripts\configure_msvc.bat
)

cl /nologo /Od /Zi /Fe.\build\ /Fd.\build\ /Fo.\build\ ^
	/I .\source /I .\source\3rdparty /I .\source\3rdparty\SDL2\include ^
    .\source\win-build-tool.c ^
    /link /incremental:no
