@echo off

if not exist .\source cd ..\
if not exist .\source (
	echo [ERROR] Run this script from prof root or a forder one level deep!
	exit /b 1
)

if not "%MSVC_CONFIGURED%" == "TRUE" (
    call .\source\build_scripts\configure_msvc.bat
)

set CommonLinkOptions=/incremental:no
set CommonClOptions=/nologo /W4 /wd4204 /wd4100 /wd4152 /wd4201 ^
    /Od /Oi /Zi /MTd /Fe.\build\ /Fd.\build\ /Fo.\build\ /DPT_DEV_BUILD /DGL_RIGOROUS_CHECKS ^
    /I %CD%\source /I %CD%\source\3rdparty /I %CD%\source\3rdparty\SDL2\include

echo [INFO] Build math3d tests...
cl %CommonClOptions% ^
	.\source\tests\math3d_tests.c ^
	/link %CommonLinkOptions% /subsystem:console

echo [INFO] Build sw_render tests...
cl %CommonClOptions% ^
	.\source\tests\sw_render_tests.c ^
	/link %CommonLinkOptions% /subsystem:console

echo [INFO] Build utility tests...
cl %CommonClOptions% ^
	.\source\tests\utility_tests.c ^
	/link %CommonLinkOptions% /subsystem:console
