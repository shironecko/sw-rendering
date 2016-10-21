@echo off
setlocal

if not exist .\source cd ..\

if not defined ANDROID_HOME echo "*** You need to define ANDROID_HOME env var!" && exit /b 1

REM create symlink of assets folder
if not exist .\build\android-project\assets (
    mkdir .\build\android-project\assets
    REM a little verbose and ugly, but keeps resource paths unifor across platforms
    mklink /D .\build\android-project\assets\assets ..\..\assets
    if ERRORLEVEL 1 (
        echo [WARNING] Assets folder symlink creation failed, probably not running this as admin...
    )
)

set XCopyOptions=/h /y /s /i /q
xcopy .\source\android-project .\build\android-project %XCopyOptions%

set Excludes=.\build\android-project\xcopy-excludes.txt
echo \android-project\ > %Excludes%
echo build.bat >> %Excludes%
echo build-android.bat >> %Excludes%
xcopy .\source\* .\build\android-project\jni\ %XCopyOptions% /exclude:%Excludes%

call ndk-build -C .\build\android-project NDK_DEBUG=1

if ERRORLEVEL 1 (
    echo [ERROR] NDK build failed!
    exit /b 1
)

echo sdk.dir=%ANDROID_HOME%> .\build\android-project\local.properties
cd .\build\android-project
call ant debug
if ERRORLEVEL 1 (
    echo [ERROR] APK build failed!
    exit /b 1
)

call ant debug install
